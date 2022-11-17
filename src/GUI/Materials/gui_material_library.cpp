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
#include <gui_material.h>

int MaterialsLib::Nm=0;
MaterialsManager* MaterialsLib::manager=nullptr;
std::vector<std::filesystem::path> MaterialsLib::mat_fname(0);
std::vector<Material*> MaterialsLib::mat_arr;
std::vector<bool> MaterialsLib::user_material;
std::vector<MaterialSelector*> MaterialsLib::selector_arr;

void MaterialsLib::add_material(std::filesystem::path const &fname)
{
    load_material(fname);
    
    std::ofstream file(PathManager::to_userprofile_path("materials_library"),std::ios::out|std::ios::trunc);
    
    for(int i=0;i<Nm;i++)
    {
        if(user_material[i]) file<<mat_fname[i].generic_string()<<std::endl;
    }
    
    file.close();
    
    for(unsigned int i=0;i<selector_arr.size();i++)
        selector_arr[i]->update_library_list();
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

Material* MaterialsLib::get_material(unsigned int n)
{
    if(n>=mat_arr.size()) return nullptr;
    return mat_arr[n];
}

std::filesystem::path MaterialsLib::get_material_name(unsigned int n)
{
    if(n>=mat_fname.size()) return "";
    return mat_fname[n];
}

int MaterialsLib::get_N_materials() { return Nm; }

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
            load_material(material_path,false);
        }
    }
    
    // User Library
    
    std::ifstream file(PathManager::to_userprofile_path("materials_library"),std::ios::in);
    
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
            load_material(material_path,true);
        }
    }
}

void MaterialsLib::insert_material(int pos,std::filesystem::path const &fname,Material *material,bool user_material_)
{
    Nm++;
    
    mat_fname.resize(Nm);
    mat_arr.resize(Nm);
    user_material.resize(Nm);
    
    for(int i=Nm-1;i>pos;i--)
    {
        mat_fname[i]=mat_fname[i-1];
        mat_arr[i]=mat_arr[i-1];
    }
    
    mat_fname[pos]=fname;
    mat_arr[pos]=material;
    user_material[pos]=user_material_;
}

Material *MaterialsLib::knows_material(unsigned int &n,Material const &material,bool (*validator)(Material*))
{
    n=0;
    
    for(int i=0;i<Nm;i++)
    {
        if(material==*(mat_arr[i])) return mat_arr[i];
        
        if((*validator)(mat_arr[i])) n++; // Deal with lists with "holes" compared to the Mats library
    }
    
    return nullptr;
}

void MaterialsLib::load_material(std::filesystem::path const &fname,bool user_material_)
{
//    if(fname[fname.size()-1]=='\r') fname=fname.substr(0,fname.size()-1);
    
    std::cout<<"    Loading "<<fname;
    
    std::filesystem::path material_path;
    
    if(user_material_) material_path=PathManager::locate_file(fname);
    else material_path=PathManager::locate_resource(fname);
    
    if(!std::filesystem::exists(material_path))
    {
        std::cout<<" ... not found"<<std::endl;
        return;
    }
    
    for(int i=0;i<Nm;i++) if(std::filesystem::equivalent(material_path,mat_fname[i]))
    {
        std::cout<<" ... duplicate"<<std::endl;
        return;
    }
    
    int insertion_pos=Nm;
    
    for(int i=0;i<Nm;i++)
    {
        if(material_path<mat_fname[i])
        {
            insertion_pos=i;
            break;
        }
    }
    
    insert_material(insertion_pos,material_path,new Material(material_path),user_material_);
    
    std::cout<<" ... done"<<std::endl;
}

void MaterialsLib::register_material_selector(MaterialSelector *selector)
{
    selector_arr.push_back(selector);
}

void MaterialsLib::remove_material_selector(MaterialSelector *selector)
{
    std::vector<MaterialSelector*>::const_iterator it;
    
    for(it=selector_arr.begin();it<selector_arr.end();it++)
    {
        if(*it==selector)
        {
            selector_arr.erase(it);
            break;
        }
    }
}
