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

#include <phys_tools.h>
#include <sensors.h>

extern const Imdouble Im;
extern std::ofstream plog;

CompletionSensor::CompletionSensor(double coeff_)
    :FT_mode(false),
     coeff(coeff_), energy_last(0), energy_max(0)
{
}

CompletionSensor::CompletionSensor(double lambda_min_,double lambda_max_,double coeff_,int Np_,std::string const &layout_)
    :FT_mode(true),
     Np(Np_),
     lambda_min(lambda_min_), lambda_max(lambda_max_),
     coeff(coeff_),
     layout(layout_),
     i_loc(Np), j_loc(Np), k_loc(Np),
     lambda_loc(Np), w_loc(Np),
     Ex_loc(Np,0), Ey_loc(Np,0), Ez_loc(Np,0)
{
}

bool CompletionSensor::completion_check()
{
    if(step<2*var_max(Nx,Ny,Nz)) return false;
    int i,j,k;
    
    if(FT_mode)
    {
        double Ex_avg_r=0,
               Ey_avg_r=0,
               Ez_avg_r=0,
               Ex_avg_i=0,
               Ey_avg_i=0,
               Ez_avg_i=0;
        
        double avg_max=0;
        
        Grid1<bool> cpl_arr(Np,false);
        Grid2<double> *t_grid=nullptr;
        
        double max_rat=0;
        
        for(i=0;i<Np;i++)
        {
            for(j=0;j<N_avg;j++)
            {
                Ex_avg_r+=Ex_loc_r(i,j);
                Ey_avg_r+=Ey_loc_r(i,j);
                Ez_avg_r+=Ez_loc_r(i,j);
                
                Ex_avg_i+=Ex_loc_i(i,j);
                Ey_avg_i+=Ey_loc_i(i,j);
                Ez_avg_i+=Ez_loc_i(i,j);
            }
            
            Ex_avg_r=std::abs(Ex_avg_r/(N_avg+0.0));
            Ey_avg_r=std::abs(Ey_avg_r/(N_avg+0.0));
            Ez_avg_r=std::abs(Ez_avg_r/(N_avg+0.0));
            
            Ex_avg_i=std::abs(Ex_avg_i/(N_avg+0.0));
            Ey_avg_i=std::abs(Ey_avg_i/(N_avg+0.0));
            Ez_avg_i=std::abs(Ez_avg_i/(N_avg+0.0));
            
            avg_max=var_max(Ex_avg_r,Ey_avg_r,Ez_avg_r,
                          Ex_avg_i,Ey_avg_i,Ez_avg_i);
            
                 if(avg_max==Ex_avg_r){ t_grid=&Ex_loc_r; }
            else if(avg_max==Ey_avg_r){ t_grid=&Ey_loc_r; }
            else if(avg_max==Ez_avg_r){ t_grid=&Ez_loc_r; }
            else if(avg_max==Ex_avg_i){ t_grid=&Ex_loc_i; }
            else if(avg_max==Ey_avg_i){ t_grid=&Ey_loc_i; }
            else if(avg_max==Ez_avg_i){ t_grid=&Ez_loc_i; }
            
            double t_min=(*t_grid)(i,0);
            double t_max=(*t_grid)(i,0);
            
            for(k=0;k<N_avg;k++)
            {
                t_min=std::min(t_min,(*t_grid)(i,k));
                t_max=std::max(t_max,(*t_grid)(i,k));
            }
            
            double span=std::abs(t_max-t_min);
            
            max_rat=std::max(max_rat,span/avg_max);
            
            if(span<=avg_max*coeff) cpl_arr[i]=true;
        }
        
        int N_ok=0;
        
        for(i=0;i<Np;i++)
            if(cpl_arr[i]) N_ok++;
        
        double ok_rat=N_ok/(Np+0.0);
        
        if(N_ok>0)
        {
            est_step.push_back(step);
            est_ratio.push_back(1.0-ok_rat);
        }
        
        if(ok_rat>=0.9) return true;
        else return false;
    }
    else
    {
        est_step.push_back(step);
        est_ratio.push_back(energy_last/energy_max);
        
        if(energy_last/energy_max<coeff) return true;
        else return false;
    }
    
    return true;
}

