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

#include <lua_material.h>
#include <lua_multilayers.h>

extern const double Pi;
extern const Imdouble Im;


//#########################
//       Multilayer
//#########################

Multilayer_Berreman_mode::Multilayer_Berreman_mode()
    :Nl(1000),
     lambda_min(400e-9), lambda_max(1000e-9),
     angle(0),
     index_sub(1.0), index_sup(1.0),
     output("Multilayer_Berreman_out")
{
}

#include <berreman_strat.h>

void script_multilayer(std::string const &name)
{
    std::string sname(name);
    std::string fname(name);
    
    sname.append("_show");
    fname.append("_show.m");
    
    std::ofstream file(fname,std::ios::out|std::ios::trunc);
    
    file<<"function out="<<sname<<"(varargin)"<<std::endl;
    file<<std::endl;
    file<<"field=0;"<<std::endl;
    file<<"power=0;"<<std::endl;
    file<<"power_avg=0;"<<std::endl;
    file<<"wavenumber=0;"<<std::endl;
    file<<std::endl;
    file<<"data=load('"<<name<<"');"<<std::endl;
    file<<std::endl;
    file<<"L=data(:,1);"<<std::endl;
    file<<std::endl;
    file<<"rte=data(:,3);"<<std::endl;
    file<<"arte=data(:,4);"<<std::endl;
    file<<"RTE=data(:,5);"<<std::endl;
    file<<std::endl;
    file<<"rtm=data(:,6);"<<std::endl;
    file<<"artm=data(:,7);"<<std::endl;
    file<<"RTM=data(:,8);"<<std::endl;
    file<<std::endl;
    file<<"tte=data(:,9);"<<std::endl;
    file<<"atte=data(:,10);"<<std::endl;
    file<<"TTE=data(:,11);"<<std::endl;
    file<<std::endl;
    file<<"ttm=data(:,12);"<<std::endl;
    file<<"attm=data(:,13);"<<std::endl;
    file<<"TTM=data(:,14);"<<std::endl;
    file<<std::endl;
    file<<"out=data;"<<std::endl;
    file<<std::endl;
    file<<"if nargin>0"<<std::endl;
    file<<"    for i=1:nargin"<<std::endl;
    file<<"        if strcmp(varargin{i},'field')==1"<<std::endl;
    file<<"            field=1;"<<std::endl;
    file<<"        end"<<std::endl;
    file<<"        if strcmp(varargin{i},'power')==1"<<std::endl;
    file<<"            power=1;"<<std::endl;
    file<<"        end"<<std::endl;
    file<<"        if strcmp(varargin{i},'power_avg')==1"<<std::endl;
    file<<"            power_avg=1;"<<std::endl;
    file<<"        end"<<std::endl;
    file<<"        if strcmp(varargin{i},'wavenumber')==1"<<std::endl;
    file<<"            L=1./(100*L);"<<std::endl;
    file<<"        end"<<std::endl;
    file<<"    end"<<std::endl;
    file<<"end"<<std::endl;
    file<<std::endl;
    file<<"min_L=min(L);"<<std::endl;
    file<<"max_L=max(L);"<<std::endl;
    file<<std::endl;
    file<<"N_fig=1;"<<std::endl;
    file<<std::endl;
    file<<"if field==1"<<std::endl;
    file<<"    figure(N_fig)"<<std::endl;
    file<<"    "<<std::endl;
    file<<"    plot(L,rte,L,rtm,L,tte,L,ttm);"<<std::endl;
    file<<"    legend('rte','rtm','tte','ttm')"<<std::endl;
    file<<"    title('Field')"<<std::endl;
    file<<"    xlim([min_L max_L])"<<std::endl;
    file<<"    "<<std::endl;
    file<<"    N_fig=N_fig+1;"<<std::endl;
    file<<"end"<<std::endl;
    file<<std::endl;
    file<<"if power==1"<<std::endl;
    file<<"    figure(N_fig)"<<std::endl;
    file<<"    "<<std::endl;
    file<<"    plot(L,RTE,L,RTM,L,TTE,L,TTM,L,1-RTE-TTE,L,1-RTM-TTM);"<<std::endl;
    file<<"    legend('RTE','RTM','TTE','TTM','ATE','ATM')"<<std::endl;
    file<<"    title('Power')"<<std::endl;
    file<<"    xlim([min_L max_L])"<<std::endl;
    file<<"    "<<std::endl;
    file<<"    N_fig=N_fig+1;"<<std::endl;
    file<<"end"<<std::endl;
    file<<std::endl;
    file<<"if power_avg==1"<<std::endl;
    file<<"    figure(N_fig)"<<std::endl;
    file<<"    "<<std::endl;
    file<<"    R=0.5*(RTE+RTM);"<<std::endl;
    file<<"    T=0.5*(TTE+TTM);"<<std::endl;
    file<<"    A=1-R-T;"<<std::endl;
    file<<"    "<<std::endl;
    file<<"    plot(L,R,L,T,L,A);"<<std::endl;
    file<<"    legend('R','T','A')"<<std::endl;
    file<<"    title('Power Average')"<<std::endl;
    file<<"    xlim([min_L max_L])"<<std::endl;
    file<<"    "<<std::endl;
    file<<"    N_fig=N_fig+1;"<<std::endl;
    file<<"end"<<std::endl;
    file<<std::endl;
    file<<"end"<<std::endl;
    
    file.close();
}

