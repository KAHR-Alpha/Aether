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

#include <filehdl.h>
#include <material.h>
#include <phys_tools.h>

extern std::ofstream plog;
extern const double Pi;
extern const Imdouble Im;

Material::Material()
    :eps_inf(1.0),
     lambda_valid_min(400e-9),
     lambda_valid_max(1000e-9),
     is_effective_material(false), // Effective material
     effective_type(EffectiveModel::BRUGGEMAN),
     description(""), // String descriptions
     script_path("")
{
}

Material::Material(Material const &mat)
    :eps_inf(mat.eps_inf),
     lambda_valid_min(mat.lambda_valid_min),
     lambda_valid_max(mat.lambda_valid_max),
     debye(mat.debye),
     drude(mat.drude),
     lorentz(mat.lorentz),
     critpoint(mat.critpoint),
     cauchy_coeffs(mat.cauchy_coeffs),
     sellmeier_B(mat.sellmeier_B),
     sellmeier_C(mat.sellmeier_C),
     spd_lambda(mat.spd_lambda),
     spd_r(mat.spd_r),
     spd_i(mat.spd_i),
     spd_type_index(mat.spd_type_index),
     er_spline(mat.er_spline),
     ei_spline(mat.ei_spline),
     is_effective_material(mat.is_effective_material), // Effective material
     effective_type(mat.effective_type),
     name(mat.name), // String descriptions
     description(mat.description),
     script_path(mat.script_path)
{
    // TODO
//    if(is_effective_material)
//    {
//        eff_mat_1=new Material;
//        eff_mat_2=new Material;
//        
//        *eff_mat_1=*mat.eff_mat_1;
//        *eff_mat_2=*mat.eff_mat_2;
//    }
}

Material::Material(std::filesystem::path const &script_path_)
    :Material()
{
    load_lua_script(script_path_);
}

Material::~Material()
{
    // TODO
//    if(eff_mat_1!=nullptr) delete eff_mat_1;
//    if(eff_mat_2!=nullptr) delete eff_mat_2;
}

void Material::add_spline_data(std::vector<double> const &lambda,
                               std::vector<double> const &data_r,
                               std::vector<double> const &data_i,
                               bool type_index)
{
    spd_lambda.push_back(lambda);
    spd_r.push_back(data_r);
    spd_i.push_back(data_i);
    spd_type_index.push_back(type_index);
    
    std::size_t i,Nl=lambda.size();
    
    std::vector<double> w(Nl),er(Nl),ei(Nl);
    
    for(i=0;i<Nl;i++)
        w[i]=m_to_rad_Hz(lambda[i]); // Back to radians
    
    er=data_r;
    ei=data_i;
        
    if(type_index) // Index to permittivity
    {
        for(i=0;i<Nl;i++)
        {
            Imdouble eps=er[i]+ei[i]*Im;
            eps=eps*eps;
            
            er[i]=eps.real();
            ei[i]=eps.imag();
        }
    }
    
    if(w[0]>w[Nl-1])
    {
        for(i=0;i<Nl/2;i++)
        {
            std::swap(w[i],w[Nl-1-i]);
            std::swap(er[i],er[Nl-1-i]);
            std::swap(ei[i],ei[Nl-1-i]);
        }
    }
    
    Cspline sp_er,sp_ei;
    
    sp_er.init(w,er);
    sp_ei.init(w,ei);
    
    er_spline.push_back(sp_er);
    ei_spline.push_back(sp_ei);
}

bool Material::fdtd_compatible()
{
    if(is_effective_material) return false;
    else
    {
        if(   cauchy_coeffs.empty()
           && sellmeier_B.empty()
           && er_spline.empty()) return true;
        else return false;
    }
    
    return false;
}

