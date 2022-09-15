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

#include <index_utils.h>
#include <berreman_strat.h>

extern const double Pi;
extern const Imdouble Im;

extern std::ofstream plog;

//####################
//    Iso_index
//####################

void iso_index::eval(double lambda,
                     double h,
                     double hd,
                     double r,
                     double rD,
                     double t,
                     double tD,
                     B_strat &strat1,
                     B_strat &strat2)
{
    double r_TE,r_TM,t_TE,t_TM;
    double r_TED,r_TMD,t_TED,t_TMD;
    
    Imdouble eps=eps_r+eps_i*Im;
    Imdouble mu=mu_r+mu_i*Im;
    
    strat1.set_iso(0,h,eps,mu);
    strat2.set_iso(0,h,eps,mu);
    
    strat1.compute_abs(lambda,0,r_TE,r_TM,t_TE,t_TM);
    strat2.compute_abs(lambda,0,r_TED,r_TMD,t_TED,t_TMD);
    
    using std::abs;
    
    R=abs(r-r_TE)+abs(t-t_TE)+abs(rD-r_TED)+abs(tD-t_TED);
//    R=abs(r-r_TE)+abs(t-t_TE);
    
    Imdouble n=std::sqrt(eps*mu);
    if(std::imag(n)<0) R+=1000.0;
    //if(mu_i>0) R+=1000;
}

void iso_index::evalC(double lambda,
                      double h,
                      Imdouble r,
                      Imdouble t,
                      B_strat &strat1)
{
    Imdouble r_TE,r_TM,t_TE,t_TM;
    
    Imdouble eps=eps_r+eps_i*Im;
    Imdouble mu=mu_r+mu_i*Im;
    
    strat1.set_iso(0,h,eps,mu);
    
    strat1.compute(lambda,0,r_TE,r_TM,t_TE,t_TM);
    
    using std::abs;
    
    //R=(abs(r-r_TE)+abs(t-t_TE))*(1.0+1.0*abs(mu_r-1.0))*(1.0+10.0*abs(mu_i));
    R=(abs(r-r_TE)+abs(t-t_TE));
    
    double m1=std::abs(mu_r-1.0);
    double m12=m1*m1;
    double m16=m12*m12*m12;
    
    double m2=std::abs(mu_i);
    double m22=m2*m2;
    double m26=m22*m22*m22;
    
    R*=1.0+m1*(1.0-std::exp(-0.1*m16));
    R*=1.0+m2*(1.0-std::exp(-0.1*m26));
        
    Imdouble n=std::sqrt(eps*mu);
    if(std::imag(n)<0) R+=10000;
}

//void iso_index::evalC(double lambda,
//                      double h,
//                      Imdouble r,
//                      Imdouble t,
//                      B_strat &strat1)
//{
//    Imdouble r_TE,r_TM,t_TE,t_TM;
//    
//    mu_r=1.0;
//    mu_i=0.0;
//    
//    Imdouble eps=eps_r+eps_i*Im;
//    Imdouble mu=mu_r+mu_i*Im;
//    
//    strat1.set_iso(0,h,eps,mu);
//    
//    strat1.compute(lambda,0,r_TE,r_TM,t_TE,t_TM);
//    
//    using std::abs;
//    
//    //R=(abs(r-r_TE)+abs(t-t_TE))*(1.0+1.0*abs(mu_r-1.0))*(1.0+10.0*abs(mu_i));
//    R=(abs(r-r_TE)+abs(t-t_TE));
//        
//    Imdouble n=std::sqrt(eps*1.0);
//    if(std::imag(n)<0) R+=10000;
////    if(mu_i<0) R+=10000;
//}

void iso_index::mutate(iso_index const &ind)
{
    double c1,c2,c3,c4;
    
    c1=c2=c3=c4=0;
    c1=randp(-1,1);
    c2=randp(-1,1);
    c3=randp(-1,1);
    c4=randp(-1,1);
    
    int a=std::rand()%4;
    
    if(a==0)
    {
        eps_r=ind.eps_r+c1;
        eps_i=ind.eps_i+c2;
        
        mu_r=ind.mu_r+c3;
        mu_i=ind.mu_i+c4;
    }
    if(a==1)
    {
        eps_r=ind.eps_r+0.1*c1;
        eps_i=ind.eps_i+0.1*c2;
        
        mu_r=ind.mu_r+0.1*c3;
        mu_i=ind.mu_i+0.1*c4;
    }
    if(a==2)
    {
        eps_r=(1.0+c1/10.0)*ind.eps_r;
        eps_i=(1.0+c2/10.0)*ind.eps_i;
        
        mu_r=(1.0+c3/10.0)*ind.mu_r;
        mu_i=(1.0+c4/10.0)*ind.mu_i;
    }
    if(a==3)
    {
        eps_r=(1.0+c1/5.0)*ind.eps_r;
        eps_i=(1.0+c2/5.0)*ind.eps_i;
        
        mu_r=(1.0+c3/5.0)*ind.mu_r;
        mu_i=(1.0+c4/5.0)*ind.mu_i;
    }
    
//    mu_r=1.0;
//    mu_i=0;
}

void iso_index::randgen()
{
    eps_r=randp(-10.0,10.0);
    eps_i=randp(-10.0,10.0);
    mu_r=randp(-10.0,10.0);
    mu_i=randp(-10.0,10.0);
}

void iso_index::show()
{
    Imdouble n=std::sqrt((eps_r+eps_i*Im)*(mu_r+mu_i*Im));
    std::cout<<eps_r<<" "
                <<eps_i<<" "
                <<mu_r<<" "
                <<mu_i<<" "
                <<std::real(n)<<" "
                <<std::imag(n)<<" / "
                <<R<<" "<<std::endl;
}

