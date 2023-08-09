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

#include <fdtd_material.h>
#include <fdtd_utils.h>


extern const Imdouble Im;
extern std::ofstream plog;

void FDTD_Material::atom_lev_alloc_mem()
{
    levels_matrix.init(N_levels,N_levels,0);
    pump_matrix.init(N_levels,0);
    lve_base_coeff.init(N_levels,0);
    lve_self_coeff.init(N_levels,0);
    
//    x_span=x2+1-x1;
//    y_span=y2+1-y1;
//    z_span=z2+1-z1;
    
    pop_matrix.init(x_span,y_span,z_span,N_levels,0);
    pop_matrix_np.init(x_span,y_span,z_span,N_levels,0);
    
    if(N_stim_trans>0)
    {
        pol_field_np.init(x_span,y_span,z_span,N_stim_trans*3,0);
        pol_field_n.init(x_span,y_span,z_span,N_stim_trans*3,0);
        pol_field_nm.init(x_span,y_span,z_span,N_stim_trans*3,0);
        
        Ex_n.init(x_span,y_span,z_span,0);
        Ey_n.init(x_span,y_span,z_span,0);
        Ez_n.init(x_span,y_span,z_span,0);
    }
    else
    {
        pol_field_np.init(1,1,1,1,0);
        pol_field_n.init(1,1,1,1,0);
        pol_field_nm.init(1,1,1,1,0);
        
        Ex_n.init(1,1,1,0);
        Ey_n.init(1,1,1,0);
        Ez_n.init(1,1,1,0);
    }
}

void FDTD_Material::atom_lev_enable()
{
    comp_simp=false;
    comp_ante=true;
    comp_post=true;
    comp_self=false;
    comp_D=false;
    
    m_type=MAT_ATOM_LEVEL;
}

void test4lvl(double wa,double Dwa,double kappa,double Dt)
{
    double P0=(2.0*Dt*Dt)/(2.0+Dwa*Dt);
    double P1=kappa;
    double P2=2.0/(Dt*Dt)-wa*wa;
    double P3=Dwa/(2.0*Dt)-1.0/(Dt*Dt);
    
    std::cout<<P0*P1<<" "<<P0*P2<<" "<<P0*P3<<std::endl;
}

