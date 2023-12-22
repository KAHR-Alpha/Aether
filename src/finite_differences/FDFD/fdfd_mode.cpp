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
#include <lua_fd.h>


extern const Imdouble Im;
extern std::ofstream plog;

void fdfd_single_particle(FDFD_Mode const &fdfd_mode)
{
    int i,j,k;
    
    int Nx=60;
    int Ny=60;
    int Nz=60;
    
    double Dx=fdfd_mode.Dx;
    double Dy=fdfd_mode.Dy;
    double Dz=fdfd_mode.Dz;
    
    double lx,ly,lz;
    
    fdfd_mode.structure->retrieve_nominal_size(lx,ly,lz);
    fdfd_mode.compute_discretization(Nx,Ny,Nz,lx,ly,lz);
    
    Grid3<unsigned int> matsgrid(Nx,Ny,Nz,0);
    fdfd_mode.structure->discretize(matsgrid,Nx,Ny,Nz,Dx,Dy,Dz);
    
    if(Ny>1)
    {
        std::cout<<"Warning, 3D case not supported yet. Aborting."<<std::endl;
        std::exit(0);
    }
    
    AngleRad pol(0);
    std::string polar_mode=fdfd_mode.polarization;
    
    if(polar_mode=="TE") pol=Degree(0);
    else if(polar_mode=="TM") pol=Degree(90);
    else if(polar_mode=="mix") pol=Degree(fdfd_mode.polar_angle);
    
    FDFD fdfd(Dx,Dy,Dz);
    
    if(fdfd_mode.solver=="LU") fdfd.solver_type=SOLVE_LU;
    else if(fdfd_mode.solver=="BiCGSTAB") fdfd.solver_type=SOLVE_BiCGSTAB;
    else
    {
        std::cout<<"Unknown solver: "<<fdfd_mode.solver<<std::endl;
        std::cout<<"Aborting..."<<std::endl;
        std::exit(0);
    }
    
    fdfd.set_padding(fdfd_mode.pad_xm,fdfd_mode.pad_xp,
                     fdfd_mode.pad_ym,fdfd_mode.pad_yp,
                     fdfd_mode.pad_zm,fdfd_mode.pad_zp);
    
    fdfd.set_pml_xm(fdfd_mode.pml_xm,
                    fdfd_mode.kappa_xm,
                    fdfd_mode.sigma_xm,
                    fdfd_mode.alpha_xm);
    
    fdfd.set_pml_xp(fdfd_mode.pml_xp,
                    fdfd_mode.kappa_xp,
                    fdfd_mode.sigma_xp,
                    fdfd_mode.alpha_xp);
    
    fdfd.set_pml_zm(fdfd_mode.pml_zm,
                    fdfd_mode.kappa_zm,
                    fdfd_mode.sigma_zm,
                    fdfd_mode.alpha_zm);
    
    fdfd.set_pml_zp(fdfd_mode.pml_zp,
                    fdfd_mode.kappa_zp,
                    fdfd_mode.sigma_zp,
                    fdfd_mode.alpha_zp);
    
    fdfd.set_matsgrid(matsgrid);
    
    Eigen::SparseVector<Imdouble> F_src;
    
    // Auxiliary computation;
    
    {
        FDFD fdfd_aux(Dx,Dy,Dz);
        
        fdfd_aux.solver_type=fdfd.solver_type;
        
        fdfd_aux.set_pml_zm(fdfd_mode.pml_zm,
                            fdfd_mode.kappa_zm,
                            fdfd_mode.sigma_zm,
                            fdfd_mode.alpha_zm);
        
        fdfd_aux.set_pml_zp(fdfd_mode.pml_zp,
                            fdfd_mode.kappa_zp,
                            fdfd_mode.sigma_zp,
                            fdfd_mode.alpha_zp);
        
        fdfd_aux.set_padding(fdfd.pml_xm+fdfd.pad_xm,
                             fdfd.pml_xp+fdfd.pad_xp,
                             0,0,
                             fdfd.pad_zm,fdfd.pad_zp);
        
        Grid3<unsigned int> matsgrid_aux(Nx,Ny,Nz,0);
        
        for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++){ for(k=0;k<Nz;k++)
        {
            matsgrid_aux(i,j,k)=matsgrid(0,0,k);
        }}}
        
        fdfd_aux.set_matsgrid(matsgrid_aux);
        
        #ifdef OLDMAT
        for(unsigned int m=0;m<fdfd_mode.materials_str.size();m++)
        {
            fdfd_aux.set_material(fdfd_mode.materials_index[m],fdfd_mode.materials_str[m]);
        }
        #endif
        
        fdfd_aux.set_injection_plane_z(fdfd_aux.zs_e+1);