void iso_index::operator = (double a)
{
    randgen();
}

void iso_index::operator = (iso_index const& ind)
{
    eps_r=ind.eps_r;
    eps_i=ind.eps_i;
    mu_r=ind.mu_r;
    mu_i=ind.mu_i;
    R=ind.R;
}

//##############################################################################
//##############################################################################

void Swap(iso_index &a,iso_index &b)
{
    iso_index tmp;
    
    tmp=b;
    b=a;
    a=tmp;
}

void Sort(Grid1<iso_index> &ind,int Nind)
{
    int i,j;
    
    for(i=0;i<Nind;i++)
    {
        for(j=i+1;j<Nind;j++)
        {
            if(ind[j].R<ind[i].R) Swap(ind[j],ind[i]);
        }
    }
}

void normD_index_calc_octave_script(std::string prefix)
{
    std::stringstream strm1,strm2;
    strm1<<prefix<<"show_index.m";
    strm2<<prefix<<"index_calc";
    
    std::ofstream file(strm1.str().c_str(),std::ios::out|std::ios::trunc);
    
    using std::endl;
    
    file<<"clear *"<<endl;
    file<<"close all"<<endl;
    file<<endl;
    file<<"a=load(\""<<strm2.str()<<"\");"<<endl;
    file<<endl;
    file<<"L=a(:,1);"<<endl;
    file<<"eps_r=a(:,2);"<<endl;
    file<<"eps_i=a(:,3);"<<endl;
    file<<"eps=eps_r+i*eps_i;"<<endl;
    file<<"mu_r=a(:,4);"<<endl;
    file<<"mu_i=a(:,5);"<<endl;
    file<<"mu=mu_r+i*mu_i;"<<endl;
    file<<endl;
    file<<"n=sqrt(eps.*mu);"<<endl;
    file<<"n_r=real(n);"<<endl;
    file<<"n_i=imag(n);"<<endl;
    file<<endl;
    file<<"plot(L,n_r,L,n_i,L,mu_r,L,mu_i)"<<endl;
}

void normC_index_calc(int Nref,double n1,double n3,double h,std::string polar_mode,std::string prefix)
{
    int i,j,k;
        
    std::srand(std::time(0));
    
    B_strat strat1(1,n1,n3);
        
    double lambda=0.3e-6;
    
    if(Nref<200) Nref=200;
    
    int Nind=400;
    int Npro=20;
    
    Grid1<iso_index> ind(Nind,iso_index());
    
    std::stringstream strm;
    strm<<prefix<<"spectdata_norm2";
    
    std::ifstream data(strm.str().c_str(),std::ios::in|std::ios::binary);
    
    std::stringstream strm2;
    strm2<<prefix<<"index_calc";
    
    std::ofstream result(strm2.str().c_str(),std::ios::out|std::ios::binary|std::ios::trunc);
    
    Imdouble r,t;
    double r_abs,r_arg,t_abs,t_arg;
    double tmp;
    
    std::ofstream cfile("tmpr/err",std::ios::out|std::ios::trunc);
    
    for(i=0;i<Nref;i++)
    {
        data>>lambda;
        
        if(polar_mode=="TM"){ data>>r_abs; data>>r_arg; }
        else{ data>>tmp; data>>tmp; }
        
        if(polar_mode=="TE"){ data>>r_abs; data>>r_arg; }
        else{ data>>tmp; data>>tmp; }
        
        data>>tmp; data>>tmp;
        
        if(polar_mode=="TM"){ data>>t_abs; data>>t_arg; }
        else{ data>>tmp; data>>tmp; }
        
        if(polar_mode=="TE"){ data>>t_abs; data>>t_arg; }
        else{ data>>tmp; data>>tmp; }
        
        data>>tmp; data>>tmp;
        
        r=r_abs*std::exp(r_arg*Im);
        t=t_abs*std::exp(t_arg*Im);
        
        if(i==0)
        {
            for(j=0;j<Nind;j++)
            {
                ind[j].randgen();
                ind[j].evalC(lambda,h,r,t,strat1);
            }
        }
        else
        {
            for(j=1;j<Nind;j++) ind[j].mutate(ind[0]);
            for(j=0;j<Nind;j++)
            {
                ind[j].evalC(lambda,h,r,t,strat1);
            }
        }
        Sort(ind,Nind);
        
        int count=0;
                
        double lim=0.001;
        
        while(ind[0].R>lim)
        {
            for(j=Npro;j<2*Npro;j++)
            {
                ind[j].randgen();
                ind[j].evalC(lambda,h,r,t,strat1);
            }
            for(j=2*Npro;j<Nind;j++)
            {
                k=std::rand()%Npro;
                ind[j].mutate(ind[k]);
                ind[j].evalC(lambda,h,r,t,strat1);
            }
            Sort(ind,Nind);
            
            count++;
            
            if(count>1000)
            {
                for(j=0;j<Nind;j++)
                {
                    ind[j].randgen();
                    ind[j].evalC(lambda,h,r,t,strat1);
                }
                Sort(ind,Nind);
                count=0;
                lim*=2;
            }
            
        }
        
        std::cout<<lambda<<" ";
        ind[0].show();
        
        result<<lambda<<" "
                <<ind[0].eps_r<<" "
                <<ind[0].eps_i<<" "
                <<ind[0].mu_r<<" "
                <<ind[0].mu_i<<" "
                <<ind[0].R<<" "<<std::endl;
    }
    
    data.close();
    normD_index_calc_octave_script(prefix);
}