Imdouble Material::get_eps(double w)
{
    if(!is_effective_material)
    {
        std::size_t i;
        
        double lambda=rad_Hz_to_m(w);
        double lambda_2=lambda*lambda;
        
        Imdouble eps_out=eps_inf;
        
        // Common dielectric models
        
        for(i=0;i<debye.size();i++) eps_out+=debye[i].eval(w);
        for(i=0;i<drude.size();i++) eps_out+=drude[i].eval(w);
        for(i=0;i<lorentz.size();i++) eps_out+=lorentz[i].eval(w);
        for(i=0;i<critpoint.size();i++) eps_out+=critpoint[i].eval(w);
        
        // Cauchy terms
        
        for(i=0;i<cauchy_coeffs.size();i++)
        {
            double lambda_2n=1.0;
            double n=0;
            
            for(std::size_t j=0;j<cauchy_coeffs[i].size();j++)
            {
                n+=cauchy_coeffs[i][j]/lambda_2n;
                lambda_2n*=lambda_2;
            }
            
            eps_out+=n*n;
        }
        
        // Sellmeier terms
        
        for(i=0;i<sellmeier_B.size();i++)
        {
            double CL=sellmeier_C[i]/lambda;
            eps_out+=sellmeier_B[i]/(1.0-CL*CL);
        }
        
        // Spline
        
        for(i=0;i<er_spline.size();i++)
            eps_out+=(er_spline[i])(w)+(ei_spline[i])(w)*Im;
         
        return eps_out;
    }
    else
    {
        eff_eps.resize(eff_mats.size());
        
        for(std::size_t i=0;i<eff_mats.size();i++)
        {
            eff_eps[i]=eff_mats[i]->get_eps(w);
        }
        
        switch(effective_type)
        {
            case EffectiveModel::BRUGGEMAN:
                return effmodel_bruggeman(eff_eps,eff_weights);
            case EffectiveModel::LOOYENGA:
                return effmodel_looyenga(eff_eps,eff_weights);
            case EffectiveModel::MAXWELL_GARNETT:
                return effmodel_maxwell_garnett(eff_eps,eff_weights,maxwell_garnett_host);
            case EffectiveModel::SUM:
                return effmodel_sum(eff_eps,eff_weights);
            case EffectiveModel::SUM_INV:
                return effmodel_inv_sum(eff_eps,eff_weights);
        }
    }
    
    return 1.0;
}

/*std::string Material::get_description() const
{
    std::string out;
    
    if(is_const()) { out="n="; out.append(std::to_string(std::sqrt(eps_inf))); }
    else out=script_path.generic_string();
//    else if(type==MAT_CONST_IM) return eps_inf+eps_inf_im*Im;
//    else if(type==MAT_DIELEC) { out="model: "; out.append(script_name); }
//    else if(type==MAT_SPLINE) { out="spline: "; out.append(script_name); }
//    else if(type==MAT_SELLMEIER) { out="sellmeier: "; out.append(script_name); }
    
    return out;
}*/