//        fdfd_aux.solve_prop_2D(fdfd_mode.lambda,
//                               fdfd_mode.inc_theta,
//                               fdfd_mode.inc_phi,pol);
        
        fdfd_aux.draw(0,Nx/2,Ny/2,Nz/2,fdfd_mode.prefix);
        
        
        fdfd_aux.get_injection_cbox(fdfd.xs_s,fdfd.xs_e,0,1,fdfd.zs_s,fdfd.zs_e,
                                    true,false,true,F_src);
    }
    
    Nx=fdfd.Nx;
    Ny=fdfd.Ny;
    Nz=fdfd.Nz;
    
    #ifdef OLDMAT
    for(unsigned int m=0;m<fdfd_mode.materials_str.size();m++)
    {
        fdfd.set_material(fdfd_mode.materials_index[m],fdfd_mode.materials_str[m]);
    }
    #endif
    
    fdfd.set_injection_cbox(0,0,0,0,0,0,F_src);
//    fdfd.solve_prop_2D(fdfd_mode.lambda,fdfd_mode.inc_theta,fdfd_mode.inc_phi,pol);
    
    fdfd.draw(0,Nx/2,Ny/2,Nz/2,fdfd_mode.prefix);
    
    if(fdfd_mode.output_map)
    {
        Nx=fdfd.pad_xm+fdfd.Nx_s+fdfd.pad_xp;
        Nz=fdfd.pad_zm+fdfd.Nz_s+fdfd.pad_zp;
        
        std::string fname_Ex_real=fdfd_mode.prefix;
        std::string fname_Ey_real=fdfd_mode.prefix;
        std::string fname_Ez_real=fdfd_mode.prefix;
        std::string fname_Hx_real=fdfd_mode.prefix;
        std::string fname_Hy_real=fdfd_mode.prefix;
        std::string fname_Hz_real=fdfd_mode.prefix;
        
        fname_Ex_real.append("_Ex_real"); fname_Ey_real.append("_Ey_real"); fname_Ez_real.append("_Ez_real");
        fname_Hx_real.append("_Hx_real"); fname_Hy_real.append("_Hy_real"); fname_Hz_real.append("_Hz_real");
        
        std::ofstream f_Ex_real(fname_Ex_real,std::ios::out|std::ios::trunc);
        std::ofstream f_Ey_real(fname_Ey_real,std::ios::out|std::ios::trunc);
        std::ofstream f_Ez_real(fname_Ez_real,std::ios::out|std::ios::trunc);
        
        std::ofstream f_Hx_real(fname_Hx_real,std::ios::out|std::ios::trunc);
        std::ofstream f_Hy_real(fname_Hy_real,std::ios::out|std::ios::trunc);
        std::ofstream f_Hz_real(fname_Hz_real,std::ios::out|std::ios::trunc);
        
        std::string fname_Ex_imag=fdfd_mode.prefix;
        std::string fname_Ey_imag=fdfd_mode.prefix;
        std::string fname_Ez_imag=fdfd_mode.prefix;
        std::string fname_Hx_imag=fdfd_mode.prefix;
        std::string fname_Hy_imag=fdfd_mode.prefix;
        std::string fname_Hz_imag=fdfd_mode.prefix;
        
        fname_Ex_imag.append("_Ex_imag"); fname_Ey_imag.append("_Ey_imag"); fname_Ez_imag.append("_Ez_imag");
        fname_Hx_imag.append("_Hx_imag"); fname_Hy_imag.append("_Hy_imag"); fname_Hz_imag.append("_Hz_imag");
        
        std::ofstream f_Ex_imag(fname_Ex_imag,std::ios::out|std::ios::trunc);
        std::ofstream f_Ey_imag(fname_Ey_imag,std::ios::out|std::ios::trunc);
        std::ofstream f_Ez_imag(fname_Ez_imag,std::ios::out|std::ios::trunc);
        
        std::ofstream f_Hx_imag(fname_Hx_imag,std::ios::out|std::ios::trunc);
        std::ofstream f_Hy_imag(fname_Hy_imag,std::ios::out|std::ios::trunc);
        std::ofstream f_Hz_imag(fname_Hz_imag,std::ios::out|std::ios::trunc);
        
        for(k=0;k<Nz;k++)
        {
            for(i=0;i<Nx;i++)
            {
                f_Ex_real<<fdfd.interp_Ex(fdfd.xs_s-fdfd.pad_xm+i,0,
                                          fdfd.zs_s-fdfd.pad_zm+k).real()<<" ";
                f_Ex_imag<<fdfd.interp_Ex(fdfd.xs_s-fdfd.pad_xm+i,0,
                                          fdfd.zs_s-fdfd.pad_zm+k).imag()<<" ";
                f_Ey_real<<fdfd.interp_Ey(fdfd.xs_s-fdfd.pad_xm+i,0,
                                          fdfd.zs_s-fdfd.pad_zm+k).real()<<" ";
                f_Ey_imag<<fdfd.interp_Ey(fdfd.xs_s-fdfd.pad_xm+i,0,
                                          fdfd.zs_s-fdfd.pad_zm+k).imag()<<" ";
                f_Ez_real<<fdfd.interp_Ez(fdfd.xs_s-fdfd.pad_xm+i,0,
                                          fdfd.zs_s-fdfd.pad_zm+k).real()<<" ";
                f_Ez_imag<<fdfd.interp_Ez(fdfd.xs_s-fdfd.pad_xm+i,0,
                                          fdfd.zs_s-fdfd.pad_zm+k).imag()<<" ";
                                          
                f_Hx_real<<fdfd.interp_Hx(fdfd.xs_s-fdfd.pad_xm+i,0,
                                          fdfd.zs_s-fdfd.pad_zm+k).real()<<" ";
                f_Hx_imag<<fdfd.interp_Hx(fdfd.xs_s-fdfd.pad_xm+i,0,
                                          fdfd.zs_s-fdfd.pad_zm+k).imag()<<" ";
                f_Hy_real<<fdfd.interp_Hy(fdfd.xs_s-fdfd.pad_xm+i,0,
                                          fdfd.zs_s-fdfd.pad_zm+k).real()<<" ";
                f_Hy_imag<<fdfd.interp_Hy(fdfd.xs_s-fdfd.pad_xm+i,0,
                                          fdfd.zs_s-fdfd.pad_zm+k).imag()<<" ";
                f_Hz_real<<fdfd.interp_Hz(fdfd.xs_s-fdfd.pad_xm+i,0,
                                          fdfd.zs_s-fdfd.pad_zm+k).real()<<" ";
                f_Hz_imag<<fdfd.interp_Hz(fdfd.xs_s-fdfd.pad_xm+i,0,
                                          fdfd.zs_s-fdfd.pad_zm+k).imag()<<" ";
            }
            
            f_Ex_real<<std::endl; f_Ex_imag<<std::endl;
            f_Ey_real<<std::endl; f_Ey_imag<<std::endl;
            f_Ez_real<<std::endl; f_Ez_imag<<std::endl;
            f_Hx_real<<std::endl; f_Hx_imag<<std::endl;
            f_Hy_real<<std::endl; f_Hy_imag<<std::endl;
            f_Hz_real<<std::endl; f_Hz_imag<<std::endl;
        }
    }
}