void Multilayer_Berreman_mode::process()
{
    int i,j;
    
    std::ofstream file(output,std::ios::out|std::ios::trunc);
    
    int N_layers=layer_h.size();
    
    B_strat strat(N_layers,index_sup,index_sub);
    
    lua_material::Loader loader;
    
    std::vector<Material> mats(N_layers);
    for(i=0;i<N_layers;i++)
    {
        loader.load(&mats[i],layer_mat[i]);
    }
    
    for(i=0;i<Nl;i++)
    {
        double lambda=lambda_min+(lambda_max-lambda_min)*i/(Nl-1.0);
        double w=2.0*Pi*c_light/lambda;
        
        for(j=0;j<N_layers;j++)
            strat.set_iso(j,layer_h[j],mats[j].get_eps(w),1.0);
        
        Imdouble r_TE,t_TE,r_TM,t_TM;
        
        strat.compute(lambda,angle,r_TE,r_TM,t_TE,t_TM);
        
        using std::abs;
        using std::arg;
        using std::cos;
        using std::norm;
        
        double ang_ref=Pi/2.0;
        double ang_ref_arg=index_sup/index_sub*std::sin(angle);
        
        if(ang_ref_arg<=1.0) ang_ref=std::asin(ang_ref_arg);
        
        double transm_coeff=cos(ang_ref)/cos(angle)*index_sub/index_sup;
        
        file<<lambda<<" "<<angle.radian()<<" ";
        
        file<<abs(r_TE)<<" "<<arg(r_TE)<<" "<<norm(r_TE)<<" "
            <<abs(r_TM)<<" "<<arg(r_TM)<<" "<<norm(r_TM)<<" "
            <<abs(t_TE)<<" "<<arg(t_TE)<<" "<<transm_coeff*norm(t_TE)<<" "
            <<abs(t_TM)<<" "<<arg(t_TM)<<" "<<transm_coeff*norm(t_TM)<<std::endl;
    }
    
    file.close();
    
    script_multilayer(output);
}

int multilayer_berr_mode_add_layer(lua_State *L)
{
    Multilayer_Berreman_mode **pp_ml=reinterpret_cast<Multilayer_Berreman_mode**>(lua_touserdata(L,1));
    
    double h=lua_tonumber(L,2);
    std::string mat=lua_tostring(L,3);
    
    (*pp_ml)->layer_h.push_back(h);
    (*pp_ml)->layer_mat.push_back(mat);
    
    return 0;
}

int multilayer_berr_mode_set_angle(lua_State *L)
{
    Multilayer_Berreman_mode **pp_ml=reinterpret_cast<Multilayer_Berreman_mode**>(lua_touserdata(L,1));
    
    (*pp_ml)->angle=Degree(lua_tonumber(L,2));
    
    return 0;
}

