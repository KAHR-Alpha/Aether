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


extern const Imdouble Im;

//################
//   Multilayer
//################

Multilayer::Multilayer(int N_layers_)
    :N_layers(N_layers_), lambda(500e-9), kp(0), angle(0), sup_ind(1.0), sub_ind(1.0)
{
    h_layer.resize(N_layers);
    index_layer.resize(N_layers);
    
    for(int l=0;l<N_layers;l++)
    {
        h_layer[l]=0;
        index_layer[l]=1.0;
    }
    
    F_mat_TE.resize(N_layers+1);
    F_mat_TM.resize(N_layers+1);
    
    G_mat_TE.resize(N_layers+1);
    G_mat_TM.resize(N_layers+1);
    W_mat.resize(N_layers);
    
    
}

void Multilayer::compute(Imdouble &r_TE,Imdouble &r_TM,
                         Imdouble &t_TE,Imdouble &t_TM)
{
    k0=2.0*Pi/lambda;
    kp=sup_ind*k0*std::sin(angle);
    
    recompute_W();
    recompute_FG();
    
    Eigen::Matrix<Imdouble,2,2> M;
    
    M.setIdentity();
    
    for(int l=0;l<N_layers;l++)
        M=W_mat[l]*G_mat_TE[l].inverse()*F_mat_TE[l]*M;
    
    M=G_mat_TE[N_layers].inverse()*F_mat_TE[N_layers]*M;
    
    r_TE=-M(1,0)/M(1,1);
    t_TE=M(0,0)+M(0,1)*r_TE;
    
    M.setIdentity();
    
    for(int l=0;l<N_layers;l++)
        M=W_mat[l]*G_mat_TM[l].inverse()*F_mat_TM[l]*M;
    
    M=G_mat_TM[N_layers].inverse()*F_mat_TM[N_layers]*M;
    
    r_TM=-M(1,0)/M(1,1);
    t_TM=M(0,0)+M(0,1)*r_TM;
}

void Multilayer::compute_power(double &R_TE,double &T_TE,double &A_TE,
                               double &R_TM,double &T_TM,double &A_TM)
{
    Imdouble r_te=0,
             r_tm=0,
             t_te=0,
             t_tm=0;
    
    compute(r_te,r_tm,t_te,t_tm);
    
    double k1=k0*sup_ind;
    double kz1=std::sqrt(k1*k1-kp*kp);
    Imdouble k2=k0*sub_ind;
    Imdouble kz2=std::sqrt(k2*k2-kp*kp);
    
    R_TE=std::norm(r_te);
    R_TM=std::norm(r_tm);
    T_TE=std::norm(t_te)*std::abs(kz2/kz1);
    T_TM=std::norm(t_tm)*std::abs(sup_ind*sup_ind*kz2/(sub_ind*sub_ind*kz1));
    
    A_TE=1.0-R_TE-T_TE;
    A_TM=1.0-R_TM-T_TM;
}

void Multilayer::recompute_FG()
{
    int l;
    
    Imdouble n1,n2;
    
    for(l=0;l<=N_layers;l++)
    {
             if(l==0)
        {
            n1=sup_ind;
            n2=index_layer[0];
        }
        else if(l==N_layers)
        {
            n1=index_layer[N_layers-1];
            n2=sub_ind;
        }
        else
        {
            n1=index_layer[l-1];
            n2=index_layer[l];
        }
        
        Imdouble rte=0,
                 rtm=0,
                 tte=0,
                 ttm=0;
        
        fresnel_rt_te_tm_wn(lambda,n1,n2,kp,rte,rtm,tte,ttm);
        
        F_mat_TE[l](0,0)=+tte; F_mat_TE[l](0,1)=0;
        F_mat_TE[l](1,0)=-rte; F_mat_TE[l](1,1)=1;
        
        F_mat_TM[l](0,0)=+ttm; F_mat_TM[l](0,1)=0;
        F_mat_TM[l](1,0)=-rtm; F_mat_TM[l](1,1)=1;
        
        std::swap(n1,n2);
        
        fresnel_rt_te_tm_wn(lambda,n1,n2,kp,rte,rtm,tte,ttm);
        
        G_mat_TE[l](0,0)=1; G_mat_TE[l](0,1)=-rte;
        G_mat_TE[l](1,0)=0; G_mat_TE[l](1,1)=+tte;
        
        G_mat_TM[l](0,0)=1; G_mat_TM[l](0,1)=-rtm;
        G_mat_TM[l](1,0)=0; G_mat_TM[l](1,1)=+ttm;
    }
}

void Multilayer::recompute_W()
{
    int l;
    
    for(l=0;l<N_layers;l++)
    {
        Imdouble kn=k0*index_layer[l];
        Imdouble beta=std::sqrt(kn*kn-kp*kp);
        
        W_mat[l](0,0)=std::exp(+h_layer[l]*beta*Im); W_mat[l](0,1)=0;
        W_mat[l](1,0)=0; W_mat[l](1,1)=std::exp(-h_layer[l]*beta*Im);
    }
}

void Multilayer::set_angle(AngleOld const &angle_)
{
    angle=angle_;
}

void Multilayer::set_environment(double sup_ind_,Imdouble sub_ind_)
{
    sup_ind=sup_ind_;
    sub_ind=sub_ind_;
}

void Multilayer::set_lambda(double lambda_)
{
    lambda=lambda_;
}

void Multilayer::set_layer(int l,double h,Imdouble n)
{
    h_layer[l]=h;
    index_layer[l]=n;
}
