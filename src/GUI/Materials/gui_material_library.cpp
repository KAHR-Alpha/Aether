/*Copyright 2008-2023 - Loïc Le Cunff

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
#include <gui_material.h>
#include <lua_material.h>

bool default_material_validator(Material *material) { return true; }

// Lua bindings

namespace GUI
{
    
    //##############
    //   Material
    //##############
    
    Material::Material()
        :type(MatType::CUSTOM),
         original_requester(nullptr)
    {
    }
    
    std::string Material::get_description()
    {
        std::stringstream strm;
        
             if(type==MatType::REAL_N)
        {
            strm<<"Const refractive index: ";
            strm<<std::sqrt(eps_inf);
        }
        else if(   type==MatType::LIBRARY
                || type==MatType::USER_LIBRARY
                || type==MatType::SCRIPT)
        {
            strm<<"Library material: ";
            strm<<script_path.generic_string();
        }
        else if(type==MatType::CUSTOM)
        {
            strm<<"Custom material: ";
            if(!drude.empty()) strm<<"drude ";
            if(!debye.empty()) strm<<"debye ";
            if(!lorentz.empty()) strm<<"lorentz ";
            if(!critpoint.empty()) strm<<"critpoint ";
            if(!cauchy_coeffs.empty()) strm<<"cauchy ";
            if(!sellmeier_B.empty()) strm<<"sellmeier ";
            if(!spd_lambda.empty()) strm<<"tabulated ";
        }
        else if(type==MatType::EFFECTIVE)
        {
            strm<<"Effective material";
        }
        
        return strm.str();
    }
    
    double Material::get_lambda_validity_min()
    {
             if(type==MatType::REAL_N) return 1e-100;
        else if(type==MatType::EFFECTIVE)
        {
            double lmin=eff_mats[0]->lambda_valid_min;
            
            for(::Material *mat: eff_mats)
                lmin=std::max(lmin,mat->lambda_valid_min);
                
            return lmin;
        }
        else return lambda_valid_min;
    }

    double Material::get_lambda_validity_max()
    {
             if(type==MatType::REAL_N) return 1e100;
        else if(type==MatType::EFFECTIVE)
        {
            double lmax=eff_mats[0]->lambda_valid_max;
            
            for(::Material *mat: eff_mats)
                lmax=std::min(lmax,mat->lambda_valid_max);
                
            return lmax;
        }
        else return lambda_valid_max;
    }
    
    std::string Material::get_short_description()
    {
        std::stringstream out;
        
        switch(type)
        {
            case MatType::REAL_N:
                out<<std::sqrt(eps_inf);
                break;
            case MatType::LIBRARY:
                out<<script_path.generic_string();
                break;
            case MatType::SCRIPT:
                out<<script_path.generic_string();
                break;
            case MatType::USER_LIBRARY:
                out<<script_path.generic_string();
                break;
            case MatType::EFFECTIVE:
                {
                    switch(effective_type)
                    {
                        case EffectiveModel::BRUGGEMAN: out<<"Brugg"; break;
                        case EffectiveModel::MAXWELL_GARNETT: out<<"MG"; break;
                        case EffectiveModel::LOOYENGA: out<<"Loy"; break;
                        case EffectiveModel::SUM: out<<"Sum"; break;
                        case EffectiveModel::SUM_INV: out<<"ISum"; break;
                    }
                    
                    for(std::size_t i=0;i<eff_mats.size();i++)
                    {
                        out<<" | ";
                        
                        if(!(eff_mats[i]->name.empty()))
                        {
                            out<<eff_mats[i]->name;
                        }
                        else if(!(eff_mats[i]->script_path.empty()))
                        {
                            out<<eff_mats[i]->script_path.stem().generic_string();
                        }
                        else if(eff_mats[i]->is_const())
                        {
                            out<<std::real(eff_mats[i]->get_n(0));
                        }
                        else
                        {
                            out<<"?";
                        }
                    }
                }
                break;
        }
    
        return out.str();
    }
}

namespace lua_gui_material
{
    void create_metatable(lua_State *L)
    {
        lua_material::create_metatable(L);
        
        lua_register(L,"Material",&allocate); // overload
        metatable_add_func(L,"refractive_index",lua_material_set_index); // overload
        metatable_add_func(L,"load_script",lua_material_set_script); // overload
    }
    
    int allocate(lua_State *L)
    {
        GUI::Material *p_material=MaterialsLib::request_material(MatType::CUSTOM);
        
        lua_set_metapointer<::Material>(L,"metatable_material",p_material); // polymorphic allocation
        
        if(lua_gettop(L)>0)
        {
                 if(lua_isnumber(L,1)) p_material->set_const_n(lua_tonumber(L,1));
            else if(lua_isstring(L,1)) p_material->load_lua_script(lua_tostring(L,1));
        }
        
        MaterialsLib::consolidate(p_material);
        
        return 1;
    }
    
    int lua_material_set_index(lua_State *L)
    {
        Material *base_mat=lua_get_metapointer<Material>(L,1);
        GUI::Material *mat=dynamic_cast<GUI::Material*>(base_mat);
        
        mat->set_const_n(lua_tonumber(L,2));
        mat->type=MatType::REAL_N;
        
        return 0;
    }
    
    int lua_material_set_script(lua_State *L)
    {
        Material *base_mat=lua_get_metapointer<Material>(L,1);
        GUI::Material *mat=dynamic_cast<GUI::Material*>(base_mat);
        
        mat->load_lua_script(lua_tostring(L,2));
        mat->type=MatType::SCRIPT;
        
        return 0;
    }
    
    //################
    //   Translator
    //################
    
    Translator::Translator(std::filesystem::path const &relative_path_)
        :finalized(false),
         relative_path(relative_path_)
    {
    }
    
    void Translator::finalize()
    {
        if(finalized) return; 
        
        std::stringstream strm;
        
        // Mapping pointer - names
        
        for(std::size_t i=0;i<materials.size();i++)
        {
            name_map[materials[i]]="Material_" + std::to_string(i);
        }
        
        // Materials declaration
        
        for(std::size_t i=0;i<materials.size();i++)
        {
            strm<<name_map[materials[i]]<<"=Material()\n";
        }
        
        strm<<"\n";
        
        // Materials definition
        
        for(std::size_t i=0;i<materials.size();i++)
        {
            to_lua(materials[i],strm,name_map[materials[i]]+":");
            
            strm<<"\n";
        }
        
        header=strm.str();
        
        finalized=true;
    }
    
    void Translator::gather(GUI::Material *material)
    {
        if(!vector_contains(materials,material))
        {
            materials.push_back(material);
        }
    }
    
    std::string Translator::get_header()
    {
        if(!finalized) finalize();
        
        return header;
    }
    
    std::string Translator::operator() (GUI::Material *material) const
    {
        return name_map.at(material);
    }
    
    void Translator::save_to_file(GUI::Material *material)
    {
        std::ofstream file(material->script_path,std::ios::out|std::ios::binary|std::ios::trunc);
        
        to_lua(material,file,"");
    }
    
    void Translator::to_lua(GUI::Material *material,std::ostream &strm,std::string const &prefix)
    {
        std::size_t i;
        
        MatType type=material->type;
        
        if(!material->name.empty()) strm<<prefix<<"name(\""<<material->name<<"\")\n";
        
        if(type==MatType::REAL_N)
        {
            strm<<prefix<<"refractive_index("<<std::real(material->get_n(0))<<")\n";
        }
        else if(   type==MatType::LIBRARY
                || type==MatType::USER_LIBRARY
                || type==MatType::SCRIPT)
        {
            strm<<prefix<<"load_script(\""<<material->script_path.generic_string()<<"\")\n";
        }
        else if(type==MatType::EFFECTIVE)
        {
            strm<<prefix<<"effective_material(\"";
            
            switch(material->effective_type)
            {
                case EffectiveModel::BRUGGEMAN:
                    strm<<"bruggeman";
                    break;
                    
                case EffectiveModel::LOOYENGA:
                    strm<<"loyenga";
                    break;
                    
                case EffectiveModel::MAXWELL_GARNETT:
                    strm<<"maxwell_garnett";
                    break;
                    
                case EffectiveModel::SUM:
                    strm<<"sum";
                    break;
                    
                case EffectiveModel::SUM_INV:
                    strm<<"inverse_sum";
                    break;
            }
            
            strm<<"\",";
            
            for(i=0;i<material->eff_mats.size();i++)
            {
                if(vector_contains(materials,dynamic_cast<GUI::Material*>(material->eff_mats[i])))
                {
                    strm<<name_map[dynamic_cast<GUI::Material*>(material->eff_mats[i])];
                }
                else
                {
                    strm<<"\""<<material->eff_mats[i]->script_path.generic_string()<<"\"";
                }
                strm<<",";
                strm<<material->eff_weights[i];
            }
            
            strm<<")\n";
        }
        else if(type==MatType::CUSTOM)
        {
            if(!material->description.empty()) strm<<prefix<<"description(\""<<material->description<<"\")\n";
            
            strm<<prefix<<"validity_range("<<material->lambda_valid_min<<","<<material->lambda_valid_max<<")\n";
            strm<<prefix<<"epsilon_infinity("<<material->eps_inf<<")\n";
            
            for(i=0;i<material->debye.size();i++)
                strm<<prefix<<"add_debye("<<material->debye[i].ds<<","
                                          <<material->debye[i].t0<<")\n";
                
            for(i=0;i<material->drude.size();i++)
                strm<<prefix<<"add_drude("<<material->drude[i].wd<<","
                                          <<material->drude[i].g<<")\n";
                
            for(i=0;i<material->lorentz.size();i++)
                strm<<prefix<<"add_lorentz("<<material->lorentz[i].A<<","
                                            <<material->lorentz[i].O<<","
                                            <<material->lorentz[i].G<<")\n";

            for(i=0;i<material->critpoint.size();i++)
                strm<<prefix<<"add_critpoint("<<material->critpoint[i].A<<","
                                              <<material->critpoint[i].O<<","
                                              <<material->critpoint[i].P<<","
                                              <<material->critpoint[i].G<<")\n";

            for(i=0;i<material->cauchy_coeffs.size();i++)
            {
                strm<<prefix<<"add_cauchy(";
                
                for(std::size_t j=0;j<material->cauchy_coeffs[i].size();j++)
                {
                    strm<<prefix<<material->cauchy_coeffs[i][j];
                    if(j+1!=material->cauchy_coeffs[i].size()) strm<<prefix<<",";
                }
                
                strm<<prefix<<")\n";
            }

            for(i=0;i<material->sellmeier_B.size();i++)
                strm<<prefix<<"add_sellmeier("<<material->sellmeier_B[i]<<","<<material->sellmeier_C[i]<<")\n";
                
            for(i=0;i<material->spd_lambda.size();i++)
            {
                strm<<prefix<<"lambda={";
                for(std::size_t j=0;j<material->spd_lambda[i].size();j++)
                {
                    strm<<prefix<<material->spd_lambda[i][j];
                    
                    if(j+1<material->spd_lambda[i].size()) strm<<prefix<<",";
                    else strm<<prefix<<"}\n";
                }
                strm<<prefix<<"data_r={";
                for(std::size_t j=0;j<material->spd_r[i].size();j++)
                {
                    strm<<prefix<<material->spd_r[i][j];
                    
                    if(j+1<material->spd_r[i].size()) strm<<prefix<<",";
                    else strm<<prefix<<"}\n";
                }
                strm<<prefix<<"data_i={";
                for(std::size_t j=0;j<material->spd_i[i].size();j++)
                {
                    strm<<prefix<<material->spd_i[i][j];
                    
                    if(j+1<material->spd_i[i].size()) strm<<prefix<<",";
                    else strm<<prefix<<"}\n";
                }
                
                strm<<prefix<<"\n";
                if(material->spd_type_index[i]) strm<<prefix<<"add_data_index";
                else strm<<prefix<<"add_data_epsilon";
                
                strm<<prefix<<"(lambda,data_r,data_i)\n";
            }
        }
    }
}

//########################
//   MaterialsLibDialog
//########################

class MaterialTreeData: public wxTreeItemData
{
    public:
        GUI::Material *material;
        
        MaterialTreeData(GUI::Material *material_) : material(material_) {}
};

MaterialsLibDialog::MaterialsLibDialog(wxWindow *requester_,bool (*validator)(Material*))
    :wxDialog(nullptr,wxID_ANY,"Select a material",
              wxGetApp().default_dialog_origin(),
              wxGetApp().default_dialog_size()),
     selection_ok(false),
     new_material(false),
     material(nullptr),
     requester(requester_),
     requester_own_material(nullptr),
     accept_material(validator)
{
    // Searching for the caller's material
    
    std::size_t Nmat=MaterialsLib::size();
    
    for(std::size_t i=0;i<Nmat;i++)
    {
        GUI::Material *mat=MaterialsLib::material(i);
        
        if(mat->original_requester==requester)
        {
            requester_own_material=mat;
            break;
        }
    }
    
    // UI
    
    wxBoxSizer *sizer=new wxBoxSizer(wxHORIZONTAL);
    
    // - Tree
    
    materials=new wxTreeCtrl(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxTR_DEFAULT_STYLE|wxTR_HIDE_ROOT);
    materials->AddRoot("Materials");
    
    rebuild_tree();
    
    sizer->Add(materials,wxSizerFlags(1).Expand());
    
    // - Buttons
    
    wxBoxSizer *btn_sizer=new wxBoxSizer(wxVERTICAL);
    wxStaticBoxSizer *new_mats_sizer=new wxStaticBoxSizer(wxVERTICAL,this,"New Material");
    
    wxButton *ok_btn=new wxButton(this,wxID_ANY,"Ok");
    
    wxButton *const_btn=new wxButton(new_mats_sizer->GetStaticBox(),wxID_ANY,"Constant");
    wxButton *script_btn=new wxButton(new_mats_sizer->GetStaticBox(),wxID_ANY,"Script");
    wxButton *custom_btn=new wxButton(new_mats_sizer->GetStaticBox(),wxID_ANY,"Custom");
    wxButton *effective_btn=new wxButton(new_mats_sizer->GetStaticBox(),wxID_ANY,"Effective");
    
    new_mats_sizer->Add(const_btn,wxSizerFlags().Expand());
    new_mats_sizer->Add(script_btn,wxSizerFlags().Expand());
    new_mats_sizer->Add(custom_btn,wxSizerFlags().Expand());
    new_mats_sizer->Add(effective_btn,wxSizerFlags().Expand());
    
    wxButton *add_to_lib_btn=new wxButton(this,wxID_ANY,"Add to Lib");
    wxButton *cancel_btn=new wxButton(this,wxID_ANY,"Cancel");
    
    ok_btn->Bind(wxEVT_BUTTON,&MaterialsLibDialog::evt_ok,this);
    const_btn->Bind(wxEVT_BUTTON,&MaterialsLibDialog::evt_new_const_material,this);
    script_btn->Bind(wxEVT_BUTTON,&MaterialsLibDialog::evt_load_script,this);
    custom_btn->Bind(wxEVT_BUTTON,&MaterialsLibDialog::evt_new_custom_material,this);
    effective_btn->Bind(wxEVT_BUTTON,&MaterialsLibDialog::evt_new_effective_material,this);
    add_to_lib_btn->Bind(wxEVT_BUTTON,&MaterialsLibDialog::evt_add_to_lib,this);
    cancel_btn->Bind(wxEVT_BUTTON,&MaterialsLibDialog::evt_cancel,this);
    
    btn_sizer->Add(ok_btn,wxSizerFlags().Expand());
    btn_sizer->Add(new wxPanel(this),wxSizerFlags(1));
    btn_sizer->Add(new_mats_sizer,wxSizerFlags().Expand());
    btn_sizer->Add(new wxPanel(this),wxSizerFlags(1));
    btn_sizer->Add(add_to_lib_btn,wxSizerFlags().Expand());
    btn_sizer->Add(new wxPanel(this),wxSizerFlags(9));
    btn_sizer->Add(cancel_btn,wxSizerFlags().Expand());
    
    sizer->Add(btn_sizer,wxSizerFlags().Expand().Border(wxALL,2));
    
    SetSizer(sizer);
    ShowModal();
}

void MaterialsLibDialog::evt_add_to_lib(wxCommandEvent &event)
{
    wxTreeItemId item=materials->GetSelection();
    MaterialTreeData *tree_data=dynamic_cast<MaterialTreeData*>(materials->GetItemData(item));
    
    if(tree_data!=nullptr)
    {
        MaterialsLib::add_to_library(tree_data->material);
        rebuild_tree();
    }
}

void MaterialsLibDialog::evt_cancel(wxCommandEvent &event)
{
    Close();
}

void MaterialsLibDialog::evt_load_script(wxCommandEvent &event)
{
    wxFileName fname;
    fname=wxFileSelector("Load material script",
                         wxFileSelectorPromptStr,
                         wxEmptyString,
                         wxEmptyString,
                         wxFileSelectorDefaultWildcardStr,
                         wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    
    if(fname.IsOk()==false) return;
    
    std::filesystem::path tmp_script=fname.GetFullPath().ToStdString();
    Material tmp_material(tmp_script);
    
    if(accept_material(&tmp_material))
    {
        MaterialsLib::load_script(tmp_script);
        rebuild_tree();
    }
    else
    {
        wxMessageBox("Incompatible material in this configuration!","Error");
    }
}

void MaterialsLibDialog::evt_new_const_material(wxCommandEvent &event)
{
    MaterialsLib::forget_material(requester_own_material);
    
    material=MaterialsLib::request_material(MatType::REAL_N);
    material->original_requester=requester;
    
    selection_ok=true;
    new_material=true;
    
    Close();
}

void MaterialsLibDialog::evt_new_custom_material(wxCommandEvent &event)
{
    MaterialsLib::forget_material(requester_own_material);
    
    material=MaterialsLib::request_material(MatType::CUSTOM);
    material->original_requester=requester;

    selection_ok=true;
    new_material=true;
    
    Close();
}

void MaterialsLibDialog::evt_new_effective_material(wxCommandEvent &event)
{
    MaterialsLib::forget_material(requester_own_material);
    
    material=MaterialsLib::request_material(MatType::EFFECTIVE);
    material->original_requester=requester;

    selection_ok=true;
    new_material=true;
    
    Close();
}

void MaterialsLibDialog::evt_ok(wxCommandEvent &event)
{
    wxTreeItemId selection=materials->GetFocusedItem();
    
    MaterialTreeData *data=dynamic_cast<MaterialTreeData*>(materials->GetItemData(selection));
    
    if(data!=nullptr)
    {
        selection_ok=true;
        material=data->material;
    }

    Close();
}

void MaterialsLibDialog::rebuild_tree()
{
    wxTreeItemId root=materials->GetRootItem();
    
    materials->DeleteChildren(root);
    
    int Nmat=MaterialsLib::size();
    
    wxTreeItemId default_lib=materials->AppendItem(root,"Default Library");
    wxTreeItemId user_lib=materials->AppendItem(root,"User Library");
    wxTreeItemId scripts;
    wxTreeItemId custom_mats;
    wxTreeItemId effective_mats;
    
    for(int i=0;i<Nmat;i++)
    {
        GUI::Material *insert_mat=MaterialsLib::material(i);
        MatType mat_type=insert_mat->type;
        
        wxTreeItemId parent;
        
             if(mat_type==MatType::LIBRARY) parent=default_lib;
        else if(mat_type==MatType::USER_LIBRARY) parent=user_lib;
        else if(mat_type==MatType::SCRIPT)
        {
            if(!scripts.IsOk())
                scripts=materials->AppendItem(root,"Scripts");
            
            parent=scripts;
        }
        else if(    mat_type==MatType::CUSTOM
                 || mat_type==MatType::REAL_N)
        {
            if(!custom_mats.IsOk())
                custom_mats=materials->AppendItem(root,"Custom Materials");
            
            parent=custom_mats;
        }
        else if(mat_type==MatType::EFFECTIVE)
        {
            if(!effective_mats.IsOk())
                effective_mats=materials->AppendItem(root,"Effective Materials");
                
            parent=effective_mats;
        }
        else continue;
        
        
        std::string mat_name=insert_mat->name;
        if(mat_name.empty()) mat_name=insert_mat->script_path.generic_string();
        if(mat_name.empty()) continue;
        
        MaterialTreeData *data=new MaterialTreeData(insert_mat);
        
        materials->AppendItem(parent,mat_name,-1,-1,data);
    }
    
    if(requester!=nullptr)
    {
        for(int i=0;i<Nmat;i++)
        {
            GUI::Material *insert_mat=MaterialsLib::material(i);
            
            if(insert_mat->original_requester==requester)
            {
                MaterialTreeData *data=new MaterialTreeData(insert_mat);
        
                materials->AppendItem(root,"Selector Own Material",-1,-1,data);
            }
        }
    }
    
    materials->ExpandAll();
}

//########################
//   MaterialsLib
//########################

MaterialsEditor* MaterialsLib::editor=nullptr;
std::vector<GUI::Material*> MaterialsLib::data;
std::vector<MiniMaterialSelector*> MaterialsLib::mini_mats;

void MaterialsLib::add_to_library(GUI::Material *data_)
{
    if(data_->type==MatType::SCRIPT)
        data_->type=MatType::USER_LIBRARY;
    
    write_user_lib();
}

void MaterialsLib::consolidate(GUI::Material *material)
{
    if(!vector_contains(data,material))
    {
        for(std::size_t i=0;i<data.size();i++)
        {
            if(data[i]->Material::operator == (*material))
            {
                delete material;
                material=data[i];
                
                return;
            }
        }
    }
    
    material->type=MatType::CUSTOM; // default assumption
    
         if(material->is_effective_material) material->type=MatType::EFFECTIVE;
    else if(material->is_const()) material->type=MatType::REAL_N;
    else if(!material->script_path.empty())
    {
        Material tmp_material{material->script_path};
        
        if(tmp_material==(*material))
        {
            material->type=MatType::SCRIPT;
        }
    }
}

void MaterialsLib::consolidate(GUI::Material **pp_material)
{    
    for(std::size_t i=0;i<data.size();i++)
    {
        if(   data[i]->type==MatType::LIBRARY
           || data[i]->type==MatType::USER_LIBRARY)
        {
            Material *b_data=dynamic_cast<Material*>(data[i]);
            Material *b_test=dynamic_cast<Material*>(*pp_material);
            
            if(   b_data!=b_test
               && *(b_data)==*(b_test))
            {
                *pp_material=data[i];
                return;
            }
        }
    }
    
    consolidate(*pp_material);
}

void MaterialsLib::forget_control(MiniMaterialSelector *selector)
{
    std::size_t i;
    
    for(i=0;i<mini_mats.size();i++)
    {
        if(mini_mats[i]==selector) break;
    }
    
    if(i<mini_mats.size())
    {
        mini_mats.erase(mini_mats.begin()+i);
    }
}

void MaterialsLib::forget_editor() { editor=nullptr; }

void MaterialsLib::forget_material(GUI::Material *material)
{
    for(std::size_t i=0;i<data.size();i++)
    {
        if(material==data[i])
        {
            data.erase(data.begin()+i);
            return;
        }
    }
}

MaterialsEditor* MaterialsLib::get_manager()
{
    if(editor==nullptr)
    {
        editor=new MaterialsEditor("Materials Editor");
    }
    
    return editor;
}

bool MaterialsLib::has_manager() { return editor!=nullptr; }

void MaterialsLib::initialize()
{
    std::string fname;
    
    // Default Library
    
    std::cout<<"Materials library initialization:"<<std::endl;
    
    std::ifstream file_default(PathManager::locate_resource("mat_lib/default_materials_library"),std::ios::in);
    
    if(!file_default.is_open())
    {
        wxMessageBox("Error: default materials library missing");
    }
    else
    {
        while(std::getline(file_default,fname))
        {
            std::filesystem::path material_path{fname};
            load_material(material_path,MatType::LIBRARY);
        }
    }
    
    // User Library
    
    std::ifstream file(PathManager::to_userprofile_path("user_materials_library"),std::ios::in);
    
    if(!file.is_open())
    {
        std::cout<<"    No user specific materials to load"<<std::endl;
        return;
    }
    else
    {
        while(std::getline(file,fname))
        {
            std::filesystem::path material_path{fname};
            load_material(material_path,MatType::USER_LIBRARY);
        }
    }
    
    reorder_materials();
}

Material* MaterialsLib::knows_material(unsigned int &n,Material const &material,bool (*validator)(Material*))
{
    n=0;
    
    for(std::size_t i=0;i<data.size();i++)
    {
        if(material==*(data[i])) return data[i];
        
        if((*validator)(data[i])) n++; // Deal with lists with "holes" compared to the Mats library
    }
    
    return nullptr;
}

void MaterialsLib::load_material(std::filesystem::path const &fname,MatType type)
{
    std::cout<<"    Loading "<<fname;
    
    std::filesystem::path material_path;
    
    if(type==MatType::USER_LIBRARY) material_path=PathManager::locate_file(fname);
    else material_path=PathManager::locate_resource(fname);
    
    if(!std::filesystem::exists(material_path))
    {
        std::cout<<" ... not found"<<std::endl;
        return;
    }
    
    for(std::size_t i=0;i<data.size();i++) if(std::filesystem::equivalent(material_path,data[i]->script_path))
    {
        std::cout<<" ... duplicate"<<std::endl;
        return;
    }
    
    GUI::Material *new_data=new GUI::Material;
    new_data->load_lua_script(material_path);
    new_data->type=type;
    
    data.push_back(new_data);
    
    std::cout<<" ... done"<<std::endl;
}

void MaterialsLib::load_script(std::filesystem::path const &path)
{
    if(!std::filesystem::exists(path))
    {
        wxMessageBox("Invalid path","Error");
        return;
    }
    
    for(std::size_t i=0;i<data.size();i++)
        if(std::filesystem::equivalent(path,data[i]->script_path))
        {
            wxMessageBox("Duplicate material","Error");
            return;
        }
    
    GUI::Material *new_data=new GUI::Material;
    new_data->load_lua_script(path);
    
    data.push_back(new_data);
    
    reorder_materials();
}

GUI::Material* MaterialsLib::material(std::size_t n)
{
    if(n>=data.size()) return nullptr;
    return data[n];
}

void MaterialsLib::register_control(MiniMaterialSelector *selector)
{
    mini_mats.push_back(selector);
}

GUI::Material* MaterialsLib::request_material(MatType type)
{
    GUI::Material *new_mat=new GUI::Material;
    new_mat->type=type;
    
    data.push_back(new_mat);
    
    return new_mat;
}

void MaterialsLib::reorder_materials()
{
    for(std::size_t i=0;i<data.size();i++)
    {
        for(std::size_t j=i+1;j<data.size();j++)
        {
            if(data[j]->script_path<data[i]->script_path) std::swap(data[i],data[j]);
        }
    }
}

std::size_t MaterialsLib::size() { return data.size(); }

void MaterialsLib::consolidate()
{
    for(std::size_t i=0;i<mini_mats.size();i++)
    {
        consolidate(&(mini_mats[i]->material));
    }
}

void MaterialsLib::write_user_lib()
{
    std::ofstream file(PathManager::to_userprofile_path("user_materials_library"),std::ios::out|std::ios::trunc);
    
    for(std::size_t i=0;i<data.size();i++)
    {
        if(data[i]->type==MatType::USER_LIBRARY) file<<data[i]->script_path.generic_string()<<std::endl;
    }
}
