/*Copyright 2008-2022 - Loïc Le Cunff

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#include <multilayers.h>
#include <phys_tools.h>
#include <sources.h>

extern const Imdouble Im;
extern std::ofstream plog;

Guided_planar::Guided_planar(int type,int x1_,int x2_,int y1_,int y2_,int z1_,int z2_,
                             int polar_,double lambda_target_,double nr_target_,double ni_target_)
    :Source(x1_,x2_,y1_,y2_,z1_,z2_),
     polar(polar_), lambda_target(lambda_target_), nr_target(nr_target_), ni_target(ni_target_)
{
    for(unsigned int i=0;i<Nthreads;i++)
    {
        threads[i]=new std::thread(&Guided_planar::threaded_computation,this,i);
        
        while(!threads_ready[i])
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

Guided_planar::~Guided_planar()
{
    process_threads=false;
    alternator.signal_threads();
    
    for(unsigned int i=0;i<Nthreads;i++)
    {
        threads[i]->join();
        delete threads[i];
    }
}

void Guided_planar::deep_inject_E(FDTD &fdtd)
{
    if(step>=t_max) return;
    
    std::unique_lock<std::mutex> lock(alternator.get_main_mutex());
    
    int l;
    
    for(l=0;l<Nl;l++)
    {
        precomp[l]=Sp[l]*std::exp(w[l]*(n_eff[l]/c_light*(-Dx/2.0)-Dt*(step+tshift-0.5))*Im);
    }
    
    process_threads=true;
    
    alternator.signal_threads();
    alternator.main_wait_threads(lock);
    
    double tmp1,tmp2,C2x;
    
    if(polar==TE)
    {
        for(int k=z1;k<z2;k++)
        {
            fdtd.mats[fdtd.matsgrid(x1,y1,k)].coeffsY(tmp1,C2x,tmp2);
            fdtd.Ey(x1,y1,k)+=C2x*precomp_H[k-z1].real();
        }
    }
    else
    {
        for(int k=z1;k<z2;k++)
        {
            fdtd.mats[fdtd.matsgrid(x1,y1,k)].coeffsZ(tmp1,C2x,tmp2);
            fdtd.Ez(x1,y1,k)-=C2x*precomp_H[k-z1].real();
        }
    }
}

void Guided_planar::deep_inject_H(FDTD &fdtd)
{
    if(step>=t_max) return;
    
    std::unique_lock<std::mutex> lock(alternator.get_main_mutex());
    
    int l;
    
    for(l=0;l<Nl;l++)
    {
        precomp[l]=Sp[l]*std::exp(w[l]*(-Dt*(step+tshift))*Im);
    }
    
    alternator.signal_threads();
    alternator.main_wait_threads(lock);
    
    if(polar==TE)
    {
        for(int k=z1;k<z2;k++)
            fdtd.Hz(x1-1,y1,k)+=fdtd.dtdmx*precomp_E[k-z1].real();
    }
    else
    {
        for(int k=z1;k<z2;k++)
            fdtd.Hy(x1-1,y1,k)-=fdtd.dtdmx*precomp_E[k-z1].real();
    }
}

void Guided_planar::deep_link(FDTD const &fdtd)
{
    mats.resize(fdtd.mats.L1());
    
    for(unsigned int i=0;i<mats.size();i++)
        mats[i]=&fdtd.mats[i];
    
    matsgrid.resize(z2-z1);
    
    for(unsigned int i=0;i<matsgrid.size();i++)
        matsgrid[i]=fdtd.matsgrid(x1,y1,i+z1);
}

#include <fdfd.h>

void Guided_planar::initialize()
{
    int i;
    
    // Identifying layers
    
    std::vector<unsigned int> gc,gs;
    
    gs.push_back(0);
    gc.push_back(matsgrid[0]);
    
    for(unsigned int i=1;i<matsgrid.size();i++)
    {
        if(matsgrid[i]!=matsgrid[i-1])
        {
            gs.push_back(i);
            gc.push_back(matsgrid[i]);
        }
    }
    
    gs.push_back(matsgrid.size());
        
    expand_spectrum_S(0.1,3000);
    
    if(gc.size()>=2)
    {
        // FDMS Setup
        
        Grid3<unsigned int> tmp_matsgrid(1,1,z2-z1);
        
        for(int k=0;k<z2-z1;k++) tmp_matsgrid(0,0,k)=matsgrid[k];
        
        FDMS fdms(Dx,Dy,Dz);
        
        fdms.set_pml_zm(25,25,-1.0,0.2);
        fdms.set_pml_zp(0,25,1,0.2);
        
        fdms.set_matsgrid(tmp_matsgrid);
        
        for(unsigned int m=0;m<mats.size();m++)
        {
            fdms.set_material(m,mats[m]->base_mat);
        }
        
        //
        
        Imdouble n_eff_target=nr_target+ni_target*Im;
        Eigen::VectorXcd E(z2-z1),H(z2-z1);
        
        n_eff.resize(Nl);
        
        int l0=0;
        
        double ldist_min=std::abs(lambda[0]-lambda_target);
        
        for(int l=0;l<Nl;l++)
        {
            double u=std::abs(lambda[l]-lambda_target);
            
            if(ldist_min>u)
            {
                ldist_min=u;
                l0=l;
            }
        }
        
        E_mode.init(Nl,z2-z1,0);
        H_mode.init(Nl,z2-z1,0);
        
        precomp.resize(Nl);
        precomp_E.resize(z2-z1);
        precomp_H.resize(z2-z1);
        
        int fdms_z0=fdms.pml_zm+fdms.pad_zm;
        
        if(polar==TE) fdms.solve_modes_1D(lambda[l0],0,n_eff_target,n_eff[l0],E,H);
        else          fdms.solve_modes_1D(lambda[l0],1,n_eff_target,n_eff[l0],E,H);
        
        for(i=0;i<z2-z1;i++)
        {
            H_mode(l0,i)=H(i+fdms_z0);
            E_mode(l0,i)=E(i+fdms_z0);
        }
        
        ProgTimeDisp dspt(Nl);
        ++dspt;
        
        for(int l=l0-1;l>=0;l--)
        {
            if(polar==TE) fdms.solve_modes_1D(lambda[l],0,n_eff[l+1],n_eff[l],E,H);
            else          fdms.solve_modes_1D(lambda[l],1,n_eff[l+1],n_eff[l],E,H); 
            
            for(i=0;i<z2-z1;i++)
            {
                H_mode(l,i)=H(i+fdms_z0);
                E_mode(l,i)=E(i+fdms_z0);
            }
            
            ++dspt;
        }
        
        for(int l=l0+1;l<Nl;l++)
        {
            if(polar==TE) fdms.solve_modes_1D(lambda[l],0,n_eff[l-1],n_eff[l],E,H);
            else          fdms.solve_modes_1D(lambda[l],1,n_eff[l-1],n_eff[l],E,H); 
            
            for(i=0;i<z2-z1;i++)
            {
                H_mode(l,i)=H(i+fdms_z0);
                E_mode(l,i)=E(i+fdms_z0);
            }
            
            ++dspt;
        }
        
        Grid2<double> img(Nl,z2-z1);
        
        for(int l=0;l<Nl;l++)
        {
            plog<<lambda[l]<<" "<<n_eff[l].real()<<" "<<n_eff[l].imag()<<std::endl;
            
//            for(i=0;i<z2-z1;i++) plog<<std::abs(E_mode(l,i))<<" ";
            for(i=0;i<z2-z1;i++) img(l,i)=std::abs(H_mode(l,i).real());
        }
        
        G2_to_degra(img,"guided.png");
        
        // Pre-reconstruction
                
        tshift=0;
        t_max=0;
        
        if(polar==TE)
        {
            Imdouble tmp_Ey=0;
            
            for(int l=0;l<Nl;l++) tmp_Ey+=Sp[l]*E_mode(l,gs[1]);
            
            double Ey0=std::abs(tmp_Ey);
            
            while(std::abs(tmp_Ey)/Ey0>1e-4)
            {
                tshift-=100;
                tmp_Ey=0;
                
                for(int l=0;l<Nl;l++)
                    tmp_Ey+=Sp[l]*E_mode(l,gs[1])*std::exp(-w[l]*Dt*tshift*Im);
            }
            
            scaling=Ey0;
            
            tmp_Ey=Ey0;
            
            while(std::abs(tmp_Ey)/Ey0>1e-4)
            {
                t_max+=100;
                tmp_Ey=0;
                
                for(int l=0;l<Nl;l++)
                    tmp_Ey+=Sp[l]*E_mode(l,gs[1])*std::exp(-w[l]*Dt*t_max*Im);
            }
        }
        else
        {
            double Hy0=0;
            Imdouble tmp_Hy;
            int z0=0;
            
            for(int k=0;k<z2-z1;k++)
            {
                tmp_Hy=0;
                for(int l=0;l<Nl;l++) tmp_Hy+=Sp[l]*H_mode(l,k);
                
                if(std::abs(tmp_Hy)>Hy0) { Hy0=std::abs(tmp_Hy); z0=k; }
            }
            
            tmp_Hy=0;
            for(int l=0;l<Nl;l++) tmp_Hy+=Sp[l]*H_mode(l,z0);
                
            while(std::abs(tmp_Hy)/Hy0>1e-4)
            {
                tshift-=100;
                tmp_Hy=0;
                
                for(int l=0;l<Nl;l++)
                    tmp_Hy+=Sp[l]*H_mode(l,z0)*std::exp(-w[l]*Dt*tshift*Im);
            }
            
            scaling=Hy0*std::sqrt(mu0/e0);
            
            tmp_Hy=Hy0;
                
            while(std::abs(tmp_Hy)/Hy0>1e-4)
            {
                t_max+=100;
                tmp_Hy=0;
                
                for(int l=0;l<Nl;l++)
                    tmp_Hy+=Sp[l]*H_mode(l,z0)*std::exp(-w[l]*Dt*t_max*Im);
            }
        }
        
        t_max+=std::abs(tshift);
        
        for(int l=0;l<Nl;l++) Sp[l]/=scaling;
    }
    else Nl=0;
}

void Guided_planar::threaded_computation(unsigned int ID)
{
    std::unique_lock<std::mutex> lock(alternator.get_thread_mutex(ID));
    
    threads_ready[ID]=true;
    
    alternator.thread_wait_ok(ID,lock);
    
    int z_span=z2-z1;
    
    while(process_threads)
    {
        for(unsigned int k=z_span*ID/Nthreads;k<z_span*(ID+1)/Nthreads;k++)
        {
            precomp_H[k]=0;
            
            for(int l=0;l<Nl;l++)
                precomp_H[k]+=H_mode(l,k)*precomp[l];
        }
        
        alternator.signal_main(ID);
        alternator.thread_wait_ok(ID,lock);
        
        for(unsigned int k=z_span*ID/Nthreads;k<z_span*(ID+1)/Nthreads;k++)
        {
            precomp_E[k]=0;
            
            for(int l=0;l<Nl;l++)
                precomp_E[k]+=E_mode(l,k)*precomp[l];
        }
        
        alternator.signal_main(ID);
        alternator.thread_wait_ok(ID,lock);
    }
}

//void Guided_planar::initialize()
//{
//    unsigned int i,j;
//    
//    // Identifying layers
//    
//    std::vector<unsigned int> gc,gs;
//    
//    gs.push_back(0);
//    gc.push_back(matsgrid[0]);
//    
//    for(i=1;i<matsgrid.size();i++)
//    {
//        if(matsgrid[i]!=matsgrid[i-1])
//        {
//            gs.push_back(i);
//            gc.push_back(matsgrid[i]);
//        }
//    }
//    
//    gs.push_back(matsgrid.size());
//        
//    expand_spectrum_S(0.1,3000);
//    
//    if(gc.size()>=2)
//    {
//        Multilayer_TMM ml(gc.size()-2);
//        
//        ml.set_environment(mats[gc[0]]->base_mat,mats[gc[gc.size()-1]]->base_mat);
//        
//        for(i=1;i<gc.size()-1;i++)
//            ml.set_layer(i-1,Dz*(gs[i+1]-gs[i]),mats[gc[i]]->base_mat);
//        
//        Imdouble n_eff_target=nr_target+ni_target*Im;
//        
//        n_eff.resize(Nl);
//        
//        if(polar==TE) ml.compute_TE_dispersion(lambda_target,n_eff_target,
//                                               lambda,n_eff,0.1,0.1,1e-20,100);
//        else ml.compute_TM_dispersion(lambda_target,n_eff_target,
//                                      lambda,n_eff,0.1,0.1,1e-20,100);
//        
//        // Field holders initialization
//        
//        MLFieldHolder holder_1,holder_2;
//        
//        holder_1.set_N_layers(gc.size());
//        holder_2.set_N_layers(gc.size());
//        
//        unsigned int z0=gs[1];
//        
//        for(i=0;i<gc.size();i++)
//        {
//            holder_1.set_Nz(i,gs[i+1]-gs[i]);
//            holder_2.set_Nz(i,gs[i+1]-gs[i]);
//            
//            for(j=0;j<gs[i+1]-gs[i];j++)
//            {
//                holder_1.z(i,j)=Dz*(0.0+j+gs[i]-z0); // force uint to double to avoid overflow
//                holder_2.z(i,j)=Dz*(0.0+j+gs[i]-z0)+Dz/2.0;
//                holder_2.z(i,j)=Dz*(0.0+j+gs[i]-z0)-Dz/2.0;
//            }
//        }
//        
//        Ex.init(Nl,z2-z1,0); Ey.init(Nl,z2-z1,0); Ez.init(Nl,z2-z1,0);
//        Hx.init(Nl,z2-z1,0); Hy.init(Nl,z2-z1,0); Hz.init(Nl,z2-z1,0);
//        precomp.resize(Nl);
//        
//        // Modes computations
//        
//        for(int l=0;l<Nl;l++)
//        {
//            ml.set_lambda_full(lambda[l]);
//            
//            if(polar==TE)
//            {
//                ml.compute_mode_TE(n_eff[l],holder_1,false);
//                ml.compute_mode_TE(n_eff[l],holder_2,false);
//            }
//            else
//            {
//                ml.compute_mode_TM(n_eff[l],holder_1,false);
//                ml.compute_mode_TM(n_eff[l],holder_2,false);
//            }
//            
//            holder_1.normalize();
//            holder_2.normalize();
//            
//            for(i=0;i<gc.size();i++)
//            {
//                std::vector<Imdouble> &h_Ex=holder_1.get_Ex(i);
//                std::vector<Imdouble> &h_Ey=holder_1.get_Ey(i);
//                std::vector<Imdouble> &h_Ez=holder_2.get_Ez(i);
//                
//                std::vector<Imdouble> &h_Hx=holder_2.get_Hx(i);
//                std::vector<Imdouble> &h_Hy=holder_2.get_Hy(i);
//                std::vector<Imdouble> &h_Hz=holder_1.get_Hz(i);
//                
//                for(j=0;j<gs[i+1]-gs[i];j++)
//                {
//                    Ex(l,j+gs[i])=h_Ex[j]; E_mode(l,j+gs[i])=h_Ey[j]; E_mode(l,j+gs[i])=h_Ez[j];
//                    Hx(l,j+gs[i])=h_Hx[j]; H_mode(l,j+gs[i])=h_Hy[j]; H_mode(l,j+gs[i])=h_Hz[j];
//                }
//            }
//        }
//                
//        tshift=0;
//        
//        if(polar==TE)
//        {
//            Imdouble tmp_Ey=0;
//            
//            for(int l=0;l<Nl;l++) tmp_Ey+=Sp[l]*E_mode(l,gs[1]);
//            
//            double Ey0=std::abs(tmp_Ey);
//            
//            while(std::abs(tmp_Ey)/Ey0>1e-4)
//            {
//                tshift-=100;
//                tmp_Ey=0;
//                
//                for(int l=0;l<Nl;l++)
//                    tmp_Ey+=Sp[l]*E_mode(l,gs[1])*std::exp(-w[l]*Dt*tshift*Im);
//            }
//            
//            scaling=Ey0;
//        }
//        else
//        {
//            Imdouble tmp_Hy=0;
//            
//            for(int l=0;l<Nl;l++) tmp_Hy+=Sp[l]*H_mode(l,gs[1]);
//            
//            double Hy0=std::abs(tmp_Hy);
//            
//            while(std::abs(tmp_Hy)/Hy0>1e-4)
//            {
//                tshift-=100;
//                tmp_Hy=0;
//                
//                for(int l=0;l<Nl;l++)
//                    tmp_Hy+=Sp[l]*H_mode(l,gs[1])*std::exp(-w[l]*Dt*tshift*Im);
//            }
//            
//            scaling=Hy0*std::sqrt(mu0/e0);
//        }
//        
//        for(int l=0;l<Nl;l++) Sp[l]/=scaling;
//    }
//    else Nl=0;
//}
