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

#include <dielec_models.h>

extern const double Pi;
extern const Imdouble Im;

extern std::ofstream plog;

//###############
// Dielec model
//###############

Dielec_model::Dielec_model()
    :ei(1.0),
     Ndebye(0), Ndrude(0), Nlorentz(0), Ncp(0)
{
}

Dielec_model::Dielec_model(Dielec_model const &D)
    :ei(D.ei),
     Ndebye(D.Ndebye), Ndrude(D.Ndrude), Nlorentz(D.Nlorentz), Ncp(D.Ncp),
     debye_arr(D.debye_arr),
     drude_arr(D.drude_arr),
     lorentz_arr(D.lorentz_arr),
     cp_arr(D.cp_arr)
{
}

Dielec_model::~Dielec_model()
{
}

void Dielec_model::clear()
{
    ei=1.0;
    Ndebye=Ndrude=Nlorentz=Ncp=0;
    debye_arr.reset();
    drude_arr.reset();
    lorentz_arr.reset();
    cp_arr.reset();
}

Imdouble Dielec_model::eval(double w) const
{
    int i;
    Imdouble r=0;
    
    r+=ei;
    for(i=0;i<Ndebye;i++) r+=debye_arr[i].eval(w);
    for(i=0;i<Ndrude;i++) r+=drude_arr[i].eval(w);
    for(i=0;i<Nlorentz;i++) r+=lorentz_arr[i].eval(w);
    for(i=0;i<Ncp;i++) r+=cp_arr[i].eval(w);
    
    return r;
}

void Dielec_model::set_const(double ei_in)
{
    ei=ei_in;
}

void Dielec_model::add_debye(double ds,double t0)
{
    dielec_debye tmp_deb;
    tmp_deb.set(ds,t0);
    
    debye_arr.push_back(tmp_deb);
    Ndebye+=1;
}

void Dielec_model::add_drude(double wd,double g)
{
    dielec_drude tmp_drude;
    tmp_drude.set(wd,g);
    
    drude_arr.push_back(tmp_drude);
    Ndrude+=1;
}

void Dielec_model::add_lorentz(double A,double O,double G)
{
    dielec_lorentz tmp_lor;
    tmp_lor.set(A,O,G);
    
    lorentz_arr.push_back(tmp_lor);
    Nlorentz+=1;
}

void Dielec_model::add_critpoint(double A,double O,double P,double G)
{
    dielec_critpoint tmp_crit;
    tmp_crit.set(A,O,P,G);
    
    cp_arr.push_back(tmp_crit);
    Ncp+=1;
}

double Dielec_model::get_const()
{
    return ei;
}

void Dielec_model::get_time_exp(Grid1<Imdouble> &va,Grid1<Imdouble> &vb)
{
    int i,j;
    Imdouble tmp_a,tmp_b;
    
    int Nmod=get_N_models();
    
    va.init(Nmod);
    vb.init(Nmod);
    
    j=0;
    for(i=0;i<Ndebye;i++)
    {
        debye_arr[i].get_time_exp(tmp_a,tmp_b);
        va[j]=tmp_a; vb[j]=tmp_b;
        j++;
    }
    for(i=0;i<Ndrude;i++)
    {
        drude_arr[i].get_time_exp(tmp_a,tmp_b);
        va[j]=tmp_a; vb[j]=tmp_b;
        j++;
    }
    for(i=0;i<Nlorentz;i++)
    {
        lorentz_arr[i].get_time_exp(tmp_a,tmp_b);
        va[j]=tmp_a; vb[j]=tmp_b;
        j++;
    }
    for(i=0;i<Ncp;i++)
    {
        cp_arr[i].get_time_exp(tmp_a,tmp_b);
        va[j]=tmp_a; vb[j]=tmp_b;
        j++;
    }
}

std::string Dielec_model::get_matlab() const
{
    std::stringstream strm;
    
    strm<<"eps=0*w+"<<ei<<";\n";
    
    int i,l=0;
    
    for(i=0;i<Ndebye;i++) { strm<<debye_arr[i].get_matlab(l); l++; }
    for(i=0;i<Ndrude;i++) { strm<<drude_arr[i].get_matlab(l); l++; }
    for(i=0;i<Nlorentz;i++) { strm<<lorentz_arr[i].get_matlab(l); l++; }
    for(i=0;i<Ncp;i++) { strm<<cp_arr[i].get_matlab(l); l++; }
    
    strm<<"eps=eps+";
    
    for(i=0;i<l;i++)
    {
        strm<<"eps_"<<i;
        
        if(i<l-1) strm<<"+";
        else strm<<";\n\n";
    }
    
    return strm.str();
}

