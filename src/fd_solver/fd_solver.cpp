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

#include <fdfd.h>
#include <lua_interface.h>


extern const Imdouble Im;
extern std::ofstream plog;

void FDMS_Mode::process()
{
    std::cout<<"toto"<<std::endl;
    
    int Nx,Ny,Nz;
    double lx,ly,lz;
    
    Grid3<unsigned int> tmp_matsgrid;
    
    structure->retrieve_nominal_size(lx,ly,lz);
    compute_discretization(Nx,Ny,Nz,lx,ly,lz);
    
    structure->discretize(tmp_matsgrid,Nx,Ny,Nz,Dx,Dy,Dz);
    
    if(Nx==1)
    {
        if(Ny==1)
        {
            Grid3<unsigned int> matsgrid(1,1,Nz);
            
            for(int k=0;k<Nz;k++) matsgrid(0,0,k)=tmp_matsgrid(0,0,k);
            
            FDMS fdms(Dx,Dy,Dz);
            
//            fdms.set_pml_zm(pml_zm,kappa_zm,sigma_zm,alpha_zm);
//            fdms.set_pml_zp(pml_zp,kappa_zp,sigma_zp,alpha_zp);
            
            fdms.set_matsgrid(matsgrid);
            
            #ifdef OLDMAT
            for(unsigned int m=0;m<materials_str.size();m++)
            {
                fdms.set_material(materials_index[m],materials_str[m]);
            }
            #endif
            
            Nx=fdms.Nx;
            Ny=fdms.Ny;
            Nz=fdms.Nz;
            
            chk_var(Nx);
            chk_var(Ny);
            chk_var(Nz);
            chk_var(fdms.Nxy);
            chk_var(fdms.Nxyz);
            
            Imdouble n=1.0;
            Eigen::VectorXcd E,H;
            
        //    for(int i=0;i<10;i++) for(int j=0;j<10;j++)
        //    {
        //        Imdouble guess=1.0+3.0*i/9.0+0*j/9.0*Im;
        //        
        //        fdms.solve_modes_1D(510e-9,1,guess,n,E,H);
        //        
        //        std::cout<<n<<std::endl;
        //    }
            
        //    for(int i=0;i<100;i++)
        //    {
        //        Imdouble guess=1.0+3.0*i/99.0+.1*Im;
        //        
        //        fdms.solve_modes_1D(510e-9,1,guess,n,E,H);
        //        
        //        std::cout<<n<<std::endl;
        //    }
            
            fdms.solve_modes_1D(510e-9,1,1.4+0.5*Im,n,E,H);
            
            chk_var(n);
            for(int i=0;i<E.rows();i++)
            {
                plog<<i<<" "<<E(i).real()<<" "<<E(i).imag()<<std::endl;
            }
            
        //    fdms.solve_modes_1D(500e-9,1,3.5,n,E,H);
        }
        else
        {
            int j,k;
            
            Grid3<unsigned int> matsgrid(1,Ny,Nz);
            
            for(j=0;j<Ny;j++)
                for(k=0;k<Nz;k++)
                    matsgrid(0,j,k)=tmp_matsgrid(0,j,k);
            
            FDMS fdms(Dx,Dy,Dz);
            
//            fdms.set_pml_ym(pml_ym,kappa_ym,sigma_ym,alpha_ym);
//            fdms.set_pml_yp(pml_yp,kappa_yp,sigma_yp,alpha_yp);
//            
//            fdms.set_pml_zm(pml_zm,kappa_zm,sigma_zm,alpha_zm);
//            fdms.set_pml_zp(pml_zp,kappa_zp,sigma_zp,alpha_zp);
            
            fdms.set_matsgrid(matsgrid);
            
            #ifdef OLDMAT
            for(unsigned int m=0;m<materials_str.size();m++)
            {
                fdms.set_material(materials_index[m],materials_str[m]);
            }
            #endif
            
            Nx=fdms.Nx;
            Ny=fdms.Ny;
            Nz=fdms.Nz;
            
            chk_var(Nx);
            chk_var(Ny);
            chk_var(Nz);
            chk_var(fdms.Nxy);
            chk_var(fdms.Nxyz);
            
            Grid2<Imdouble> Ex(Ny,Nz),Ey(Ny,Nz),Ez(Ny,Nz),
                            Hx(Ny,Nz),Hy(Ny,Nz),Hz(Ny,Nz);
            
            Imdouble n=1.0;
            
            //double eta=std::sqrt(mu0/e0);
            
            for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
            {
                double y=(j-Ny/2.0)/(Ny/2.0);
                double z=(k-Nz/2.0)/(Nz/2.0);
                
                double coeff=std::exp(-y*y-z*z);
//                Ex(j,k)=coeff;
                Ey(j,k)=coeff;
                Ez(j,k)=coeff;
                
//                Hx(j,k)=coeff/eta;
//                Hy(j,k)=coeff/eta;
//                Hz(j,k)=coeff/eta;
            }
            
//            std::ofstream file("modes_2D_output",std::ios::out|std::ios::trunc);
//            
//            for(int i=0;i<20;i++) for(j=0;j<10;j++)
//            {
//                double nr=1.1+0.9*i/19.0;
//                double ni=0.2*j/9.0;
//                
//                fdms.solve_modes_2D(510e-9,nr+ni*Im,n,
//                                    Ex,Ey,Ez,Hx,Hy,Hz);
//                
//                file<<nr<<" "<<ni<<" "<<n.real()<<" "<<n.imag()<<std::endl;
//            }
//            
//            file.close();
            
            fdms.solver=solver;
            fdms.solve_modes_2D(lambda_target,nr_target+ni_target*Im,n,Ex,Ey,Ez,Hx,Hy,Hz);
            
            chk_var(n);
            
            std::ofstream file("fd_solver_output",std::ios::out|std::ios::app);
            
            file<<n.real()<<" "<<n.imag()<<std::endl;
            
            file.close();
            
            Grid2<double> Ex_a(Ny,Nz),Ey_a(Ny,Nz),Ez_a(Ny,Nz),
                          Hx_a(Ny,Nz),Hy_a(Ny,Nz),Hz_a(Ny,Nz);
            
            for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
            {
                Ex_a(j,k)=std::abs(Ex(j,k));
                Ey_a(j,k)=std::abs(Ey(j,k));
                Ez_a(j,k)=std::abs(Ez(j,k));
            }
            
            G2_to_degra(Ey_a,"Ey.png");
            G2_to_degra(Ez_a,"Ez.png");
            
            chk_var(Ey_a.max());
            chk_var(Ez_a.max());
            
//            for(int i=0;i<E.rows();i++)
//            {
//                plog<<i<<" "<<E(i).real()<<" "<<E(i).imag()<<std::endl;
//            }
        }
    }
}