std::string Material::get_matlab(std::string const &fname_) const
{
    std::filesystem::path fname(fname_);
    
    std::stringstream strm;
    
    strm<<"%    Input as a wavelength in meters\n";
    strm<<"%\n";
    strm<<"%    Example:\n";
    strm<<"%\n";
    strm<<"%       lambda=linspace("<<lambda_valid_min<<','<<lambda_valid_max<<",601);\n";
    strm<<"%       eps="<<fname.stem().generic_string()<<"(lambda);\n";
    strm<<"%\n";
    strm<<"%    Optional parameters after the wavelength:\n";
    strm<<"%\n";
    strm<<"%       'index': returns the refractive index instead of the permittivity\n";
    strm<<"%       'display': displays the index or permittivity in a new figure\n";
    strm<<"%\n";
    strm<<"%       Example: eps="<<fname.stem().generic_string()<<"(lambda,'display');\n";
    strm<<"%\n";
    strm<<"%    Description:\n";
    strm<<"%\n";
    
    strm<<"%    ";
    
    for(unsigned int i=0;i<description.length();i++)
    {
             if(description[i]=='\r') continue;
        else if(description[i]=='\n') strm<<"\n%    ";
        else strm<<description[i];
    }
    
    strm<<"\n\n";
    
    strm<<"function out="<<fname.stem().generic_string()<<"(lambda,varargin)\n\n";
    
    strm<<"% Lambda checking\n\n";
    
    if(!is_const())
    {
        strm<<"validity_min="<<lambda_valid_min<<";\n";
        strm<<"validity_max="<<lambda_valid_max<<";\n\n";

        strm<<"filter=find((lambda<validity_min)|(lambda>validity_max));\n\n";

        strm<<"if numel(filter)>0\n";
        strm<<"    disp(['WARNING: ' num2str(numel(filter)) ' requested wavelengths outside of the validity range']);\n";
        strm<<"    disp(['Min Lambda: ' num2str(validity_min)]);\n";
        strm<<"    disp(['Max Lambda: ' num2str(validity_max)]);\n";
        strm<<"end\n\n";
    }
    
    strm<<"% Model\n\n";
    
    strm<<"eps=0*lambda+"<<eps_inf<<";\n\n";
    strm<<"w=2*pi*299792458./lambda;\n";
    strm<<"lambda_2=lambda.^2;\n\n";
    
    std::size_t i;
    
    // Dielectric models
    
    for(i=0;i<debye.size();i++)
    {
        strm<<debye[i].get_matlab(i)<<"\n";
        strm<<"eps=eps+"<<debye[i].matlab_ID(i)<<";\n\n";
    }
    for(i=0;i<drude.size();i++)
    {
        strm<<drude[i].get_matlab(i)<<"\n";
        strm<<"eps=eps+"<<drude[i].matlab_ID(i)<<";\n\n";
    }
    for(i=0;i<lorentz.size();i++)
    {
        strm<<lorentz[i].get_matlab(i)<<"\n";
        strm<<"eps=eps+"<<lorentz[i].matlab_ID(i)<<";\n\n";
    }
    for(i=0;i<critpoint.size();i++)
    {
        strm<<critpoint[i].get_matlab(i)<<"\n";
        strm<<"eps=eps+"<<critpoint[i].matlab_ID(i)<<";\n\n";
    }
    
    // Cauchy terms
        
    for(i=0;i<cauchy_coeffs.size();i++)
    {
        for(std::size_t j=0;j<cauchy_coeffs[i].size();j++)
            strm<<"cauchy_"<<i<<"_"<<j<<"="<<cauchy_coeffs[i][j]<<";\n";
        
        strm<<"cauchy_"<<i<<"_eps=cauchy_"<<i<<"_"<<0;
        for(std::size_t j=1;j<cauchy_coeffs[i].size();j++)
        {
            strm<<"+cauchy_"<<i<<"_"<<j<<"./(lambda.^"<<2*j<<")";
        }
        strm<<";\n";
        strm<<"eps=eps+cauchy_"<<i<<".^2;\n\n";
    }
    
    // Sellmeier terms
    
    if(!sellmeier_B.empty())
    {
        for(i=0;i<sellmeier_B.size();i++)
        {
            strm<<"sellmeier_B_"<<i<<"="<<sellmeier_B[i]<<"; ";
            strm<<"sellmeier_C_"<<i<<"="<<sellmeier_C[i]<<";\n";
        }
    
        strm<<"\neps=eps";
        for(i=0;i<sellmeier_B.size();i++)
            strm<<"+sellmeier_B_"<<i<<"./(1.0-(sellmeier_C_"<<i<<"./lambda).^2)";
        strm<<";\n\n";
    }
    
    // Spline
    
    for(i=0;i<er_spline.size();i++)
    {
        // Real data
        
        std::size_t N=er_spline[i].get_N();
        
        strm<<"spline_"<<i<<"_er_w=[";
        for(std::size_t j=0;j<N;j++)
        {
            strm<<er_spline[i].get_x_base(j);
            if(j+1<N) strm<<" ";
        }
        strm<<"];\n";
        
        strm<<"spline_"<<i<<"_er=[";
        for(std::size_t j=0;j<N;j++)
        {
            strm<<er_spline[i].get_y_base(j);
            if(j+1<N) strm<<" ";
        }
        strm<<"];\n\n";
        
        // Imag data
        
        N=ei_spline[i].get_N();
        
        strm<<"spline_"<<i<<"_ei_w=[";
        for(std::size_t j=0;j<N;j++)
        {
            strm<<ei_spline[i].get_x_base(j);
            if(j+1<N) strm<<" ";
        }
        strm<<"];\n";
        
        strm<<"spline_"<<i<<"_ei=[";
        for(std::size_t j=0;j<N;j++)
        {
            strm<<ei_spline[i].get_y_base(j);
            if(j+1<N) strm<<" ";
        }
        strm<<"];\n\n";
        
        strm<<"spline_"<<i<<"_eps=spline(spline_"<<i<<"_er_w,spline_"<<i<<"_er,w)";
                       strm<<"+1i*spline(spline_"<<i<<"_ei_w,spline_"<<i<<"_ei,w)\n";
        strm<<"eps=eps+spline_"<<i<<"_eps;\n";
    }
    
    strm<<"% Options checking\n\n";
    
    strm<<"op_display=0;\n";
    strm<<"op_index=0;\n\n";
    
    strm<<"for k=1:(nargin-1)\n";
    strm<<"    if strcmp(varargin{k},'display')==1\n";
    strm<<"        op_display=1;\n";
    strm<<"    end\n";
    strm<<"    if strcmp(varargin{k},'index')==1\n";
    strm<<"        op_index=1;\n";
    strm<<"    end\n";
    strm<<"end\n\n";
    
    strm<<"if op_index\n";
    strm<<"    out=sqrt(eps);\n";
    strm<<"else\n";
    strm<<"    out=eps;\n";
    strm<<"end\n\n";
    
    strm<<"if op_display\n";
    strm<<"    figure(111);\n";
    strm<<"    h=plot(lambda,real(out),lambda,imag(out));\n";
    strm<<"    xlabel('Wavelength (m)')\n";
    strm<<"    xlim([min(lambda) max(lambda)])\n\n";
    
    strm<<"    set(h(1),'LineWidth',2,'Color',[0,0,1]);\n";
    strm<<"    set(h(2),'LineWidth',2,'Color',[1,0,0]);\n\n";
    
    strm<<"    if op_index\n";
    strm<<"        legend('real(n)','imag(n)')\n";
    strm<<"    else\n";
    strm<<"        legend('real(eps)','imag(eps)')\n";
    strm<<"    end\n";
    strm<<"end\n\n";
    
    strm<<"end";
    
    return strm.str();
}

