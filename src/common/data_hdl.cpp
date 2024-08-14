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

#include <bitmap3.h>
#include <data_hdl.h>
#include <logger.h>

extern const Imdouble Im;

//###############
//   SpAngH
//###############

SpAng::SpAng()
    :lambda(500e-9),
     NEE(0),
     NEM(0),
     NME(0),
     NMM(0)
{
}

SpAng::SpAng(SpAng const & hld)
    :lambda(hld.lambda),
     NEE(hld.NEE),
     NEM(hld.NEM),
     NME(hld.NME),
     NMM(hld.NMM),
     d_EE(hld.d_EE), d_EM(hld.d_EM), d_ME(hld.d_ME), d_MM(hld.d_MM),
     a_EE(hld.a_EE), a_EM(hld.a_EM), a_ME(hld.a_ME), a_MM(hld.a_MM)
{
}

void SpAng::show()
{
    int l;
    
    Plog::print(lambda ,"\n");
    Plog::print("EE\n");
    for(l=0;l<NEE;l++) Plog::print(a_EE[l], " ", d_EE[l], "\n");
    Plog::print("EM\n");
    for(l=0;l<NEM;l++) Plog::print(a_EM[l], " ", d_EM[l], "\n");
    Plog::print("ME\n");
    for(l=0;l<NME;l++) Plog::print(a_ME[l], " ", d_ME[l], "\n");
    Plog::print("MM\n");
    for(l=0;l<NMM;l++) Plog::print(a_MM[l], " ", d_MM[l], "\n");
}

void SpAng::operator = (SpAng const &hld)
{
    lambda=hld.lambda;
    NEE=hld.NEE;
    NEM=hld.NEM;
    NME=hld.NME;
    NMM=hld.NMM;
    d_EE=hld.d_EE; d_EM=hld.d_EM; d_ME=hld.d_ME; d_MM=hld.d_MM;
    a_EE=hld.a_EE; a_EM=hld.a_EM; a_ME=hld.a_ME; a_MM=hld.a_MM;
}

//###############
//   Spectrum
//###############

Spectrum::Spectrum()
    :N(0)
{
}

Spectrum::Spectrum(int N_in)
    :N(N_in),
     source_pol("TE"),
     resp_pol("TE"),
     type("ref"),
     lambda(N,0),
     ang(N,0),
     spect(N,0)
{
}

Spectrum::Spectrum(Spectrum const &sp)
    :N(sp.N),
     source_pol(sp.source_pol),
     resp_pol(sp.resp_pol),
     type(sp.type),
     lambda(sp.lambda),
     ang(sp.ang),
     spect(sp.spect)
{
}

Spectrum::Spectrum(std::vector<double> const &lambda_,
                   double ang_,
                   std::vector<Imdouble> const &spect_)
    :N(lambda_.size()),
     source_pol("TE"),
     resp_pol("TE"),
     type("ref"),
     lambda(lambda_),
     ang(N,ang_),
     spect(spect_)
{
    if(N!=spect.size())
    {
        chk_msg("Incompatible data, impossible to set the Spectrum type, crash imminent");
    }
}

Spectrum::Spectrum(std::vector<double> const &lambda_,
                   std::vector<AngleRad> const &ang_,
                   std::vector<Imdouble> const &spect_)
    :N(lambda_.size()),
     source_pol("TE"),
     resp_pol("TE"),
     type("ref"),
     lambda(lambda_),
     ang(ang_),
     spect(spect_)
{    
    if(N!=ang.size() || N!=spect.size())
    {
        chk_msg("Incompatible data, impossible to set the Spectrum type, crash imminent");
    }
}

bool Spectrum::is_in(double l)
{
    double lambda_min=lambda[0];
    double lambda_max=lambda[0];
    
    for(unsigned int i=0;i<lambda.size();i++)
    {
        lambda_min=std::min(lambda[i],lambda_min);
        lambda_max=std::max(lambda[i],lambda_max);
    }
    
    if(l<lambda_min || l>lambda_max) return false;
    
    return true;
}

bool Spectrum::is_in_f(double f)
{
    double l=c_light/f;
    return is_in(l);
}

bool Spectrum::is_in_w(double w)
{
    double l=2.0*Pi*c_light/w;
    return is_in(l);
}

void Spectrum::get(double l,double &ang_o,Imdouble &spect_o)
{
    int k=0;
    double u=0;
    
    vector_locate_linear(k,u,lambda,l);
    
    ang_o=(1.0-u)*ang[k]+u*ang[k+1];
    spect_o=(1.0-u)*spect[k]+u*spect[k+1];
}

void Spectrum::get_f(double f,double &ang_o,Imdouble &spect_o)
{
    double l=c_light/f;
    get(l,ang_o,spect_o);
}