//#include <bitset>

class DiffOrder
{
    public:
        int p,q;
        double power;
        double dir_x,dir_y,dir_z;
        
        DiffOrder()
            :p(0), q(0),
             power(0),
             dir_x(0), dir_y(0), dir_z(0)
        {
        }
        
        DiffOrder(DiffOrder const &diff)
            :p(diff.p), q(diff.q),
             power(diff.power),
             dir_x(diff.dir_x), dir_y(diff.dir_y), dir_z(diff.dir_z)
        {
        }
        
        void operator = (DiffOrder const &diff)
        {
            p=diff.p; q=diff.q;
            power=diff.power;
            
            dir_x=diff.dir_x;
            dir_y=diff.dir_y;
            dir_z=diff.dir_z;
        }
};

double compute_diffracted_orders_power(int &pmin,int &pmax,int &qmin,int &qmax,std::vector<DiffOrder> &diff_orders,
                                       Grid2<Imdouble> const &Ex,
                                       Grid2<Imdouble> const &Ey,
                                       Grid2<Imdouble> const &Ez,
                                       double D1,double D2,
                                       double lambda,double n_index,double beta_x,double beta_y,
                                       bool down)
{
    int i,j,p,q;
    Imdouble coeff;
    
    double k0=2.0*Pi/lambda;
    double kn=k0*n_index;
    double w=2.0*Pi*c_light/lambda;
    
    int span1=Ex.L1();
    int span2=Ex.L2();
    
    double L1=D1*span1;
    double L2=D2*span2;
        
    pmin=static_cast<int>(-L1*(n_index/lambda+beta_x/(2.0*Pi)));
    pmax=static_cast<int>(+L1*(n_index/lambda-beta_x/(2.0*Pi)));
    
    qmin=static_cast<int>(-L2*(n_index/lambda+beta_y/(2.0*Pi)));
    qmax=static_cast<int>(+L2*(n_index/lambda-beta_y/(2.0*Pi)));
    
    double F_tot=0;
    
    diff_orders.clear();
    DiffOrder tmp;
    
    for(p=pmin;p<=pmax;p++){ for(q=qmin;q<=qmax;q++)
    {
        Imdouble ax=0,ay=0,az=0;
        
        double k1=beta_x+2.0*p*Pi/L1;
        double k2=beta_y+2.0*q*Pi/L2;
        
        double k3s=kn*kn-k1*k1-k2*k2;
        
        Vector3 k_diff(k1,k2,0);
        
        double F_pq=0;
        
//        if(p==-1)
//        {
//            std::cout<<beta_x<<" "<<beta_y<<" "<<k1<<" "<<k2<<" "<<k3s<<" "<<L1<<" "<<L2<<std::endl;
//        }
//        chk_var(p);
//        chk_var(q);
        
        if(k3s>0)
        {
            double k1d=k1*D1;
            double k2d=k2*D2;
            
            for(j=0;j<span2;j++)
            {
                for(i=0;i<span1;i++)
                {
                    coeff=std::exp(-(k1d*i+k2d*j)*Im);
                    
                    ax+=Ex(i,j)*coeff;
                    ay+=Ey(i,j)*coeff;
                    az+=Ez(i,j)*coeff;
                }
            }
            
            double int_coeff=D1*D2/(L1*L2);
            
            ax*=int_coeff;
            ay*=int_coeff;
            az*=int_coeff;
            
//            chk_var(p);
//            chk_var(q);
//            chk_var(ax);
//            chk_var(ay);
//            chk_var(az);
            
            ImVector3 a_pq(ax,ay,az);
            ImVector3 k_pq(k1,k2,std::sqrt(k3s));
            k_diff.z=std::sqrt(k3s);
            
            if(down)
            {
                k_pq.z=-k_pq.z;
                k_diff.z=-k_diff.z;
            }
            
            ImVector3 CC=crossprod(a_pq,crossprod(k_pq,a_pq.conj()));
            
            F_pq=L1*L2*CC.z.real()/(2.0*w*mu0);
            
            if(down) F_pq=-F_pq;
        }
        
        tmp.p=p;
        tmp.q=q;
        tmp.power=F_pq;
        
        k_diff.normalize();
        
        tmp.dir_x=k_diff.x;
        tmp.dir_y=k_diff.y;
        tmp.dir_z=k_diff.z;
        
        diff_orders.push_back(tmp);
        
        F_tot+=F_pq;
    }}
    
    return F_tot;
}