Imdouble Material::get_n(double w)
{
    return std::sqrt(get_eps(w));
}

bool Material::is_const() const
{
    if(is_effective_material)
    {
        // TODO
//        if(eff_mat_1->is_const() && eff_mat_2->is_const()) return true;
//        else return false;
        
        return false;
    }
    else
    {
        if(   debye.empty()
           && drude.empty()
           && lorentz.empty()
           && critpoint.empty()
           && cauchy_coeffs.empty()
           && sellmeier_B.empty()
           && er_spline.empty()) return true;
        else return false;
    }
}

void Material::operator = (Material const &mat)
{
    eps_inf=mat.eps_inf;
    
    lambda_valid_min=mat.lambda_valid_min;
    lambda_valid_max=mat.lambda_valid_max;
    
    debye=mat.debye;
    drude=mat.drude;
    lorentz=mat.lorentz;
    critpoint=mat.critpoint;
    
    name=mat.name;
    description=mat.description;
    script_path=mat.script_path;
    
    cauchy_coeffs=mat.cauchy_coeffs;
    
    sellmeier_B=mat.sellmeier_B;
    sellmeier_C=mat.sellmeier_C;
    
    spd_lambda=mat.spd_lambda;
    spd_r=mat.spd_r;
    spd_i=mat.spd_i;
    
    spd_type_index=mat.spd_type_index;
    
    er_spline=mat.er_spline;
    ei_spline=mat.ei_spline;
    
    // TODO
    
    is_effective_material=mat.is_effective_material;
    effective_type=mat.effective_type;
//    eff_weight=mat.eff_weight;
//    
//    if(is_effective_material)
//    {
//        eff_mat_1=new Material;
//        eff_mat_2=new Material;
//        
//        *eff_mat_1=*mat.eff_mat_1;
//        *eff_mat_2=*mat.eff_mat_2;
//    }
}