void normC_index_calc_var(int Nl,double n1,double n3,double h,std::string polar_mode,std::string prefix)
{
    int i,j,l;
        
    std::srand(std::time(0));
    
    B_strat strat1(1,n1,n3);
        
    iso_index test_ind;
    
    std::stringstream strm;
    strm<<prefix<<"spectdata_norm2";
    
    Nl=fcountlines(strm.str());
    
    std::ifstream data(strm.str().c_str(),std::ios::in|std::ios::binary);
    
    std::stringstream strm2;
    strm2<<prefix<<"index_calc";
    
    std::ofstream result(strm2.str().c_str(),std::ios::out|std::ios::binary|std::ios::trunc);
    
    double r_abs,r_arg,t_abs,t_arg;
    double tmp;
        
    bool computation_done=0;
    
    Grid1<double> lambda(Nl,0);
    Grid1<Imdouble> r_data(Nl,0);
    Grid1<Imdouble> t_data(Nl,0);
    Grid1<double> eps_r_f(Nl,0),eps_i_f(Nl,0);
    Grid1<double> mu_r_f(Nl,0),mu_i_f(Nl,0);
    Grid1<double> obj_f(Nl,0);
    Grid1<bool> p_comp(Nl,0);
    Grid1<int> p_count(Nl,0);
    Grid1<double> t_avg(Nl,0);
    
    int max_comp=15;
    
    for(l=0;l<Nl;l++)
    {
        data>>lambda[l];
        
        if(polar_mode=="TM"){ data>>r_abs; data>>r_arg; }
        else{ data>>tmp; data>>tmp; }
        
        if(polar_mode=="TE"){ data>>r_abs; data>>r_arg; }
        else{ data>>tmp; data>>tmp; }
        
        data>>tmp; data>>tmp;
        
        if(polar_mode=="TM"){ data>>t_abs; data>>t_arg; }
        else{ data>>tmp; data>>tmp; }
        
        if(polar_mode=="TE"){ data>>t_abs; data>>t_arg; }
        else{ data>>tmp; data>>tmp; }
        
        data>>tmp; data>>tmp;
        
        r_data[l]=r_abs*std::exp(r_arg*Im);
        t_data[l]=t_abs*std::exp(t_arg*Im);
        
        obj_f[l]=1000000.0;
        p_comp[l]=true;
        p_count[l]=0;
    }
    
    t_avg=1.0;
    
    while(!computation_done)
    {
        for(l=0;l<Nl;l++) if(p_comp[l])
        {
            double eps_r_max=300;
            double eps_r_min=-300;
            double mu_r_max=20;
            double mu_r_min=-20;
            
            double eps_i_max=300;
            double eps_i_min=-300;
            double mu_i_max=20;
            double mu_i_min=-20;
                        
            double eps_r,eps_i,mu_r,mu_i,obj;
            double b_eps_r=0,b_eps_i=0,b_mu_r=0,b_mu_i=0,b_obj=100000;
            
            double tmp1;
            
            if(l>0)
            {
                if(obj_f[l-1]<obj_f[l])
                {
                    using std::abs;
                    
                    eps_r_max=eps_r_f[l-1]+0.5*abs(eps_r_f[l-1]);
                    eps_r_min=eps_r_f[l-1]-0.5*abs(eps_r_f[l-1]);
                    eps_i_max=eps_i_f[l-1]+0.5*abs(eps_i_f[l-1]);
                    eps_i_min=eps_i_f[l-1]-0.5*abs(eps_i_f[l-1]);
                    
                    mu_r_max=mu_r_f[l-1]+0.5*abs(mu_r_f[l-1]);
                    mu_r_min=mu_r_f[l-1]-0.5*abs(mu_r_f[l-1]);
                    mu_i_max=mu_i_f[l-1]+0.5*abs(mu_i_f[l-1]);
                    mu_i_min=mu_i_f[l-1]-0.5*abs(mu_i_f[l-1]);
                    
                    eps_r_max+=1.0;
                    eps_r_min-=1.0;
                    mu_r_max+=1.0;
                    mu_r_min-=1.0;
                    
                    eps_i_max+=1.0;
                    eps_i_min-=1.0;
                    mu_i_max+=1.0;
                    mu_i_min-=1.0;
                }
            }
            
            i=0;
            
            while(i<=15)
            {
                bool c=1;
                j=0;
                
                while(c)
                {
                    eps_r=randp(eps_r_min,eps_r_max);
                    eps_i=randp(eps_i_min,eps_i_max);
                    mu_r=randp(mu_r_min,mu_r_max);
                    mu_i=randp(mu_i_min,mu_i_max);
                                        
                    test_ind.eps_r=eps_r;
                    test_ind.eps_i=eps_i;
                    test_ind.mu_r=mu_r;
                    test_ind.mu_i=mu_i;
                    
                    test_ind.evalC(lambda[l],h,r_data[l],t_data[l],strat1);
                    obj=test_ind.R;
                    
                    if(obj<b_obj)
                    {
                        b_obj=obj;
                        b_eps_r=eps_r;
                        b_eps_i=eps_i;
                        b_mu_r=mu_r;
                        b_mu_i=mu_i;
                        
                        j=0;
                    }
                    else j++;
                    
                    if(j>=1000) c=0;
                }
                
                tmp1=(eps_r_max-eps_r_min)/4.0;
                eps_r_min=b_eps_r-tmp1;
                eps_r_max=b_eps_r+tmp1;
                
                tmp1=(eps_i_max-eps_i_min)/4.0;
                eps_i_min=b_eps_i-tmp1;
                eps_i_max=b_eps_i+tmp1;
                
                tmp1=(mu_r_max-mu_r_min)/4.0;
                mu_r_min=b_mu_r-tmp1;
                mu_r_max=b_mu_r+tmp1;
                
                tmp1=(mu_i_max-mu_i_min)/4.0;
                mu_i_min=b_mu_i-tmp1;
                mu_i_max=b_mu_i+tmp1;
                
                i++;
                
                if(b_obj>=0.1 && i==15)
                {
                    eps_r_max=300;
                    eps_r_min=-300;
                    mu_r_max=20;
                    mu_r_min=-20;
                    
                    eps_i_max=300;
                    eps_i_min=-300;
                    mu_i_max=20;
                    mu_i_min=-20;
                    
                    b_obj=100000;
                    
                    i=0;
                }
            }
            
            if(b_obj<obj_f[l])
            {
                eps_r_f[l]=b_eps_r;
                eps_i_f[l]=b_eps_i;
                mu_r_f[l]=b_mu_r;
                mu_i_f[l]=b_mu_i;
                obj_f[l]=b_obj;
                
                std::cout<<lambda[l]<<" "<<p_count[l]<<" "
                        <<eps_r_f[l]<<" "
                        <<eps_i_f[l]<<" "
                        <<mu_r_f[l]<<" "
                        <<mu_i_f[l]<<" "
                        <<obj_f[l]<<std::endl;
            }
            
            p_count[l]+=1;
        }
        
        //double obj_disp=obj_f.std_dev();
        double obj_mean=0;        
        
        int N_avg=static_cast<int>(Nl*0.1);
        
        for(l=0;l<Nl;l++)
        {
            using std::min;
            using std::max;
            
            j=0;
            t_avg[l]=0;
            
            for(i=max(0,l-N_avg);i<min(Nl,l+N_avg);i++)
            {
                t_avg[l]+=obj_f[i];
                j++;
            }
            
            t_avg[l]/=j;
            
            obj_mean+=std::log10(obj_f[l]);
        }
        
        obj_mean/=Nl;
        
        for(l=0;l<Nl;l++)
        {
            if(obj_f[l]>=2.0*t_avg[l]) p_comp[l]=1;
            else p_comp[l]=0;
            if(p_count[l]>max_comp) p_comp[l]=0;
        }
        
        std::cout<<std::endl<<obj_mean<<std::endl<<std::endl;
        
        computation_done=1;
        for(l=0;l<Nl;l++) if(p_comp[l]) computation_done=0;
    }
    
    for(l=1;l<Nl-1;l++)
    {
        if(obj_f[l]>5.0*(obj_f[l+1]+obj_f[l-1]))
        {
            double x=(lambda[l]-lambda[l-1])/(lambda[l+1]-lambda[l-1]);
            
            eps_r_f[l]=(1.0-x)*eps_r_f[l-1]+x*eps_r_f[l+1];
            eps_i_f[l]=(1.0-x)*eps_i_f[l-1]+x*eps_i_f[l+1];
            mu_r_f[l]=(1.0-x)*mu_r_f[l-1]+x*mu_r_f[l+1];
            mu_i_f[l]=(1.0-x)*mu_i_f[l-1]+x*mu_i_f[l+1];
            
            test_ind.eps_r=eps_r_f[l];
            test_ind.eps_i=eps_i_f[l];
            test_ind.mu_r=mu_r_f[l];
            test_ind.mu_i=mu_i_f[l];
            
            test_ind.evalC(lambda[l],h,r_data[l],t_data[l],strat1);
            
            obj_f[l]=test_ind.R;
        }
    }
    
    for(l=0;l<Nl;l++)
    {
        result<<lambda[l]<<" "
                <<eps_r_f[l]<<" "
                <<eps_i_f[l]<<" "
                <<mu_r_f[l]<<" "
                <<mu_i_f[l]<<" "
                <<obj_f[l]<<" "<<std::endl;
    }
    
    int Nx=1024;
    int Ny=1024;
    
    double tmp_ex_r;
    double tmp_ex_i;
    
    Grid2<double> tmpdat(Nx,Ny,0);
    
    for(l=0;l<Nl;l++)
    {
        if(l<0)
        {
            for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++)
            {
                tmp_ex_r=eps_r_f[l]*0.9+0.2*eps_r_f[l]*i/(Nx-1.0);
                tmp_ex_i=eps_i_f[l]*0.9+0.2*eps_i_f[l]*j/(Ny-1.0);
                
                test_ind.eps_r=tmp_ex_r;
                test_ind.eps_i=tmp_ex_i;
                test_ind.mu_r=1.0;
                test_ind.mu_i=0.0;
                
                test_ind.evalC(lambda[l],h,r_data[l],t_data[l],strat1);
                tmpdat(i,j)=1.0/test_ind.R;
            }}
            
            double datmax=tmpdat.max();
            
            for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++)
            {
                if(tmpdat(i,j)<0.99*datmax) tmpdat(i,j)=0.99*datmax;
            }}
            
            std::stringstream strm;
            strm<<"RenderW/func"<<l<<".png";
            
            G2_to_degra(tmpdat,strm.str());
        }
        
        test_ind.eps_r=eps_r_f[l];
        test_ind.eps_i=eps_i_f[l];
        test_ind.mu_r=1.0;
        test_ind.mu_i=0.0;
        
        test_ind.evalC(lambda[l],h,r_data[l],t_data[l],strat1);
        