/*void FDTD_Material::atom_lev_load_script(ScriptHandler &script)
{
    int i,j,k,l,m;
    
    atom_lev_enable();
    
    N_stim_trans=0;
    AL_pop=0;
    
    double eps_loc=1.0;
    
    if(script.has("epsilon")) script.get_options(eps_loc);
    if(script.has("index"))
    {
        script.get_options(eps_loc);
        eps_loc*=eps_loc;
    }
    
    ei=eps_loc; //setting the environment epsilon
    
    const_recalc(); //recomputing the basic constants
    
    show();
    
    if(script.has("levels")) script.get_options(N_levels);
    else{ std::cout<<"Material error, no levels defined"<<std::endl; }
    if(script.has("population")) script.get_options(AL_pop);
    
    while(script.has("add_stim_transition"))
    {
        int lv_up=0;
        int lv_down=0;
        double wlgth=500e-9;
        
        double t_b=0,t_g=0,t_d=0;
        
        script.get_options(lv_down);
        script.get_options(lv_up);
        script.get_options(wlgth);
        script.get_options(t_b);
        script.get_options(t_g);
        script.get_options(t_d);
                
        N_stim_trans+=1;
        stim_lv_dn.push_back(lv_down);
        stim_lv_up.push_back(lv_up);
        stim_wlgth.push_back(wlgth);
        
        pol_C1.push_back((4.0-2.0*t_g*Dt*Dt)/(2.0+t_b*Dt));
        pol_C2.push_back((t_b*Dt-2.0)/(2.0+t_b*Dt));
        pol_C3.push_back((2.0*Dt*Dt*t_d)/(2.0+t_b*Dt));
        
        std::cout<<pol_C1[0]<<" "<<pol_C2[0]<<" "<<pol_C3[0]<<std::endl;
        test4lvl(t_g,t_b,t_d,Dt);
    }
    
    atom_lev_alloc_mem();
    
    while(script.has("add_transition"))
    {
        int lv_start=0;
        int lv_end=0;
        double t_time=0;
        
        script.get_options(lv_start);
        script.get_options(lv_end);
        script.get_options(t_time);
        
        double inv_t_time=1.0/t_time;
        
        levels_matrix(lv_start,lv_start)-=inv_t_time;
        levels_matrix(lv_end,lv_start)+=inv_t_time;
    }
    
    while(script.has("add_pump"))
    {
        int lv=0;
        double rate=0;
        
        script.get_options(lv);
        script.get_options(rate);
                
        pump_matrix[lv]+=rate;
    }
    
    std::cout<<ei<<std::endl;
    std::cout<<AL_pop<<std::endl<<std::endl;
    levels_matrix.show();
    std::cout<<std::endl;
    pump_matrix.show();
    
    atom_lev_precompute();
    
    debug_msg("Initializing levels");
    
    for(i=0;i<x_span;i++)
        for(j=0;j<y_span;j++)
            for(k=0;k<z_span;k++)
                if(mat_present(i,j,k)) { pop_matrix(i,j,k,0)=AL_pop;}
    
    if(script.has("prepopulate_time"))
    {
        std::string ptype; script.get_options(ptype);
        int Nt=0;
        
        if(ptype=="time")
        {
            double tmp; script.get_options(tmp);
            Nt=static_cast<int>(tmp/Dt);
        }
        else if(ptype=="step") script.get_options(Nt);
        
        std::cout<<Nt<<std::endl;
    }
    
    if(script.has("prepopulate_DN"))
    {
        int tlvu,tlvd;
        
        script.get_options(tlvd);
        script.get_options(tlvu);
        
        Grid1<long double> sim_N(N_levels,0), sim_Np(N_levels,0);
        
        bool run_popsim=true;
        
        sim_N[0]=AL_pop;
        
        double t_pop=0,at_pop=0;
        
        int inc=0;
        
        while(run_popsim)
        {
            sim_Np=0;
            
            for(l=0;l<N_levels;l++)
                sim_Np[l]+=pump_matrix[l]*lve_base_coeff[l];
            
            for(l=N_levels-1;l>=0;l--)
            {
                t_pop=lve_self_coeff[l]*sim_N[l];
                
                at_pop=0;
                for(m=l+1;m<N_levels;m++)
                    at_pop+=levels_matrix(l,m)*(sim_Np[m]+sim_N[m]);
                
                at_pop*=lve_base_coeff[l]/2.0;
                
                sim_Np[l]+=t_pop+at_pop;
            }
            
//            plog<<inc<<" ";
//            for(l=0;l<N_levels;l++) plog<<sim_N(l)<<" ";
//            plog<<std::endl;
            
            if(sim_Np[tlvd]-sim_Np[tlvu]<0)
            {
                run_popsim=false;
                
                for(i=0;i<x_span;i++)
                    for(j=0;j<y_span;j++)
                        for(k=0;k<z_span;k++)
                            if(mat_present(i,j,k))
                            {
                                for(l=0;l<N_levels;l++)
                                    pop_matrix(i,j,k,l)=sim_N[l];
                            }
                std::cout<<inc<<std::endl;
            }
            else
            {
                sim_N=sim_Np;
            }
            
            inc++;
        }
    }
    
//    std::system("pause");
    
    if(script.has("prepopulate_inf"))
    {
        bool tmp;
        script.get_options(tmp);
        
        if(tmp)
        {
            pop_matrix=0;
            
            for(i=0;i<x_span;i++){ for(j=0;j<y_span;j++){ for(k=0;k<z_span;k++)
            {
                if(mat_present(i,j,k))
                {
                    long double N_sum=0;
                    
                    for(l=N_levels-1;l>0;l--)
                    {
                        double t_sum=pump_matrix[l];
                        
                        for(m=l+1;m<N_levels;m++)
                        {
                            t_sum+=levels_matrix(l,m)*pop_matrix(i,j,k,m);
                        }
                        
                        pop_matrix(i,j,k,l)=-t_sum/levels_matrix(l,l);
                        
                        N_sum+=pop_matrix(i,j,k,l);
                    }
                    
                    pop_matrix(i,j,k,0)=AL_pop-N_sum;
                }
            }}}
        }
    }
    
    debug_msg("Atomic level mat initilization: done");
}*/