bool Material::operator == (Material const &mat) const
{
    if(eps_inf!=mat.eps_inf) return false;
    
    // Common dielectric models
    
    if(debye!=mat.debye) return false;
    if(drude!=mat.drude) return false;
    if(lorentz!=mat.lorentz) return false;
    if(critpoint!=mat.critpoint) return false;
        
    // Cauchy
    if(cauchy_coeffs!=mat.cauchy_coeffs) return false;
    
    // Sellmeier
    if(sellmeier_B!=mat.sellmeier_B) return false;
    if(sellmeier_C!=mat.sellmeier_C) return false;
    
    // For file-based materials
    if(spd_lambda!=mat.spd_lambda) return false;
    if(spd_r!=mat.spd_r) return false;
    if(spd_i!=mat.spd_i) return false;
    if(spd_type_index!=mat.spd_type_index) return false;
    
    if(er_spline!=mat.er_spline) return false;
    if(ei_spline!=mat.ei_spline) return false;
    
    // Effective Material
    
    if(is_effective_material!=mat.is_effective_material) return false;
    
    if(is_effective_material)
    {
        if(effective_type!=mat.effective_type) return false;
        
        if(eff_weights!=mat.eff_weights) return false; // checks vecvtor size as well
        
        bool null_check=true;
        
        for(std::size_t i=0;i<eff_mats.size();i++)
        {
            null_check=null_check && (eff_mats[i]!=nullptr && mat.eff_mats[i]!=nullptr);
        }
        
        if(!null_check) return false;
        else
        {
            for(std::size_t i=0;i<eff_mats.size();i++)
            {
                if(*(eff_mats[i])!=*(mat.eff_mats[i])) return false;
            }
        }
    }
    
    bool path1_check=std::filesystem::exists(script_path);
    bool path2_check=std::filesystem::exists(mat.script_path);
    
    if(path1_check==path2_check)
    {
        if(path1_check)
        {
            if(!std::filesystem::equivalent(script_path,mat.script_path)) return false;
        }
    }
    else return false;
    
    return true;
}

bool Material::operator!=(Material const &mat) const
{
    return !(*this==mat);
}

void Material::reset()
{
    eps_inf=1.0;
    
    lambda_valid_min=400e-9;
    lambda_valid_max=1000e-9;
    
    debye.clear();
    drude.clear();
    lorentz.clear();
    critpoint.clear();
    
    cauchy_coeffs.clear();
    
    sellmeier_B.clear();
    sellmeier_C.clear();
    
    spd_lambda.clear();
    spd_r.clear();
    spd_i.clear();
    
    spd_type_index.clear();
    
    er_spline.clear();
    ei_spline.clear();
    
    if(is_effective_material)
    {
        is_effective_material=false;
        
        effective_type=EffectiveModel::BRUGGEMAN;
        
        // TODO
        
//        delete eff_mat_1;
//        delete eff_mat_2;
//        
//        eff_mat_1=nullptr;
//        eff_mat_2=nullptr;
        
        is_effective_material=false;
        
//        eff_weight=0;
    }
    
    name="";
    description="";
    script_path="";
}

void Material::set_const_eps(double eps_)
{
    eps_inf=eps_;
}

void Material::set_const_n(double n)
{
    eps_inf=n*n;
}

//######################
//   Effective Models
//######################

Imdouble effmodel_bruggeman(Imdouble eps_1,Imdouble eps_2,
                            double weight_1,double weight_2)
{
    if(weight_1<=0) return eps_2;
    else if(weight_2<=0) return eps_1;
    
    Imdouble a=-2.0*(weight_1+weight_2);
    Imdouble b=2.0*(weight_1*eps_1+weight_2*eps_2)-weight_1*eps_2-weight_2*eps_1;
    Imdouble c=(weight_1+weight_2)*eps_1*eps_2;
    
    Imdouble sq=std::sqrt(b*b/(4.0*a*a)-c/a);
    
    Imdouble R=-b/(2.0*a)+sq;
    
    if(R.imag()<0) R=-b/(2.0*a)-sq;
    
    return R;
}