//        plog<<lambda[l]<<" "<<std::log10(obj_f[l])<<" "<<std::log10(test_ind.R)<<std::endl;
    }
    
    data.close();
    normD_index_calc_octave_script(prefix);
}

///###############################################
/// Analytical formulae for the r,t coeff of an
///   anisotropic slab with diagonal tensors
///###############################################

void aniso_diag_slab_analyt(double lambda,double h,double ang,double n1,double n3,Grid1<Imdouble> &eps_r,Grid1<Imdouble> &mu_r,
                            Imdouble &r_TE,Imdouble &r_TM,Imdouble &t_TE,Imdouble &t_TM)
{
    Imdouble eps1_x=n1*n1;
    Imdouble eps1_y=n1*n1;
    Imdouble eps1_z=n1*n1;
    Imdouble mu1_x=1.0;
    Imdouble mu1_y=1.0;
    Imdouble mu1_z=1.0;
    
    Imdouble eps2_x=eps_r[0];
    Imdouble eps2_y=eps_r[1];
    Imdouble eps2_z=eps_r[2];
    Imdouble mu2_x=mu_r[0];
    Imdouble mu2_y=mu_r[1];
    Imdouble mu2_z=mu_r[2];
    
    Imdouble eps3_x=n3*n3;
    Imdouble eps3_y=n3*n3;
    Imdouble eps3_z=n3*n3;
    Imdouble mu3_x=1.0;
    Imdouble mu3_y=1.0;
    Imdouble mu3_z=1.0;
    
    eps1_x*=e0;
    eps1_y*=e0;
    eps1_z*=e0;
    mu1_x*=mu0;
    mu1_y*=mu0;
    mu1_z*=mu0;
    
    eps2_x*=e0;
    eps2_y*=e0;
    eps2_z*=e0;
    mu2_x*=mu0;
    mu2_y*=mu0;
    mu2_z*=mu0;
    
    eps3_x*=e0;
    eps3_y*=e0;
    eps3_z*=e0;
    mu3_x*=mu0;
    mu3_y*=mu0;
    mu3_z*=mu0;
    
    ang*=Pi/180.0;
    
    double s_ang=std::sin(ang);
        
    using std::sqrt;
    using std::abs;
    using std::arg;
    
    double w=2.0*Pi*c_light/lambda;
    double kx=2.0*Pi/lambda*s_ang;
    double w2=w*w;
    double kx2=kx*kx;
    
    Imdouble b1=sqrt(mu1_x/mu1_z*(kx2-w2*eps1_y*mu1_z));
    Imdouble b2=sqrt(mu2_x/mu2_z*(kx2-w2*eps2_y*mu2_z));
    Imdouble b3=sqrt(mu3_x/mu3_z*(kx2-w2*eps3_y*mu3_z));
        
    Imdouble g1=b1/(w*mu1_x)*Im;
    Imdouble g2=b2/(w*mu2_x)*Im;
    Imdouble g3=b3/(w*mu3_x)*Im;
    
    Imdouble g12=(g1+g2)/(g1-g2);
    Imdouble g23=(g2+g3)/(g2-g3);
    
    Imdouble expcoeff1=std::exp(-h*b2);
    Imdouble expcoeff2=std::exp(-2.0*h*b2);
    
    r_TE=(g12+g23*expcoeff2)/(1.0+g12*g23*expcoeff2);
    t_TE=2.0*g1/(g1-g2)*2.0*g2/(g2-g3)*expcoeff1/(1.0+g12*g23*expcoeff2);
    
    b1=sqrt(eps1_x/eps1_z*(kx2-w2*mu1_y*eps1_z));
    b2=sqrt(eps2_x/eps2_z*(kx2-w2*mu2_y*eps2_z));
    b3=sqrt(eps3_x/eps3_z*(kx2-w2*mu3_y*eps3_z));
    
    g1=b1/(w*eps1_x)*Im;
    g2=b2/(w*eps2_x)*Im;
    g3=b3/(w*eps3_x)*Im;
    
    g12=(g1+g2)/(g1-g2);
    g23=(g2+g3)/(g2-g3);
    
    expcoeff1=std::exp(-h*b2);
    expcoeff2=std::exp(-2.0*h*b2);
    
    r_TM=(g12+g23*expcoeff2)/(1.0+g12*g23*expcoeff2)*(-1.0);
    t_TM=2.0*g1/(g1-g2)*2.0*g2/(g2-g3)*expcoeff1/(1.0+g12*g23*expcoeff2)*n1/n3;
}