void FDTD_Material::atom_lev_precompute()
{
    int l;
    
    for(l=0;l<N_levels;l++)
    {
        double tg=levels_matrix(l,l);
        
        lve_base_coeff[l]=Dt/(1.0-tg*Dt/2.0);
        lve_self_coeff[l]=(1.0+tg*Dt/2.0)/(1.0-tg*Dt/2.0);
    }
}

void FDTD_Material::AL_ante(int i,int j,int k,
                       Grid3<double> const &Ex,
                       Grid3<double> const &Ey,
                       Grid3<double> const &Ez)
{
    int l;
    int S_ind=pol_field_np.get_ind(i-x1,j-y1,k-z1);
    
    for(l=0;l<N_stim_trans;l++)
    {
        int &lvu=stim_lv_up[l];
        int &lvd=stim_lv_dn[l];
        
        double DN=pop_matrix.agt(S_ind,lvd)-pop_matrix.agt(S_ind,lvu);
        
        //DN=1e26;
        
        pol_field_np.agt(S_ind,3*l+0)=pol_C1[l]*pol_field_n.agt(S_ind,3*l+0)+
                                      pol_C2[l]*pol_field_nm.agt(S_ind,3*l+0)+
                                      pol_C3[l]*Ex(i,j,k)*DN;
        
        pol_field_np.agt(S_ind,3*l+1)=pol_C1[l]*pol_field_n.agt(S_ind,3*l+1)+
                                      pol_C2[l]*pol_field_nm.agt(S_ind,3*l+1)+
                                      pol_C3[l]*Ey(i,j,k)*DN;
        
        pol_field_np.agt(S_ind,3*l+2)=pol_C1[l]*pol_field_n.agt(S_ind,3*l+2)+
                                      pol_C2[l]*pol_field_nm.agt(S_ind,3*l+2)+
                                      pol_C3[l]*Ez(i,j,k)*DN;
                                      
        //if(i==Nx/2 && k==Nz/2+40) plog<<pol_field_np(i-x1,j-y1,k-z1,1)<<std::endl;
    }
    
    if(N_stim_trans>0)
    {
        Ex_n(i-x1,j-y1,k-z1)=Ex(i,j,k);
        Ey_n(i-x1,j-y1,k-z1)=Ey(i,j,k);
        Ez_n(i-x1,j-y1,k-z1)=Ez(i,j,k);
    }
}

