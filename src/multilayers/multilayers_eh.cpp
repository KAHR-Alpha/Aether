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

extern const double Pi;
extern const Imdouble Im;

//#######################
//   Multilayer_TMM
//#######################

Multilayer_TMM::Multilayer_TMM()
    :N_layers(0), lambda(500e-9), angle(0), kp(0), sup_ind(1.0), sub_ind(1.0)
{
}

Multilayer_TMM::Multilayer_TMM(int N_layers_)
    :N_layers(N_layers_), lambda(500e-9), angle(0), kp(0), sup_ind(1.0), sub_ind(1.0)
{
    h_layer.resize(N_layers);
    index_layer.resize(N_layers);
    material_layer.resize(N_layers);
    
    for(int l=0;l<N_layers;l++)
    {
        h_layer[l]=0;
        index_layer[l]=1.0;
    }
    
    L_mat_TE.resize(N_layers);
    L_mat_TM.resize(N_layers);
}

Multilayer_TMM::Multilayer_TMM(Multilayer_TMM const &ml)
    :N_layers(ml.N_layers),
     lambda(ml.lambda),
     k0(ml.k0),
     angle(ml.angle),
     kp(ml.kp),
     sup_ind(ml.sup_ind),
     sub_ind(ml.sub_ind),
     h_layer(ml.h_layer),
     index_layer(ml.index_layer)
     
{
    L_mat_TE.resize(N_layers);
    L_mat_TM.resize(N_layers);
}

void Multilayer_TMM::compute(Imdouble &r_TE,Imdouble &r_TM,
                             Imdouble &t_TE,Imdouble &t_TM)
{
    k0=2.0*Pi/lambda;
    kp=sup_ind*k0*std::sin(angle);
    
    recompute_L();
    
    Imdouble kn1,kn2;
    Imdouble b1,b2;
    Imdouble n1,n2;
    Imdouble eps1,eps2;
    Imdouble g1,g2;
    
    if(N_layers>0)
    {
        n1=sup_ind;
        n2=index_layer[0];
    }
    else
    {
        n1=sup_ind;
        n2=sub_ind;
    }
    
    eps1=n1*n1;
    eps2=n2*n2;
    
    kn1=k0*n1;
    kn2=k0*n2;
    
    b1=std::sqrt(kn1*kn1-kp*kp);
    b2=std::sqrt(kn2*kn2-kp*kp);
    
    Eigen::Matrix<Imdouble,2,2> M;
    
    //TE
    
    M(0,0)=0.5*(1.0+b1/b2);
    M(0,1)=0.5*(1.0-b1/b2);
    M(1,0)=0.5*(1.0-b1/b2);
    M(1,1)=0.5*(1.0+b1/b2);
    
    for(int l=0;l<N_layers;l++)
        M=L_mat_TE[l]*M;
    
    r_TE=-M(1,0)/M(1,1);
    t_TE=M(0,0)+M(0,1)*r_TE;
    
    //TM
    
    g1=b1/(k0*eps1);
    g2=b2/(k0*eps2);
        
    M(0,0)=0.5*(1.0+g1/g2);
    M(0,1)=0.5*(1.0-g1/g2);
    M(1,0)=0.5*(1.0-g1/g2);
    M(1,1)=0.5*(1.0+g1/g2);
    
    for(int l=0;l<N_layers;l++)
        M=L_mat_TM[l]*M;
    
    r_TM=-M(1,0)/M(1,1);
    t_TM=M(0,0)+M(0,1)*r_TM;
}