void eval_aniso_analyt(Grid1<double> const &lambda,
                         double n1,
                         double n3,
                         double h,
                         std::string polar_mode,
                         Grid1<Imdouble> const &eps_ref,
                         Grid1<Imdouble> const &mu_ref,
                         Grid1<Imdouble> const &tmp_coeff,
                         Grid2<double> const &angle,
                         Grid2<Imdouble> const &r_ref,
                         Grid2<Imdouble> const &t_ref,
                         Grid1<double> &rating)
{
    int i,l;
    
    Imdouble r_TE,r_TM,t_TE,t_TM;
    
    Imdouble eps1,eps2,eps3;
    Imdouble mu1,mu2,mu3;
    
    Grid1<Imdouble> eps_r(3,0),mu_r(3,0);
    
    eps1=eps2=eps3=0;
    mu1=mu2=mu3=0;
    
    int Nl=lambda.L1();
    int Na=angle.L2();
    
    using std::abs;
    
    for(l=0;l<Nl;l++)
    {
        double R=0;
        
        if(polar_mode=="TE")
        {
            eps1=eps2=eps3=eps_ref[l];
            mu1=mu2=mu_ref[l];
            
            mu3=tmp_coeff[l];
        }
        else if (polar_mode=="TM")
        {
            eps1=eps2=eps_ref[l];
            mu1=mu2=mu3=mu_ref[l];
            
            eps3=tmp_coeff[l];
        }
        
        eps_r[0]=eps1;
        eps_r[1]=eps2;
        eps_r[2]=eps3;
        mu_r[0]=mu1;
        mu_r[1]=mu2;
        mu_r[2]=mu3;
        
        for(i=0;i<Na;i++)
        {
            aniso_diag_slab_analyt(lambda[l],h,angle(l,i),n1,n3,eps_r,mu_r,r_TE,r_TM,t_TE,t_TM);
            
            if(polar_mode=="TE")
            {
                R+=abs(r_ref(l,i)-r_TE)/abs(r_ref(l,i))+abs(t_ref(l,i)-t_TE)/abs(t_ref(l,i));
            }
            else if(polar_mode=="TM")
            {
                R+=abs(r_ref(l,i)-r_TM)/abs(r_ref(l,i))+abs(t_ref(l,i)-t_TM)/abs(t_ref(l,i));
            }
        }
        
        rating[l]=R;
    }
}