void fdfd_periodic(FDFD_Mode const &fdfd_mode)
{
    int i,j,k,l;
    
    int Nx=60;
    int Ny=60;
    int Nz=60;
    
    double Dx=fdfd_mode.Dx;
    double Dy=fdfd_mode.Dx;
    double Dz=fdfd_mode.Dx;
    
    double lx,ly,lz;
    
    fdfd_mode.structure->retrieve_nominal_size(lx,ly,lz);
    fdfd_mode.compute_discretization(Nx,Ny,Nz,lx,ly,lz);
    
    Grid3<unsigned int> matsgrid(Nx,Ny,Nz,0);
    fdfd_mode.structure->discretize(matsgrid,Nx,Ny,Nz,Dx,Dy,Dz);
    
    if(Ny>1)
    {
        std::cout<<"Warning, 3D case not supported yet.\n Aborting."<<std::endl;
        std::exit(EXIT_FAILURE);
    }
    
    AngleRad pol(0);
    std::string polar_mode=fdfd_mode.polarization;
    
    if(polar_mode=="TE") pol=Degree(0);
    else if(polar_mode=="TM") pol=Degree(90);
    else if(polar_mode=="mix") pol=Degree(fdfd_mode.polar_angle);
    
    FDFD fdfd(Dx,Dy,Dz);
    
    if(fdfd_mode.solver=="LU") fdfd.solver_type=SOLVE_LU;
    else if(fdfd_mode.solver=="BiCGSTAB") fdfd.solver_type=SOLVE_BiCGSTAB;
    else
    {
        std::cout<<"Unknown solver: "<<fdfd_mode.solver<<std::endl;
        std::cout<<"Aborting..."<<std::endl;
        std::exit(0);
    }
    
    fdfd.set_padding(fdfd_mode.pad_xm,fdfd_mode.pad_xp,
                     fdfd_mode.pad_ym,fdfd_mode.pad_yp,
                     fdfd_mode.pad_zm,fdfd_mode.pad_zp);
    
    fdfd.set_pml_xm(fdfd_mode.pml_xm,
                    fdfd_mode.kappa_xm,
                    fdfd_mode.sigma_xm,
                    fdfd_mode.alpha_xm);
    
    fdfd.set_pml_xp(fdfd_mode.pml_xp,
                    fdfd_mode.kappa_xp,
                    fdfd_mode.sigma_xp,
                    fdfd_mode.alpha_xp);
    
    fdfd.set_pml_zm(fdfd_mode.pml_zm,
                    fdfd_mode.kappa_zm,
                    fdfd_mode.sigma_zm,
                    fdfd_mode.alpha_zm);
    
    fdfd.set_pml_zp(fdfd_mode.pml_zp,
                    fdfd_mode.kappa_zp,
                    fdfd_mode.sigma_zp,
                    fdfd_mode.alpha_zp);
    
    fdfd.set_matsgrid(matsgrid);
    
    Nx=fdfd.Nx;
    Ny=fdfd.Ny;
    Nz=fdfd.Nz;
    
    for(unsigned int m=0;m<fdfd_mode.materials.size();m++)
        fdfd.set_material(m,fdfd_mode.materials[m]);
    
    fdfd.set_injection_plane_z(fdfd.zs_e+1);
    
    std::ofstream file,map_file,
                  diffract_file_up,
                  diffract_file_down;
    
    std::string fname=fdfd_mode.prefix;
    
    file.open(fname+"fdfd_spectral_data",std::ios::out|std::ios::trunc);
    
    if(fdfd_mode.output_diffraction)
    {
        diffract_file_up.open(fname+"fdfd_diffraction_up_data",std::ios::out|std::ios::trunc);
        diffract_file_down.open(fname+"fdfd_diffraction_down_data",std::ios::out|std::ios::trunc);
    }
        
//    if(fdfd_mode.output_map)
//    {
//        map_file.open(fdfd_mode.prefix+"fdfd_maps_data",std::ios::out|std::ios::trunc|std::ios::binary);
//        
//        int N_maps=fdfd_mode.N_phi*fdfd_mode.N_theta*fdfd_mode.Nl;
//        
//        map_file.write(reinterpret_cast<char*>(&N_maps),sizeof(N_maps));
//        
//        unsigned int disc_type=0;
//        
//        map_file.write(reinterpret_cast<char*>(&disc_type),sizeof(disc_type));
//        
//        std::bitset<7> data_type;
//        
//        data_type.set(0);
//        data_type.set(1);
//        data_type.set(2);
//        
//        unsigned long data_type_l=0;
//        
//        map_file.write(reinterpret_cast<char*>(&data_type_l),sizeof(data_type_l));
//        
//        map_file.write(reinterpret_cast<char*>(&Nx),sizeof(Nx));
//        map_file.write(reinterpret_cast<char*>(&Nz),sizeof(Nz));
//        map_file.write(reinterpret_cast<char*>(&Dx),sizeof(Dx));
//        map_file.write(reinterpret_cast<char*>(&Dz),sizeof(Dz));
//    }
    
    Grid2<Imdouble> diff_Ex(Nx,Ny),
                    diff_Ey(Nx,Ny),
                    diff_Ez(Nx,Ny);
    
    std::vector<DiffOrder> diff_orders;
    
    for(int n=0;n<fdfd_mode.N_phi;n++)
    {
        AngleRad phi=fdfd_mode.phi_min;
        
        if(fdfd_mode.N_phi>1)
            phi=interpolate_linear(fdfd_mode.phi_min,fdfd_mode.phi_max,n/(fdfd_mode.N_phi-1.0));
        
        for(int m=0;m<fdfd_mode.N_theta;m++)
        {
            AngleRad theta=fdfd_mode.theta_min;
            
            if(fdfd_mode.N_theta>1)
                theta=interpolate_linear(fdfd_mode.theta_min,fdfd_mode.theta_max,m/(fdfd_mode.N_theta-1.0));
            
            for(l=0;l<fdfd_mode.Nl;l++)
            {
                double lambda=fdfd_mode.lambda_min;
                
                if(fdfd_mode.Nl>1)
                    lambda=interpolate_linear(fdfd_mode.lambda_min,fdfd_mode.lambda_max,l/(fdfd_mode.Nl-1.0));
                
                fdfd.solve_prop_2D(lambda,theta,phi,pol);
                
                file<<fdfd.lambda<<" "<<theta.degree()<<" "<<phi.degree()<<" ";
                
                double R=0,T=0;
                
                //###################
                //   Upper sensing
                //###################
                
                double inj_index=fdfd.mats[fdfd.matsgrid(0,0,fdfd.inj_zp)].get_eps(2.0*Pi*c_light/fdfd.lambda).real();
                inj_index=std::sqrt(inj_index);
                
                // Specular reflection
                
                k=fdfd.zs_e+3;
                for(i=0;i<Nx;i++)
                {
                    R+=0.5*std::real(fdfd.get_Ex(i,0,k)*0.5*std::conj(fdfd.get_Hy(i,0,k)+fdfd.get_Hy(i,0,k-1))-
                                     fdfd.get_Ey(i,0,k)*0.5*std::conj(fdfd.get_Hx(i,0,k)+fdfd.get_Hx(i,0,k-1)));
                }
                
                R/=Nx*0.5*std::sqrt(e0/mu0)*std::cos(fdfd.inc_theta)*inj_index;
                
                // Diffraction
                
                if(fdfd_mode.output_diffraction)
                {
                    double norm=Nx*0.5*std::sqrt(e0/mu0)*std::cos(fdfd.inc_theta)*inj_index*Dx*Dy;
                    
                    for(i=0;i<Nx;i++)
                    for(j=0;j<Ny;j++)
                    {
                        diff_Ex(i,j)=fdfd.interp_Ex(i,j,k);
                        diff_Ey(i,j)=fdfd.interp_Ey(i,j,k);
                        diff_Ez(i,j)=fdfd.interp_Ez(i,j,k);
                    }
                    
                    int pmin,pmax,qmin,qmax;
                    
                    double F_tot=compute_diffracted_orders_power(pmin,pmax,qmin,qmax,diff_orders,
                                                                 diff_Ex,diff_Ey,diff_Ez,
                                                                 Dx,Dy,lambda,inj_index,fdfd.kx,fdfd.ky,false);
                    
                    diffract_file_up<<fdfd.lambda<<" "<<theta.degree()<<" "<<phi.degree()<<" ";
                    diffract_file_up<<pmin<<" "<<pmax<<" "<<qmin<<" "<<qmax<<" ";
                    
                    for(std::size_t p=0;p<diff_orders.size();p++)
                    {
                        diffract_file_up<<diff_orders[p].p<<" "<<diff_orders[p].q<<" ";
                        diffract_file_up<<diff_orders[p].power/norm<<" ";
                        diffract_file_up<<diff_orders[p].dir_x<<" "<<diff_orders[p].dir_y<<" "<<diff_orders[p].dir_z<<" ";
                    }
                    
                    diffract_file_up<<F_tot/norm<<std::endl;
                }
                
                //##################
                //   Down sensing
                //##################
                
                // Direct Transmission
                
                double tra_index=fdfd.mats[fdfd.matsgrid(0,0,fdfd.zs_s)].get_eps(2.0*Pi*c_light/fdfd.lambda).real();
                tra_index=std::sqrt(tra_index);
                
                k=fdfd.zs_s;
                for(i=0;i<Nx;i++)
                {
                    T-=0.5*std::real(fdfd.get_Ex(i,0,k)*0.5*std::conj(fdfd.get_Hy(i,0,k)+fdfd.get_Hy(i,0,k-1))-
                                     fdfd.get_Ey(i,0,k)*0.5*std::conj(fdfd.get_Hx(i,0,k)+fdfd.get_Hx(i,0,k-1)));
                }
                
                T/=Nx*0.5*std::sqrt(e0/mu0)*std::cos(fdfd.inc_theta)*inj_index;
                
                // Diffraction
                
                if(fdfd_mode.output_diffraction)
                {
                    double norm=Nx*0.5*std::sqrt(e0/mu0)*std::cos(fdfd.inc_theta)*inj_index*Dx*Dy;
                    
                    for(i=0;i<Nx;i++)
                    for(j=0;j<Ny;j++)
                    {
                        diff_Ex(i,j)=fdfd.interp_Ex(i,j,k);
                        diff_Ey(i,j)=fdfd.interp_Ey(i,j,k);
                        diff_Ez(i,j)=fdfd.interp_Ez(i,j,k);
                    }
                    
                    int pmin,pmax,qmin,qmax;
                    
                    double F_tot=compute_diffracted_orders_power(pmin,pmax,qmin,qmax,diff_orders,
                                                                 diff_Ex,diff_Ey,diff_Ez,
                                                                 Dx,Dy,lambda,tra_index,fdfd.kx,fdfd.ky,true);
                    
                    diffract_file_down<<fdfd.lambda<<" "<<theta.degree()<<" "<<phi.degree()<<" ";
                    diffract_file_down<<pmin<<" "<<pmax<<" "<<qmin<<" "<<qmax<<" ";
                    
                    for(std::size_t p=0;p<diff_orders.size();p++)
                    {
                        diffract_file_down<<diff_orders[p].p<<" "<<diff_orders[p].q<<" ";
                        diffract_file_down<<diff_orders[p].power/norm<<" ";
                        diffract_file_down<<diff_orders[p].dir_x<<" "<<diff_orders[p].dir_y<<" "<<diff_orders[p].dir_z<<" ";
                    }
                    
                    diffract_file_down<<F_tot/norm<<std::endl;
                }
                
                //double ref_index=fdfd.mats[fdfd.matsgrid(0,0,fdfd.zs_e)].get_eps(2.0*Pi*c_light/fdfd.lambda).real();
                
                file<<R<<" "<<T<<" "<<1.0-R-T<<std::endl;
                
                fdfd.draw(0,Nx/2,Ny/2,Nz/2,fdfd_mode.prefix+"_"+std::to_string(lambda*1e9));
                
//                if(fdfd_mode.output_map)
//                {
//                    map_file.write(reinterpret_cast<char*>(&lambda),sizeof(double));
//                    
//                    double theta_map=theta.degree();
//                    double phi_map=phi.degree();
//                }
            }
        }
    }
    
    file.close();
    
    
    /* if(fdfd_mode.output_map)
    {
        Nx=fdfd.pad_xm+fdfd.Nx_s+fdfd.pad_xp;
        Nz=fdfd.pad_zm+fdfd.Nz_s+fdfd.pad_zp;
        
        std::string fname_Ex_real=fdfd_mode.prefix;
        std::string fname_Ey_real=fdfd_mode.prefix;
        std::string fname_Ez_real=fdfd_mode.prefix;
        std::string fname_Hx_real=fdfd_mode.prefix;
        std::string fname_Hy_real=fdfd_mode.prefix;
        std::string fname_Hz_real=fdfd_mode.prefix;
        
        fname_Ex_real.append("_Ex_real"); fname_Ey_real.append("_Ey_real"); fname_Ez_real.append("_Ez_real");
        fname_Hx_real.append("_Hx_real"); fname_Hy_real.append("_Hy_real"); fname_Hz_real.append("_Hz_real");
        
        std::ofstream f_Ex_real(fname_Ex_real,std::ios::out|std::ios::trunc);
        std::ofstream f_Ey_real(fname_Ey_real,std::ios::out|std::ios::trunc);
        std::ofstream f_Ez_real(fname_Ez_real,std::ios::out|std::ios::trunc);
        
        std::ofstream f_Hx_real(fname_Hx_real,std::ios::out|std::ios::trunc);
        std::ofstream f_Hy_real(fname_Hy_real,std::ios::out|std::ios::trunc);
        std::ofstream f_Hz_real(fname_Hz_real,std::ios::out|std::ios::trunc);
        
        std::string fname_Ex_imag=fdfd_mode.prefix;
        std::string fname_Ey_imag=fdfd_mode.prefix;
        std::string fname_Ez_imag=fdfd_mode.prefix;
        std::string fname_Hx_imag=fdfd_mode.prefix;
        std::string fname_Hy_imag=fdfd_mode.prefix;
        std::string fname_Hz_imag=fdfd_mode.prefix;
        
        fname_Ex_imag.append("_Ex_imag"); fname_Ey_imag.append("_Ey_imag"); fname_Ez_imag.append("_Ez_imag");
        fname_Hx_imag.append("_Hx_imag"); fname_Hy_imag.append("_Hy_imag"); fname_Hz_imag.append("_Hz_imag");
        
        std::ofstream f_Ex_imag(fname_Ex_imag,std::ios::out|std::ios::trunc);
        std::ofstream f_Ey_imag(fname_Ey_imag,std::ios::out|std::ios::trunc);
        std::ofstream f_Ez_imag(fname_Ez_imag,std::ios::out|std::ios::trunc);
        
        std::ofstream f_Hx_imag(fname_Hx_imag,std::ios::out|std::ios::trunc);
        std::ofstream f_Hy_imag(fname_Hy_imag,std::ios::out|std::ios::trunc);
        std::ofstream f_Hz_imag(fname_Hz_imag,std::ios::out|std::ios::trunc);
        
        for(k=0;k<Nz;k++)
        {
            for(i=0;i<Nx;i++)
            {
                f_Ex_real<<fdfd.interp_Ex(fdfd.xs_s-fdfd.pad_xm+i,0,
                                          fdfd.zs_s-fdfd.pad_zm+k).real()<<" ";
                f_Ex_imag<<fdfd.interp_Ex(fdfd.xs_s-fdfd.pad_xm+i,0,
                                          fdfd.zs_s-fdfd.pad_zm+k).imag()<<" ";
                f_Ey_real<<fdfd.interp_Ey(fdfd.xs_s-fdfd.pad_xm+i,0,
                                          fdfd.zs_s-fdfd.pad_zm+k).real()<<" ";
                f_Ey_imag<<fdfd.interp_Ey(fdfd.xs_s-fdfd.pad_xm+i,0,
                                          fdfd.zs_s-fdfd.pad_zm+k).imag()<<" ";
                f_Ez_real<<fdfd.interp_Ez(fdfd.xs_s-fdfd.pad_xm+i,0,
                                          fdfd.zs_s-fdfd.pad_zm+k).real()<<" ";
                f_Ez_imag<<fdfd.interp_Ez(fdfd.xs_s-fdfd.pad_xm+i,0,
                                          fdfd.zs_s-fdfd.pad_zm+k).imag()<<" ";
                                          
                f_Hx_real<<fdfd.interp_Hx(fdfd.xs_s-fdfd.pad_xm+i,0,
                                          fdfd.zs_s-fdfd.pad_zm+k).real()<<" ";
                f_Hx_imag<<fdfd.interp_Hx(fdfd.xs_s-fdfd.pad_xm+i,0,
                                          fdfd.zs_s-fdfd.pad_zm+k).imag()<<" ";
                f_Hy_real<<fdfd.interp_Hy(fdfd.xs_s-fdfd.pad_xm+i,0,
                                          fdfd.zs_s-fdfd.pad_zm+k).real()<<" ";
                f_Hy_imag<<fdfd.interp_Hy(fdfd.xs_s-fdfd.pad_xm+i,0,
                                          fdfd.zs_s-fdfd.pad_zm+k).imag()<<" ";
                f_Hz_real<<fdfd.interp_Hz(fdfd.xs_s-fdfd.pad_xm+i,0,
                                          fdfd.zs_s-fdfd.pad_zm+k).real()<<" ";
                f_Hz_imag<<fdfd.interp_Hz(fdfd.xs_s-fdfd.pad_xm+i,0,
                                          fdfd.zs_s-fdfd.pad_zm+k).imag()<<" ";
            }
            
            f_Ex_real<<std::endl; f_Ex_imag<<std::endl;
            f_Ey_real<<std::endl; f_Ey_imag<<std::endl;
            f_Ez_real<<std::endl; f_Ez_imag<<std::endl;
            f_Hx_real<<std::endl; f_Hx_imag<<std::endl;
            f_Hy_real<<std::endl; f_Hy_imag<<std::endl;
            f_Hz_real<<std::endl; f_Hz_imag<<std::endl;
        }
    }*/
}

