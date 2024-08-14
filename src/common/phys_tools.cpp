/*Copyright 2008-2024 - Loïc Le Cunff

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#include <logger.h>
#include <phys_tools.h>

extern const Imdouble Im;

// Formula obtained thanks to https://www.telescope-optics.net/conics_and_aberrations.htm

double conic(double x,double R,double K)
{
    return std::sqrt(2.0*R*x-(1.0+K)*x*x);
}

void conic_far_focus(double &x,double &y,double R,double K)
{
    y=0;
    x=R/(1.0-std::sqrt(std::abs(K)));
    
    if(K>0)
    {
        x=conic_midpoint(R,K);
        y=R*std::sqrt(K)/(1.0+K);
    }
    else if(K==-1) conic_near_focus(x,y,R,K);
}

double conic_invert(double y,double R,double K)
{
    if(K==-1) return y*y/(2.0*R);
    else return (R-std::sqrt(R*R-(1.0+K)*y*y))/(1.0+K);
}

double conic_midpoint(double R,double K)
{
    if(K>-1.0) return R/(1.0+K);
    else return R;
}

void conic_near_focus(double &x,double &y,double R,double K)
{
    y=0;
    x=R/(1.0+std::sqrt(std::abs(K)));
    
    if(K>0)
    {
        x=conic_midpoint(R,K);
        y=-R*std::sqrt(K)/(1.0+K);
    }
}

double eV_to_m(double E) { return h_planck*c_light/(elem_charge*E); }

void fresnel_rt_te_tm(AngleRad angle_inc,Imdouble n1,Imdouble n2,
                      Imdouble &r_te,Imdouble &r_tm,Imdouble &t_te,Imdouble &t_tm)
{
    Imdouble angle_ref=std::asin(n1/n2*std::sin(angle_inc));
    
    double cti=std::cos(angle_inc);
    Imdouble ctr=std::cos(angle_ref);
    
    r_te=(n1*cti-n2*ctr)/(n1*cti+n2*ctr);
    r_tm=(n2*cti-n1*ctr)/(n2*cti+n1*ctr);
    
    t_te=2.0*n1*cti/(n1*cti+n2*ctr);
    t_tm=2.0*n2*cti/(n2*cti+n1*ctr);
}

void fresnel_rt_te_tm_wn(double lambda,Imdouble n1,Imdouble n2,double kp,
                         Imdouble &r_te,Imdouble &r_tm,Imdouble &t_te,Imdouble &t_tm)
{
    double k0=2.0*Pi/lambda;
    Imdouble k1=n1*k0;
    Imdouble k2=n2*k0;
    
    Imdouble sr1=std::sqrt(k1*k1-kp*kp);
    Imdouble sr2=std::sqrt(k2*k2-kp*kp);
    
    Imdouble e1=n1*n1;
    Imdouble e2=n2*n2;
    
    r_te=(sr1-sr2)/(sr1+sr2);
    t_te=2.0*sr1/(sr1+sr2);
    
    r_tm=(e2*sr1-e1*sr2)/(e2*sr1+e1*sr2);
    t_tm=2.0*e2*sr1/(e2*sr1+e1*sr2);
}

void fresnel_rt_te_tm_E_wn(double lambda,Imdouble n1,Imdouble n2,double kp,
                         Imdouble &r_te,Imdouble &r_tm,Imdouble &t_te,Imdouble &t_tm)
{
    double k0=2.0*Pi/lambda;
    Imdouble k1=n1*k0;
    Imdouble k2=n2*k0;
    
    Imdouble sr1=std::sqrt(k1*k1-kp*kp);
    Imdouble sr2=std::sqrt(k2*k2-kp*kp);
    
    Imdouble e1=n1*n1;
    Imdouble e2=n2*n2;
    
    r_te=(sr1-sr2)/(sr1+sr2);
    t_te=2.0*sr1/(sr1+sr2);
    
    r_tm=(e2*sr1-e1*sr2)/(e2*sr1+e1*sr2);
    t_tm=2.0*e2*sr1/(e2*sr1+e1*sr2);
    
    r_tm=-r_tm;
    t_tm=n1/n2*t_tm;
//    t_tm=sr2/e2*e1/sr1*t_tm;
}

double gaussian_spectrum(double w,double lambda_min,double lambda_max,double amp)
{
    double w1=2.0*Pi*c_light/std::max(lambda_min,lambda_max);
    double w2=2.0*Pi*c_light/std::min(lambda_min,lambda_max);
    
    double w0=0.5*(w1+w2);
    
    double dw=(w2-w1)/(2.0*std::sqrt(-std::log(amp)));
    
    double arg=(w-w0)/dw;
    
    return std::exp(-arg*arg);
}

double Hz_to_m(double f) { return c_light/f; }

double inv_cm_to_m(double n) { return 1.0/(100.0*n); }

double J_to_m(double E) { return h_planck*c_light/E; }

double m_to_eV(double lambda) { return h_planck*c_light/lambda/elem_charge; }

double m_to_inv_cm(double lambda) { return 1.0/(100.0*lambda); }

double m_to_J(double lambda) { return h_planck*c_light/lambda; }

double m_to_Hz(double lambda) { return c_light/lambda; }

double m_to_microns(double lambda) { return lambda*1e6; }

double m_to_mm(double lambda) { return lambda*1e3; }

double m_to_nm(double lambda) { return lambda*1e9; }

double m_to_rad_Hz(double lambda) { return 2.0*Pi*c_light/lambda; }

double m_to_THz(double lambda) { return c_light/lambda/1e12; }

double microns_to_m(double lambda) { return lambda*1e-6; }

double mm_to_m(double lambda) { return lambda*1e-3; }

double nm_to_m(double lambda) { return lambda*1e-9; }

void plane_wave(double lambda,double n,Vector3 const &dir,double polar,double x,double y,double z,double t,ImVector3 &E,ImVector3 &H)
{
    using std::cos;
    using std::sin;
    
    double g=Pi/2.0*polar;
    
    double kn=2.0*Pi*n/lambda;
    double w=2.0*Pi*c_light/lambda;
    
    Vector3 ku=dir;
    ku.normalize();
    
    Vector3 tv,p;
    
    if(std::abs(ku.x)==0 && std::abs(ku.y)==0)
    {
        p(0,1,0);
        
        tv.crossprod(ku,p);
        tv.normalize();
    }
    else
    {
        tv(0,0,1);
        p.crossprod(tv,ku);
        p.normalize();
        
        tv.crossprod(ku,p);
        tv.normalize();
    }
    
    Vector3 k=kn*ku;
    
    E=cos(g)*p+sin(g)*tv;
    E*=std::exp((x*k.x+y*k.y+z*k.z-w*t)*Im);
    
    H.crossprod(k,E);
    H/=mu0*w;
}

void plane_wave(double lambda,double n,double th,double phi,AngleRad polar,double x,double y,double z,double t,ImVector3 &E,ImVector3 &H)
{
    using std::cos;
    using std::sin;
    
    double g=polar;
    
    double kn=2.0*Pi*n/lambda;
    double w=2.0*Pi*c_light/lambda;
    
    Vector3 ku(cos(phi)*sin(th),sin(phi)*sin(th),cos(th));
    Vector3 p(-sin(phi),cos(phi),0);
    Vector3 k=kn*ku;
    Vector3 tv;
    
    tv.crossprod(ku,p);
    
    E=cos(g)*p+sin(g)*tv;
    E*=std::exp((x*k.x+y*k.y+z*k.z-w*t)*Im);
    
    H.crossprod(k,E);
    H/=mu0*w;
}

double planck_distribution_wavelength(double lambda,double T)
{
    double hc=h_planck*c_light;
    double l5=lambda*lambda*lambda*lambda*lambda;
    
    return 2.0*hc*c_light/(l5*(std::exp(hc/(lambda*k_boltzmann*T))-1.0));
}

double rad_Hz_to_m(double w) { return 2.0*Pi*c_light/w; }

SeedParticle::SeedParticle()
    :stable(false),
     x(0), y(0), r(0),
     Fx(0), Fy(0)
{
}
        
void SeedParticle::add_force(double x_,double y_,double r_)
{
    double x_d=x-x_;
    double y_d=y-y_;
    
    double n=std::sqrt(x_d*x_d+y_d*y_d);
    
    if(n<=r+r_)
    {
        double coeff=std::max(1.0-0.99*n/(r+r_),0.0);
        
        Fx+=r/10.0*coeff*x_d/n;
        Fy+=r/10.0*coeff*y_d/n;
    }
}
        
void SeedParticle::advance(double x_max,double y_max)
{
    x+=Fx;
    y+=Fy;
    
    while(x<0) x+=x_max;
    while(y<0) y+=y_max;
    
    while(x>x_max) x-=x_max;
    while(y>y_max) y-=y_max;
    
    if((Fx*Fx+Fy*Fy)/(r*r)>=1e-9) stable=false;
    else stable=true;
    
    Fx=Fy=0;
}
        
void SeedParticle::clamp_force(double fmax)
{
    if(Fx*Fx+Fy*Fy>fmax*fmax)
    {
        double Fn=std::sqrt(Fx*Fx+Fy*Fy);
        
        Fx=fmax*Fx/Fn;
        Fy=fmax*Fy/Fn;
    }
}

void SeedParticle::set(double x_,double y_,double r_)
{
    x=x_;
    y=y_;
    r=r_;
}

void random_packing(std::vector<SeedParticle> &particles,double x_max,double y_max,double r,int Npart,int seed)
{
    particles.resize(Npart);
    
    std::mt19937 gen(std::time(0));
    
    if(seed>=0) gen.seed(seed);
    
    particles[0].set(0,0,r);
    
    for(int i=1;i<Npart;i++)
    {
        particles[i].set(randp(x_max,gen),randp(y_max,gen),r);
    }
    
    int coeff_x[9]={0,1,1,0,-1,-1,-1,0,1};
    int coeff_y[9]={0,0,1,1,1,0,-1,-1,-1};
    
    std::vector<std::vector<std::vector<int>>> accel_grid;
    
    int Nx=static_cast<int>(x_max/(2.0*r));
    int Ny=static_cast<int>(y_max/(2.0*r));
    
    double Dx=x_max/Nx;
    double Dy=y_max/Ny;
    
    accel_grid.resize(Nx);
    for(int i=0;i<Nx;i++) accel_grid[i].resize(Ny);
    
    bool unstable=true;
    
    int counter=0;
    
    while(unstable)
    {
        unstable=false;
        
        for(int i=0;i<Nx;i++)
            for(int j=0;j<Ny;j++)
                accel_grid[i][j].clear();
        
        for(int l=0;l<Npart;l++)
        {
            int i=static_cast<int>(particles[l].x/Dx)%Nx;
            int j=static_cast<int>(particles[l].y/Dy)%Ny;
            
            accel_grid[i][j].push_back(l);
        }
        
        for(int i=0;i<Nx;i++) for(int j=0;j<Ny;j++)
        {
            for(unsigned int k=0;k<accel_grid[i][j].size();k++)
            {
                int p=accel_grid[i][j][k];
                
                for(int l=0;l<9;l++)
                {
                    std::vector<int> &tmp_arr=accel_grid[(i+coeff_x[l]+Nx)%Nx][(j+coeff_y[l]+Ny)%Ny];
                    
                    for(unsigned int m=0;m<tmp_arr.size();m++)
                    {
                        int q=tmp_arr[m];
                        
                        for(int n=0;n<9;n++)
                            if(p!=q && p!=0) particles[p].add_force(particles[q].x+coeff_x[n]*x_max,
                                                                    particles[q].y+coeff_y[n]*y_max,
                                                                    particles[q].r);
                    }
                }
            }
        }
        
        for(int i=0;i<Npart;i++) particles[i].clamp_force(0.01);
        
        int N_unstable=0;
        
        for(int i=0;i<Npart;i++)
        {
            if(!particles[i].stable)
            {
                unstable=true;
                N_unstable++;
            }
        }
        
        if(counter%100==0) Plog::print("Stability: ", 1.0-N_unstable/(Npart+0.0), "\n");
        
//        if(counter%50000==0)
//        {            
//            for(int i=1;i<Npart;i++)
//            {
//                particles[i].set(randp(x_max,gen),randp(y_max,gen),r);
//            }
//        }
//        else
//        {
            for(int i=0;i<Npart;i++) particles[i].advance(x_max,y_max);
//        }
        
        counter++;
    }
    
    double min_dist=x_max;
    
    for(int i=0;i<Npart;i++)
    {
        double x1=particles[i].x;
        double y1=particles[i].y;
        
        for(int j=0;j<Npart;j++)
        {
            if(i!=j)
            {
                for(int k=0;k<9;k++)
                {
                    double x2=particles[j].x-x1+coeff_x[k]*x_max;
                    double y2=particles[j].y-y1+coeff_y[k]*y_max;
                    
                    min_dist=std::min(min_dist,std::sqrt(x2*x2+y2*y2));
                }
            }
        }
    }
    
    chk_var(min_dist/(2.0*particles[0].r));
}

Imdouble spp_get_kspp(double w,Imdouble eps_1,Imdouble eps_2)
{
    return w/c_light*std::sqrt(eps_1*eps_2/(eps_1+eps_2));
}

double THz_to_m(double f) { return c_light/(f*1e12); }