FDMS::FDMS(double Dx_,double Dy_,double Dz_)
    :solver("default")
{
    Dx=Dx_;
    Dy=Dy_;
    Dz=Dz_;
}

void FDMS::solve_modes_1D(double lambda,int polar,
                          Imdouble const &n_guess,Imdouble &n_mode,
                          Eigen::VectorXcd &E_out,
                          Eigen::VectorXcd&H_out)
{
    int k;
    
    double k0=2.0*Pi/lambda;
    double w=m_to_rad_Hz(lambda);
    Imdouble udz=1.0/Dz;
    
    typedef Eigen::Triplet<Imdouble> T;
    
    if(polar==0)
    {
        Eigen::SparseMatrix<Imdouble> TE_mat(Nz,Nz);
        
        Eigen::SparseMatrix<Imdouble> Bxz(Nz,Nz);
        Eigen::SparseMatrix<Imdouble> Ayz(Nz,Nz);
        
        Eigen::SparseMatrix<Imdouble> Py(Nz,Nz);
        Eigen::SparseMatrix<Imdouble> Qz(Nz,Nz);
        
        Eigen::SparseMatrix<Imdouble> inv_Qx(Nz,Nz);
        
        std::vector<T> Trp_a,Trp_b,Trp_p,Trp_q,Trp_iq;
        
        Trp_a.reserve(2*Nz-1);
        Trp_b.reserve(2*Nz-1);
        Trp_p.reserve(Nz);
        Trp_q.reserve(Nz);
        Trp_iq.reserve(Nz);
        
        for(k=0;k<Nz;k++)
        {
            udz=1.0/get_Dz(k,w);
            
            if(k>0) Trp_b.push_back(T(k,k-1,-udz));
                    Trp_b.push_back(T(k,k,udz));
            
            udz=1.0/get_Dz(k+0.5,w);
            
                       Trp_a.push_back(T(k,k,-udz));
            if(k<Nz-1) Trp_a.push_back(T(k,k+1,udz));
            
            Trp_p.push_back(T(k,k,-w*e0*mats[matsgrid(0,0,k)].get_eps(w)*Im));
            Trp_iq.push_back(T(k,k,1.0/(w*mu0*Im)));
            Trp_q.push_back(T(k,k,w*mu0*Im));
        }
        
        Ayz.setFromTriplets(Trp_a.begin(),Trp_a.end());
        Bxz.setFromTriplets(Trp_b.begin(),Trp_b.end());
        Py.setFromTriplets(Trp_p.begin(),Trp_p.end());
        Qz.setFromTriplets(Trp_q.begin(),Trp_q.end());
        inv_Qx.setFromTriplets(Trp_iq.begin(),Trp_iq.end());
        
        TE_mat=Qz*(Bxz*inv_Qx*Ayz+Py);
        
        Eigen::VectorXcd b0(Nz),b1(Nz);
        
        for(k=0;k<Nz;k++)
        {
            double z=(k-Nz/2)/static_cast<double>(Nz/2);
            
            b0(k)=std::exp(-5.0*z*z);
        }
        
        b1=b0;
        
        Imdouble beta_2=inverse_power_iteration(TE_mat,(n_guess*k0)*(n_guess*k0),b1);
        Imdouble beta=std::sqrt(beta_2);
        n_mode=beta/k0;
        
//        chk_var(n_eff);
//         
//        for(k=0;k<Nz;k++)
//        {
//            plog<<k<<" "<<std::abs(b0(k))<<" "<<std::abs(b1(k))<<std::endl;
//        }
    }
    else
    {
        Eigen::SparseMatrix<Imdouble> TM_mat(Nz,Nz);
        
        Eigen::SparseMatrix<Imdouble> Byz(Nz,Nz);
        Eigen::SparseMatrix<Imdouble> Axz(Nz,Nz);
        
        Eigen::SparseMatrix<Imdouble> Pz(Nz,Nz);
        Eigen::SparseMatrix<Imdouble> Qy(Nz,Nz);
        
        Eigen::SparseMatrix<Imdouble> inv_Px(Nz,Nz);
        
        std::vector<T> Trp_a,Trp_b,Trp_p,Trp_q,Trp_ip;
        
        for(k=0;k<Nz;k++)
        {
            udz=1.0/get_Dz(k,w);
            
            if(k>0) Trp_b.push_back(T(k,k-1,-udz));
                    Trp_b.push_back(T(k,k,udz));
            
            udz=1.0/get_Dz(k+0.5,w);
            
                       Trp_a.push_back(T(k,k,-udz));
            if(k<Nz-1) Trp_a.push_back(T(k,k+1,udz));
            
            Trp_ip.push_back(T(k,k,-1.0/(w*e0*mats[matsgrid(0,0,k)].get_eps(w)*Im)));
            Trp_p.push_back(T(k,k,-w*e0*mats[matsgrid(0,0,k)].get_eps(w)*Im));
            Trp_q.push_back(T(k,k,w*mu0*Im));
        }
        
        Axz.setFromTriplets(Trp_a.begin(),Trp_a.end());
        Byz.setFromTriplets(Trp_b.begin(),Trp_b.end());
        Pz.setFromTriplets(Trp_p.begin(),Trp_p.end());
        Qy.setFromTriplets(Trp_q.begin(),Trp_q.end());
        inv_Px.setFromTriplets(Trp_ip.begin(),Trp_ip.end());
        
//        TM_mat=Pz*(Axz*inv_Px*Byz+Qy);

        TM_mat=(Axz*inv_Px*Byz+Qy)*Pz;
        
        Eigen::VectorXcd b0(Nz),b1(Nz),b2(Nz);
        
        for(k=0;k<Nz;k++)
        {
            double z=(k-Nz/2)/static_cast<double>(Nz/2);
            
            b0(k)=std::exp(-150.0*z*z);
        }
        
        b1=b0;
        
        Imdouble beta_2=inverse_power_iteration(TM_mat,(n_guess*k0)*(n_guess*k0),b1,1e-10,10000);
        Imdouble beta=std::sqrt(beta_2);
        n_mode=beta/k0;
        
        double bmax=std::abs(b1(0));
        Imdouble bmax_i;
        for(k=0;k<Nz;k++)
        {
            if(std::abs(b1(k))>=bmax)
            {
                bmax=std::abs(b1(k));
                bmax_i=b1(k);
            }
        }
        
        b1/=bmax_i;
        
        E_out=b1;
        H_out=Pz*b1/(beta*Im);
        
//        b2=TM_mat*b1;
//        
//        chk_var(n_mode);
//         
//        for(k=0;k<Nz;k++)
//        {
//            plog<<k<<" "<<std::abs(b0(k))<<" "
//                <<std::real(b1(k))<<" "<<std::real(b2(k)/beta_2)<<" "
//                <<std::imag(b1(k))<<" "<<std::imag(b2(k)/beta_2)<<std::endl;
//        }
        
//        chk_var(beta_2);
        
//        for(k=0;k<Nz;k++)
//        {
//            plog<<k<<" "<<std::abs(b0(k))<<" "
//                <<std::real(b2(k)/b1(k))<<" "<<std::imag(b2(k)/b1(k))<<std::endl;
//        }
    }
}

