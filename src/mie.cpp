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

#include <mie.h>

extern const double Pi;
extern const Imdouble Im;
extern std::ostream plog;

void BesselJ(Grid1<Imdouble> &z,Imdouble x)
{
    int j;
    
    Imdouble Sx=std::sin(x);
    
    z[0]=Sx/x;
    z[1]=Sx/(x*x)-std::cos(x)/x;
    
    int NRec=z.L1();
    
    for(j=2;j<NRec;j++) z[j]=static_cast<Imdouble>(2.0*(j-1)+1.0)/x*z[j-1]-z[j-2];
}

void BesselJ(Grid1<double> &z,double x)
{
    int j;
    
    double Sx=std::sin(x);
    
    z[0]=Sx/x;
    z[1]=Sx/(x*x)-std::cos(x)/x;
    
    int NRec=z.L1();
    
    for(j=2;j<NRec;j++) 
    {
        z[j]=(2.0*(j-1)+1.0)/x*z[j-1]-z[j-2];
        
//        if(std::abs(z[j])<1e-6)  // The use of this is completely unknown, probably debug code
//        {
//            for(int k=j+1;k<NRec;k++) z(k)=0;
//            break;
//        }
    }
}

void BesselJ(std::vector<double> &z,double x)
{
    int j;
    
    double Sx=std::sin(x);
    
    z[0]=Sx/x;
    z[1]=Sx/(x*x)-std::cos(x)/x;
    
    int NRec=z.size();
    
    for(j=2;j<NRec;j++) 
        z[j]=(2.0*(j-1)+1.0)/x*z[j-1]-z[j-2];
}

void BesselY(Grid1<double> &z,double x)
{
    int j;
    
    double Cx=std::cos(x);
    
    z[0]=-Cx/x;
    z[1]=-Cx/(x*x)-std::sin(x)/x;
    
    int NRec=z.L1();
    
    for(j=2;j<NRec;j++) 
    {
        z[j]=(2.0*(j-1)+1.0)/x*z[j-1]-z[j-2];
    }
}

void BesselY(std::vector<double> &z,double x)
{
    int j;
    
    double Cx=std::cos(x);
    
    z[0]=-Cx/x;
    z[1]=-Cx/(x*x)-std::sin(x)/x;
    
    int NRec=z.size();
    
    for(j=2;j<NRec;j++) 
    {
        z[j]=(2.0*(j-1)+1.0)/x*z[j-1]-z[j-2];
    }
}

void BesselH(Grid1<Imdouble> &z,double x)
{
    int j;
    
    double Cx=std::cos(x);
    double Sx=std::sin(x);
    
    z[0]=Sx/x-Im*Cx/x;
    z[1]=Sx/(x*x)-Cx/x-Im*(Cx/(x*x)+Sx/x);
    
    int NRec=z.L1();
    
    for(j=2;j<NRec;j++) z[j]=(2*(j-1)+1)/x*z[j-1]-z[j-2];
}

void BesselH(std::vector<Imdouble> &z,double x)
{
    int j;
    
    double Cx=std::cos(x);
    double Sx=std::sin(x);
    
    z[0]=Sx/x-Im*Cx/x;
    z[1]=Sx/(x*x)-Cx/x-Im*(Cx/(x*x)+Sx/x);
    
    int NRec=z.size();
    
    for(j=2;j<NRec;j++) z[j]=(2*(j-1)+1)/x*z[j-1]-z[j-2];
}

void CalcPiTau(Grid1<double> &CPi,Grid1<double> &CTau,double mu)
{
    CPi[0]=0;
    CPi[1]=1;
    
    CTau[0]=0;
    CTau[1]=mu;
    
    int NRec=CPi.L1();
    
    for(int j=2;j<NRec;j++)
    {
        CPi[j]=(2.0*j-1.0)/(j-1.0)*mu*CPi[j-1]-j/(j-1.0)*CPi[j-2];
        CTau[j]=j*mu*CPi[j]-(j+1.0)*CPi[j-1];
    }
}

void Dlog(Grid1<Imdouble> &Dn,Imdouble x)
{
    int j;
    
    int N=Dn.L1();
    Imdouble a=0;
    
    Dn[N-1]=0;
    
    for(j=N-2;j>=0;j--)
    {
        a=(j+1.0)/x;
        Dn[j]=a-1.0/(Dn[j+1]+a);
    }
}