int multilayer_berr_mode_set_output(lua_State *L)
{
    Multilayer_Berreman_mode **pp_ml=reinterpret_cast<Multilayer_Berreman_mode**>(lua_touserdata(L,1));
    
    (*pp_ml)->output=lua_tostring(L,2);
    
    return 0;
}

int multilayer_berr_mode_set_spectrum(lua_State *L)
{
    Multilayer_Berreman_mode **pp_ml=reinterpret_cast<Multilayer_Berreman_mode**>(lua_touserdata(L,1));
    
    (*pp_ml)->Nl=lua_tointeger(L,2);
    (*pp_ml)->lambda_min=lua_tonumber(L,3);
    (*pp_ml)->lambda_max=lua_tonumber(L,4);
    
    return 0;
}

int multilayer_berr_mode_set_sub_index(lua_State *L)
{
    Multilayer_Berreman_mode **pp_ml=reinterpret_cast<Multilayer_Berreman_mode**>(lua_touserdata(L,1));
    
    (*pp_ml)->index_sub=lua_tonumber(L,2);
    
    return 0;
}

int multilayer_berr_mode_set_sup_index(lua_State *L)
{
    Multilayer_Berreman_mode **pp_ml=reinterpret_cast<Multilayer_Berreman_mode**>(lua_touserdata(L,1));
    
    (*pp_ml)->index_sup=lua_tonumber(L,2);
    
    return 0;
}

//#########################
//       Multilayer
//#########################

Multilayer_mode::Multilayer_mode()
    :Nl(1000),
     lambda_min(400e-9), lambda_max(1000e-9),
     angle(0),
     index_sub(1.0), index_sup(1.0),
     output("multilayer_out")
{
}

#include <phys_tools.h>

void Multilayer_mode::process()
{
    int i,j;
    
    std::ofstream file(output,std::ios::out|std::ios::trunc);
    
    int N_layers=layer_h.size();
    
    lua_material::Loader loader;
    
    std::vector<Material> mats(N_layers);
    for(i=0;i<N_layers;i++)
    {
        loader.load(&mats[i],layer_mat[i]);
    }
    
    Multilayer mlt(N_layers);
    
    mlt.set_environment(index_sup,index_sub);
    mlt.set_angle(angle);
    
    for(i=0;i<Nl;i++)
    {
        double lambda=lambda_min+(lambda_max-lambda_min)*i/(Nl-1.0);
        double w=2.0*Pi*c_light/lambda;
        
        mlt.set_lambda(lambda);
        
        for(j=0;j<N_layers;j++)
            mlt.set_layer(j,layer_h[j],mats[j].get_n(w));
        
        Imdouble r_TE,t_TE,r_TM,t_TM;
        
        mlt.compute(r_TE,r_TM,t_TE,t_TM);
        
        using std::abs;
        using std::arg;
        using std::cos;
        using std::norm;
        
        double ang_ref=Pi/2.0;
        double ang_ref_arg=index_sup/index_sub*std::sin(angle);
        
        if(ang_ref_arg<=1.0) ang_ref=std::asin(ang_ref_arg);
        
        double transm_coeff_TE=cos(ang_ref)/cos(angle)*index_sub/index_sup;
        double transm_coeff_TM=cos(ang_ref)/cos(angle)*index_sup/index_sub;
        
        file<<lambda<<" "<<angle.radian()<<" ";
        
        file<<abs(r_TE)<<" "<<arg(r_TE)<<" "<<norm(r_TE)<<" "
            <<abs(r_TM)<<" "<<arg(r_TM)<<" "<<norm(r_TM)<<" "
            <<abs(t_TE)<<" "<<arg(t_TE)<<" "<<transm_coeff_TE*norm(t_TE)<<" "
            <<abs(t_TM)<<" "<<arg(t_TM)<<" "<<transm_coeff_TM*norm(t_TM)<<std::endl;
    }
    
    file.close();
    
    script_multilayer(output);
}