double eval_aniso_analyt(Grid1<double> const &lambda,
                         double n1,
                         double n3,
                         double h,
                         std::string polar_mode,
                         Grid1<Imdouble> const &eps_ref,
                         Grid1<Imdouble> const &mu_ref,
                         Grid1<Imdouble> const &tmp_coeff,
                         Grid2<double> const &angle,
                         Grid2<Imdouble> const &r_ref,
                         Grid2<Imdouble> const &t_ref)
{
    Grid1<double> rating(lambda.L1(),0);
    
    eval_aniso_analyt(lambda,n1,n3,h,polar_mode,eps_ref,mu_ref,tmp_coeff,angle,r_ref,t_ref,rating);
    
    return rating.sum();
}

//################

void dielec_evalfit(int Nl,int Ndr,int Nlo,int Ncp,Grid2<double> &matdata,int Nind,EvoS &mod_coeff)
{
    int i,j,k,l;
    
    Grid1<Imdouble> teps(Nl,0);
    
    Grid1<double> w(Nl,0);
    Grid1<double> w2(Nl,0);
    
    for(l=0;l<Nl;l++)
    {
        w[l]=matdata(l,1);
        w2[l]=w[l]*w[l];
    }
    
    double wd2;
    double g;
    double A;
    double O;
    double O2;
    double G;
    double P;
    
    double nr,ni,nrc,nic;
    
    for(i=0;i<Nind;i++)
    {
        bool drop=0;
        
        teps=static_cast<Imdouble>(0);
        
        for(l=0;l<Nl;l++) teps[l]+=mod_coeff.coeff(0,i);
        if(mod_coeff.coeff(0,i)<1.0) drop=1;
        
        k=1;
        for(j=0;j<Ndr;j++)
        {
            wd2=mod_coeff.coeff(k,i); if(wd2<0) drop=1;
            wd2*=wd2;
            g=mod_coeff.coeff(k+1,i); if(g<0) drop=1;
            
            for(l=0;l<Nl;l++) teps[l]-=wd2/(w2[l]+g*w[l]*Im);
            
            k+=2;
        }
        for(j=0;j<Nlo;j++)
        {
            A=mod_coeff.coeff(k,i);
            O2=mod_coeff.coeff(k+1,i); if(O2<0) drop=1;
            O2*=O2;
            G=mod_coeff.coeff(k+2,i); if(G<0) drop=1;
            
            if(O2<G*G/4.0) drop=1;
            
            for(l=0;l<Nl;l++) teps[l]+=A*O2/(O2-w2[l]-G*w[l]*Im);
            
            k+=3;
        }
        for(j=0;j<Ncp;j++)
        {
            A=mod_coeff.coeff(k,i);
            O=mod_coeff.coeff(k+1,i);
            P=mod_coeff.coeff(k+2,i);
            G=mod_coeff.coeff(k+3,i);
            
            for(l=0;l<Nl;l++)
                teps[l]+=A*O*(std::exp(P*Im)/(O-w[l]+G*Im)+
                              std::exp(-P*Im)/(O+w[l]+G*Im));
            
            k+=4;
        }
        
        mod_coeff.rating[i]=0;
        
        for(l=0;l<Nl;l++)
        {
            using std::abs;
            using std::real;
            using std::imag;
            
            nr=matdata(l,2);
            ni=matdata(l,3);
            
            nrc=real(std::sqrt(teps[l]));
            nic=imag(std::sqrt(teps[l]));
            
            if(nic<0) drop=true;
            
            mod_coeff.rating[i]+=abs(nr-nrc)+abs(ni-nic);
        }
        
        if(drop) mod_coeff.rating[i]+=1000;
    }
}