Imdouble Multilayer_TMM::compute_chara_TE(Imdouble const &n_eff)
{
    if(n_eff.real()<0 || n_eff.imag()<0) return 1.0;
    
    kp=k0*n_eff;
    
    Imdouble n,g,eps;
    
    double w=m_to_rad_Hz(lambda);
    
    for(int l=0;l<N_layers;l++)
    {
        n=index_layer[l];
        eps=n*n;
        g=std::sqrt(eps*k0*k0-kp*kp);
        
        if(g.imag()<0) g=-g;
        
        Imdouble ch=std::cos(g*h_layer[l]);
        Imdouble sh=std::sin(g*h_layer[l])*Im;
        
        L_mat_TE[l](0,0)=ch;
        L_mat_TE[l](0,1)=-(w*mu0)/g*sh;
        L_mat_TE[l](1,0)=-g/(w*mu0)*sh;
        L_mat_TE[l](1,1)=ch;
    }
    
    Eigen::Matrix<Imdouble,2,2> M;
    
    M(0,0)=1.0; M(0,1)=0.0;
    M(1,0)=0.0; M(1,1)=1.0;
    
    for(int l=0;l<N_layers;l++)
        M=L_mat_TE[l]*M;
    
    Imdouble k_sub=k0*sub_ind;
    Imdouble k_sup=k0*sup_ind;
    
    Imdouble g_sub=std::sqrt(k_sub*k_sub-kp*kp);
    Imdouble g_sup=std::sqrt(k_sup*k_sup-kp*kp);
    
    if(g_sub.imag()<0) g_sub=-g_sub;
    if(g_sup.imag()<0) g_sup=-g_sup;
    
    return M(1,0)+g_sup*g_sub/(w*w*mu0*mu0)*M(0,1)+g_sup/(w*mu0)*M(1,1)+g_sub/(w*mu0)*M(0,0);
}

Imdouble Multilayer_TMM::compute_chara_TM(Imdouble const &n_eff)
{
    if(n_eff.real()<0 || n_eff.imag()<0) return 1.0;
    
    kp=k0*n_eff;
    
    Imdouble n,g,eps;
    
    double w=m_to_rad_Hz(lambda);
    
    for(int l=0;l<N_layers;l++)
    {
        n=index_layer[l];
        eps=n*n;
        g=std::sqrt(eps*k0*k0-kp*kp);
        eps*=e0;
        
        if(g.imag()<0) g=-g;
        
        Imdouble ch=std::cos(g*h_layer[l]);
        Imdouble sh=std::sin(g*h_layer[l])*Im;
        
        L_mat_TM[l](0,0)=ch;
        L_mat_TM[l](0,1)=g/(w*eps)*sh;
        L_mat_TM[l](1,0)=w*eps/g*sh;
        L_mat_TM[l](1,1)=ch;
    }
    
    Eigen::Matrix<Imdouble,2,2> M;
    
    M(0,0)=1.0; M(0,1)=0.0;
    M(1,0)=0.0; M(1,1)=1.0;
    
    for(int l=0;l<N_layers;l++)
        M=L_mat_TM[l]*M;
    
    Imdouble k_sub=k0*sub_ind;
    Imdouble k_sup=k0*sup_ind;
    
    Imdouble eps_sub=sub_ind*sub_ind*e0;
    Imdouble eps_sup=sup_ind*sup_ind*e0;
    
    Imdouble g_sub=std::sqrt(k_sub*k_sub-kp*kp);
    Imdouble g_sup=std::sqrt(k_sup*k_sup-kp*kp);
    
    if(g_sub.imag()<0) g_sub=-g_sub;
    if(g_sup.imag()<0) g_sup=-g_sup;
    
    return (-M(0,1)-g_sup*g_sub/(w*w*eps_sub*eps_sup)*M(1,0)
           +g_sup/(w*eps_sup)*M(0,0)+g_sub/(w*eps_sub)*M(1,1))*e0;
}