void Dlog(std::vector<Imdouble> &Dn,Imdouble x)
{
    int j;
    
    int N=Dn.size();
    Imdouble a=0;
    
    Dn[N-1]=0;
    
    for(j=N-2;j>=0;j--)
    {
        a=(j+1.0)/x;
        Dn[j]=a-1.0/(Dn[j+1]+a);
    }
}

void CalcBesselS(Grid1<Imdouble> &Psi,Imdouble x)
{
    int j;

    Psi[0]=std::sin(x);
    Psi[1]=1.0/x*Psi[0]-std::cos(x);
    
    int Ni=Psi.L1();
    
    for(j=2;j<Ni;j++) Psi[j]=(2.0*(j-1.0)+1.0)/x*Psi[j-1]-Psi[j-2];
}

void CalcBesselS(Grid1<Imdouble> &Psi,Grid1<Imdouble> &Xi,double x)
{
    int j;
    
    int Ni=Psi.L1();
    
    Grid1<Imdouble> Khi(Ni);
    
    double Cx=std::cos(x);
    double Sx=std::sin(x);

    Psi[0]=Sx;
    Psi[1]=1.0/x*Psi[0]-Cx;

    Khi[0]=Cx;
    Khi[1]=1.0/x*Khi[0]+Sx;

    for(j=2;j<Ni;j++)
    {
        Psi[j]=(2.0*(j-1.0)+1.0)/x*Psi[j-1]-Psi[j-2];
        Khi[j]=(2.0*(j-1.0)+1.0)/x*Khi[j-1]-Khi[j-2];
    }

    for(j=0;j<Ni;j++) Xi[j]=Psi[j]-Im*Khi[j];
}

void CalcBesselDS(Grid1<Imdouble> &PsiD,Grid1<Imdouble> &Psi,Imdouble x)
{
    PsiD[0]=std::cos(x);
    
    int Ni=PsiD.L1();
    
    for(int j=1;j<Ni;j++)
    {
        PsiD[j]=Psi[j-1]-static_cast<Imdouble>(j)/x*Psi[j];
    }
}

void CalcBesselDS(Grid1<Imdouble> &PsiD,
                  Grid1<Imdouble> &XiD,
                  Grid1<Imdouble> &Psi,
                  Grid1<Imdouble> &Xi,double x)
{
    Imdouble Cx=std::cos(x);
    
    PsiD[0]=Cx;
    XiD[0]=Cx+Im*std::sin(x);
    
    int Ni=PsiD.L1();
    
    for(int j=1;j<Ni;j++)
    {
        PsiD[j]=Psi[j-1]-j/x*Psi[j];
        XiD[j]=Xi[j-1]-j/x*Xi[j];
    }
}

//###############
//   Mie public
//###############

Mie::Mie()
    :NRec(50),
     lambda(632.8e-9),
     radius(525e-9),
     index_ext(1.0),
     index_in(1.51,0.95),
     recalc_necessary(true),
     an(NRec,0),
     bn(NRec,0),
     coeff_pi(NRec,0),
     coeff_tau(NRec,0)
{
    xm_recalc();
}

Mie::~Mie()
{
}

void Mie::compute_pi_tau(double mu)
{
    coeff_pi[0]=0;
    coeff_pi[1]=1;
    
    coeff_tau[0]=0;
    coeff_tau[1]=mu;
    
    for(int j=2;j<NRec;j++)
    {
        coeff_pi[j]=(2.0*j-1.0)/(j-1.0)*mu*coeff_pi[j-1]
                    -j/(j-1.0)*coeff_pi[j-2];
        coeff_tau[j]=j*mu*coeff_pi[j]-(j+1.0)*coeff_pi[j-1];
    }
}

double Mie::get_an_abs(int n)
{
    if(recalc_necessary) compute_ab();
    return std::abs(an[n]);
}

double Mie::get_bn_abs(int n)
{
    if(recalc_necessary) compute_ab();
    return std::abs(bn[n]);
}

double Mie::get_Cabs()
{
    return get_Cext()-get_Cscatt();
}

double Mie::get_Cext()
{
    int i;
    double k=2.0*Pi/lambda*index_ext;
    Imdouble s=0;
    
    if(recalc_necessary) compute_ab();
    
    for(i=1;i<NRec;i++)
    {
        s+=(2.0*i+1)*(an[i]+bn[i]);
    }
    
    return 2.0*Pi/k/k*std::real(s);
}