void index_fit(std::string data_fname,int Ndr,int Nlo,int Ncp)
{
    int i,j;
    
    std::srand(std::time(0));
//    std::srand(666);
        
    int Ngen=20000;
    int Nind=1000;
    int Npro=20;
    
    int Nl=fcountlines(data_fname);
    
    std::ofstream file("result",std::ios::out|std::ios::trunc);
    std::ofstream fileb("resultb",std::ios::out|std::ios::trunc);
    std::ifstream file2(data_fname.c_str(),std::ios::in);
    
    if(!file2.is_open())
    {
        std::cout<<"Error, file not found..."<<std::endl;
    }
    
    Grid2<double> matdata(Nl,4,0);
    
    for(i=0;i<Nl;i++)
    {
        file2>>matdata(i,0);
        matdata(i,1)=2.0*Pi*c_light/matdata(i,0);
        file2>>matdata(i,2);
        file2>>matdata(i,3);
        
        std::cout<<matdata(i,0)<<" "<<matdata(i,2)<<" "<<matdata(i,3)<<std::endl;
    }
        
    int Np=1+Ndr*2+Nlo*3+Ncp*4; //Number of coefficients
    
    std::cout<<Np<<std::endl;
    
    EvoS mod_coeff(Np,Nind,Npro);
    
    double wref=(matdata(0,1)+matdata(Nl-1,1))/2.0;
    
    mod_coeff.scl[0]=1.0; mod_coeff.setsign(0,0,1);
    j=1;
    
    for(i=0;i<Ndr;i++)
    {
        mod_coeff.scl[j]=wref; mod_coeff.setsign(j,0,1);
        mod_coeff.scl[j+1]=wref; mod_coeff.setsign(j+1,0,1);
        j+=2;
    }
    for(i=0;i<Nlo;i++)
    {
        mod_coeff.scl[j]=1.0; mod_coeff.setsign(j,0,1);
        mod_coeff.scl[j+1]=wref; mod_coeff.setsign(j+1,0,1);
        mod_coeff.scl[j+2]=wref; mod_coeff.setsign(j+2,0,1);
        j+=3;
    }
    for(i=0;i<Ncp;i++)
    {
        mod_coeff.scl[j]=1.0; mod_coeff.setsign(j,0,1);
        mod_coeff.scl[j+1]=wref; mod_coeff.setsign(j+1,0,1);
        mod_coeff.scl[j+2]=1.0; mod_coeff.setsign(j+2,1,1);
        mod_coeff.scl[j+3]=wref; mod_coeff.setsign(j+3,0,1);
        j+=4;
    }
    
    std::cout<<"bluh"<<std::endl;
    
    mod_coeff.randgen();
    
    std::cout<<std::endl;
    for(i=0;i<Np;i++) std::cout<<mod_coeff.coeff(i,0)<<std::endl;
    
    dielec_evalfit(Nl,Ndr,Nlo,Ncp,matdata,Nind,mod_coeff);
    mod_coeff.sort();
    
    std::cout<<std::endl;
    for(i=0;i<Nind;i++) std::cout<<mod_coeff.rating[i]<<std::endl;
    std::cout<<std::endl;
    std::cout<<"First best:"<<std::endl;
    for(i=0;i<Np;i++) std::cout<<mod_coeff.coeff(i,0)<<std::endl;
    std::cout<<mod_coeff.rating[0]<<std::endl;
    std::cout<<"First average:"<<std::endl;
    for(i=0;i<Np;i++) std::cout<<mod_coeff.coeff(i,Nind/2)<<std::endl;
    std::cout<<mod_coeff.rating[Nind/2]<<std::endl;
    std::cout<<"First worst:"<<std::endl;
    for(i=0;i<Np;i++) std::cout<<mod_coeff.coeff(i,Nind-1)<<std::endl;
    std::cout<<mod_coeff.rating[Nind-1]<<std::endl;
    
    
    //ProgDisp dsp(Ngen,"Calc");
    ProgTimeDisp dsp(Ngen,500);
    
    mod_coeff.mutate();
    dielec_evalfit(Nl,Ndr,Nlo,Ncp,matdata,Nind,mod_coeff);
    mod_coeff.sort();
    
    for(i=0;i<Ngen;i++)
    {
        mod_coeff.mutate();
        dielec_evalfit(Nl,Ndr,Nlo,Ncp,matdata,Nind,mod_coeff);
        mod_coeff.sort();
        
        plog<<mod_coeff.rating[0]<<std::endl;
        
        ++dsp;
    }
    
    std::cout<<"Final best:"<<std::endl;
    for(i=0;i<Np;i++) std::cout<<mod_coeff.coeff(i,0)<<std::endl;
    std::cout<<mod_coeff.rating[0]<<std::endl;
    std::cout<<"Final average:"<<std::endl;
    for(i=0;i<Np;i++) std::cout<<mod_coeff.coeff(i,Nind/2)<<std::endl;
    std::cout<<mod_coeff.rating[Nind/2]<<std::endl;
    std::cout<<"Final worst:"<<std::endl;
    for(i=0;i<Np;i++) std::cout<<mod_coeff.coeff(i,Nind-1)<<std::endl;
    std::cout<<mod_coeff.rating[Nind-1]<<std::endl;
    
    std::cout<<std::endl;
    for(i=0;i<Np;i++) std::cout<<mod_coeff.coeff(i,0)<<std::endl;
    std::cout<<mod_coeff.rating[0]<<std::endl;
    std::cout<<mod_coeff.rating[0]/Nl<<std::endl;
    
//    file<<ind[0].ei<<std::endl;
    for(i=0;i<Np;i++) file<<mod_coeff.coeff(i,0)<<std::endl;
//    for(i=0;i<Ndr;i++)
//    {
//        file<<ind[0].wd[i]<<std::endl<<ind[0].g[i]<<std::endl;
//    }
//    for(i=0;i<Nlo;i++)
//    {
//        file<<ind[0].A_L[i]<<std::endl<<ind[0].Om_L[i]<<std::endl<<ind[0].Gam_L[i]<<std::endl;
//    }
    fileb<<data_fname<<std::endl<<Ndr<<std::endl<<Nlo<<std::endl<<Ncp;
//    
//    delete[] ind;
}


///##############################
///
///##############################

