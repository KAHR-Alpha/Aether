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

#include <sensors.h>


extern const Imdouble Im;
extern std::ofstream plog;

//###################################
// Planar Integrated Poynting sensor 
//###################################

ModesDcpl::ModesDcpl(int type_,
                     int x1_,int x2_,
                     int y1_,int y2_,
                     int z1_,int z2_)
{
    step=0;
    
    set_type(type_);
    set_loc(x1_,x2_,y1_,y2_,z1_,z2_);
    
    if(type==NORMAL_X){ span=z2-z1; }
    if(type==NORMAL_Y){ span=z2-z1; }
    if(type==NORMAL_Z){ span=y2-y1; }
}

ModesDcpl::~ModesDcpl()
{
}

void ModesDcpl::deep_feed(Grid3<double> const &Ex,Grid3<double> const &Ey,Grid3<double> const &Ez,
                     Grid3<double> const &Hx,Grid3<double> const &Hy,Grid3<double> const &Hz)
{
    int j,k,l;
    
    double SEy,SHy;
    
    if(type==NORMAL_X)
    {
        for(l=0;l<Nl;l++)
        {
            double w=2.0*Pi*c_light/lambda[l];
            
            Imdouble coeff=std::exp(w*step*Dt*Im);
                        
            for(k=z1;k<z2;k++)
            {
                SEy=SHy=0;
                for(j=y1;j<y2;j++)
                {
                    SEy+=Ey(x1,j,k);
                    SHy+=Hy(x1,j,k);
                }
                
                sp_Ey(k-z1,l)+=SEy*coeff;
                sp_Hy(k-z1,l)+=SHy*coeff;
            }
        }
    }
}

void ModesDcpl::set_spectrum(std::vector<double> const &lambda_)
{
    Sensor::set_spectrum(lambda_);
    
    sp_Ey.init(span,Nl,0);
    sp_Hy.init(span,Nl,0);
}

//void ModesDcpl::set_spectrum(Grid1<double> const &lambda_in)
//{
//    Sensor::set_spectrum(lambda_in);
//    
//    sp_Ey.init(span,Nl,0);
//    sp_Hy.init(span,Nl,0);
//}

void ModesDcpl::treat(std::string fname_out,Slab_guide &slab,int ng_ref)
{
    int i,l;
    
    double lambda_min=lambda[0];
    for(std::size_t i=1;i<lambda.size();i++) lambda_min=std::min(lambda_min,lambda[0]);
    
//    lambda.min();
    slab.compute_guiding_char(lambda_min);
    
    int max_TE=slab.n_TE;
    int max_TM=slab.n_TM;
    
    Grid1<double> z_Ey(span,0), z_Hy(span,0);
    Grid1<Imdouble> t_Ey(span,0), t_Hy(span,0);
    
    for(i=0;i<span;i++)
    {
        z_Ey[i]=Dz*(i+z1+0.5-ng_ref);
        z_Hy[i]=Dz*(i+z1+0.75-ng_ref);
    }
    
    std::ofstream file(fname_out.c_str(),std::ios::out|std::ios::trunc);
        
    ProgDisp dsp(Nl,"Computing modes sensor results");
    
    for(l=0;l<Nl;l++)
    {
        Grid1<Imdouble> coeff_TE,coeff_TM;
        
        for(i=0;i<span;i++)
        {
            t_Ey[i]=sp_Ey(i,l);
            t_Hy[i]=sp_Hy(i,l);
        }
        
        double resi_TE,resi_TM;
        
        //modes_dcpl(lambda[l],slab,t_Ey,z_Ey,t_Hy,z_Hy,coeff_TE,coeff_TM);
        modes_dcpl(lambda[l],slab,t_Ey,z_Ey,t_Hy,z_Hy,coeff_TE,coeff_TM,resi_TE,resi_TM);
        
        file<<lambda[l]<<" ";
        
//        int Npad_TE=max_TE-coeff_TE.L1();
//                
//        for(i=0;i<coeff_TE.L1();i++) file<<std::abs(coeff_TE[i])<<" ";
//        for(i=0;i<Npad_TE;i++) file<<0<<" ";
//        
//        int Npad_TM=max_TM-coeff_TM.L1();
//        
//        for(i=0;i<coeff_TM.L1();i++) file<<std::abs(coeff_TM[i])<<" ";
//        for(i=0;i<Npad_TM;i++) file<<0<<" ";
        
        int Npad_TE=max_TE-coeff_TE.L1();
        
        for(i=0;i<coeff_TE.L1();i++)
        {
            double En=gd_mode_energy_simp(lambda[l],slab.beta_TE[i],slab,"TE");
            file<<En*std::norm(coeff_TE[i])<<" ";
        }
        for(i=0;i<Npad_TE;i++) file<<0<<" ";
        
        
        int Npad_TM=max_TM-coeff_TM.L1();
        
        for(i=0;i<coeff_TM.L1();i++)
        {
            double En=gd_mode_energy_simp(lambda[l],slab.beta_TM[i],slab,"TM");
            file<<En*std::norm(coeff_TM[i])<<" ";
        }
        for(i=0;i<Npad_TM;i++) file<<0<<" ";
        file<<resi_TE<<" "<<resi_TM<<std::endl;
        
        ++dsp;
    }
    
    file.close();
}