void sparse_matrix_inject(Eigen::SparseMatrix<Imdouble> const &source,
                          Eigen::SparseMatrix<Imdouble> &target,
                          int row_shift,int col_shift)
{
    typedef Eigen::Triplet<Imdouble> Trp;
    
    std::vector<Trp> trp;
    
    trp.reserve(source.nonZeros()+target.nonZeros());
    
    for(int i=0;i<target.outerSize(); ++i)
    {
        for(Eigen::SparseMatrix<Imdouble>::InnerIterator it(target,i);it;++it)
        {
            trp.push_back(Trp(it.row(),it.col(),it.value()));
        }
    }
    
    for(int i=0;i<source.outerSize(); ++i)
    {
        for(Eigen::SparseMatrix<Imdouble>::InnerIterator it(source,i);it;++it)
        {
            trp.push_back(Trp(it.row()+row_shift,it.col()+col_shift,it.value()));
        }
    }
    
    target.setFromTriplets(trp.begin(),trp.end());
}

/*void FDMS::solve_modes_2D(double lambda,
                          Imdouble const &n_guess,Imdouble &n_mode,
                          Grid2<Imdouble> &Ex_,Grid2<Imdouble> &Ey_,Grid2<Imdouble> &Ez_,
                          Grid2<Imdouble> &Hx_,Grid2<Imdouble> &Hy_,Grid2<Imdouble> &Hz_)
{
    int i,j,k;
    
    double k0=2.0*Pi/lambda;
    double w=m_to_rad_Hz(lambda);
    
    typedef Eigen::Triplet<Imdouble> T;
    
    int Nyz=Ny*Nz;
    
    Eigen::SparseMatrix<Imdouble> inv_Px(Nyz,Nyz),Py(Nyz,Nyz),Pz(Nyz,Nyz),
                                  inv_Qx(Nyz,Nyz),Qy(Nyz,Nyz),Qz(Nyz,Nyz),
                                  Axy(Nyz,Nyz),Axz(Nyz,Nyz),Ayz(Nyz,Nyz),Azy(Nyz,Nyz),
                                  Bxy(Nyz,Nyz),Bxz(Nyz,Nyz),Byz(Nyz,Nyz),Bzy(Nyz,Nyz);
    
    std::vector<T> trp_ipx,trp_py,trp_pz,
                   trp_iqx,trp_qy,trp_qz,
                   trp_axy,trp_axz,trp_ayz,trp_azy,
                   trp_bxy,trp_bxz,trp_byz,trp_bzy;
    
    trp_ipx.reserve(Nyz); trp_py.reserve(Nyz); trp_pz.reserve(Nyz);
    trp_iqx.reserve(Nyz); trp_qy.reserve(Nyz); trp_qz.reserve(Nyz);
    trp_axy.reserve(2*Nyz-1); trp_axz.reserve(2*Nyz-1); trp_ayz.reserve(2*Nyz-1); trp_azy.reserve(2*Nyz-1);
    trp_bxy.reserve(2*Nyz-1); trp_bxz.reserve(2*Nyz-1); trp_byz.reserve(2*Nyz-1); trp_bzy.reserve(2*Nyz-1);
    
    Imdouble udy,udz;
    
    for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
    {
        i=index(0,j,k);
        
        // E Field
        
        udy=1.0/get_Dy(j,w);
        udz=1.0/get_Dz(k,w);
        
        // - Ex
        
        trp_ipx.push_back(T(i,i,-1.0/(w*e0*mats(matsgrid(0,j,k)).get_eps(w)*Im)));
        
        if(j>0) trp_bzy.push_back(T(i,index(0,j-1,k),-udy));
                trp_bzy.push_back(T(i,i,udy));
        
        if(k>0) trp_byz.push_back(T(i,index(0,j,k-1),-udz));
                trp_byz.push_back(T(i,i,udz));
        
        // - Ey
        
        trp_py.push_back(T(i,i,-w*e0*mats(matsgrid(0,j,k)).get_eps(w)*Im));
        
        if(k>0) trp_bxz.push_back(T(i,index(0,j,k-1),-udz));
                trp_bxz.push_back(T(i,i,udz));
        
        // - Ez
        
        trp_pz.push_back(T(i,i,-w*e0*mats(matsgrid(0,j,k)).get_eps(w)*Im));
        
        if(j>0) trp_bxy.push_back(T(i,index(0,j-1,k),-udy));
                trp_bxy.push_back(T(i,i,udy));
        
        // H Field
        
        udy=1.0/get_Dy(j+0.5,w);
        udz=1.0/get_Dz(k+0.5,w);
        
        // - Hx
        
        trp_iqx.push_back(T(i,i,1.0/(w*mu0*Im)));
        
                   trp_azy.push_back(T(i,index(0,j,k),-udy));
        if(j<Ny-1) trp_azy.push_back(T(i,index(0,j+1,k),udy));
        
                   trp_ayz.push_back(T(i,index(0,j,k),-udz));
        if(k<Nz-1) trp_ayz.push_back(T(i,index(0,j,k+1),udz));
                   
        // - Hy
        
        trp_qy.push_back(T(i,i,w*mu0*Im));
        
                   trp_axz.push_back(T(i,index(0,j,k),-udz));
        if(k<Nz-1) trp_axz.push_back(T(i,index(0,j,k+1),udz));
        
        // - Hz
        
        trp_qz.push_back(T(i,i,w*mu0*Im));
        
                   trp_axy.push_back(T(i,index(0,j,k),-udy));
        if(j<Ny-1) trp_axy.push_back(T(i,index(0,j+1,k),udy));
    }
    
    inv_Px.setFromTriplets(trp_ipx.begin(),trp_ipx.end()); trp_ipx.clear();
    Py.setFromTriplets(trp_py.begin(),trp_py.end()); trp_py.clear();
    Pz.setFromTriplets(trp_pz.begin(),trp_pz.end()); trp_pz.clear();
    
    inv_Qx.setFromTriplets(trp_iqx.begin(),trp_iqx.end()); trp_iqx.clear();
    Qy.setFromTriplets(trp_qy.begin(),trp_qy.end()); trp_qy.clear();
    Qz.setFromTriplets(trp_qz.begin(),trp_qz.end()); trp_qz.clear();
    
    Axy.setFromTriplets(trp_axy.begin(),trp_axy.end()); trp_axy.clear();
    Axz.setFromTriplets(trp_axz.begin(),trp_axz.end()); trp_axz.clear();
    Ayz.setFromTriplets(trp_ayz.begin(),trp_ayz.end()); trp_ayz.clear();
    Azy.setFromTriplets(trp_azy.begin(),trp_azy.end()); trp_azy.clear();
    
    Bxy.setFromTriplets(trp_bxy.begin(),trp_bxy.end()); trp_bxy.clear();
    Bxz.setFromTriplets(trp_bxz.begin(),trp_bxz.end()); trp_bxz.clear();
    Byz.setFromTriplets(trp_byz.begin(),trp_byz.end()); trp_byz.clear();
    Bzy.setFromTriplets(trp_bzy.begin(),trp_bzy.end()); trp_bzy.clear();
    
    Eigen::SparseMatrix<Imdouble> Cyz=inv_Qx*Ayz,
                                  Czy=inv_Qx*Azy,
                                  Dyz=inv_Px*Byz,
                                  Dzy=inv_Px*Bzy;
    
    Eigen::SparseMatrix<Imdouble> tmp_mat(Nyz,Nyz);
    Eigen::SparseMatrix<Imdouble> em_mat(4*Nyz,4*Nyz);
    
    tmp_mat=-Axy*Dyz;    sparse_matrix_inject(tmp_mat,em_mat,0,2*Nyz);
    tmp_mat=Axy*Dzy+Qz;  sparse_matrix_inject(tmp_mat,em_mat,0,3*Nyz);
    tmp_mat=-Axz*Dyz-Qy; sparse_matrix_inject(tmp_mat,em_mat,Nyz,2*Nyz);
    tmp_mat=Axz*Dzy;     sparse_matrix_inject(tmp_mat,em_mat,Nyz,3*Nyz);
    
    tmp_mat=-Bxy*Cyz;    sparse_matrix_inject(tmp_mat,em_mat,2*Nyz,0);
    tmp_mat=Bxy*Czy+Pz;  sparse_matrix_inject(tmp_mat,em_mat,2*Nyz,Nyz);
    tmp_mat=-Bxz*Cyz-Py; sparse_matrix_inject(tmp_mat,em_mat,3*Nyz,0);
    tmp_mat=Bxz*Czy;     sparse_matrix_inject(tmp_mat,em_mat,3*Nyz,Nyz);
    
    tmp_mat/=Im;
    
    Eigen::VectorXcd field(4*Nyz);
    
    for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
    {
        i=index(0,j,k);
        
        field(i+0*Nyz)=Ey_(j,k);
        field(i+1*Nyz)=Ez_(j,k);
        field(i+2*Nyz)=Hy_(j,k);
        field(i+3*Nyz)=Hz_(j,k);
    }
    
    Imdouble beta=inverse_power_iteration(em_mat,n_guess*k0,field,1e-10,1000);
    n_mode=beta/k0;
    
    int n=0;
    
    for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
    {
        i=index(0,j,k);
        
        Ey_(j,k)=field(i+0*Nyz);
        Ez_(j,k)=field(i+1*Nyz);
        Hy_(j,k)=field(i+2*Nyz);
        Hz_(j,k)=field(i+3*Nyz);
    }
}*/