void Multilayer_TMM::compute_mode_TE(Imdouble const &n_eff,MLFieldHolder &holder,bool auto_z)
{
    int k,l;
    
    kp=k0*n_eff;
    
    Imdouble n,g,eps,ch,sh;
    
    double w=m_to_rad_Hz(lambda);
    
    for(l=0;l<N_layers;l++)
    {
        n=index_layer[l];
        eps=n*n;
        g=std::sqrt(eps*k0*k0-kp*kp);
        
        if(g.imag()<0) g=-g;
        
        ch=std::cos(g*h_layer[l]);
        sh=std::sin(g*h_layer[l])*Im;
        
        L_mat_TE[l](0,0)=ch;
        L_mat_TE[l](0,1)=-(w*mu0)/g*sh;
        L_mat_TE[l](1,0)=-g/(w*mu0)*sh;
        L_mat_TE[l](1,1)=ch;
    }
    
    Imdouble k_sub=k0*sub_ind;
    Imdouble k_sup=k0*sup_ind;
    
    Imdouble g_sub=std::sqrt(k_sub*k_sub-kp*kp);
    Imdouble g_sup=std::sqrt(k_sup*k_sup-kp*kp);
    
    if(g_sub.imag()<0) g_sub=-g_sub;
    if(g_sup.imag()<0) g_sup=-g_sup;
    
    if(auto_z) holder.set_N_layers(N_layers+2);
    
    Eigen::Matrix<Imdouble,2,1> base_field,disp_field;
    Eigen::Matrix<Imdouble,2,2> M,z_mat;
    
    base_field(0,0)=1.0;
    base_field(1,0)=g_sup/(mu0*w);
    
    M(0,0)=1.0; M(0,1)=0.0;
    M(1,0)=0.0; M(1,1)=1.0;
    
    int Nz=0;
    double z=0,Dz=0,zmax=0,layer_sum=0;
    Imdouble Hx,Ey,Hz;
    
    // Superstrate
    
    if(auto_z)
    {
        Dz=lambda/std::abs(sup_ind)/100.0;
        zmax=-std::log(1e-3)/std::imag(g_sup);
        zmax=std::min(zmax,5.0*lambda);
        Nz=nearest_integer(zmax/Dz);
        Nz=std::max(2,Nz);
        Dz=zmax/(Nz-1.0);
        
        holder.set_Nz(0,Nz);
    }
    else Nz=holder.get_z(0).size();
    
    for(k=0;k<Nz;k++)
    {
        if(auto_z)
        {
            z=-zmax+k*Dz;
            holder.z(0,k)=z;
        }
        else z=holder.z(0,k);
            
        Ey=std::exp(-z*g_sup*Im);
        Hx=g_sup/(mu0*w)*Ey;
        Hz=kp/(mu0*w)*Ey;
        
        holder.E(0,k)=std::abs(Ey);
        holder.H(0,k)=std::sqrt(std::norm(Hx)+std::norm(Hz));
        
        holder.Ey(0,k)=Ey;
        holder.Hx(0,k)=Hx;
        holder.Hz(0,k)=Hz;
    }
    
    // Layers
    
    for(l=0;l<N_layers;l++)
    {
        n=index_layer[l];
        eps=n*n;
        g=std::sqrt(eps*k0*k0-kp*kp);
        
        if(g.imag()<0) g=-g;
        
        if(auto_z)
        {
            Dz=lambda/std::abs(n)/100.0;
            Nz=nearest_integer(h_layer[l]/Dz);
            Nz=std::max(2,Nz);
            Dz=h_layer[l]/(Nz-1.0);
            
            holder.set_Nz(l+1,Nz);
        }
        else Nz=holder.get_z(l+1).size();
        
        for(k=0;k<Nz;k++)
        {
            if(auto_z)
            {
                z=k*Dz;
                holder.z(l+1,k)=z+layer_sum;
            }
            else z=holder.z(l+1,k)-layer_sum;
            
            ch=std::cos(g*z);
            sh=std::sin(g*z)*Im;
            
            z_mat(0,0)=ch;
            z_mat(0,1)=-(w*mu0)/g*sh;
            z_mat(1,0)=-g/(w*mu0)*sh;
            z_mat(1,1)=ch;
            
            disp_field=(z_mat*M)*base_field;
            Ey=disp_field(0,0);
            Hx=disp_field(1,0);
            Hz=kp/(mu0*w)*Ey;
            
            holder.E(l+1,k)=std::abs(Ey);
            holder.H(l+1,k)=std::sqrt(std::norm(Hx)+std::norm(Hz));
            
            holder.Ey(l+1,k)=Ey;
            holder.Hx(l+1,k)=Hx;
            holder.Hz(l+1,k)=Hz;
        }
        
        layer_sum+=h_layer[l];
        M=L_mat_TE[l]*M;
    }
    
    // Substrate
    
    g=g_sub;
    
    if(auto_z)
    {
        Dz=lambda/std::abs(sub_ind)/100.0;
        zmax=-std::log(1e-3)/std::imag(g_sub);
        zmax=std::min(zmax,5.0*lambda);
        Nz=nearest_integer(zmax/Dz);
        Nz=std::max(2,Nz);
        Dz=zmax/(Nz-1.0);
        
        holder.set_Nz(N_layers+1,Nz);
    }
    else Nz=holder.get_z(N_layers+1).size();
    
    disp_field=M*base_field;
    
    for(k=0;k<Nz;k++)
    {
        if(auto_z)
        {
            z=k*Dz;
            holder.z(N_layers+1,k)=z+layer_sum;
        }
        else z=holder.z(N_layers+1,k)-layer_sum;
        
        Ey=std::exp(z*g_sub*Im)*disp_field(0,0);
        Hx=-g_sup/(mu0*w)*Ey;
        Hz=kp/(mu0*w)*Ey;
        
        holder.E(N_layers+1,k)=std::abs(Ey);
        holder.H(N_layers+1,k)=std::sqrt(std::norm(Hx)+std::norm(Hz));;
        
        holder.Ey(N_layers+1,k)=Ey;
        holder.Hx(N_layers+1,k)=Hx;
        holder.Hz(N_layers+1,k)=Hz;
    }
}