int multilayer_mode_add_layer(lua_State *L)
{
    Multilayer_mode **pp_ml=reinterpret_cast<Multilayer_mode**>(lua_touserdata(L,1));
    
    double h=lua_tonumber(L,2);
    std::string mat=lua_tostring(L,3);
    
    (*pp_ml)->layer_h.push_back(h);
    (*pp_ml)->layer_mat.push_back(mat);
    
    return 0;
}

int multilayer_mode_set_angle(lua_State *L)
{
    Multilayer_mode **pp_ml=reinterpret_cast<Multilayer_mode**>(lua_touserdata(L,1));
    
    (*pp_ml)->angle=Degree(lua_tonumber(L,2));
    
    return 0;
}

int multilayer_mode_set_output(lua_State *L)
{
    Multilayer_mode **pp_ml=reinterpret_cast<Multilayer_mode**>(lua_touserdata(L,1));
    
    (*pp_ml)->output=lua_tostring(L,2);
    
    return 0;
}

int multilayer_mode_set_spectrum(lua_State *L)
{
    Multilayer_mode **pp_ml=reinterpret_cast<Multilayer_mode**>(lua_touserdata(L,1));
    
    (*pp_ml)->Nl=lua_tointeger(L,2);
    (*pp_ml)->lambda_min=lua_tonumber(L,3);
    (*pp_ml)->lambda_max=lua_tonumber(L,4);
    
    return 0;
}

int multilayer_mode_set_sub_index(lua_State *L)
{
    Multilayer_mode **pp_ml=reinterpret_cast<Multilayer_mode**>(lua_touserdata(L,1));
    
    (*pp_ml)->index_sub=lua_tonumber(L,2);
    
    return 0;
}

int multilayer_mode_set_sup_index(lua_State *L)
{
    Multilayer_mode **pp_ml=reinterpret_cast<Multilayer_mode**>(lua_touserdata(L,1));
    
    (*pp_ml)->index_sup=lua_tonumber(L,2);
    
    return 0;
}

//####################
//   Multilayer TMM
//####################

Multilayer_TMM_mode::Multilayer_TMM_mode()
    :mode(MODE_NONE),
     Nl(481),
     lambda_min(370e-9), lambda_max(850e-9),
     angle(0),
     output("multilayer_out"), polar("TE")
{
}

#include <phys_tools.h>

void Multilayer_TMM_mode::add_layer(double h,std::string mat)
{
    layer_h.push_back(h);
    layer_mat.push_back(mat);
}

void Multilayer_TMM_mode::compute_angle(double angle_)
{
    angle=Degree(angle_);
    mode=MODE_ANGLE;
}

void Multilayer_TMM_mode::compute_guided(std::string polar_,
                                         double lambda_guess_,
                                         double nr_guess_,
                                         double ni_guess_)
{
    polar=polar_;
    lambda_guess=lambda_guess_;
    nr_guess=nr_guess_;
    ni_guess=ni_guess_;
    mode=MODE_GUIDED;
}

void Multilayer_TMM_mode::set_output(std::string output_)
{
    output=output_;
}

void Multilayer_TMM_mode::set_spectrum(double lambda_min_,double lambda_max_,int Nl_)
{
    lambda_min=lambda_min_;
    lambda_max=lambda_max_;
    Nl=Nl_;
}

void Multilayer_TMM_mode::set_substrate(std::string mat)
{
    mat_sub_str=mat;
}

void Multilayer_TMM_mode::set_superstrate(std::string mat)
{
    mat_sup_str=mat;
}