void recalc_layered(double n1,double n3,double h_base,double h_layer,double n_layer,std::string base_fname,std::string out_fname)
{
    int l;
    int Nl=fcountlines(base_fname);
    
    std::ifstream data_in(base_fname.c_str(),std::ios::in);
    std::ofstream file_out(out_fname.c_str(),std::ios::out|std::ios::trunc);
    
    B_strat strat(2,n1,n3);
    
    strat.set_iso(1,h_layer,n_layer*n_layer,1.0);
    
    for(l=0;l<Nl;l++)
    {
        double tmp1,tmp2;
        
        double lambda;
        Imdouble eps,mu;
        
        data_in>>lambda;
        data_in>>tmp1; data_in>>tmp2; eps=tmp1+Im*tmp2;
        data_in>>tmp1; data_in>>tmp2; mu=tmp1+Im*tmp2;
        data_in>>tmp1;
        
        double r_TE,t_TE;
        
        strat.set_iso(0,h_base,eps,mu);
        
        strat.compute_abs(lambda,0,r_TE,tmp1,t_TE,tmp2);
        
        file_out<<lambda<<" "<<r_TE<<" "<<t_TE<<std::endl;
        
//        std::cout<<lambda<<" "<<eps<<" "<<mu<<std::endl;
    }
    
    data_in.close();
    file_out.close();
}

/*void mode_recalc_layered(ScriptHandler &script)
{
    int i,j,l;
    int Ncalc=0;
    
    double n1,n3,h_base;
    std::string base_fname;
    
    std::vector<double> h_layer;
    std::vector<double> n_layer;
    std::vector<std::string> compare_fname;
    std::vector<std::string> out_fname;
    
    if(script.has("superstrate_index"))
    {
        std::stringstream strm(script.get_options()); strm>>n1;
    }
    if(script.has("substrate_index"))
    {
        std::stringstream strm(script.get_options()); strm>>n3;
    }
    if(script.has("set_thickness"))
    {
        std::stringstream strm(script.get_options()); strm>>h_base;
    }
    if(script.has("set_base")) base_fname=script.get_options();
    
    while(script.has("add_calculation"))
    {
        double tmp_h,tmp_n;
        std::string tmp_out,tmp_comp;
        
        std::stringstream strm(script.get_options());
        
        strm>>tmp_h;
        strm>>tmp_n;
        strm>>tmp_comp;
        strm>>tmp_out;
        
        h_layer.push_back(tmp_h);
        n_layer.push_back(tmp_n);
        compare_fname.push_back(tmp_comp);
        out_fname.push_back(tmp_out);
        
        Ncalc++;
    }
    
    script.show_unused();
    
    for(i=0;i<Ncalc;i++)
    {
        recalc_layered(n1,n3,h_base,h_layer[i],n_layer[i],base_fname,out_fname[i]);
    }
    
    
    int Nc=20;
    Nc=50;
    Grid1<double> bound(Nc+2,0);
        
    bound[0]=0;
    bound[Nc]=100.0;
    bound[Nc+1]=400.0;
    
    for(j=Nc-1;j>0;j--) bound[j]=bound[j+1]*0.8;
    
    Grid1<double> bound_m(Nc+1,0);
    for(j=0;j<=Nc;j++) bound_m[j]=0.5+j;
    
    for(j=0;j<=Nc;j++) bound_m[j]=0.5*(bound[j]+bound[j+1]);
    
    bound.show();
    
    std::ofstream octave_file("test_compare.m",std::ios::out|std::ios::trunc);
    
    octave_file<<"clear *"<<std::endl;
    octave_file<<"close all"<<std::endl;
    octave_file<<"x=[";
    
    for(j=0;j<Nc;j++) octave_file<<bound_m[j]<<",";
    octave_file<<bound_m[Nc]<<"];"<<std::endl;
    octave_file<<"y=[";
    
    for(i=0;i<Ncalc;i++)
    {
        int Nl=fcountlines(compare_fname[i]);
        
        std::ifstream file_simu(compare_fname[i].c_str(),std::ios::in);
        std::ifstream file_mod(out_fname[i].c_str(),std::ios::in);
        
        Grid1<double> cls(Nc+1,0);
        
        for(l=0;l<Nl;l++)
        {
            double lambda,tmp1;
            double r_simu,t_simu,r_mod,t_mod;
            
            file_simu>>lambda; file_simu>>r_simu; file_simu>>t_simu;
            file_mod>>tmp1; file_mod>>r_mod; file_mod>>t_mod;
            
            double err_r=std::abs((r_simu-r_mod)/r_simu)*100.0;
            double err_t=std::abs((t_simu-t_mod)/t_simu)*100.0;
            
            for(j=0;j<=Nc;j++)
            {
                if(err_r>bound[j] && err_r<=bound[j+1]) cls[j]+=1;
                if(err_t>bound[j] && err_r<=bound[j+1]) cls[j]+=1;
            }
            
//            if(err_r>bound(Nc)) cls(Nc)+=1;
//            if(err_t>bound(Nc)) cls(Nc)+=1;
            
//            std::cout<<err_r<<" "<<err_t<<std::endl;
        }
        
        for(j=0;j<=Nc;j++) cls[j]/=(2.0*Nl);
        
        for(j=0;j<=Nc;j++) octave_file<<cls[j]<<",";
        
        if(i==Ncalc-1) octave_file<<"];"<<std::endl;
        else octave_file<<";"<<std::endl;
        
        cls.show();
    }
    
    //octave_file<<"plot(x,y)"<<std::endl;
    octave_file<<"semilogx(x,y)"<<std::endl;
    //octave_file<<"axis([0 "<<Nc+1<<"])"<<std::endl;
    octave_file<<"axis(["<<bound_m[0]<<" "<<bound_m[Nc]<<"])"<<std::endl;
    
    
}*/