void Spectrum::get_w(double w,double &ang_o,Imdouble &spect_o)
{
    double l=2.0*Pi*c_light/w;
    get(l,ang_o,spect_o);
}

double Spectrum::max_amp()
{
    double R=0;
    
    for(unsigned int l=0;l<N;l++) R=std::max(R,std::abs(spect[l]));
    
    return R;
}

void Spectrum::get_polarizations(std::string &s_pol,std::string &r_pol)
{
    s_pol=source_pol;
    r_pol=resp_pol;
}

void Spectrum::set_polarizations(std::string const &s_pol,std::string const &r_pol)
{
    source_pol=s_pol;
    resp_pol=r_pol;
}

void Spectrum::set_type_ref() { type="ref"; }
void Spectrum::set_type_trans() { type="trans"; }

void Spectrum::operator = (Spectrum const &sp)
{
    N=sp.N;
    source_pol=sp.source_pol;
    resp_pol=sp.resp_pol;
    type=sp.type;
    
    lambda.resize(N); lambda=sp.lambda;
    ang.resize(N);    ang=sp.ang;
    spect.resize(N);  spect=sp.spect;
}

std::ostream & operator<< (std::ostream &out,Spectrum const &sp)
{
    unsigned int l;
    
    out<<sp.N<<std::endl;
    out<<sp.source_pol<<" "<<sp.resp_pol<<" "<<sp.type<<std::endl;
    
    for(l=0;l<sp.N;l++)
    {
        out<<sp.lambda[l]<<" "
           <<sp.ang[l]<<" "
           <<std::real(sp.spect[l])<<" "
           <<std::imag(sp.spect[l])<<std::endl;
    }
    
    return out;
}

std::istream & operator>> (std::istream &in,Spectrum &sp)
{
    unsigned int l;
    
    in>>sp.N;
    in>>sp.source_pol;
    in>>sp.resp_pol;
    in>>sp.type;
    
    if(sp.N>0)
    {
        sp.lambda.resize(sp.N);
        sp.ang.resize(sp.N);
        sp.spect.resize(sp.N);
        
        double a,b;
        
        for(l=0;l<sp.N;l++)
        {
            in>>sp.lambda[l];
            
            in>>a;
            sp.ang[l]=a;
            
            in>>a;
            in>>b;
            
            sp.spect[l]=a+b*Im;
        }
    }
    
    return in;
}

//###############
//###############

SpectrumCollec::SpectrumCollec()
    :N_EE(0),
     N_EM(0),
     N_MM(0),
     N_ME(0)
{
}

void SpectrumCollec::add_collec(SpectrumCollec const &sp_col)
{
    int i,j;
    
    for(i=0;i<4;i++)
    {
        int const *pN=0;
        std::vector<Spectrum *> const *pS=0;
        
        if(i==0) { pN=&sp_col.N_EE; pS=&sp_col.sp_EE; }
        if(i==1) { pN=&sp_col.N_EM; pS=&sp_col.sp_EM; }
        if(i==2) { pN=&sp_col.N_MM; pS=&sp_col.sp_MM; }
        if(i==3) { pN=&sp_col.N_ME; pS=&sp_col.sp_ME; }
        
        for(j=0;j<*pN;j++)
        {
            Spectrum &sp=*((*pS)[j]);
            
            add_spectrum(sp);
        }
    }
}

void SpectrumCollec::add_spectrum(Spectrum const &sp)
{
    int *N_t=0; //N target
    std::vector<Spectrum *> *sp_t=0; //spectrum target;
    
    if(sp.source_pol=="TE")
    {
        if(sp.resp_pol=="TE") { N_t=&N_EE; sp_t=&sp_EE; }
        if(sp.resp_pol=="TM") { N_t=&N_EM; sp_t=&sp_EM; }
    }
    if(sp.source_pol=="TM")
    {
        if(sp.resp_pol=="TE") { N_t=&N_ME; sp_t=&sp_ME; }
        if(sp.resp_pol=="TM") { N_t=&N_MM; sp_t=&sp_MM; }
    }
    
    if(N_t==0) return;
    
    int &N=*N_t;
    std::vector<Spectrum*> &S=*sp_t; //to make things easier to read
        
    S.push_back(new Spectrum);
    *(S[N])=sp;
    
    *N_t+=1;
}

bool SpectrumCollec::check_polrot()
{
    int l;
    double R=0;
    
    for(l=0;l<N_EM;l++)
    {
        R=std::max(R,sp_EM[l]->max_amp());
    }
    for(l=0;l<N_ME;l++)
    {
        R=std::max(R,sp_ME[l]->max_amp());
    }
    
    if(R>=0.01) return 1;
    else return 0;
}

Spectrum SpectrumCollec::get_spectrum_EE(int n)
{
    return *sp_EE[n];
}