int Dielec_model::get_N_models()
{
    return Ndebye+Ndrude+Nlorentz+Ncp;
}

double Dielec_model::get_sigma()
{
    int i;
    double sig=0;
    for(i=0;i<Ndrude;i++)
    {
        sig+=drude_arr[i].get_sigma();
    }
    
    return sig;
}

void Dielec_model::show()
{
    int i;
    
    std::cout<<"Eps_inf: "<<ei<<std::endl;
    for(i=0;i<Ndebye;i++)
    {
        std::cout<<"Debye "<<i<<":"<<std::endl;
        debye_arr[i].show();
    }
    for(i=0;i<Ndrude;i++)
    {
        std::cout<<"Drude "<<i<<":"<<std::endl;
        drude_arr[i].show();
    }
    for(i=0;i<Nlorentz;i++)
    {
        std::cout<<"Lorentz "<<i<<":"<<std::endl;
        lorentz_arr[i].show();
    }
    for(i=0;i<Ncp;i++)
    {
        std::cout<<"Critical-points "<<i<<":"<<std::endl;
        cp_arr[i].show();
    }
}

void Dielec_model::operator = (Dielec_model const &D)
{
    ei=D.ei;
    
    Ndebye=D.Ndebye; if(Ndebye>0) debye_arr.init(Ndebye);
    Ndrude=D.Ndrude; if(Ndrude>0) drude_arr.init(Ndrude);
    Nlorentz=D.Nlorentz; if(Nlorentz>0) lorentz_arr.init(Nlorentz);
    Ncp=D.Ncp; if(Ncp>0) cp_arr.init(Ncp);
    
    if(Ndebye>0) debye_arr=D.debye_arr;
    if(Ndrude>0) drude_arr=D.drude_arr;
    if(Nlorentz>0) lorentz_arr=D.lorentz_arr;
    if(Ncp>0) cp_arr=D.cp_arr;
}

//####################
//####################

//Debye

dielec_debye::dielec_debye()
    :ds(0), t0(0)
{}

dielec_debye::dielec_debye(dielec_debye const &D)
    :ds(D.ds), t0(D.t0)
{}

std::string dielec_debye::get_matlab(int ID) const
{
    std::stringstream strm;
    
    strm<<"ds_"<<ID<<"="<<ds<<";\n";
    strm<<"t0_"<<ID<<"="<<t0<<";\n\n";
    
    strm<<"eps_"<<ID<<"=ds_"<<ID<<"./(1.0-w*t0_"<<ID<<"*i);\n\n";
    
    return strm.str();
}

Imdouble dielec_debye::eval(double w) const
{
    return ds/(1.0-w*t0*Im);
}

void dielec_debye::get_time_exp(Imdouble &a_o,Imdouble &b_o)
{
    a_o=ds/t0;
    b_o=-1.0/t0;
}

void dielec_debye::set(double dsi,double t0i)
{
    ds=dsi; t0=t0i;
}

void dielec_debye::show()
{
    std::cout<<"Ds: "<<ds<<" t0: "<<t0<<std::endl;
}

void dielec_debye::operator = (dielec_debye const &D)
{
    ds=D.ds;
    t0=D.t0;
}

//Drude

dielec_drude::dielec_drude()
    :wd(0), wd2(0), g(1.0)
{}

dielec_drude::dielec_drude(dielec_drude const &D)
    :wd(D.wd), wd2(D.wd2), g(D.g)
{}

Imdouble dielec_drude::eval(double w) const
{
    return -wd2/(w*w+w*g*Im);
}

std::string dielec_drude::get_matlab(int ID) const
{
    std::stringstream strm;
    
    strm<<"wd2_"<<ID<<"="<<wd2<<";\n";
    strm<<"g_"<<ID<<"="<<g<<";\n\n";
    
    strm<<"eps_"<<ID<<"=-wd2_"<<ID<<"./(w.^2+w*g_"<<ID<<"*i);\n\n";
    
    return strm.str();
}