double Mie::get_Cscatt()
{
    int i;
    double k=2.0*Pi/lambda*index_ext;
    double s=0;
    
    if(recalc_necessary) compute_ab();
    
    for(i=1;i<NRec;i++)
    {
        s+=(2.0*i+1)*(std::norm(an[i])+std::norm(bn[i]));
    }
    
    return 2.0*Pi/k/k*s;
}

double Mie::get_radiation(double theta,double phi)
{
    int n;
    double R=0;
    double mu=std::cos(theta);
    
    if(recalc_necessary) compute_ab();
    
    compute_pi_tau(mu);
    
    Imdouble Et=0,Ep=0;
    
    for(n=1;n<NRec;n++)
    {
        double C=(2.0*n+1)/(n*(n+1.0));
        
        Et+=C*(an[n]*coeff_tau[n]+bn[n]*coeff_pi[n]);
        Ep+=C*(an[n]*coeff_pi[n]+bn[n]*coeff_tau[n]);
    }
    
    Et*=std::cos(phi);
    Ep*=-std::sin(phi);
    
    R=0.5*index_ext/(c_light*mu0)*std::real(Et*std::conj(Et)+Ep*std::conj(Ep));
    
    return R;
}

double Mie::get_Qabs()
{
    return get_Qext()-get_Qscatt();
}

double Mie::get_Qext()
{
    return get_Cext()/(Pi*radius*radius);
}

double Mie::get_Qscatt()
{
    return get_Cscatt()/(Pi*radius*radius);
}

void Mie::set_NRec(int NRec_)
{
    NRec=NRec_;
    
    an.resize(NRec);
    bn.resize(NRec);
    coeff_pi.resize(NRec);
    coeff_tau.resize(NRec);
    
    for(int i=0;i<NRec;i++)
    {
        an[i]=0;
        bn[i]=0;
        coeff_pi[i]=0;
        coeff_tau[i]=0;
    }
    
    recalc_necessary=true;
}

void Mie::set_lambda(double lambda_)
{
    lambda=lambda_;
    
    xm_recalc();
    recalc_necessary=true;
}

void Mie::set_radius(double radius_)
{
    radius=radius_;
    
    xm_recalc();
    recalc_necessary=true;
}

void Mie::set_index(Imdouble index_in_,double index_ext_)
{
    set_index_in(index_in_);
    set_index_ext(index_ext_);
}

void Mie::set_index_ext(double index_ext_)
{
    index_ext=index_ext_;
    
    xm_recalc();
    recalc_necessary=true;
}

void Mie::set_index_in(Imdouble index_in_)
{
    index_in=index_in_;
    
    xm_recalc();
    recalc_necessary=true;
}

//###############
//   Mie private
//###############

void Mie::xm_recalc()
{
    x_part=2.0*Pi*index_ext*radius/lambda;
    m=index_in/index_ext;
}

void Mie::compute_ab()
{
    std::vector<Imdouble> Dn(5*NRec,0);
    
    std::vector<double> jn(NRec,0);
    std::vector<double> yn(NRec,0);
    
    Dlog(Dn,m*x_part);
    
    BesselJ(jn,x_part);
    BesselY(yn,x_part);
    
    an[0]=bn[0]=0;
    
    Imdouble a,b;
    Imdouble Psi,Psi1,Xi,Xi1;
    
    for(int j=1;j<NRec;j++)
    {
        a=Dn[j]/m+j/x_part;
        b=Dn[j]*m+j/x_part;
        
        Psi=x_part*jn[j];
        Psi1=x_part*jn[j-1];
        
        Xi=x_part*(jn[j]+Im*yn[j]);
        Xi1=x_part*(jn[j-1]+Im*yn[j-1]);
        
        an[j]=(a*Psi-Psi1)/(a*Xi-Xi1);
        bn[j]=(b*Psi-Psi1)/(b*Xi-Xi1);
    }
}

double EField(double x,double y,double z,Mie &mie)
{
    Imdouble X,Y,Z;
    
    EField_complex(x,y,z,mie,X,Y,Z);
    
    return std::real(X*std::conj(X)+Y*std::conj(Y)+Z*std::conj(Z));
}