void Multilayer_TMM::compute_mode_TM(Imdouble const &n_eff,MLFieldHolder &holder,bool auto_z)
{
    int k,l;
    
    kp=k0*n_eff;
    
    Imdouble n,g,eps,ch,sh;
    
    double w=m_to_rad_Hz(lambda);
    
    for(l=0;l<N_layers;l++)
    {
        n=index_layer[l];
        eps=n*n;
        g=std::sqrt(eps*k0*k0-kp*kp);
        eps*=e0;
        
        if(g.imag()<0) g=-g;
        
        ch=std::cos(g*h_layer[l]);
        sh=std::sin(g*h_layer[l])*Im;
        
        L_mat_TM[l](0,0)=ch;
        L_mat_TM[l](0,1)=g/(w*eps)*sh;
        L_mat_TM[l](1,0)=w*eps/g*sh;
        L_mat_TM[l](1,1)=ch;
    }
    
    Imdouble k_sub=k0*sub_ind;
    Imdouble k_sup=k0*sup_ind;
    
    Imdouble g_sub=std::sqrt(k_sub*k_sub-kp*kp);
    Imdouble g_sup=std::sqrt(k_sup*k_sup-kp*kp);
    
    if(g_sub.imag()<0) g_sub=-g_sub;
    if(g_sup.imag()<0) g_sup=-g_sup;
    
    holder.set_N_layers(N_layers+2);
    
    Eigen::Matrix<Imdouble,2,1> base_field,disp_field;
    Eigen::Matrix<Imdouble,2,2> M,z_mat;
    
    eps=e0*sup_ind*sup_ind;
    
    base_field(0,0)=-g_sup/(w*eps);
    base_field(1,0)=1.0;
    
    M(0,0)=1.0; M(0,1)=0.0;
    M(1,0)=0.0; M(1,1)=1.0;
    
    int Nz=0;
    double z=0,Dz=0,zmax=0,layer_sum=0;
    Imdouble Ex,Hy,Ez;
    
    // Superstrate
    
    if(auto_z)
    {
        Dz=lambda/std::abs(sup_ind)/100.0;
        zmax=-std::log(1e-3)/std::imag(g_sup);
        zmax=std::min(zmax,5.0*lambda);
        Nz=nearest_integer(zmax/Dz);
        Nz=std::max(2,Nz);
        Dz=zmax/(Nz-1.0);
        
        holder.set_Nz(0,Nz);
    }
    else Nz=holder.get_z(0).size();
    
    for(k=0;k<Nz;k++)
    {
        if(auto_z)
        {
            z=-zmax+k*Dz;
            holder.z(0,k)=z;
        }
        else z=holder.z(0,k);
        
        Hy=std::exp(-z*g_sup*Im);
        Ex=-g_sup/(w*eps)*Hy;
        Ez=-kp/(w*eps)*Hy;
        
        holder.E(0,k)=std::sqrt(std::norm(Ex)+std::norm(Ez));
        holder.H(0,k)=std::abs(Hy);
        
        holder.Ex(0,k)=Ex;
        holder.Hy(0,k)=Hy;
        holder.Ez(0,k)=Ez;
        
    }
    
    // Layers
    
    for(l=0;l<N_layers;l++)
    {
        n=index_layer[l];
        eps=n*n;
        g=std::sqrt(eps*k0*k0-kp*kp);
        eps*=e0;
        
        if(g.imag()<0) g=-g;
        
        if(auto_z)
        {
            Dz=lambda/std::abs(n)/100.0;
            Nz=nearest_integer(h_layer[l]/Dz);
            Nz=std::max(2,Nz);
            Dz=h_layer[l]/(Nz-1.0);
            
            holder.set_Nz(l+1,Nz);
        }
        else Nz=holder.get_z(l+1).size();
        
        for(k=0;k<Nz;k++)
        {
            if(auto_z)
            {
                z=k*Dz;
                holder.z(l+1,k)=z+layer_sum;
            }
            else z=holder.z(l+1,k)-layer_sum;
            
            ch=std::cos(g*z);
            sh=std::sin(g*z)*Im;
            
            z_mat(0,0)=ch;
            z_mat(0,1)=g/(w*eps)*sh;
            z_mat(1,0)=w*eps/g*sh;
            z_mat(1,1)=ch;
            
            disp_field=(z_mat*M)*base_field;
            
            Ex=disp_field(0,0);
            Hy=disp_field(1,0);
            Ez=-kp/(w*eps)*Hy;
            
            holder.E(l+1,k)=std::sqrt(std::norm(Ex)+std::norm(Ez));
            holder.H(l+1,k)=std::abs(Hy);
            
            holder.Ex(l+1,k)=Ex;
            holder.Hy(l+1,k)=Hy;
            holder.Ez(l+1,k)=Ez;
        }
        
        layer_sum+=h_layer[l];
        M=L_mat_TM[l]*M;
    }
    
    // Substrate
    
    if(auto_z)
    {
        Dz=lambda/std::abs(sub_ind)/100.0;
        zmax=-std::log(1e-3)/std::imag(g_sub);
        zmax=std::min(zmax,5.0*lambda);
        Nz=nearest_integer(zmax/Dz);
        Nz=std::max(2,Nz);
        Dz=zmax/(Nz-1.0);
        
        holder.set_Nz(N_layers+1,Nz);
    }
    else Nz=holder.get_z(N_layers+1).size();
    
    g=g_sub;
    eps=e0*sub_ind*sub_ind;
    
    disp_field=M*base_field;
    
    for(k=0;k<Nz;k++)
    {
        if(auto_z)
        {
            z=k*Dz;
            holder.z(N_layers+1,k)=z+layer_sum;
        }
        else z=holder.z(N_layers+1,k)-layer_sum;
        
        Hy=std::exp(z*g_sub*Im)*disp_field(1,0);
        Ex=g_sub/(w*eps)*Hy;
        Ez=-kp/(w*eps)*Hy;
        
        holder.E(N_layers+1,k)=std::sqrt(std::norm(Ex)+std::norm(Ez));
        holder.H(N_layers+1,k)=std::abs(Hy);
        
        holder.Ex(N_layers+1,k)=Ex;
        holder.Hy(N_layers+1,k)=Hy;
        holder.Ez(N_layers+1,k)=Ez;
    }
}

