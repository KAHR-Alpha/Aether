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

extern const Imdouble Im;

extern std::ofstream plog;

//###############
// Dielec model
//###############

Dielec_model::Dielec_model()
    :eps_inf(1.0)
{
}

Dielec_model::Dielec_model(Dielec_model const &D)
    :eps_inf(D.eps_inf),
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
    eps_inf=1.0;
    debye_arr.clear();
    drude_arr.clear();
    lorentz_arr.clear();
    cp_arr.clear();
}

Imdouble Dielec_model::eval(double w) const
{
    std::size_t i;
    Imdouble r=0;
    
    r+=eps_inf;
    for(i=0;i<debye_arr.size();i++) r+=debye_arr[i].eval(w);
    for(i=0;i<drude_arr.size();i++) r+=drude_arr[i].eval(w);
    for(i=0;i<lorentz_arr.size();i++) r+=lorentz_arr[i].eval(w);
    for(i=0;i<cp_arr.size();i++) r+=cp_arr[i].eval(w);
    
    return r;
}

double Dielec_model::get_const()
{
    return eps_inf;
}

void Dielec_model::get_time_exp(Grid1<Imdouble> &va,Grid1<Imdouble> &vb)
{
    Imdouble tmp_a,tmp_b;
    
    int Nmod=get_N_models();
    
    va.init(Nmod);
    vb.init(Nmod);
    
    std::size_t i;
    int j=0;
    
    for(i=0;i<debye_arr.size();i++)
    {
        debye_arr[i].get_time_exp(tmp_a,tmp_b);
        va[j]=tmp_a; vb[j]=tmp_b;
        j++;
    }
    for(i=0;i<drude_arr.size();i++)
    {
        drude_arr[i].get_time_exp(tmp_a,tmp_b);
        va[j]=tmp_a; vb[j]=tmp_b;
        j++;
    }
    for(i=0;i<lorentz_arr.size();i++)
    {
        lorentz_arr[i].get_time_exp(tmp_a,tmp_b);
        va[j]=tmp_a; vb[j]=tmp_b;
        j++;
    }
    for(i=0;i<cp_arr.size();i++)
    {
        cp_arr[i].get_time_exp(tmp_a,tmp_b);
        va[j]=tmp_a; vb[j]=tmp_b;
        j++;
    }
}

std::string Dielec_model::get_matlab() const
{
    std::stringstream strm;
    
    strm<<"eps=0*w+"<<eps_inf<<";\n";
    
    int l=0;
    
    for(std::size_t i=0;i<debye_arr.size();i++) { strm<<debye_arr[i].get_matlab(l); l++; }
    for(std::size_t i=0;i<drude_arr.size();i++) { strm<<drude_arr[i].get_matlab(l); l++; }
    for(std::size_t i=0;i<lorentz_arr.size();i++) { strm<<lorentz_arr[i].get_matlab(l); l++; }
    for(std::size_t i=0;i<cp_arr.size();i++) { strm<<cp_arr[i].get_matlab(l); l++; }
    
    strm<<"eps=eps+";
    
    for(int i=0;i<l;i++)
    {
        strm<<"eps_"<<i;
        
        if(i<l-1) strm<<"+";
        else strm<<";\n\n";
    }
    
    return strm.str();
}

int Dielec_model::get_N_models()
{
    return debye_arr.size()+
           drude_arr.size()+
           lorentz_arr.size()+
           cp_arr.size();
}

double Dielec_model::get_sigma()
{
    double sig=0;
    
    for(std::size_t i=0;i<drude_arr.size();i++)
    {
        sig+=drude_arr[i].get_sigma();
    }
    
    return sig;
}