void EField_complex(double x,double y,double z,Mie &mie,Imdouble &X,Imdouble &Y,Imdouble &Z)
{
    if(mie.recalc_necessary) mie.compute_ab();

    X=0; Y=0; Z=0;
    
    double theta,phi,rho;
    
    theta=std::atan2(std::sqrt(x*x+y*y),z);
    phi=std::atan2(y,x);
    rho=2.0*Pi*mie.index_ext*std::sqrt(x*x+y*y+z*z)/mie.lambda;
    
    if(rho<=mie.x_part)
    {
        X=Y=Z=std::sqrt(1.0/3.0);
        return;
    }
    
    int NRec=mie.NRec;
    
    Imdouble hn,hn1;
    Grid1<double> jn(NRec);
    BesselJ(jn,rho);
    Grid1<double> yn(NRec);
    BesselY(yn,rho);
    
    Grid1<double> CPi(NRec);
    Grid1<double> CTau(NRec);
    
    CalcPiTau(CPi,CTau,std::cos(theta));
    
    int j;
    Imdouble En;
    
    double Cosphi=std::cos(phi);
    double Sinphi=std::sin(phi);
    
    double tmp;
    Imdouble ian,bn,tmp2;
    
    double Sintheta=std::sin(theta);
    
    for(j=1;j<NRec;j++)
    {
        hn=jn[j]+Im*yn[j];
        hn1=jn[j-1]+Im*yn[j-1];
        
        En=std::pow(Im,j)*static_cast<double>((2.0*j+1.0)/(j*(j+1.0)));
        
        tmp=(rho*jn[j-1]-j*jn[j])/rho;
        tmp2=(rho*hn1-static_cast<double>(j)*hn)/rho;
        
        ian=Im*mie.an[j];
        bn=mie.bn[j];
        
        X+=En*(-Im*Cosphi*static_cast<double>(j*(j+1))*Sintheta*CPi[j]*jn[j]/rho);
        Y+=En*Cosphi*(CPi[j]*jn[j]-Im*CTau[j]*tmp);
        Z+=En*(-Sinphi)*(CTau[j]*jn[j]-Im*CPi[j]*tmp);
        
        X+=En*(ian*Cosphi*static_cast<double>(j*(j+1))*Sintheta*CPi[j]*hn/rho);
        Y+=En*Cosphi*(ian*CTau[j]*tmp2-bn*CPi[j]*hn);
        Z+=En*(-Sinphi)*(ian*CPi[j]*tmp2-bn*CTau[j]*hn);
    }
}

void Mie_mode::process()
{
    int l;
    
    Material mat;
    mat.load_lua_script(part_material);
    
    Mie mie;
    
    mie.set_index_ext(env_index);
    mie.set_radius(radius);
    
    for(unsigned int ext=0;ext<extract_type.size();ext++)
    {
        int &Nl=extract_Nl[ext];
        int &type=extract_type[ext];
        
        std::ofstream file(extract_fname[ext],std::ios::out|std::ios::trunc);
        
        for(l=0;l<Nl;l++)
        {
            double &lmin=extract_lmin[ext];
            double &lmax=extract_lmax[ext];
            
            double lambda=lmin+(lmax-lmin)*l/(Nl-1.0);
            double w=2.0*Pi*c_light/lambda;
            
            Imdouble l_index=mat.get_n(w);
            
            mie.set_lambda(lambda);
            mie.set_index_in(l_index);
            
            file<<lambda<<" ";
                 if(type==MIE_CABS) file<<mie.get_Cabs()<<std::endl;
            else if(type==MIE_CEXT) file<<mie.get_Cext()<<std::endl;
            else if(type==MIE_CSCATT) file<<mie.get_Cscatt()<<std::endl;
            else if(type==MIE_QABS) file<<mie.get_Qabs()<<std::endl;
            else if(type==MIE_QEXT) file<<mie.get_Qext()<<std::endl;
            else if(type==MIE_QSCATT) file<<mie.get_Qscatt()<<std::endl;
            else if(type==MIE_FULL_CQ)
            {
                file<<mie.get_Cabs()<<" ";
                file<<mie.get_Cext()<<" ";
                file<<mie.get_Cscatt()<<" ";
                file<<mie.get_Qabs()<<" ";
                file<<mie.get_Qext()<<" ";
                file<<mie.get_Qscatt()<<std::endl;
            }
        }
        
        file.close();
    }
}