void Multilayer_TMM::compute_power(double &R_TE,double &T_TE,double &A_TE,
                                   double &R_TM,double &T_TM,double &A_TM)
{
    Imdouble r_te=0,
             r_tm=0,
             t_te=0,
             t_tm=0;
    
    compute(r_te,r_tm,t_te,t_tm);
    
    Imdouble k1=k0*sup_ind;
    Imdouble kz1=std::sqrt(k1*k1-kp*kp);
    Imdouble k2=k0*sub_ind;
    Imdouble kz2=std::sqrt(k2*k2-kp*kp);
    
    R_TE=std::norm(r_te);
    R_TM=std::norm(r_tm);
    T_TE=std::norm(t_te)*std::abs(kz2/kz1);
    T_TM=std::norm(t_tm)*std::abs(sup_ind*sup_ind*kz2/(sub_ind*sub_ind*kz1));
    
    A_TE=1.0-R_TE-T_TE;
    A_TM=1.0-R_TM-T_TM;
}

void Multilayer_TMM::compute_TE_dispersion(double lambda_target,Imdouble n_eff_target,
                                           std::vector<double> const &lambda_disp,std::vector<Imdouble> &n_eff,
                                           double span_r,double span_i,double limit,int max_fail)
{
    // Initial refinement
    
    set_lambda_full(lambda_target);
    n_eff_target=nearest_zero_chara_TE_MC(n_eff_target,span_r,span_i,limit,max_fail);
    
    // Nearest point in the spectrum
    
    int k;
    double u;
    
    vector_locate_linear(k,u,lambda_disp,lambda_target);
    
    set_lambda_full(lambda_disp[k]);
    n_eff[k]=nearest_zero_chara_TE_MC(n_eff_target,span_r,span_i,limit,max_fail);
    
    // Bottom part of the spectrum
    
    for(int i=k-1;i>=0;i--) // Not unsigned because of overflow
    {
        set_lambda_full(lambda_disp[i]);
        n_eff[i]=nearest_zero_chara_TE_MC(n_eff[i+1],span_r,span_i,limit,max_fail);
    }
    
    // Top part of the spectrum
    
    for(unsigned int i=k+1;i<lambda_disp.size();i++)
    {
        set_lambda_full(lambda_disp[i]);
        n_eff[i]=nearest_zero_chara_TE_MC(n_eff[i-1],span_r,span_i,limit,max_fail);
    }
}