void Multilayer_TMM_mode::process()
{
    int i,j;
    
    int N_layers=layer_h.size();
    
    Material mat_sup,mat_sub;
    
    lua_material::Loader ld;
    
    ld.load(&mat_sub,mat_sub_str);
    ld.load(&mat_sup,mat_sup_str);
    
    std::vector<Material> mats(N_layers);
    for(i=0;i<N_layers;i++)
    {
        ld.load(&mats[i],layer_mat[i]);
    }
    
    Multilayer_TMM mlt(N_layers);
        
    if(mode==MODE_ANGLE)
    {
        std::ofstream file(output,std::ios::out|std::ios::trunc);
        
        mlt.set_angle(angle);
        
        for(i=0;i<Nl;i++)
        {
            double lambda=lambda_min+(lambda_max-lambda_min)*i/(Nl-1.0);
            double w=2.0*Pi*c_light/lambda;
            
            mlt.set_lambda(lambda);
            
            mlt.set_environment(mat_sup.get_n(w),mat_sub.get_n(w));
            
            double index_sup=mat_sup.get_n(w).real();
            double index_sub=mat_sub.get_n(w).real();
            
            for(j=0;j<N_layers;j++)
                mlt.set_layer(j,layer_h[j],mats[j].get_n(w));
            
            Imdouble r_TE,t_TE,r_TM,t_TM;
            
            mlt.compute(r_TE,r_TM,t_TE,t_TM);
            
            using std::abs;
            using std::arg;
            using std::cos;
            using std::norm;
            
            double ang_ref=Pi/2.0;
            double ang_ref_arg=index_sup/index_sub*std::sin(angle);
            
            if(ang_ref_arg<=1.0) ang_ref=std::asin(ang_ref_arg);
            
            double transm_coeff_TE=cos(ang_ref)/cos(angle)*index_sub/index_sup;
            double transm_coeff_TM=cos(ang_ref)/cos(angle)*index_sup/index_sub;
            
            file<<lambda<<" "<<angle.radian()<<" ";
            
            file<<abs(r_TE)<<" "<<arg(r_TE)<<" "<<norm(r_TE)<<" "
                <<abs(r_TM)<<" "<<arg(r_TM)<<" "<<norm(r_TM)<<" "
                <<abs(t_TE)<<" "<<arg(t_TE)<<" "<<transm_coeff_TE*norm(t_TE)<<" "
                <<abs(t_TM)<<" "<<arg(t_TM)<<" "<<transm_coeff_TM*norm(t_TM)<<std::endl;
        }
        
        file.close();
        
        script_multilayer(output);
    }
    else if(mode==MODE_GUIDED)
    {
        std::vector<double> lambda(Nl);
        linspace(lambda,lambda_min,lambda_max);
        
        std::vector<Imdouble> n_eff(Nl);
        
        mlt.set_lambda(lambda_guess);
        double w=m_to_rad_Hz(lambda_guess);
        
        mlt.set_environment(mat_sup.get_n(w),mat_sub.get_n(w));
            
        for(j=0;j<N_layers;j++)
            mlt.set_layer(j,layer_h[j],mats[j].get_n(w));
        
        Imdouble n_eff_target=0;
        
        if(polar=="TE") n_eff_target=mlt.nearest_zero_chara_TE_MC(nr_guess+ni_guess*Im,0.1,0.1,1e-20,100);
        else n_eff_target=mlt.nearest_zero_chara_TM_MC(nr_guess+ni_guess*Im,0.1,0.1,1e-20,100);
        
        // Nearest point in the spectrum
        
        int k;
        double u;
        
        vector_locate_linear(k,u,lambda,lambda_guess);
        
        mlt.set_lambda(lambda[k]);
        w=m_to_rad_Hz(lambda[k]);
        
        mlt.set_environment(mat_sup.get_n(w),mat_sub.get_n(w));
            
        for(j=0;j<N_layers;j++)
            mlt.set_layer(j,layer_h[j],mats[j].get_n(w));
        
        if(polar=="TE") n_eff[k]=mlt.nearest_zero_chara_TE_MC(n_eff_target,0.1,0.1,1e-20,100);
        else n_eff[k]=mlt.nearest_zero_chara_TM_MC(n_eff_target,0.1,0.1,1e-20,100);
        
        // Bottom part of the spectrum
        
        for(int i=k-1;i>=0;i--) // Not unsigned because of overflow
        {
            mlt.set_lambda(lambda[i]);
            w=m_to_rad_Hz(lambda[i]);
            
            mlt.set_environment(mat_sup.get_n(w),mat_sub.get_n(w));
                
            for(j=0;j<N_layers;j++)
                mlt.set_layer(j,layer_h[j],mats[j].get_n(w));
            
            if(polar=="TE") n_eff[i]=mlt.nearest_zero_chara_TE_MC(n_eff[i+1],0.1,0.1,1e-20,100);
            else n_eff[i]=mlt.nearest_zero_chara_TM_MC(n_eff[i+1],0.1,0.1,1e-20,100);
        }
        
        // Top part of the spectrum
        
        for(unsigned int i=k+1;i<lambda.size();i++)
        {
            mlt.set_lambda(lambda[i]);
            w=m_to_rad_Hz(lambda[i]);
            
            mlt.set_environment(mat_sup.get_n(w),mat_sub.get_n(w));
                
            for(j=0;j<N_layers;j++)
                mlt.set_layer(j,layer_h[j],mats[j].get_n(w));
            
            if(polar=="TE") n_eff[i]=mlt.nearest_zero_chara_TE_MC(n_eff[i-1],0.1,0.1,1e-20,100);
            else n_eff[i]=mlt.nearest_zero_chara_TM_MC(n_eff[i-1],0.1,0.1,1e-20,100);
        }
        
        std::ofstream file(output,std::ios::out|std::ios::trunc);
        
        for(unsigned int i=0;i<lambda.size();i++)
        {
            file<<lambda[i]<<" "
                <<n_eff[i].real()<<" "
                <<n_eff[i].imag()<<std::endl;
        }
        
        file.close();
    }
}