void Dielec_model::show()
{
    std::size_t i;
    
    std::cout<<"Eps_inf: "<<eps_inf<<std::endl;
    for(i=0;i<debye_arr.size();i++)
    {
        std::cout<<"Debye "<<i<<":"<<std::endl;
        debye_arr[i].show();
    }
    for(i=0;i<drude_arr.size();i++)
    {
        std::cout<<"Drude "<<i<<":"<<std::endl;
        drude_arr[i].show();
    }
    for(i=0;i<lorentz_arr.size();i++)
    {
        std::cout<<"Lorentz "<<i<<":"<<std::endl;
        lorentz_arr[i].show();
    }
    for(i=0;i<cp_arr.size();i++)
    {
        std::cout<<"Critical-points "<<i<<":"<<std::endl;
        cp_arr[i].show();
    }
}

void Dielec_model::operator = (Dielec_model const &D)
{
    eps_inf=D.eps_inf;
    
    debye_arr=D.debye_arr;
    drude_arr=D.drude_arr;
    lorentz_arr=D.lorentz_arr;
    cp_arr=D.cp_arr;
}

//####################
//####################

//Debye

DebyeModel::DebyeModel()
    :ds(0), t0(0)
{}

DebyeModel::DebyeModel(DebyeModel const &D)
    :ds(D.ds), t0(D.t0)
{}

std::string DebyeModel::get_matlab(int ID) const
{
    std::stringstream strm;
    
    strm<<"ds_"<<ID<<"="<<ds<<";\n";
    strm<<"t0_"<<ID<<"="<<t0<<";\n\n";
    
    strm<<matlab_ID(ID)<<"=ds_"<<ID<<"./(1.0-w*t0_"<<ID<<"*i);";
    
    return strm.str();
}

Imdouble DebyeModel::eval(double w) const
{
    return ds/(1.0-w*t0*Im);
}

void DebyeModel::get_time_exp(Imdouble &a_o,Imdouble &b_o)
{
    a_o=ds/t0;
    b_o=-1.0/t0;
}

std::string DebyeModel::matlab_ID(int ID) const
{
    return "eps_debye_"+std::to_string(ID);
}

void DebyeModel::set(double dsi,double t0i)
{
    ds=dsi; t0=t0i;
}

void DebyeModel::show()
{
    std::cout<<"Ds: "<<ds<<" t0: "<<t0<<std::endl;
}

void DebyeModel::operator = (DebyeModel const &D)
{
    ds=D.ds;
    t0=D.t0;
}

bool DebyeModel::operator == (DebyeModel const &D) const
{
    return ds==D.ds && t0==D.t0;
}

//Drude

DrudeModel::DrudeModel()
    :wd(0), wd2(0), g(1.0)
{}

DrudeModel::DrudeModel(DrudeModel const &D)
    :wd(D.wd), wd2(D.wd2), g(D.g)
{}

Imdouble DrudeModel::eval(double w) const
{
    return -wd2/(w*w+w*g*Im);
}

std::string DrudeModel::get_matlab(int ID) const
{
    std::stringstream strm;
    
    strm<<"wd2_"<<ID<<"="<<wd2<<";\n";
    strm<<"g_"<<ID<<"="<<g<<";\n\n";
    
    strm<<matlab_ID(ID)<<"=-wd2_"<<ID<<"./(w.^2+w*g_"<<ID<<"*i);";
    
    return strm.str();
}

void DrudeModel::get_time_exp(Imdouble &a_o,Imdouble &b_o)
{
    a_o=-wd2/g;
    b_o=-g;
}

double DrudeModel::get_sigma()
{
    return e0*wd2/g;
}

std::string DrudeModel::matlab_ID(int ID) const
{
    return "eps_drude_"+std::to_string(ID);
}

void DrudeModel::set(double wd_,double g_)
{
    wd=wd_;
    g=g_;
    
    wd2=wd*wd;
}

void DrudeModel::show()
{
    std::cout<<"Wd: "<<wd<<" Wd2: "<<wd2<<" g: "<<g<<std::endl;
}

void DrudeModel::operator = (DrudeModel const &D)
{
    wd=D.wd;
    wd2=D.wd2;
    g=D.g;
}

bool DrudeModel::operator == (DrudeModel const &D) const
{
    return wd==D.wd && g==D.g;
}

//Lorentz

LorentzModel::LorentzModel()
    :A(0), O(1.0), O2(1.0), G(0)
{}