Imdouble effmodel_looyenga(Imdouble eps_1,Imdouble eps_2,
                           double weight_1,double weight_2)
{
    double v2=weight_2/(weight_1+weight_2);
    
    Imdouble p_eps_1=std::pow(eps_1,1.0/3.0);
    Imdouble p_eps_2=std::pow(eps_2,1.0/3.0);
    
    return std::pow((p_eps_2-p_eps_1)*v2+p_eps_1,3.0);
}

Imdouble effmodel_maxwell_garnett_1(Imdouble eps_1,Imdouble eps_2,
                                    double weight_1,double weight_2)
{
    Imdouble eps_h=eps_1;
    
    Imdouble fact=weight_1*(eps_1-eps_h)/(eps_1+2.0*eps_h)
                 +weight_2*(eps_2-eps_h)/(eps_2+2.0*eps_h);
    
    return eps_h*(1.0+2.0*fact)/(1.0-fact);
}

Imdouble effmodel_maxwell_garnett_2(Imdouble eps_1,Imdouble eps_2,
                                    double weight_1,double weight_2)
{
    Imdouble eps_h=eps_2;
    
    Imdouble fact=weight_1*(eps_1-eps_h)/(eps_1+2.0*eps_h)
                 +weight_2*(eps_2-eps_h)/(eps_2+2.0*eps_h);
    
    return eps_h*(1.0+2.0*fact)/(1.0-fact);
}

Imdouble effmodel_sum(Imdouble eps_1,Imdouble eps_2,
                      double weight_1,double weight_2)
{
    return weight_1*eps_1+weight_2*eps_2;
}

Imdouble effmodel_sum_inv(Imdouble eps_1,Imdouble eps_2,
                      double weight_1,double weight_2)
{
    return 1.0/(weight_1/eps_1+weight_2/eps_2);
}

Imdouble effmodel_bruggeman(std::vector<Imdouble> const &eps,
                            std::vector<double> const &weights)
{
    double weight_sum=weights[0]+weights[1];
    
    return effmodel_bruggeman(eps[0],eps[1],weights[0]/weight_sum,weights[1]/weight_sum);
}

Imdouble effmodel_looyenga(std::vector<Imdouble> const &eps,
                           std::vector<double> const &weights)
{
    double weight_sum=weights[0]+weights[1];
    
    return effmodel_looyenga(eps[0],eps[1],weights[0]/weight_sum,weights[1]/weight_sum);
}

Imdouble effmodel_maxwell_garnett(std::vector<Imdouble> const &eps,
                                  std::vector<double> const &weights,int host)
{
    Imdouble eps_host=eps[host];
    
    Imdouble factor=0;
    double w_sum=0;
    
    for(std::size_t i=0;i<eps.size();i++)
    {
        factor+=weights[i]*(eps[i]-eps_host)/(eps[i]+2.0*eps_host);
        w_sum+=weights[i];
    }
    
    factor/=w_sum;
    
    Imdouble eps_mg=eps_host*(1.0+2.0*factor)/(1.0-factor);
    return eps_mg;
}

Imdouble effmodel_sum(std::vector<Imdouble> const &eps,
                      std::vector<double> const &weights)
{    
    Imdouble eps_r=0;
    double w_sum=0;
    
    for(std::size_t i=0;i<eps.size();i++)
    {
        eps_r+=weights[i]*eps[i];
        w_sum+=weights[i];
    }
    
    eps_r/=w_sum;
    
    return eps_r;
}

Imdouble effmodel_inv_sum(std::vector<Imdouble> const &eps,
                          std::vector<double> const &weights)
{    
    Imdouble inv_eps_r=0;
    double w_sum=0;
    
    for(std::size_t i=0;i<eps.size();i++)
    {
        inv_eps_r+=weights[i]/eps[i];
        w_sum+=weights[i];
    }
    
    inv_eps_r/=w_sum;
    
    return 1.0/inv_eps_r;
}
