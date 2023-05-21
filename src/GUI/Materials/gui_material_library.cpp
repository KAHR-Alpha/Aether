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

bool default_material_validator(Material *material) { return true; }

// Lua bindings

namespace GUI
{
    void create_material_metatable(lua_State *L)
    {
        lua_register(L,"Material",&allocate);
        
        create_obj_metatable(L,"metatable_material");
        
        metatable_add_func(L,"refractive_index",lua_material_set_index);
        metatable_add_func(L,"load_script",lua_material_set_script);
    }
    
    int allocate(lua_State *L)
    {
        Material *p_material=MaterialsLib::request_material(MatType::CUSTOM);
        
        lua_set_metapointer<::Material>(L,"metatable_material",p_material);
        
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
        // TODO dynamic cast
        chk_var("Index");
        GUI::Material *mat=lua_get_metapointer<GUI::Material>(L,1);
        
        mat->set_const_n(lua_tonumber(L,2));
        mat->type=MatType::REAL_N;
        
        MaterialsLib::consolidate(mat);
        
        chk_var("Index/");
        return 0;
    }

    int lua_material_set_script(lua_State *L)
    {
        // TODO dynamic cast
        GUI::Material *mat=lua_get_metapointer<GUI::Material>(L,1);
        mat->load_lua_script(lua_tostring(L,2));
        
        MaterialsLib::consolidate(mat);
        
        return 0;
    }
    
    //##############
    //   Material
    //##############
    
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
    
    std::string Material::get_inline_lua()
    {
        std::string str;
    
        // TODO
        if(is_const()) str = "const_material(" + std::to_string(get_n(0).real()) + ")";
        else str = "\"" + script_path.generic_string() + "\"";
        
        return str;
    }
    
    double Material::get_lambda_validity_min()
    {
             if(type==MatType::REAL_N) return 1e-100;
        else if(type==MatType::EFFECTIVE)
        {
            return std::max(eff_mat_1->lambda_valid_min,
                            eff_mat_2->lambda_valid_min);
        }
        else return lambda_valid_min;
    }

    double Material::get_lambda_validity_max()
    {
             if(type==MatType::REAL_N) return 1e100;
        else if(type==MatType::EFFECTIVE)
        {
            return std::min(eff_mat_1->lambda_valid_max,
                            eff_mat_2->lambda_valid_max);
        }
        else return lambda_valid_max;
    }
    