void create_metatables_multilayers(lua_State *L)
{
    create_obj_metatable(L,"metatable_multilayer");
    
    metatable_add_func(L,"add_layer",multilayer_mode_add_layer);
    metatable_add_func(L,"angle",multilayer_mode_set_angle);
    metatable_add_func(L,"output",multilayer_mode_set_output);
    metatable_add_func(L,"spectrum",multilayer_mode_set_spectrum);
    metatable_add_func(L,"substrate_index",multilayer_mode_set_sub_index);
    metatable_add_func(L,"superstrate_index",multilayer_mode_set_sup_index);
    
    create_obj_metatable(L,"metatable_multilayer_berreman");
    
    metatable_add_func(L,"add_layer",multilayer_berr_mode_add_layer);
    metatable_add_func(L,"angle",multilayer_berr_mode_set_angle);
    metatable_add_func(L,"output",multilayer_berr_mode_set_output);
    metatable_add_func(L,"spectrum",multilayer_berr_mode_set_spectrum);
    metatable_add_func(L,"substrate_index",multilayer_berr_mode_set_sub_index);
    metatable_add_func(L,"superstrate_index",multilayer_berr_mode_set_sup_index);
    
    create_obj_metatable(L,"metatable_multilayer_tmm");
    
    lua_wrapper<0,Multilayer_TMM_mode,double,std::string>::bind(L,"add_layer",&Multilayer_TMM_mode::add_layer);
    lua_wrapper<1,Multilayer_TMM_mode,double>::bind(L,"compute_angle",&Multilayer_TMM_mode::compute_angle);
    lua_wrapper<2,Multilayer_TMM_mode,std::string,double,double,double>::bind(L,"compute_guided",&Multilayer_TMM_mode::compute_guided);
    lua_wrapper<3,Multilayer_TMM_mode,std::string>::bind(L,"output",&Multilayer_TMM_mode::set_output);
    lua_wrapper<4,Multilayer_TMM_mode,double,double,int>::bind(L,"spectrum",&Multilayer_TMM_mode::set_spectrum);
    lua_wrapper<5,Multilayer_TMM_mode,std::string>::bind(L,"substrate",&Multilayer_TMM_mode::set_substrate);
    lua_wrapper<6,Multilayer_TMM_mode,std::string>::bind(L,"superstrate",&Multilayer_TMM_mode::set_superstrate);
}