void mode_3D(FDFD_Mode const &fdfd_mode)
{
    int Nx=60;
    int Ny=60;
    int Nz=60;
    
    double Dx=fdfd_mode.Dx;
    double Dy=fdfd_mode.Dy;
    double Dz=fdfd_mode.Dz;
    
    double lx,ly,lz;
    
    fdfd_mode.structure->retrieve_nominal_size(lx,ly,lz);
    fdfd_mode.compute_discretization(Nx,Ny,Nz,lx,ly,lz);
    
    Grid3<unsigned int> matsgrid(Nx,Ny,Nz,0);
    fdfd_mode.structure->discretize(matsgrid,Nx,Ny,Nz,Dx,Dy,Dz);
    
    AngleRad pol(0);
    std::string polar_mode=fdfd_mode.polarization;
    
    if(polar_mode=="TE") pol=Degree(0);
    else if(polar_mode=="TM") pol=Degree(90);
    else if(polar_mode=="mix") pol=Degree(fdfd_mode.polar_angle);
    
    FDFD fdfd(Dx,Dy,Dz);
    
    fdfd.set_padding(0,0,
                     fdfd_mode.pad_ym,fdfd_mode.pad_yp,
                     fdfd_mode.pad_zm,fdfd_mode.pad_zp);
                     
    fdfd.set_pml_zm(fdfd_mode.pml_zm,
                    fdfd_mode.kappa_zm,
                    fdfd_mode.sigma_zm,
                    fdfd_mode.alpha_zm);
    
    fdfd.set_pml_zp(fdfd_mode.pml_zp,
                    fdfd_mode.kappa_zp,
                    fdfd_mode.sigma_zp,
                    fdfd_mode.alpha_zp);
    
    fdfd.set_matsgrid(matsgrid);
        
    Nx=fdfd.Nx;
    Ny=fdfd.Ny;
    Nz=fdfd.Nz;
    
    #ifdef OLDMAT
    for(unsigned int m=0;m<fdfd_mode.materials_str.size();m++)
    {
        fdfd.set_material(fdfd_mode.materials_index[m],fdfd_mode.materials_str[m]);
    }
    #endif
    
    fdfd.set_injection_plane_z(fdfd.zs_e+1);
//    fdfd.solve_prop_3D_SAM(fdfd_mode.lambda,fdfd_mode.inc_theta,fdfd_mode.inc_phi,pol);
    
    fdfd.draw(0,Nx/2,Ny/2,Nz/2);
}

void mode_fdfd(FDFD_Mode const &fdfd_mode)
{
//    mode_3D(fdfd_mode);
    if(fdfd_mode.pml_xm>0 || fdfd_mode.pml_xp>0) fdfd_single_particle(fdfd_mode);
    else
    {
        fdfd_periodic(fdfd_mode);
    }
}