void CompletionSensor::deep_feed(FDTD const &fdtd)
{
    if(FT_mode)
    {
        int i;
        
        int offset=step%N_avg;
                
        for(i=0;i<Np;i++)
        {
            int &i_=i_loc[i],
                &j_=j_loc[i],
                &k_=k_loc[i];
            
            Imdouble coeff=std::exp(w_loc[i]*step*Dt*Im);
            
            Ex_loc[i]+=fdtd.local_Ex(i_,j_,k_)*coeff;
            Ey_loc[i]+=fdtd.local_Ey(i_,j_,k_)*coeff;
            Ez_loc[i]+=fdtd.local_Ez(i_,j_,k_)*coeff;
            
            Ex_loc_r(i,offset)=Ex_loc[i].real();
            Ey_loc_r(i,offset)=Ey_loc[i].real();
            Ez_loc_r(i,offset)=Ez_loc[i].real();
            
            Ex_loc_i(i,offset)=Ex_loc[i].imag();
            Ey_loc_i(i,offset)=Ey_loc[i].imag();
            Ez_loc_i(i,offset)=Ez_loc[i].imag();
        }
    }
    else
    {
        int i,j,k;
        
        energy_last=0;
        double Ex,Ey,Ez;
        
        for(k=0;k<Nz;k++) for(j=0;j<Ny;j++) for(i=0;i<Nx;i++)  
        {
            Ex=fdtd.Ex(i,j,k);
            Ey=fdtd.Ey(i,j,k);
            Ez=fdtd.Ez(i,j,k);
            
            energy_last+=Ex*Ex+Ey*Ey+Ez*Ez;
        }
        
        energy_max=std::max(energy_max,energy_last);
    }
}

int CompletionSensor::estimate()
{
    int i;
    int Nt_est=Nt;
    
    int N=est_ratio.size();
    
    chk_msg_sc(N);
    
    if(N>2)
    {
        std::vector<double> x(N,0),y(N,0),w(N,0);
        
        for(i=0;i<N;i++)
        {
            x[i]=est_step[i];
//            y[i]=std::log(1e-4+est_ratio[i]);
            y[i]=std::log(est_ratio[i]);
            w[i]=i/(N-1.0);
        }
        
        double at,bt;
        
//        lin_reg(x,y,at,bt);
        lin_reg_weight(x,y,w,at,bt);
        
        double a=std::exp(bt);
        double b=-at;
        
//        Nt_est=static_cast<int>(-std::log(coeff/a)/b);
        double target=0.1;
        if(!FT_mode) target=coeff;

        double Nt_est_d=-std::log(target/a)/b;
        
        if(Nt_est_d>=1e9 || b<0) Nt_est=1e9;
        else Nt_est=static_cast<int>(Nt_est_d);
        
        chk_msg_sc(a);
        chk_msg_sc(b);
        chk_msg_sc(Nt_est);
    }
    
    if(Nt_est<0  || step<2*var_max(Nx,Ny,Nz)) return Nt;
    
    return Nt_est;
}