Spectrum SpectrumCollec::get_spectrum_EM(int n)
{
    return *sp_EM[n];
}

Spectrum SpectrumCollec::get_spectrum_MM(int n)
{
    return *sp_MM[n];
}

Spectrum SpectrumCollec::get_spectrum_ME(int n)
{
    return *sp_ME[n];
}
        
void SpectrumCollec::extract_data(SpAng &data,std::string type,double ang_max)
{
    int l;
    
    double lambda=data.lambda;
    double tmp_ang;
    Imdouble tmp_sp;
        
    for(l=0;l<N_EE;l++)
    {
        Spectrum &sp=*sp_EE[l];
        
        if(sp.type==type && sp.is_in(lambda))
        {
            sp.get(lambda,tmp_ang,tmp_sp);
            
            if(tmp_ang<=ang_max)
            {
                data.d_EE.push_back(tmp_sp);
                data.a_EE.push_back(tmp_ang);
                data.NEE+=1;
            }
        }
    }
    for(l=0;l<N_EM;l++)
    {
        Spectrum &sp=*sp_EM[l];
        
        if(sp.type==type && sp.is_in(lambda))
        {
            sp.get(lambda,tmp_ang,tmp_sp);
            
            if(tmp_ang<=ang_max)
            {
                data.d_EM.push_back(tmp_sp);
                data.a_EM.push_back(tmp_ang);
                data.NEM+=1;
            }
        }
    }
    for(l=0;l<N_ME;l++)
    {
        Spectrum &sp=*sp_ME[l];
        
        if(sp.type==type && sp.is_in(lambda))
        {
            sp.get(lambda,tmp_ang,tmp_sp);
            
            if(tmp_ang<=ang_max)
            {
                data.d_ME.push_back(tmp_sp);
                data.a_ME.push_back(tmp_ang);
                data.NME+=1;
            }
        }
    }
    for(l=0;l<N_MM;l++)
    {
        Spectrum &sp=*sp_MM[l];
        
        if(sp.type==type && sp.is_in(lambda))
        {
            sp.get(lambda,tmp_ang,tmp_sp);
            
            if(tmp_ang<=ang_max)
            {
                data.d_MM.push_back(tmp_sp);
                data.a_MM.push_back(tmp_ang);
                data.NMM+=1;
            }
        }
    }
    
}

void SpectrumCollec::read(std::string fname)
{
    std::ifstream file(fname.c_str(),std::ios::in);
    
    file>>*this;
    
    file.close();
}

void SpectrumCollec::write(std::string fname)
{
    std::ofstream file(fname.c_str(),std::ios::out|std::ios::trunc);
    
    file<<*this;
    
    file.close();
}

std::ostream & operator<< (std::ostream &out,SpectrumCollec const &sp_col)
{
    int i,j;
    
    out<<sp_col.N_EE<<std::endl;
    out<<sp_col.N_EM<<std::endl;
    out<<sp_col.N_MM<<std::endl;
    out<<sp_col.N_ME<<std::endl;
    
    for(i=0;i<4;i++)
    {
        int const *pN=0;
        std::vector<Spectrum *> const *pS=0;
        
        if(i==0) { pN=&sp_col.N_EE; pS=&sp_col.sp_EE; }
        if(i==1) { pN=&sp_col.N_EM; pS=&sp_col.sp_EM; }
        if(i==2) { pN=&sp_col.N_MM; pS=&sp_col.sp_MM; }
        if(i==3) { pN=&sp_col.N_ME; pS=&sp_col.sp_ME; }
        
        for(j=0;j<*pN;j++)
        {
            Spectrum const &sp=*((*pS)[j]);
            out<<sp<<std::endl;
        }
    }
    
    return out;
}

std::istream & operator>> (std::istream &in,SpectrumCollec &sp_col)
{
    int i,j;
    
    in>>sp_col.N_EE;
    in>>sp_col.N_EM;
    in>>sp_col.N_MM;
    in>>sp_col.N_ME;
    
    for(i=0;i<4;i++)
    {
        int *pN=0;
        std::vector<Spectrum *> *pS=0;
        
        if(i==0) { pN=&sp_col.N_EE; pS=&sp_col.sp_EE; }
        if(i==1) { pN=&sp_col.N_EM; pS=&sp_col.sp_EM; }
        if(i==2) { pN=&sp_col.N_MM; pS=&sp_col.sp_MM; }
        if(i==3) { pN=&sp_col.N_ME; pS=&sp_col.sp_ME; }
        
        for(j=0;j<*pN;j++)
        {
            pS->push_back(new Spectrum);
            Spectrum &sp=*((*pS)[j]);
            
            in>>sp;
        }
    }
    
    return in;
}