void FDTD_Material::AL_post(int i,int j,int k,
                       Grid3<double> const &Ex,
                       Grid3<double> const &Ey,
                       Grid3<double> const &Ez)
{
    int l,m,p;
    
    double t_pop=0;
    double at_pop=0;
    
    for(l=0;l<N_levels;l++)
        pop_matrix_np(i-x1,j-y1,k-z1,l)=0;
    
    // Updating stimulated transitions
    
    
    
    for(l=0;l<N_stim_trans;l++)
    {
        double pscal_EP=0;
        pscal_EP+=0;
        
        pscal_EP+=(Ex(i,j,k)+Ex_n(i-x1,j-y1,k-z1))*(pol_field_np(i-x1,j-y1,k-z1,3*l+0)-pol_field_n(i-x1,j-y1,k-z1,3*l+0));
        pscal_EP+=(Ey(i,j,k)+Ey_n(i-x1,j-y1,k-z1))*(pol_field_np(i-x1,j-y1,k-z1,3*l+1)-pol_field_n(i-x1,j-y1,k-z1,3*l+1));
        pscal_EP+=(Ez(i,j,k)+Ez_n(i-x1,j-y1,k-z1))*(pol_field_np(i-x1,j-y1,k-z1,3*l+2)-pol_field_n(i-x1,j-y1,k-z1,3*l+2));
        
        pscal_EP/=2.0*hbar*(2.0*Pi*c_light/stim_wlgth[l])*Dt;
        
        int &lvu=stim_lv_up[l];
        int &lvd=stim_lv_dn[l];
        
        //if(i==Nx/2) plog<<pscal_EP<<std::endl;
//        if(i==Nx/2)
//        {
//            static int st=0;
//            static double scal_sum=0;
//            plog<<st<<" "<<scal_sum<<std::endl;
//            scal_sum+=pscal_EP;
//            
//            st++;
//        }
        
        pop_matrix_np(i-x1,j-y1,k-z1,lvu)+=pscal_EP*lve_base_coeff[lvu];
        pop_matrix_np(i-x1,j-y1,k-z1,lvd)-=pscal_EP*lve_base_coeff[lvd];
    }
    
    // Adding pumps
    
    for(l=0;l<N_levels;l++)
        pop_matrix_np(i-x1,j-y1,k-z1,l)+=pump_matrix[l]*lve_base_coeff[l];
    
    // Updating populations from high to low energies
    
    for(l=N_levels-1;l>=0;l--)
    {
        t_pop=lve_self_coeff[l]*pop_matrix(i-x1,j-y1,k-z1,l);
        
        at_pop=0;
        
        for(m=l+1;m<N_levels;m++)
        {
            at_pop+=levels_matrix(l,m)*(pop_matrix_np(i-x1,j-y1,k-z1,m)+pop_matrix(i-x1,j-y1,k-z1,m));
        }
        
        at_pop*=lve_base_coeff[l]/2.0;
        
        pop_matrix_np(i-x1,j-y1,k-z1,l)+=t_pop+at_pop;
    }
    
    // Polarization time rotation
    
    for(p=0;p<3*N_stim_trans;p++)
    {
        pol_field_nm(i-x1,j-y1,k-z1,p)=pol_field_n(i-x1,j-y1,k-z1,p);
        pol_field_n(i-x1,j-y1,k-z1,p)=pol_field_np(i-x1,j-y1,k-z1,p);
    }
    
    // Levels time rotation
    
    for(l=0;l<N_levels;l++)
    {
        pop_matrix(i-x1,j-y1,k-z1,l)=pop_matrix_np(i-x1,j-y1,k-z1,l);
    }
    
    if(i==Nx/2+10)
    {
        static int st=0;
        
        static double dnsum=0;
        
        int eh=-2;
        
        if(st%10==0)
        {
            long double p_sum=0;
            long double tdn=pop_matrix(i-x1,j-y1,k-z1,0)-pop_matrix(i-x1,j-y1,k-z1,1);
            dnsum+=tdn;
            
            
            
            if(eh==0)
            {
                plog<<st<<" ";
                for(l=0;l<N_levels;l++){ plog<<pop_matrix(i-x1,j-y1,k-z1,l)<<" "; p_sum+=pop_matrix(i-x1,j-y1,k-z1,l); }
                plog<<p_sum<<std::endl;
            }
            else if(eh==1)
            {
                plog<<st<<" ";
                plog<<(tdn>=0?1:-1)*std::log10(std::abs(tdn)+1e-4)<<std::endl;
                plog<<dnsum<<std::endl;
            }
            else if(eh==2)
            {
                plog<<st<<" ";
                plog<<pop_matrix(i-x1,j-y1,k-z1,1)<<" "<<pop_matrix(i-x1,j-y1,k-z1,2)<<std::endl;
            }
        }
        
        st++;
    }
}

void FDTD_Material::AL_apply_E(int i,int j,int k,Grid3<double> &E,int dir)
{
    double polsum=0;
    
    for(int p=0;p<N_stim_trans;p++)
    {
        polsum+=pol_field_np(i-x1,j-y1,k-z1,3*p+dir)-pol_field_n(i-x1,j-y1,k-z1,3*p+dir);
    }
    
    //if(i==Nx/2 && k==Nz/2) plog<<polsum<<std::endl;
    
    E(i,j,k)-=polsum/(e0*ei);
}