void Multilayer_TMM::compute_TM_dispersion(double lambda_target,Imdouble n_eff_target,
                                           std::vector<double> const &lambda_disp,std::vector<Imdouble> &n_eff,
                                           double span_r,double span_i,double limit,int max_fail)
{
    // Initial refinement
    
    set_lambda_full(lambda_target);
    n_eff_target=nearest_zero_chara_TM_MC(n_eff_target,span_r,span_i,limit,max_fail);
    
    // Nearest point in the spectrum
    
    int k;
    double u;
    
    vector_locate_linear(k,u,lambda_disp,lambda_target);
    
    set_lambda_full(lambda_disp[k]);
    n_eff[k]=nearest_zero_chara_TM_MC(n_eff_target,span_r,span_i,limit,max_fail);
    
    // Bottom part of the spectrum
    
    for(int i=k-1;i>=0;i--) // Not unsigned because of overflow
    {
        set_lambda_full(lambda_disp[i]);
        n_eff[i]=nearest_zero_chara_TM_MC(n_eff[i+1],span_r,span_i,limit,max_fail);
    }
    
    // Top part of the spectrum
    
    for(unsigned int i=k+1;i<lambda_disp.size();i++)
    {
        set_lambda_full(lambda_disp[i]);
        n_eff[i]=nearest_zero_chara_TM_MC(n_eff[i-1],span_r,span_i,limit,max_fail);
    }
}

Imdouble Multilayer_TMM::nearest_zero_chara_TE_MC(Imdouble const &n_eff_guess,
                                                  double span_r,double span_i,
                                                  double limit,int max_fail)
{
    Imdouble n_best=n_eff_guess,n_try=n_eff_guess;
    double best_val=std::abs(compute_chara_TE(n_best));
    
    int Nfail=0;
    double try_val=1.0;
    
    while(std::sqrt(span_r*span_r+span_i*span_i)>=limit)
    {
        n_try.real(n_best.real()+span_r*randp(-1.0,1.0));
        n_try.imag(n_best.imag()+span_i*randp(-1.0,1.0));
        
        try_val=std::abs(compute_chara_TE(n_try));
        
        if(try_val<best_val) { n_best=n_try; best_val=try_val; }
        else Nfail++;
        
        if(Nfail==max_fail) { span_r/=2.0; span_i/=2.0; Nfail=0; }
    }
    
    return n_best;
}

Imdouble Multilayer_TMM::nearest_zero_chara_TM_MC(Imdouble const &n_eff_guess,
                                                  double span_r,double span_i,
                                                  double limit,int max_fail)
{
    Imdouble n_best=n_eff_guess,n_try=n_eff_guess;
    double best_val=std::abs(compute_chara_TM(n_best));
    
    int Nfail=0;
    double try_val=1.0;
    
    while(std::sqrt(span_r*span_r+span_i*span_i)>=limit)
    {
        n_try.real(n_best.real()+span_r*randp(-1.0,1.0));
        n_try.imag(n_best.imag()+span_i*randp(-1.0,1.0));
        
        try_val=std::abs(compute_chara_TM(n_try));
        
        if(try_val<best_val) { n_best=n_try; best_val=try_val; }
        else Nfail++;
        
        if(Nfail==max_fail) { span_r/=2.0; span_i/=2.0; Nfail=0; }
    }
    
    return n_best;
}