void FDMS::solve_modes_2D(double lambda,
                          Imdouble const &n_guess,Imdouble &n_mode,
                          Grid2<Imdouble> &Ex_,Grid2<Imdouble> &Ey_,Grid2<Imdouble> &Ez_,
                          Grid2<Imdouble> &Hx_,Grid2<Imdouble> &Hy_,Grid2<Imdouble> &Hz_)
{
    int i,j,k;
    
    double k0=2.0*Pi/lambda;
    double w=m_to_rad_Hz(lambda);
    
    typedef Eigen::Triplet<Imdouble> T;
    
    int Nyz=Ny*Nz;
    
    Eigen::SparseMatrix<Imdouble> inv_Px(Nyz,Nyz),Py(Nyz,Nyz),Pz(Nyz,Nyz),
                                  inv_Qx(Nyz,Nyz),Qy(Nyz,Nyz),Qz(Nyz,Nyz),
                                  Axy(Nyz,Nyz),Axz(Nyz,Nyz),Ayz(Nyz,Nyz),Azy(Nyz,Nyz),
                                  Bxy(Nyz,Nyz),Bxz(Nyz,Nyz),Byz(Nyz,Nyz),Bzy(Nyz,Nyz);
    
    std::vector<T> trp_ipx,trp_py,trp_pz,
                   trp_iqx,trp_qy,trp_qz,
                   trp_axy,trp_axz,trp_ayz,trp_azy,
                   trp_bxy,trp_bxz,trp_byz,trp_bzy;
    
    trp_ipx.reserve(Nyz); trp_py.reserve(Nyz); trp_pz.reserve(Nyz);
    trp_iqx.reserve(Nyz); trp_qy.reserve(Nyz); trp_qz.reserve(Nyz);
    trp_axy.reserve(2*Nyz-1); trp_axz.reserve(2*Nyz-1); trp_ayz.reserve(2*Nyz-1); trp_azy.reserve(2*Nyz-1);
    trp_bxy.reserve(2*Nyz-1); trp_bxz.reserve(2*Nyz-1); trp_byz.reserve(2*Nyz-1); trp_bzy.reserve(2*Nyz-1);
    
    Imdouble udy,udz;
    
    for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
    {
        i=index(0,j,k);
        
        // E Field
        
        udy=1.0/get_Dy(j,w);
        udz=1.0/get_Dz(k,w);
        
        // - Ex
        
        trp_ipx.push_back(T(i,i,-1.0/(w*e0*mats[matsgrid(0,j,k)].get_eps(w)*Im)));
        
        if(j>0) trp_bzy.push_back(T(i,index(0,j-1,k),-udy));
                trp_bzy.push_back(T(i,i,udy));
        
        if(k>0) trp_byz.push_back(T(i,index(0,j,k-1),-udz));
                trp_byz.push_back(T(i,i,udz));
        
        // - Ey
        
        trp_py.push_back(T(i,i,-w*e0*mats[matsgrid(0,j,k)].get_eps(w)*Im));
        
        if(k>0) trp_bxz.push_back(T(i,index(0,j,k-1),-udz));
                trp_bxz.push_back(T(i,i,udz));
        
        // - Ez
        
        trp_pz.push_back(T(i,i,-w*e0*mats[matsgrid(0,j,k)].get_eps(w)*Im));
        
        if(j>0) trp_bxy.push_back(T(i,index(0,j-1,k),-udy));
                trp_bxy.push_back(T(i,i,udy));
        
        // H Field
        
        udy=1.0/get_Dy(j+0.5,w);
        udz=1.0/get_Dz(k+0.5,w);
        
        // - Hx
        
        trp_iqx.push_back(T(i,i,1.0/(w*mu0*Im)));
        
                   trp_azy.push_back(T(i,index(0,j,k),-udy));
        if(j<Ny-1) trp_azy.push_back(T(i,index(0,j+1,k),udy));
        
                   trp_ayz.push_back(T(i,index(0,j,k),-udz));
        if(k<Nz-1) trp_ayz.push_back(T(i,index(0,j,k+1),udz));
                   
        // - Hy
        
        trp_qy.push_back(T(i,i,w*mu0*Im));
        
                   trp_axz.push_back(T(i,index(0,j,k),-udz));
        if(k<Nz-1) trp_axz.push_back(T(i,index(0,j,k+1),udz));
        
        // - Hz
        
        trp_qz.push_back(T(i,i,w*mu0*Im));
        
                   trp_axy.push_back(T(i,index(0,j,k),-udy));
        if(j<Ny-1) trp_axy.push_back(T(i,index(0,j+1,k),udy));
    }
    
    inv_Px.setFromTriplets(trp_ipx.begin(),trp_ipx.end()); trp_ipx.clear();
    Py.setFromTriplets(trp_py.begin(),trp_py.end()); trp_py.clear();
    Pz.setFromTriplets(trp_pz.begin(),trp_pz.end()); trp_pz.clear();
    
    inv_Qx.setFromTriplets(trp_iqx.begin(),trp_iqx.end()); trp_iqx.clear();
    Qy.setFromTriplets(trp_qy.begin(),trp_qy.end()); trp_qy.clear();
    Qz.setFromTriplets(trp_qz.begin(),trp_qz.end()); trp_qz.clear();
    
    Axy.setFromTriplets(trp_axy.begin(),trp_axy.end()); trp_axy.clear();
    Axz.setFromTriplets(trp_axz.begin(),trp_axz.end()); trp_axz.clear();
    Ayz.setFromTriplets(trp_ayz.begin(),trp_ayz.end()); trp_ayz.clear();
    Azy.setFromTriplets(trp_azy.begin(),trp_azy.end()); trp_azy.clear();
    
    Bxy.setFromTriplets(trp_bxy.begin(),trp_bxy.end()); trp_bxy.clear();
    Bxz.setFromTriplets(trp_bxz.begin(),trp_bxz.end()); trp_bxz.clear();
    Byz.setFromTriplets(trp_byz.begin(),trp_byz.end()); trp_byz.clear();
    Bzy.setFromTriplets(trp_bzy.begin(),trp_bzy.end()); trp_bzy.clear();
    
    Eigen::SparseMatrix<Imdouble> Cyz=inv_Qx*Ayz,
                                  Czy=inv_Qx*Azy,
                                  Dyz=inv_Px*Byz,
                                  Dzy=inv_Px*Bzy;
    
    Eigen::SparseMatrix<Imdouble> tmp_mat(Nyz,Nyz);
    Eigen::SparseMatrix<Imdouble> em_mat(2*Nyz,2*Nyz);
    
    tmp_mat=Axy*Dyz*Bxy*Cyz-(Axy*Dzy+Qz)*(Bxz*Cyz+Py);
    sparse_matrix_inject(tmp_mat,em_mat,0,0);
    
    tmp_mat=-Axy*Dyz*(Bxy*Czy+Pz)+(Axy*Dzy+Qz)*Bxz*Czy;
    sparse_matrix_inject(tmp_mat,em_mat,0,Nyz);
    
    tmp_mat=(Axz*Dyz+Qy)*Bxy*Cyz-Axz*Dzy*(Bxz*Cyz+Py);
    sparse_matrix_inject(tmp_mat,em_mat,Nyz,0);
    
    tmp_mat=-(Axz*Dyz+Qy)*(Bxy*Czy+Pz)+Axz*Dzy*Bxz*Czy;
    sparse_matrix_inject(tmp_mat,em_mat,Nyz,Nyz);
    
    Eigen::VectorXcd field(2*Nyz);
    
    for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
    {
        i=index(0,j,k);
        
        field(i+0*Nyz)=Ey_(j,k);
        field(i+1*Nyz)=Ez_(j,k);
    }
    
    Imdouble mb2=0;
    
    if(solver=="default") mb2=inverse_power_iteration(em_mat,-(n_guess*k0)*(n_guess*k0),field,1e-10,1000);
    else if(solver=="bicgstab") mb2=inverse_power_iteration_BICGSTAB(em_mat,-(n_guess*k0)*(n_guess*k0),field,1e-10,1000);
    n_mode=std::sqrt(-mb2)/k0;
    
    for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
    {
        i=index(0,j,k);
        
        Ey_(j,k)=field(i+0*Nyz);
        Ez_(j,k)=field(i+1*Nyz);
    }
}