void dielec_drude::get_time_exp(Imdouble &a_o,Imdouble &b_o)
{
    a_o=-wd2/g;
    b_o=-g;
}

double dielec_drude::get_sigma()
{
    return e0*wd2/g;
}
void dielec_drude::set(double wdi,double gi)
{
    wd=wdi;
    g=gi;
    
    wd2=wd*wd;
}

void dielec_drude::show()
{
    std::cout<<"Wd: "<<wd<<" Wd2: "<<wd2<<" g: "<<g<<std::endl;
}

void dielec_drude::operator = (dielec_drude const &D)
{
    wd=D.wd;
    wd2=D.wd2;
    g=D.g;
}

//Lorentz

dielec_lorentz::dielec_lorentz()
    :A(0), O(1.0), O2(1.0), G(0)
{}

dielec_lorentz::dielec_lorentz(dielec_lorentz const &D)
    :A(D.A), O(D.O), O2(D.O2), G(D.G)
{}

Imdouble dielec_lorentz::eval(double w) const
{
    return A*O2/(O2-w*w-w*G*Im);
}

std::string dielec_lorentz::get_matlab(int ID) const
{
    std::stringstream strm;
    
    strm<<"A_"<<ID<<"="<<A<<";\n";
    strm<<"O2_"<<ID<<"="<<O2<<";\n";
    strm<<"G_"<<ID<<"="<<G<<";\n\n";
    
    strm<<"eps_"<<ID<<"=A_"<<ID<<"*O2_"<<ID<<"./(O2_"<<ID<<"-w.^2-w*G_"<<ID<<"*i);\n\n";
    
    return strm.str();
}

void dielec_lorentz::get_time_exp(Imdouble &a_o,Imdouble &b_o)
{
    double alp=G/2.0;
    Imdouble bet=std::sqrt(O*O-alp*alp);
    Imdouble eta=A*O*O/bet;
    
    a_o=-eta*Im;
    b_o=-alp+bet*Im;
}

void dielec_lorentz::set(double Ai,double Oi,double Gi)
{
    A=Ai; O=Oi; G=Gi;
    O2=O*O;
}

void dielec_lorentz::show()
{
    std::cout<<"A: "<<A<<" O: "<<O<<" O2: "<<O2<<" G: "<<G<<std::endl;
}

void dielec_lorentz::operator =(dielec_lorentz const &D)
{
    A=D.A;
    O=D.O;
    O2=D.O2;
    G=D.G;
}

//Critical points

dielec_critpoint::dielec_critpoint()
    :A(0), O(1), P(0), G(1)
{}

dielec_critpoint::dielec_critpoint(dielec_critpoint const &D)
    :A(D.A), O(D.O), P(D.P), G(D.G)
{}

std::string dielec_critpoint::get_matlab(int ID) const
{
    std::stringstream strm;
    
    strm<<"A_"<<ID<<"="<<A<<";\n";
    strm<<"O_"<<ID<<"="<<O<<";\n";
    strm<<"P_"<<ID<<"="<<P<<";\n";
    strm<<"G_"<<ID<<"="<<G<<";\n\n";
    
    strm<<"eps_"<<ID<<"=A_"<<ID<<"*O_"<<ID<<"*(exp(P_"<<ID<<"*i)./(O_"<<ID<<"-w-G_"<<ID<<"*i)+exp(-P_"<<ID<<"*i)./(O_"<<ID<<"+w+G_"<<ID<<"*i));\n\n";
    
    return strm.str();
}

Imdouble dielec_critpoint::eval(double w) const
{
    using std::exp;
    return A*O*(exp(P*Im)/(O-w-G*Im)+exp(-P*Im)/(O+w+G*Im));
}

void dielec_critpoint::get_time_exp(Imdouble &a_o,Imdouble &b_o)
{
    a_o=-2.0*A*O*std::exp(-P*Im)*Im;
    b_o=-G+O*Im;
}

void dielec_critpoint::set(double Ai,double Oi,double P_i,double Gi)
{
    A=Ai; O=Oi; P=P_i; G=Gi;
}

void dielec_critpoint::show()
{
    std::cout<<"A: "<<A<<" O: "<<O<<" P: "<<P<<" G: "<<G<<std::endl;
}

void dielec_critpoint::operator = (dielec_critpoint const &D)
{
    A=D.A;
    O=D.O;
    P=D.P;
    G=D.G;
}
