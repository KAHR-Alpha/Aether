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


extern const Imdouble Im;
extern std::ofstream plog;

//#####################
//   Multilayer_TMM_UD
//#####################

Multilayer_TMM_UD::Multilayer_TMM_UD()
    :N_layers(0), lambda(500e-9), angle(0), kp(0), sup_ind(1.0), sub_ind(1.0)
{
}

Multilayer_TMM_UD::Multilayer_TMM_UD(int N_layers_)
    :N_layers(N_layers_), lambda(500e-9), angle(0), kp(0), sup_ind(1.0), sub_ind(1.0)
{
    h_layer.resize(N_layers);
    index_layer.resize(N_layers);
    
    for(int l=0;l<N_layers;l++)
    {
        h_layer[l]=0;
        index_layer[l]=1.0;
    }
    
    L_mat_TE.resize(N_layers);
    L_mat_TM.resize(N_layers);
}

Multilayer_TMM_UD::Multilayer_TMM_UD(Multilayer_TMM_UD const &ml)
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

void Multilayer_TMM_UD::compute(Imdouble &r_TE,Imdouble &r_TM,
                                Imdouble &t_TE,Imdouble &t_TM)
{
    k0=2.0*Pi/lambda;
    
    if(angle>=Pi/2.0)
    {
        r_TE=r_TM=1.0;
        t_TE=t_TM=0;
        return;
    }
    
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

void Multilayer_TMM_UD::compute_power(double &R_TE,double &T_TE,double &A_TE,
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
    
    if(std::isnan(R_TE))
        plog<<r_te<<std::endl;
    
    if(std::imag(kz2)!=0)
    {
        T_TE=T_TM=0;
    }
    else
    {
        T_TE=std::norm(t_te)*std::abs(kz2/kz1);
        T_TM=std::norm(t_tm)*std::abs(sup_ind*sup_ind*kz2/(sub_ind*sub_ind*kz1));
    }
        
    A_TE=1.0-R_TE-T_TE;
    A_TM=1.0-R_TM-T_TM;
}

void Multilayer_TMM_UD::operator =  (Multilayer_TMM_UD const &ml)
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

void Multilayer_TMM_UD::recompute_L()
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

void Multilayer_TMM_UD::set_angle(AngleRad const &angle_)
{
    angle=angle_;
}

void Multilayer_TMM_UD::set_environment(Imdouble sup_ind_,Imdouble sub_ind_)
{
    sup_ind=sup_ind_;
    sub_ind=sub_ind_;
}

void Multilayer_TMM_UD::set_lambda(double lambda_)
{
    lambda=lambda_;
    k0=2.0*Pi/lambda;
}

void Multilayer_TMM_UD::set_layer(int l,double h,Imdouble n)
{
    h_layer[l]=h;
    index_layer[l]=n;
}

void Multilayer_TMM_UD::set_N_layers(int N_layers_)
{
    N_layers=N_layers_;
    
    h_layer.resize(N_layers);
    index_layer.resize(N_layers);
    
    for(int l=0;l<N_layers;l++)
    {
        h_layer[l]=0;
        index_layer[l]=1.0;
    }
    
    L_mat_TE.resize(N_layers);
    L_mat_TM.resize(N_layers);
}