LorentzModel::LorentzModel(LorentzModel const &D)
    :A(D.A), O(D.O), O2(D.O2), G(D.G)
{}

Imdouble LorentzModel::eval(double w) const
{
    return A*O2/(O2-w*w-w*G*Im);
}

std::string LorentzModel::get_matlab(int ID) const
{
    std::stringstream strm;
    
    strm<<"A_"<<ID<<"="<<A<<";\n";
    strm<<"O2_"<<ID<<"="<<O2<<";\n";
    strm<<"G_"<<ID<<"="<<G<<";\n\n";
    
    strm<<matlab_ID(ID)<<"=A_"<<ID<<"*O2_"<<ID<<"./(O2_"<<ID<<"-w.^2-w*G_"<<ID<<"*i);";
    
    return strm.str();
}

void LorentzModel::get_time_exp(Imdouble &a_o,Imdouble &b_o)
{
    double alp=G/2.0;
    Imdouble bet=std::sqrt(O*O-alp*alp);
    Imdouble eta=A*O*O/bet;
    
    a_o=-eta*Im;
    b_o=-alp+bet*Im;
}

std::string LorentzModel::matlab_ID(int ID) const
{
    return "eps_lorentz_"+std::to_string(ID);
}

void LorentzModel::set(double Ai,double Oi,double Gi)
{
    A=Ai; O=Oi; G=Gi;
    O2=O*O;
}

void LorentzModel::show()
{
    std::cout<<"A: "<<A<<" O: "<<O<<" O2: "<<O2<<" G: "<<G<<std::endl;
}

void LorentzModel::operator =(LorentzModel const &D)
{
    A=D.A;
    O=D.O;
    O2=D.O2;
    G=D.G;
}

bool LorentzModel::operator == (LorentzModel const &D) const
{
    return A==D.A && O==D.O && G==D.G;
}

//Critical points

CritpointModel::CritpointModel()
    :A(0), O(1), P(0), G(1)
{}

CritpointModel::CritpointModel(CritpointModel const &D)
    :A(D.A), O(D.O), P(D.P), G(D.G)
{}

std::string CritpointModel::get_matlab(int ID) const
{
    std::stringstream strm;
    
    strm<<"A_"<<ID<<"="<<A<<";\n";
    strm<<"O_"<<ID<<"="<<O<<";\n";
    strm<<"P_"<<ID<<"="<<P<<";\n";
    strm<<"G_"<<ID<<"="<<G<<";\n\n";
    
    strm<<matlab_ID(ID)<<"=A_"<<ID<<"*O_"<<ID<<"*(exp(P_"<<ID<<"*i)./(O_"<<ID<<"-w-G_"<<ID<<"*i)+exp(-P_"<<ID<<"*i)./(O_"<<ID<<"+w+G_"<<ID<<"*i));";
    
    return strm.str();
}

Imdouble CritpointModel::eval(double w) const
{
    using std::exp;
    return A*O*(exp(P*Im)/(O-w-G*Im)+exp(-P*Im)/(O+w+G*Im));
}

void CritpointModel::get_time_exp(Imdouble &a_o,Imdouble &b_o)
{
    a_o=-2.0*A*O*std::exp(-P*Im)*Im;
    b_o=-G+O*Im;
}

std::string CritpointModel::matlab_ID(int ID) const
{
    return "eps_critpoints_"+std::to_string(ID);
}

void CritpointModel::set(double Ai,double Oi,double P_i,double Gi)
{
    A=Ai; O=Oi; P=P_i; G=Gi;
}

void CritpointModel::show()
{
    std::cout<<"A: "<<A<<" O: "<<O<<" P: "<<P<<" G: "<<G<<std::endl;
}

void CritpointModel::operator = (CritpointModel const &D)
{
    A=D.A;
    O=D.O;
    P=D.P;
    G=D.G;
}

bool CritpointModel::operator == (CritpointModel const &D) const
{
    return    A==D.A
           && O==D.O
           && P==D.P
           && G==D.G;
}