    std::string Material::get_lua(std::string const &script_name)
    {
        // TODO
        std::stringstream strm;
        
        strm<<script_name<<"=Material()\n";
        
        if(!name.empty())
            strm<<script_name<<":name(\""<<name<<"\")\n";
        
        if(type==MatType::REAL_N)
        {
            strm<<script_name<<":refractive_index("<<std::to_string(get_n(0).real())<<")\n";
        }
        else if(type==MatType::LIBRARY || type==MatType::USER_LIBRARY)
        {
            strm<<script_name<<":load_script("<<script_path.generic_string()<<")\n";
        }
        else if(type==MatType::CUSTOM)
        {
            stream_lua(strm,script_name+":");
        }
        
        return strm.str();
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
                    // TODO
                    /*wxString w_script_1,w_script_2;
                    
                    w_script_1=eff_mat_1_selector->get_name();
                    w_script_2=eff_mat_2_selector->get_name();
                    
                    int eff_mat_type=get_effective_material_type();*/
                    
                    switch(effective_type)
                    {
                        case MAT_EFF_BRUGGEMAN: out<<"Brugg"; break;
                        case MAT_EFF_MG1: out<<"MG1"; break;
                        case MAT_EFF_MG2: out<<"MG2"; break;
                        case MAT_EFF_LOYENGA: out<<"Loy"; break;
                        case MAT_EFF_SUM: out<<"Sum"; break;
                        case MAT_EFF_SUM_INV: out<<"ISum"; break;
                    }
                    
                    //out<<" | "<<w_script_1<<" | "<<w_script_2;
                }
                break;
        }
    
        return out.str();
    }

    void Material::write_lua_script()
    {
        std::ofstream file(script_path,std::ios::out|std::ios::binary|std::ios::trunc);
        
        stream_lua(file,"");
    }
    
    void Material::stream_lua(std::ostream &strm, std::string const &prefix)
    {
        std::size_t i;
        
        if(!description.empty()) strm<<prefix<<"description(\""<<description<<"\")\n\n";
        
        strm<<prefix<<"validity_range("<<lambda_valid_min<<","<<lambda_valid_max<<")\n\n";
        strm<<prefix<<"epsilon_infinity("<<eps_inf<<")\n\n";
        
        for(i=0;i<debye.size();i++)
            strm<<prefix<<"add_debye("<<debye[i].ds<<","<<debye[i].t0<<")\n";
            
        for(i=0;i<drude.size();i++)
            strm<<prefix<<"add_drude("<<drude[i].wd<<","<<drude[i].g<<")\n";
            
        for(i=0;i<lorentz.size();i++)
            strm<<prefix<<"add_lorentz("<<lorentz[i].A<<","<<lorentz[i].O<<","<<lorentz[i].G<<")\n";

        for(i=0;i<critpoint.size();i++)
            strm<<prefix<<"add_critpoint("<<critpoint[i].A<<","<<critpoint[i].O<<","<<critpoint[i].P<<","<<critpoint[i].G<<")\n";

        for(i=0;i<cauchy_coeffs.size();i++)
        {
            strm<<prefix<<"add_cauchy(";
            
            for(std::size_t j=0;j<cauchy_coeffs[i].size();j++)
            {
                strm<<prefix<<cauchy_coeffs[i][j];
                if(j+1!=cauchy_coeffs[i].size()) strm<<prefix<<",";
            }
            
            strm<<prefix<<")\n";
        }

        for(i=0;i<sellmeier_B.size();i++)
            strm<<prefix<<"add_sellmeier("<<sellmeier_B[i]<<","<<sellmeier_C[i]<<")\n";
            
        for(i=0;i<spd_lambda.size();i++)
        {
            strm<<prefix<<"lambda={";
            for(std::size_t j=0;j<spd_lambda[i].size();j++)
            {
                strm<<prefix<<spd_lambda[i][j];
                
                if(j+1<spd_lambda[i].size()) strm<<prefix<<",";
                else strm<<prefix<<"}\n";
            }
            strm<<prefix<<"data_r={";
            for(std::size_t j=0;j<spd_r[i].size();j++)
            {
                strm<<prefix<<spd_r[i][j];
                
                if(j+1<spd_r[i].size()) strm<<prefix<<",";
                else strm<<prefix<<"}\n";
            }
            strm<<prefix<<"data_i={";
            for(std::size_t j=0;j<spd_i[i].size();j++)
            {
                strm<<prefix<<spd_i[i][j];
                
                if(j+1<spd_i[i].size()) strm<<prefix<<",";
                else strm<<prefix<<"}\n";
            }
            
            strm<<prefix<<"\n";
            if(spd_type_index[i]) strm<<prefix<<"add_data_index";
            else strm<<prefix<<"add_data_epsilon";
            
            strm<<prefix<<"(lambda,data_r,data_i)\n\n";
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

MaterialsLibDialog::MaterialsLibDialog(bool (*validator)(Material*))
    :wxDialog(nullptr,wxID_ANY,"Select a material",
              wxGetApp().default_dialog_origin(),
              wxGetApp().default_dialog_size()),
     selection_ok(false),
     material(nullptr),
     accept_material(validator)
{
    wxBoxSizer *sizer=new wxBoxSizer(wxHORIZONTAL);
    
    // Tree
    
    materials=new wxTreeCtrl(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxTR_DEFAULT_STYLE|wxTR_HIDE_ROOT);
    materials->AddRoot("Materials");
    
    rebuild_tree();
    
    sizer->Add(materials,wxSizerFlags(1).Expand());
    
    // Buttons
    
    wxBoxSizer *btn_sizer=new wxBoxSizer(wxVERTICAL);
    
    wxButton *ok_btn=new wxButton(this,wxID_ANY,"Ok");
    wxButton *load_script_btn=new wxButton(this,wxID_ANY,"Load Script");
    wxButton *add_to_lib_btn=new wxButton(this,wxID_ANY,"Add to Lib");
    wxButton *cancel_btn=new wxButton(this,wxID_ANY,"Cancel");
    
    ok_btn->Bind(wxEVT_BUTTON,&MaterialsLibDialog::evt_ok,this);
    load_script_btn->Bind(wxEVT_BUTTON,&MaterialsLibDialog::evt_load_script,this);
    add_to_lib_btn->Bind(wxEVT_BUTTON,&MaterialsLibDialog::evt_add_to_lib,this);
    cancel_btn->Bind(wxEVT_BUTTON,&MaterialsLibDialog::evt_cancel,this);
    
    btn_sizer->Add(ok_btn,wxSizerFlags().Expand());
    btn_sizer->Add(new wxPanel(this),wxSizerFlags(1));
    btn_sizer->Add(load_script_btn,wxSizerFlags().Expand());
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
    
    int Nmat=MaterialsLib::get_N_materials();
    
    wxTreeItemId default_lib=materials->AppendItem(root,"Default Library");
    wxTreeItemId user_lib=materials->AppendItem(root,"User Library");
    wxTreeItemId scripts=materials->AppendItem(root,"Scripts");
    wxTreeItemId custom_mats=materials->AppendItem(root,"Custom Materials");
    wxTreeItemId effective_mats=materials->AppendItem(root,"Effective Materials");
    
    for(int i=0;i<Nmat;i++)
    {
        MatType mat_type=MaterialsLib::get_material_type(i);
        
        wxTreeItemId parent;
        
             if(mat_type==MatType::LIBRARY) parent=default_lib;
        else if(mat_type==MatType::USER_LIBRARY) parent=user_lib;
        else if(mat_type==MatType::SCRIPT) parent=scripts;
        else if(    mat_type==MatType::CUSTOM
                 || mat_type==MatType::REAL_N) parent=custom_mats;
        else if(mat_type==MatType::EFFECTIVE) parent=effective_mats;
        else continue;
        
        GUI::Material *insert_mat=MaterialsLib::get_material_data(i);
        
        std::string mat_name=insert_mat->name;
        if(mat_name.empty()) mat_name=insert_mat->script_path.generic_string();
        if(mat_name.empty()) continue;
        
        MaterialTreeData *data=new MaterialTreeData(insert_mat);
        
        materials->AppendItem(parent,mat_name,-1,-1,data);
    }
    
    materials->ExpandAll();
}

//########################
//   MaterialsLib
//########################

MaterialsManager* MaterialsLib::manager=nullptr;
std::vector<GUI::Material*> MaterialsLib::data;

void MaterialsLib::add_material(std::filesystem::path const &fname)
{
    load_material(fname,MatType::USER_LIBRARY);
    
    write_user_lib();
}

void MaterialsLib::add_to_library(GUI::Material *data_)
{
    if(data_->type==MatType::SCRIPT)
        data_->type=MatType::USER_LIBRARY;
    
    write_user_lib();
}

void MaterialsLib::forget_manager() { manager=nullptr; }

MaterialsManager* MaterialsLib::get_manager()
{
    if(manager==nullptr)
    {
        manager=new MaterialsManager("Materials Editor");
    }
    
    return manager;
}

GUI::Material* MaterialsLib::get_material_data(unsigned int n)
{
    if(n>=data.size()) return nullptr;
    return data[n];
}

std::filesystem::path MaterialsLib::get_material_path(unsigned int n)
{
    if(n>=data.size()) return "";
    return data[n]->script_path;
}

MatType MaterialsLib::get_material_type(unsigned int n)
{
    return data[n]->type;
}

std::size_t MaterialsLib::get_N_materials() { return data.size(); }

bool MaterialsLib::has_manager() { return manager!=nullptr; }

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

void MaterialsLib::write_user_lib()
{
    std::ofstream file(PathManager::to_userprofile_path("user_materials_library"),std::ios::out|std::ios::trunc);
    
    for(std::size_t i=0;i<data.size();i++)
    {
        if(data[i]->type==MatType::USER_LIBRARY) file<<data[i]->script_path.generic_string()<<std::endl;
    }
}