void Multilayer_TMM::operator = (Multilayer_TMM const &ml)
{
    N_layers=ml.N_layers;
    lambda=ml.lambda;
    
    k0=ml.k0;
    angle=ml.angle;
    kp=ml.kp;
    sup_ind=ml.sup_ind;
    sub_ind=ml.sub_ind;
    
    h_layer=ml.h_layer;
    index_layer=ml.index_layer;
    
    L_mat_TE.resize(N_layers);
    L_mat_TM.resize(N_layers);
}

void Multilayer_TMM::recompute_L()
{
    int l;
    
    Imdouble kn1,kn2,b1,b2,n1,n2,eps1,eps2,g1,g2;
    
    for(l=0;l<N_layers;l++)
    {
        if(l==N_layers-1)
        {
            n1=index_layer[l];
            n2=sub_ind;
        }
        else
        {
            n1=index_layer[l];
            n2=index_layer[l+1];
        }
        
        eps1=n1*n1;
        eps2=n2*n2;
        
        kn1=k0*n1;
        kn2=k0*n2;
        
        b1=std::sqrt(kn1*kn1-kp*kp);
        b2=std::sqrt(kn2*kn2-kp*kp);
        
        g1=b1/(k0*eps1);
        g2=b2/(k0*eps2);
        
        Imdouble exp_p=std::exp(+h_layer[l]*b1*Im);
        Imdouble exp_m=std::exp(-h_layer[l]*b1*Im);
        
        L_mat_TE[l](0,0)=0.5*(1.0+b1/b2)*exp_p;
        L_mat_TE[l](0,1)=0.5*(1.0-b1/b2)*exp_m;
        L_mat_TE[l](1,0)=0.5*(1.0-b1/b2)*exp_p;
        L_mat_TE[l](1,1)=0.5*(1.0+b1/b2)*exp_m;
        
        L_mat_TM[l](0,0)=0.5*(1.0+g1/g2)*exp_p;
        L_mat_TM[l](0,1)=0.5*(1.0-g1/g2)*exp_m;
        L_mat_TM[l](1,0)=0.5*(1.0-g1/g2)*exp_p;
        L_mat_TM[l](1,1)=0.5*(1.0+g1/g2)*exp_m;
    }
}

void Multilayer_TMM::set_angle(Angle const &angle_)
{
    angle=angle_;
}

void Multilayer_TMM::set_environment(Imdouble sup_ind_,Imdouble sub_ind_)
{
    sup_ind=sup_ind_;
    sub_ind=sub_ind_;
}

void Multilayer_TMM::set_environment(Material const &sup_mat_,Material const &sub_mat_)
{
    sup_mat=sup_mat_;
    sub_mat=sub_mat_;
}

void Multilayer_TMM::set_lambda(double lambda_)
{
    lambda=lambda_;
    k0=2.0*Pi/lambda;
}

void Multilayer_TMM::set_lambda_full(double lambda_)
{
    lambda=lambda_;
    k0=2.0*Pi/lambda;
    
    set_lambda(lambda);
    double w=m_to_rad_Hz(lambda);
    
    set_environment(sup_mat.get_n(w),sub_mat.get_n(w));
    
    for(int j=0;j<N_layers;j++)
        set_layer(j,h_layer[j],material_layer[j].get_n(w));
}

void Multilayer_TMM::set_layer(int l,double h,Imdouble n)
{
    h_layer[l]=h;
    index_layer[l]=n;
}

void Multilayer_TMM::set_layer(int l,double h,Material const &mat)
{
    h_layer[l]=h;
    material_layer[l]=mat;
}

void Multilayer_TMM::set_N_layers(int N_layers_)
{
    N_layers=N_layers_;
    
    h_layer.resize(N_layers);
    index_layer.resize(N_layers);
    material_layer.resize(N_layers);
    
    for(int l=0;l<N_layers;l++)
    {
        h_layer[l]=0;
        index_layer[l]=1.0;
        material_layer[l].set_const_n(1.0);
    }
    
    L_mat_TE.resize(N_layers);
    L_mat_TM.resize(N_layers);
}