void CompletionSensor::initialize()
{
    if(FT_mode)
    {
        int i,j,k;
        
        bool known_layout=false;
        std::string tmp_layout("nnn");
        
        std::vector<char> combi(4);
        
        combi[0]='n';
        combi[1]='d';
        combi[2]='u';
        combi[3]='b';
        
        chk_var(layout);
        
        for(i=0;i<4;i++) for(j=0;j<4;j++) for(k=0;k<4;k++)
        {
            tmp_layout[0]=combi[i];
            tmp_layout[1]=combi[j];
            tmp_layout[2]=combi[k];
            
            if(layout==tmp_layout) known_layout=true;
        }
            
        if(layout=="v") known_layout=true;
        
        if(layout=="nnn" || known_layout==false) layout="nnb";
        
        if(layout=="v")
        {
            for(i=0;i<Np;i++)
            {
                i_loc[i]=static_cast<int>(randp(xs_s,xs_e-1)+0.5);
                j_loc[i]=static_cast<int>(randp(ys_s,ys_e-1)+0.5);
                k_loc[i]=static_cast<int>(randp(zs_s,zs_e-1)+0.5);
            }
        }
        else
        {
            double tot_area=0;
            int Np_xd=0,Np_xu=0;
            int Np_yd=0,Np_yu=0;
            int Np_zd=0,Np_zu=0;
            
            if(layout[0]=='d') Np_xd=Ny*Nz;
            if(layout[0]=='u') Np_xu=Ny*Nz;
            if(layout[0]=='b') Np_xd=Np_xu=Ny*Nz;
            
            if(layout[1]=='d') Np_yd=Nx*Nz;
            if(layout[1]=='u') Np_yu=Nx*Nz;
            if(layout[1]=='b') Np_yd=Np_yu=Nx*Nz;
            
            if(layout[2]=='d') Np_zd=Nx*Ny;
            if(layout[2]=='u') Np_zu=Nx*Ny;
            if(layout[2]=='b') Np_zd=Np_zu=Nx*Ny;
            
            tot_area=Np_xd+Np_xu+Np_yd+Np_yu+Np_zd+Np_zu;
            
            int N_alloc=0,N_left=Np;
            
            N_alloc=static_cast<int>(Np*Np_xd/tot_area+0.5);
            N_alloc=(N_alloc<N_left ? N_alloc : N_left); N_left-=N_alloc;
            Np_xd=N_alloc;
            
            N_alloc=static_cast<int>(Np*Np_xu/tot_area+0.5);
            N_alloc=(N_alloc<N_left ? N_alloc : N_left); N_left-=N_alloc;
            Np_xu=N_alloc;
            
            N_alloc=static_cast<int>(Np*Np_yd/tot_area+0.5);
            N_alloc=(N_alloc<N_left ? N_alloc : N_left); N_left-=N_alloc;
            Np_yd=N_alloc;
            
            N_alloc=static_cast<int>(Np*Np_yu/tot_area+0.5);
            N_alloc=(N_alloc<N_left ? N_alloc : N_left); N_left-=N_alloc;
            Np_yu=N_alloc;
            
            N_alloc=static_cast<int>(Np*Np_zd/tot_area+0.5);
            N_alloc=(N_alloc<N_left ? N_alloc : N_left); N_left-=N_alloc;
            Np_zd=N_alloc;
            
            N_alloc=static_cast<int>(Np*Np_zu/tot_area+0.5);
            N_alloc=(N_alloc<N_left ? N_alloc : N_left); N_left-=N_alloc;
            Np_zu=N_alloc;
            
            chk_var(Np_xd);
            chk_var(Np_xu);
            chk_var(Np_yd);
            chk_var(Np_yu);
            chk_var(Np_zd);
            chk_var(Np_zu);
            
            int offset=0;
            
            for(i=offset;i<offset+Np_xd;i++)
            {
                i_loc[i]=xs_s;
                j_loc[i]=static_cast<int>(randp(ys_s,ys_e-1)+0.5);
                k_loc[i]=static_cast<int>(randp(zs_s,zs_e-1)+0.5);
            }
            offset+=Np_xd;
            
            for(i=offset;i<offset+Np_xu;i++)
            {
                i_loc[i]=xs_e-1;
                j_loc[i]=static_cast<int>(randp(ys_s,ys_e-1)+0.5);
                k_loc[i]=static_cast<int>(randp(zs_s,zs_e-1)+0.5);
            }
            offset+=Np_xu;
            
            for(i=offset;i<offset+Np_yd;i++)
            {
                i_loc[i]=static_cast<int>(randp(xs_s,xs_e-1)+0.5);
                j_loc[i]=ys_s;
                k_loc[i]=static_cast<int>(randp(zs_s,zs_e-1)+0.5);
            }
            offset+=Np_yd;
            
            for(i=offset;i<offset+Np_yu;i++)
            {
                i_loc[i]=static_cast<int>(randp(xs_s,xs_e-1)+0.5);
                j_loc[i]=ys_e-1;
                k_loc[i]=static_cast<int>(randp(zs_s,zs_e-1)+0.5);
            }
            offset+=Np_yu;
            
            for(i=offset;i<offset+Np_zd;i++)
            {
                i_loc[i]=static_cast<int>(randp(xs_s,xs_e-1)+0.5);
                j_loc[i]=static_cast<int>(randp(ys_s,ys_e-1)+0.5);
                k_loc[i]=zs_s;
            }
            offset+=Np_zd;
            
            for(i=offset;i<offset+Np_zu;i++)
            {
                i_loc[i]=static_cast<int>(randp(xs_s,xs_e-1)+0.5);
                j_loc[i]=static_cast<int>(randp(ys_s,ys_e-1)+0.5);
                k_loc[i]=zs_e-1;
            }
            offset+=Np_zu;
        }
        
        for(i=0;i<Np;i++)
        {
            lambda_loc[i]=randp(lambda_min,lambda_max);
            w_loc[i]=m_to_rad_Hz(lambda_loc[i]);
        }
        
        double T=std::max(lambda_min,lambda_max)/c_light;
        N_avg=static_cast<int>(T/Dt);
        
        Ex_loc_r.init(Np,N_avg,0);
        Ey_loc_r.init(Np,N_avg,0);
        Ez_loc_r.init(Np,N_avg,0);
        
        Ex_loc_i.init(Np,N_avg,0);
        Ey_loc_i.init(Np,N_avg,0);
        Ez_loc_i.init(Np,N_avg,0);
    }
}
