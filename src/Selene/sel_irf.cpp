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

#include <selene.h>

extern std::ofstream plog;

namespace Sel
{

//####################
//      IRF
//####################

IRF::IRF()
    :type(IRF_Type::NONE),
     scatt_ref(0),
     splitting_factor(0),
     Nl(0), Nth(0)
{
}

IRF::IRF(IRF const &irf)
    :type(irf.type),
     name(irf.name),
     ref_fname(irf.ref_fname),
     tra_fname(irf.tra_fname),
     scatt_ref(irf.scatt_ref),
     scatt_A(irf.scatt_A),
     scatt_B(irf.scatt_B),
     scatt_g(irf.scatt_g),
     grat_Nth(irf.grat_Nth),
     grat_Nphi(irf.grat_Nphi),
     grat_No(irf.grat_No),
     grat_No_r(irf.grat_No_r),
     grat_No_t(irf.grat_No_t),
     grat_ref_path(irf.grat_ref_path),
     grat_tra_path(irf.grat_tra_path),
     ml_model(irf.ml_model),
     ml_heights(irf.ml_heights),
     ml_materials(irf.ml_materials),
     splitting_factor(irf.splitting_factor),
     Nl(irf.Nl), Nth(irf.Nth)
{
    grat_th_angle=irf.grat_th_angle;
    grat_phi_angle=irf.grat_phi_angle;
    grat_orders_prob=irf.grat_orders_prob;
    grat_orders_dir=irf.grat_orders_dir;
    
    lambda_data=irf.lambda_data;
    ang_th_data=irf.ang_th_data;
    ref_data=irf.ref_data;
    ref_scatt_data=irf.ref_scatt_data;
    tra_data=irf.tra_data;
    tra_scatt_data=irf.tra_scatt_data;
    
    g2=irf.g2;
    g2_ref=irf.g2_ref;
    g2_tra=irf.g2_tra;
    
    g3=irf.g3;
    g3_ref=irf.g3_ref;
    g3_tra=irf.g3_tra;
}

void IRF::bootstrap()
{
    if(type==IRF_Type::MULTILAYER)
        ml_model.set_N_layers(ml_heights.size());
}

void IRF::compute_snell_reflection(Vector3 &out_dir,Vector3 const &in_dir,
                                   Vector3 const &Fnorm,double n_scal)
{
    out_dir=in_dir-2.0*Fnorm*n_scal;
    out_dir.normalize();
}

void IRF::compute_snell_refration(Vector3 &out_dir,Vector3 const &in_dir,
                                  Vector3 const &Fnorm,double n_scal,
                                  bool is_near_normal,double lambda,double n1,double n2)
{
    Vector3 T=in_dir-Fnorm*n_scal;
        
    if(is_near_normal)
    {
        Vector3 Tb;
        Tb.rand_sph();
        
        T.crossprod(Tb,in_dir);
    }
    
    T.normalize();
    
    double t_scal=scalar_prod(in_dir,T);
    
    double k1=2.0*Pi*n1/lambda; double k1_2=k1*k1;
    double k2_2=2.0*Pi*n2/lambda; k2_2*=k2_2;
    
    if(n_scal>0) out_dir=std::sqrt(k2_2-k1_2*t_scal*t_scal)*Fnorm+k1*t_scal*T;
    else out_dir=-std::sqrt(k2_2-k1_2*t_scal*t_scal)*Fnorm+k1*t_scal*T;
}

bool IRF::determine_polarization(Vector3 &S_vec,Vector3 const &Fnorm,
                                 Vector3 const &in_dir,Vector3 const &in_polar,
                                 bool is_near_normal)
{
    S_vec=crossprod(in_dir,Fnorm);
    
    if(is_near_normal)
    {
        S_vec=in_polar;
        S_vec=S_vec-Fnorm*scalar_prod(Fnorm,S_vec);
        S_vec.normalize();
    }
    
    double cos2_TE=scalar_prod(in_polar,S_vec);
    cos2_TE*=cos2_TE;
    
    if(randp()<=cos2_TE) return true;
    else return false;
}

bool IRF::get_response(Vector3 &out_dir,Vector3 &out_polar,
                       Vector3 const &in_dir,Vector3 const &in_polar,
                       Vector3 const &Fnorm,Vector3 const &Ftangent,
                       double lambda,double n1,double n2)
{
    double n_scal=scalar_prod(in_dir,Fnorm);
    bool is_near_normal=near_normal(n_scal);
    
    bool ray_abs=false;
    
    // Probabilistic polarization
    
    Vector3 S_vec;
    bool is_TE=determine_polarization(S_vec,Fnorm,in_dir,in_polar,is_near_normal);
    
         if(type==IRF_Type::FRESNEL) ray_abs=get_response_fresnel(out_dir,in_dir,Fnorm,n_scal,lambda,n1,n2,is_TE,is_near_normal);
    else if(type==IRF_Type::MULTILAYER) ray_abs=get_response_multilayer(out_dir,in_dir,Fnorm,n_scal,lambda,n1,n2,is_TE,is_near_normal);
    else if(type==IRF_Type::PERF_ABS)
    {
        return true;
    }
    else if(type==IRF_Type::PERF_ANTIREF) ray_abs=get_response_perf_antiref(in_dir,out_dir,Fnorm,lambda,n1,n2);
    else if(type==IRF_Type::PERF_MIRROR) compute_snell_reflection(out_dir,in_dir,Fnorm,n_scal);
    else if(type==IRF_Type::SCATT_ABS)
    {
        double p=randp();
        
        if(p<=scatt_ref)
        {
            Vector3 V;
            V.rand_sph();
            
            if(sgn(scalar_prod(V,Fnorm))==sgn(scalar_prod(in_dir,Fnorm))) V=-V;
            
            out_dir=V;
        }
        else return true;
    }
    else if(type==IRF_Type::GRATING) ray_abs=get_response_grating(in_dir,out_dir,Fnorm,Ftangent,lambda,n1,n2);
    else if(type==IRF_Type::SNELL_SCATT_FILE) ray_abs=get_response_snell_scatt_file(in_dir,out_dir,Fnorm,lambda,n1,n2);
    else if(type==IRF_Type::SNELL_FILE) ray_abs=get_response_snell_file(in_dir,out_dir,Fnorm,lambda,n1,n2);
    else if(type==IRF_Type::SNELL_SPLITTER) ray_abs=get_response_snell_splitter(in_dir,out_dir,Fnorm,lambda,n1,n2);
    
    out_dir.normalize();
    
    if(   type==IRF_Type::FRESNEL
       || type==IRF_Type::MULTILAYER
       || type==IRF_Type::PERF_ANTIREF
       || type==IRF_Type::PERF_MIRROR
       || type==IRF_Type::SNELL_SPLITTER)
    {
        if(is_TE) out_polar=S_vec;
        else out_polar=crossprod(out_dir,S_vec);
    }
    
    return ray_abs;
}

bool IRF::get_response_fresnel(Vector3 &out_dir,Vector3 const &in_dir,
                               Vector3 const &Fnorm,double n_scal,
                               double lambda,double n1,double n2,
                               bool is_TE,bool is_near_normal)
{
    double cos_thi=std::abs(n_scal);
    
    double thi=std::acos(cos_thi);
    double thr=std::asin(n1/n2*sin(thi));
    double cos_thr=std::cos(thr);
    
    bool total_ref=false;
    if(n1/n2*std::sin(thi)>=1.0) total_ref=true;
    
    // Powers
    
    double rte=(n1*cos_thi-n2*cos_thr)/(n1*cos_thi+n2*cos_thr);
    double rtm=(n1*cos_thr-n2*cos_thi)/(n1*cos_thr+n2*cos_thi);
    
//    double r=0.5*(rte*rte+rtm*rtm);
    double r=rtm*rtm;
    if(is_TE) r=rte*rte;
    
    if(total_ref) r=1;
    
    // Child
    
    double p=randp();
    
    if(p<=r) compute_snell_reflection(out_dir,in_dir,Fnorm,n_scal);
    else compute_snell_refration(out_dir,in_dir,Fnorm,n_scal,is_near_normal,lambda,n1,n2);
    
    return false;
}

bool IRF::get_response_grating(Vector3 const &in_dir,Vector3 &out_dir,
                               Vector3 const &Fnorm,Vector3 const &Ftangent,double lambda,double n1,double n2)
{
    int i;
    double n_scal=scalar_prod(in_dir,Fnorm);
    
    // Local frame
    
    Vector3 n=Fnorm;
    if(n_scal>=0) n=-Fnorm;
    
    Vector3 t=Ftangent;
    Vector3 r=crossprod(n,t);
    
    double cos_th=std::abs(n_scal);
    double th=std::acos(cos_th);
    
    double x,y;
    //double z;
    x=scalar_prod(in_dir,t);
    y=scalar_prod(in_dir,r);
    
    double phi=std::atan2(y,x);
//    std::cout<<" "<<std::endl;
//    chk_var(in_dir);
//    chk_var(Fnorm);
//    chk_var(n);
//    chk_var(t);
//    chk_var(r);
//    chk_var(th*180/Pi);
//    chk_var(phi*180/Pi);
    
    // Interpolation index
    
    int ind_th=0,ind_phi=0;
    double u=0,v=0;
    
    vector_locate_linear(ind_th,u,grat_th_angle,th);
    vector_locate_linear(ind_phi,v,grat_phi_angle,phi);
    
    double cu=1.0-u;
    double cv=1.0-v;
        
    double p=randp(1.0);
//    chk_var(p);
//    chk_var(u);
//    chk_var(v);
//    chk_var(ind_th);
//    chk_var(ind_phi);
//    chk_var(grat_orders_prob(ind_th  ,ind_phi  ,0));
//    p=0.7;
//    
//    std::cout<<std::endl;
//    for(int i=0;i<grat_No;i++)
//    {
//        std::cout<<grat_orders_prob(ind_th  ,ind_phi  ,i)<<" "<<grat_orders_dir(ind_th  ,ind_phi  ,i)<<std::endl;
//    }
    
    // Absorption
    
    double abs_prob=+cu*cv*grat_orders_prob(ind_th  ,ind_phi  ,0)
                    + u*cv*grat_orders_prob(ind_th+1,ind_phi  ,0)
                    +  u*v*grat_orders_prob(ind_th+1,ind_phi+1,0)
                    + cu*v*grat_orders_prob(ind_th  ,ind_phi+1,0);
    
    if(p<abs_prob) return true;
    
    // Orders
    
//    static int plop=0;
//    
//    if(plop==1) std::exit(0);
//    plop++;
    
    for(i=1;i<grat_No;i++)
    {
        double proba_interp_1=+cu*cv*grat_orders_prob(ind_th  ,ind_phi  ,i-1)
                              + u*cv*grat_orders_prob(ind_th+1,ind_phi  ,i-1)
                              +  u*v*grat_orders_prob(ind_th+1,ind_phi+1,i-1)
                              + cu*v*grat_orders_prob(ind_th  ,ind_phi+1,i-1);
                              
        double proba_interp_2=+cu*cv*grat_orders_prob(ind_th  ,ind_phi  ,i)
                              + u*cv*grat_orders_prob(ind_th+1,ind_phi  ,i)
                              +  u*v*grat_orders_prob(ind_th+1,ind_phi+1,i)
                              + cu*v*grat_orders_prob(ind_th  ,ind_phi+1,i);
        
        if(proba_interp_1<=p && p<proba_interp_2)
        {
            Vector3 V=+cu*cv*grat_orders_dir(ind_th  ,ind_phi  ,i)
                      + u*cv*grat_orders_dir(ind_th+1,ind_phi  ,i)
                      +  u*v*grat_orders_dir(ind_th+1,ind_phi+1,i)
                      + cu*v*grat_orders_dir(ind_th  ,ind_phi+1,i);
            
            V.z=1.0-V.norm_sqr();
            
//            chk_var(V.z);
            if(V.z<0) return true;
            
            V.z=sqrt(V.z);
            
            if(i>=grat_No_r+1) V.z=-V.z;
            
            out_dir=V.x*t+V.y*r+V.z*n;
            
//            chk_var(V);
//            chk_var(p);
//            chk_var(proba_interp_1);
//            chk_var(proba_interp_2);
//            std::cin.get();
                        
            return false;
        }
    }
    
    out_dir=in_dir;
    
    return false;
}

bool IRF::get_response_multilayer(Vector3 &out_dir,Vector3 const &in_dir,
                                  Vector3 const &Fnorm,double n_scal,
                                  double lambda,double n1,double n2,
                                  bool is_TE,bool is_near_normal)
{
    // Multilayer setup
    
    ml_model.set_lambda(lambda);
    ml_model.set_environment(n1,n2);
    
    std::size_t Nl=ml_heights.size();
    
    if(n_scal<=0)
    {
        for(std::size_t i=0;i<Nl;i++)
            ml_model.set_layer(i,ml_heights[i],ml_materials[i]->get_n(m_to_rad_Hz(lambda)));
    }
    else
    {
        for(std::size_t i=0;i<Nl;i++)
            ml_model.set_layer(Nl-1-i,ml_heights[i],ml_materials[i]->get_n(m_to_rad_Hz(lambda)));
    }
    
    double cos_thi=std::abs(n_scal);
    double thi=std::acos(cos_thi);
    
    ml_model.set_angle(thi);
    
    // Powers
    
    double R_TE,R_TM,T_TE,T_TM,A_TE,A_TM;
    ml_model.compute_power(R_TE,T_TE,A_TE,R_TM,T_TM,A_TM);
    
//    double R=0.5*(R_TE+R_TM);
//    double T=0.5*(T_TE+T_TM);
    
    double R=R_TM;
    double T=T_TM;
    
    if(is_TE)
    {
        R=R_TE;
        T=T_TE;
    }
    
    // Child
        
    double p=randp();
    
         if(p<=R) compute_snell_reflection(out_dir,in_dir,Fnorm,n_scal);
    else if(p<=R+T) compute_snell_refration(out_dir,in_dir,Fnorm,n_scal,is_near_normal,lambda,n1,n2);
    else return true;
    
    return false;
}

bool IRF::get_response_perf_antiref(Vector3 const &in_dir,Vector3 &out_dir,
                                    Vector3 const &Fnorm,double lambda,double n1,double n2)
{
    double n_scal=scalar_prod(in_dir,Fnorm);
    
    double cos_thi=std::abs(n_scal);
        
    double thi=std::acos(cos_thi);
    
    bool total_ref=false;
    if(n1/n2*std::sin(thi)>=1.0) total_ref=true;
    
    if(total_ref)
    {
        out_dir=in_dir-2.0*Fnorm*n_scal;
        out_dir.normalize();
    }
    else
    {
        Vector3 T=in_dir-Fnorm*n_scal;
        
        if(near_normal(n_scal))
        {
            Vector3 Tb;
            Tb.rand_sph();
            
            T.crossprod(Tb,in_dir);
        }
        
        T.normalize();
        
        double t_scal=scalar_prod(in_dir,T);
        
        double k1=2.0*Pi*n1/lambda; double k1_2=k1*k1;
        double k2_2=2.0*Pi*n2/lambda; k2_2*=k2_2;
        
        if(n_scal>0) out_dir=std::sqrt(k2_2-k1_2*t_scal*t_scal)*Fnorm+k1*t_scal*T;
        else out_dir=-std::sqrt(k2_2-k1_2*t_scal*t_scal)*Fnorm+k1*t_scal*T;
    }
    
    return false;
}

bool IRF::get_response_snell_scatt_file(Vector3 const &in_dir,Vector3 &out_dir,
                                           Vector3 const &Fnorm,double lambda,double n1,double n2)
{
    double n_scal=scalar_prod(in_dir,Fnorm);
    
    double cos_thi=std::abs(n_scal);
    
    double thi=std::acos(cos_thi);
    //double thr=std::asin(n1/n2*sin(thi));
    //double cos_thr=std::cos(thr);
    
    int k=0;
    double u=0;
    
    ang_th_data.vector_locate_linear(thi,k,u);
    
    double r=ref_data.lin_interp(k,u);
    double r_sc=ref_scatt_data.lin_interp(k,u);
    double t=tra_data.lin_interp(k,u);
    double t_sc=tra_scatt_data.lin_interp(k,u);
    
    double p=randp();
        
    if(p<=r)
    {
        out_dir=in_dir-2.0*Fnorm*n_scal;
        out_dir.normalize();
    }
    else if(p<=(r+r_sc))
    {
        Vector3 V;
        V.rand_sph();
        
        if(sgn(scalar_prod(V,Fnorm))==sgn(scalar_prod(in_dir,Fnorm))) V=-V;
        
        out_dir=V;
    }
    else if(p<=(r+r_sc+t))
    {
        Vector3 T=in_dir-Fnorm*n_scal;
        
        if(near_normal(n_scal))
        {
            Vector3 Tb;
            Tb.rand_sph();
            
            T.crossprod(Tb,in_dir);
        }
        
        T.normalize();
        
        double t_scal=scalar_prod(in_dir,T);
        
        double k1=2.0*Pi*n1/lambda; double k1_2=k1*k1;
        double k2_2=2.0*Pi*n2/lambda; k2_2*=k2_2;
        
        if(n_scal>0) out_dir=std::sqrt(k2_2-k1_2*t_scal*t_scal)*Fnorm+k1*t_scal*T;
        else out_dir=-std::sqrt(k2_2-k1_2*t_scal*t_scal)*Fnorm+k1*t_scal*T;
    }
    else if(p<=(r+r_sc+t+t_sc))
    {
        Vector3 V;
        V.rand_sph();
        
        if(sgn(scalar_prod(V,Fnorm))!=sgn(scalar_prod(in_dir,Fnorm))) V=-V;
        
        out_dir=V;
    }
    else
    {
        out_dir=in_dir;
        return true;
    }
    
    return false;
}

bool IRF::get_response_snell_file(Vector3 const &in_dir,Vector3 &out_dir,
                                     Vector3 const &Fnorm,double lambda,double n1,double n2)
{
    double n_scal=scalar_prod(in_dir,Fnorm);
    
    double cos_thi=std::abs(n_scal);
    
    double thi=std::acos(cos_thi);
    //double thr=std::asin(n1/n2*sin(thi));
    //double cos_thr=std::cos(thr);
    
    int k=0;
    double u=0;
    
    ang_th_data.vector_locate_linear(thi,k,u);
    
    double r=ref_data.lin_interp(k,u);
    double t=tra_data.lin_interp(k,u);
    
    double p=randp();
    
    if(p<=r)
    {
        out_dir=in_dir-2.0*Fnorm*n_scal;
        out_dir.normalize();
    }
    else if(p<=(r+t))
    {
        Vector3 T=in_dir-Fnorm*n_scal;
        
        if(near_normal(n_scal))
        {
            Vector3 Tb;
            Tb.rand_sph();
            
            T.crossprod(Tb,in_dir);
        }
        
        T.normalize();
        
        double t_scal=scalar_prod(in_dir,T);
        
        double k1=2.0*Pi*n1/lambda; double k1_2=k1*k1;
        double k2_2=2.0*Pi*n2/lambda; k2_2*=k2_2;
        
        if(n_scal>0) out_dir=std::sqrt(k2_2-k1_2*t_scal*t_scal)*Fnorm+k1*t_scal*T;
        else out_dir=-std::sqrt(k2_2-k1_2*t_scal*t_scal)*Fnorm+k1*t_scal*T;
    }
    else
    {
        out_dir=in_dir;
        return true;
    }
    
    return false;
}

bool IRF::get_response_snell_splitter(Vector3 const &in_dir,Vector3 &out_dir,
                                      Vector3 const &Fnorm,double lambda,double n1,double n2)
{
    double n_scal=scalar_prod(in_dir,Fnorm);
    
    double p=randp();
    
    if(p<=splitting_factor)
    {
        out_dir=in_dir-2.0*Fnorm*n_scal;
        out_dir.normalize();
    }
    else
    {
        Vector3 T=in_dir-Fnorm*n_scal;
        
        if(near_normal(n_scal))
        {
            Vector3 Tb;
            Tb.rand_sph();
            
            T.crossprod(Tb,in_dir);
        }
        
        T.normalize();
        
        double t_scal=scalar_prod(in_dir,T);
        
        double k1=2.0*Pi*n1/lambda; double k1_2=k1*k1;
        double k2_2=2.0*Pi*n2/lambda; k2_2*=k2_2;
        
        if(n_scal>0) out_dir=std::sqrt(k2_2-k1_2*t_scal*t_scal)*Fnorm+k1*t_scal*T;
        else out_dir=-std::sqrt(k2_2-k1_2*t_scal*t_scal)*Fnorm+k1*t_scal*T;
    }
    
    return false;
}

bool IRF::near_normal(double n_scal)
{
    return 1.0-std::abs(n_scal)<1e-10;
}

void IRF::operator = (IRF const &irf)
{
    type=irf.type;
    
    name=irf.name;
    
    ref_fname=irf.ref_fname;
    tra_fname=irf.tra_fname;
    
    scatt_ref=irf.scatt_ref;
    
    scatt_A=irf.scatt_A;
    scatt_B=irf.scatt_B;
    scatt_g=irf.scatt_g;
    
    grat_Nth=irf.grat_Nth;
    grat_Nphi=irf.grat_Nphi;
    grat_No=irf.grat_No;
    grat_No_r=irf.grat_No_r;
    grat_No_t=irf.grat_No_t;
    
    grat_ref_path=irf.grat_ref_path;
    grat_tra_path=irf.grat_tra_path;
        
    grat_th_angle=irf.grat_th_angle;
    grat_phi_angle=irf.grat_phi_angle;
    grat_orders_prob=irf.grat_orders_prob;
    grat_orders_dir=irf.grat_orders_dir;
    
    ml_model=irf.ml_model;
    ml_heights=irf.ml_heights;
    ml_materials=irf.ml_materials;
    
    splitting_factor=irf.splitting_factor;
    
    Nl=irf.Nl;
    Nth=irf.Nth;
    
    lambda_data=irf.lambda_data;
    ang_th_data=irf.ang_th_data;
    ref_data=irf.ref_data;
    ref_scatt_data=irf.ref_scatt_data;
    tra_data=irf.tra_data;
    tra_scatt_data=irf.tra_scatt_data;
    
    g2=irf.g2;
    g2_ref=irf.g2_ref;
    g2_tra=irf.g2_tra;
    
    g3=irf.g3;
    g3_ref=irf.g3_ref;
    g3_tra=irf.g3_tra;
}

void IRF::set_type(IRF_Type type_)
{
    type=type_;
}

void IRF::set_type_grating(std::string ref_fname,std::string tra_fname)
{
    int l,p,q;
    
    type=IRF_Type::GRATING;
    
    grat_ref_path=ref_fname;
    grat_tra_path=tra_fname;
    
    std::ifstream file_ref(ref_fname,std::ios::in|std::ios::binary);
    std::ifstream file_tra(tra_fname,std::ios::in|std::ios::binary);
    
    if(!file_ref.is_open())
    {
        std::cout<<"Couldn't open "<<ref_fname<<std::endl;
        std::exit(EXIT_FAILURE);
    }
    if(!file_tra.is_open())
    {
        std::cout<<"Couldn't open "<<tra_fname<<std::endl;
        std::exit(EXIT_FAILURE);
    }
    
    std::string buffer_string;
    std::vector<std::string> string_data;
    
    // Lambda
    std::getline(file_ref,buffer_string,'\n');
    
    // Theta
    std::getline(file_ref,buffer_string,'\n');
    split_string(string_data,buffer_string,' ');
    
    grat_Nth=string_data.size();
    grat_th_angle.resize(grat_Nth);
    for(int i=0;i<grat_Nth;i++) grat_th_angle[i]=Degree(std::stod(string_data[i]));
    
    // Phi
    std::getline(file_ref,buffer_string,'\n');
    split_string(string_data,buffer_string,' ');
    
    grat_Nphi=string_data.size();
    grat_phi_angle.resize(grat_Nphi);
    for(int i=0;i<grat_Nphi;i++) grat_phi_angle[i]=Degree(std::stod(string_data[i]));
    
    // Shifting transmission file get pointer
    
    std::getline(file_tra,buffer_string,'\n');
    std::getline(file_tra,buffer_string,'\n');
    std::getline(file_tra,buffer_string,'\n');
    
    // Orders
    
    int pmin_r,pmax_r,qmin_r,qmax_r;
    int pmin_t,pmax_t,qmin_t,qmax_t;
    
    file_ref>>pmin_r; file_ref>>pmax_r;
    file_ref>>qmin_r; file_ref>>qmax_r;
    
    file_tra>>pmin_t; file_tra>>pmax_t;
    file_tra>>qmin_t; file_tra>>qmax_t;
            
    int Np_r=pmax_r-pmin_r+1;
    int Np_t=pmax_t-pmin_t+1;
    
    grat_No_r=(pmax_r-pmin_r+1)*(qmax_r-qmin_r+1);
    grat_No_t=(pmax_t-pmin_t+1)*(qmax_t-qmin_t+1);
    
    grat_No=grat_No_r+grat_No_t+1;
    
    grat_orders_prob.init(grat_Nth,grat_Nphi,grat_No,0);
    grat_orders_dir.init(grat_Nth,grat_Nphi,grat_No,0);
    
    // Processing
    
    int Nl=grat_Nth*grat_Nphi;
    
    double val,x,y;
    double z;
    std::string buf,th_str,phi_str;
    int a_th,a_phi;
    
    for(l=0;l<Nl;l++)
    {
        file_ref>>buf;
        file_ref>>th_str;
        file_ref>>phi_str;
        
        double th=Degree(std::stod(th_str));
        double phi=Degree(std::stod(phi_str));
        
        a_th=vector_locate(grat_th_angle,th);
        a_phi=vector_locate(grat_phi_angle,phi);
        
        for(int k=0;k<grat_No_r;k++)
        {
            file_ref>>p;
            file_ref>>q;
            
            file_ref>>val;
            file_ref>>x;
            file_ref>>y;
            
            int t=1+p-pmin_r+(q-qmin_r)*Np_r;
            
            grat_orders_prob(a_th,a_phi,t)=val;
            grat_orders_dir(a_th,a_phi,t)=Vector3(x,y,0);
        }
    }
    
    for(l=0;l<Nl;l++)
    {
        file_tra>>buf;
        file_tra>>th_str;
        file_tra>>phi_str;
        
        double th=Degree(std::stod(th_str));
        double phi=Degree(std::stod(phi_str));
        
        a_th=vector_locate(grat_th_angle,th);
        a_phi=vector_locate(grat_phi_angle,phi);
        
        for(int k=0;k<grat_No_t;k++)
        {
            file_tra>>p;
            file_tra>>q;
            
            file_tra>>val;
            file_tra>>x;
            file_tra>>y;
            
            int t=1+grat_No_r+p-pmin_t+(q-qmin_t)*Np_t;
            
//            if(phi==0 && th==0)
//            {
//                std::cout<<"A_"<<val<<" "<<x<<" "<<y<<std::endl;
//            }
            
            grat_orders_prob(a_th,a_phi,t)=val;
            grat_orders_dir(a_th,a_phi,t)=Vector3(x,y,0);
        }
    }
    
    // Absorption
    
    for(a_th=0;a_th<grat_Nth;a_th++){ for(a_phi=0;a_phi<grat_Nphi;a_phi++)
    {
        double S=0;
        
        for(p=1;p<grat_No;p++) S+=grat_orders_prob(a_th,a_phi,p);
        
        grat_orders_prob(a_th,a_phi,0)=1.0-S;
        
        for(p=1;p<grat_No;p++) grat_orders_prob(a_th,a_phi,p)+=grat_orders_prob(a_th,a_phi,p-1);
    }}
}

void IRF::set_type_fresnel()
{
    type=IRF_Type::FRESNEL;
}

void IRF::set_type_fresnel_ABg(double A,double B,double g)
{
    type=IRF_Type::FRESNEL_ABG;
    
    scatt_A=A;
    scatt_B=B;
    scatt_g=g;
}

void IRF::set_type_multilayer()
{
    type=IRF_Type::MULTILAYER;
}

void IRF::set_type_scatt_abs(double ref)
{
    type=IRF_Type::SCATT_ABS;
    scatt_ref=ref;
}

void get_simp_pq(std::string fname,int &pmin,int &pmax,int &qmin,int &qmax)
{
    std::ifstream file(fname,std::ios::in|std::ios::binary);
    
    std::string buf;
    
    file>>buf;
    file>>pmin;
    file>>pmax;
    file>>qmin;
    file>>qmax;
}

void IRF::set_type_snell_file(std::string fname)
{
    int i;
    
    double tmp;
    
    type=IRF_Type::SNELL_FILE;
    
    int Nc=fcountlines(fname);
    
    chk_msg_sc(Nc);
    
    ang_th_data.init(Nc,0);
    ref_data.init(Nc,0);
    tra_data.init(Nc,0);
    
    std::ifstream file(fname,std::ios::in);
    
    for(i=0;i<Nc;i++)
    {
        file>>tmp;
        file>>ang_th_data[i];
        file>>tmp;
        file>>ref_data[i];
        file>>tra_data[i];
        file>>tmp;
        
        ang_th_data[i]*=Pi/180.0;
    }
    
    file.close();
}

void IRF::set_type_snell_scatt_file(std::string fname)
{
    int i;
    
    type=IRF_Type::SNELL_SCATT_FILE;
    
    std::vector<std::vector<double>> data;
    
    ascii_data_loader(fname,data);
    
    int Nc=data[0].size();
    
    chk_msg_sc(Nc);
    
    ang_th_data.init(Nc,0);
    ref_data.init(Nc,0);
    ref_scatt_data.init(Nc,0);
    tra_data.init(Nc,0);
    tra_scatt_data.init(Nc,0);
    
    for(i=0;i<Nc;i++)
    {
        ang_th_data[i]=data[0][i];
        ref_data[i]=data[1][i];
        ref_scatt_data[i]=data[2][i];
        tra_data[i]=data[3][i];
        tra_scatt_data[i]=data[4][i];
        
        ang_th_data[i]*=Pi/180.0;
    }
}

void IRF::set_type_snell_splitter(double splitting_factor_)
{
    splitting_factor=splitting_factor_;
    type=IRF_Type::SNELL_SPLITTER;
}

}
