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
#include <string_tools.h>

#ifdef _WIN32
#include <userenv.h>
#include <Processthreadsapi.h>
#endif

#if defined(unix) || defined(__unix__) || defined(__unix)
#define UNIX_PLATFORM
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h> 
#endif

#include <algorithm>
#include <cstdlib>

std::ofstream plog;


AsciiDataLoader::AsciiDataLoader()
    :file_ok(false)
{
}

AsciiDataLoader::AsciiDataLoader(std::string const &fname,double limit)
    :file_ok(false)
{
    initialize(fname,limit);
}

void AsciiDataLoader::fill_buffer()
{
    buffer_position=0;
    
    std::string str;
    
    std::vector<bool> reuseable_buffer;
    std::vector<std::string> split;
    
    for(int i=0;i<Nl_buffer;i++)
    {
        if(file.eof())
        {
            file.clear();
            file.seekg(data_start);
        }
        
        std::getline(file,str,'\n');
        clean_data_string(str,&reuseable_buffer);
        
        while(str.size()==0)
        {
            if(file.eof())
            {
                file.clear();
                file.seekg(data_start);
            }
            
            std::getline(file,str,'\n');
            clean_data_string(str,&reuseable_buffer);
        }
        
        split_string(split,str,' ');
        
        for(int j=0;j<Nc;j++)
            buffer[j][i]=std::stod(split[j]);
    }
}

void AsciiDataLoader::initialize(std::string const &fname,double limit)
{
    data_start=0;
    data_read_position=0;
    Nl=0;
    Nc=0;
    Nl_buffer=0;
    buffer_position=0;
    
    if(file.is_open()) file.close();
    
    file.open(fname,std::ios::in|std::ios::binary);
    
    if(!file.is_open())
    {
        std::cout<<"Error: Could not open "<<fname<<std::endl;
        return;
    }
    
    std::string str;
    std::vector<bool> reuseable_buffer;
    
    while(!file.eof())
    {
        std::getline(file,str,'\n');
        clean_data_string(str,&reuseable_buffer);
        
        if(str.size()!=0)
        {
            if(contains_non_numerics(str))
            {
                Nl=0;
                data_start=file.tellg();
            }
            else Nl++;
        }
    }
    
    // Header Extraction
    
    file.clear();
    file.seekg(0);
    
    std::string tmp_header(data_start,'\0');
    file.read(tmp_header.data(),data_start);
    
    split_string(header,tmp_header,'\n');
    
    // Data handling initialization
    
    file.clear();
    file.seekg(data_start);
    
    while(!file.eof())
    {
        std::getline(file,str,'\n');
        clean_data_string(str);
        
        if(str.size()!=0)
        {
            std::stringstream strm_init(str);
            double tmp;
            
            while(!strm_init.eof())
            {
                strm_init>>tmp;
                Nc++;
            }
            
            break;
        }
    }
    
    file.clear();
    file.seekg(data_start);
    
    Nl_buffer=std::min(Nl,static_cast<int>(limit/(Nc*sizeof(double))));
    
    buffer.resize(Nc);
    for(int i=0;i<Nc;i++) buffer[i].resize(Nl_buffer);
    
    buffer_position=Nl_buffer;
    
    file_ok=true;
}

void AsciiDataLoader::load_full(std::vector<std::vector<double>> &data_holder)
{
    std::vector<std::string> strings_holder;
    
    file.seekg(data_start);
    
    while(!file.eof())
    {
        std::string str;
        std::getline(file,str,'\n');
        
        clean_data_string(str);
//        if(str.size()!=0 && !contains_non_numerics(str))
        if(str.size()!=0)
            strings_holder.push_back(str);
    }
        
    int Nl=strings_holder.size();
    
    if(Nl>0)
    {
        int i,j;
        std::stringstream strm_init(strings_holder[0]);
        int Nc=0;
        double tmp;
        
        while(!strm_init.eof())
        {
            strm_init>>tmp;
            Nc++;
        }
        
        data_holder.resize(Nc);
        for(i=0;i<Nc;i++) data_holder[i].resize(Nl,0);
        
        for(i=0;i<Nl;i++)
        {
            std::stringstream strm(strings_holder[i]);
            
            for(j=0;j<Nc;j++)
            {
                strm>>data_holder[j][i];
            }
        }
    }
}

void AsciiDataLoader::load_seq(std::vector<double> &data)
{
    if(buffer_position==Nl_buffer) fill_buffer();
    
    data.resize(buffer.size());
    
    for(unsigned int i=0;i<buffer.size();i++)
        data[i]=buffer[i][buffer_position];
    
    buffer_position++;
}

//#####################
//   PathManager
//#####################

std::filesystem::path PathManager::executable_path;
std::filesystem::path PathManager::pwd;
std::filesystem::path PathManager::resources_path;
std::filesystem::path PathManager::tmp_path;
std::filesystem::path PathManager::user_profile_path;
std::filesystem::path PathManager::user_profile_materials;

bool PathManager::belongs_to_resources(std::filesystem::path path)
{
    namespace fs=std::filesystem;
    
    if(fs::exists(path))
    {
        path=fs::canonical(path);
        std::filesystem::path rsc_path=fs::canonical(resources_path);
        
        std::string path_string=path.generic_string();
        std::string rsc_string=rsc_path.generic_string();
                
        if(rsc_string.size()>path_string.size()) return false;
        
        int start=0;
        
        #ifdef WIN32
        if(std::tolower(rsc_string[0])!=std::tolower(path_string[0])) return false;
        start=1;
        #endif
        
        for(std::size_t i=start;i<rsc_string.size();i++)
        {
            if(rsc_string[i]!=path_string[i]) return false;
        }
        
        return true;
    }
    else return false;
}

std::filesystem::path PathManager::locate_file(std::filesystem::path const &path_,
                                               std::filesystem::path const &caller_path)
{
    if(path_.is_absolute())
    {
        if(!std::filesystem::exists(path_))
        {
            std::cerr<<"Couldn't locate absolute path "<<path_<<" ...\nAborting...\n";
            std::exit(EXIT_FAILURE);
        }
        
        return path_;
    }
    else
    {
        std::filesystem::path path;
        
        path=pwd/path_;
        if(std::filesystem::exists(path))
        {
            std::cout<<"Found from PWD\n";
            return path.lexically_normal();
        }
        
        path=caller_path/path_;
        if(std::filesystem::exists(path))
        {
            std::cout<<"Found from caller origin\n";
            return path.lexically_normal();
        }
        
        path==executable_path/path_;
        if(std::filesystem::exists(path))
        {
            std::cout<<"Found from executable location\n";
            return path.lexically_normal();
        }
        
        path=resources_path/path_;
        if(!std::filesystem::exists(path))
        {
            std::cerr<<"Couldn't locate "<<path<<" anywhere...\nAborting...\n";
            std::exit(EXIT_FAILURE);
        }
        
        std::cout<<"Found from standard resources location\n";
        return path.lexically_normal();
    }
}

std::filesystem::path PathManager::locate_resource(std::filesystem::path const &path_)
{
    std::filesystem::path path=executable_path/path_;
    
    if(!std::filesystem::exists(path))
        path=resources_path/path_;
    
    return path;
}

std::filesystem::path PathManager::to_default_path(std::filesystem::path const &path,
                                                   std::filesystem::path const &base_path)
{
    std::string path_str=std::filesystem::absolute(path).generic_string();
    std::string resources_str=std::filesystem::absolute(resources_path).generic_string();
    std::string user_str=std::filesystem::absolute(user_profile_path).generic_string();
    
    std::string::size_type pos;
    
    // If it belongs to the standard folder
    pos=path_str.find(resources_str);
    if(pos==0)
    {
        path_str=path_str.substr(resources_str.size());
        return path_str;
    }
    
    // If it belongs to the user profile folder
    pos=path_str.find(user_str);
    if(pos==0)
    {
        path_str=path_str.substr(resources_str.size());
        return path_str;
    }
    
    if(base_path.empty()) return path;
    else return to_relative_file(path,base_path);
}

std::filesystem::path PathManager::to_temporary_path(std::filesystem::path const &path_)
{
    std::filesystem::path path=tmp_path/path_;
    return path;
}

std::filesystem::path PathManager::to_userprofile_path(std::filesystem::path const &path_)
{
    std::filesystem::path path=user_profile_path/path_;
    return path;
}

void PathManager::initialize()
{
    retrieve_executable_path();
    retrieve_user_profile_path();
    
    pwd=std::filesystem::current_path();
    
    resources_path=executable_path;
    #ifdef UNIX_PLATFORM
    resources_path=resources_path / ".." / "share/Aether/";
    #endif
    
    tmp_path=std::filesystem::temp_directory_path();
    tmp_path/="Aether/";
    
    create_directories(tmp_path);
    create_directories(user_profile_path);
    create_directories(user_profile_materials);
    
    std::cout<<"Default directories initialization...\n";
    std::cout<<"Executable directory: "<<executable_path.generic_string()<<"\n";
    std::cout<<"Working directory: "<<pwd.generic_string()<<"\n";
    std::cout<<"Resources directory: "<<resources_path.generic_string()<<"\n";
    std::cout<<"Temporary directory: "<<tmp_path.generic_string()<<"\n";
    std::cout<<"User profile directory: "<<user_profile_path.generic_string()<<"\n\n";
}

void PathManager::retrieve_executable_path()
{
    unsigned int buffer_size=10000;
    char buffer[10000];
    
    #ifdef _WIN32
    GetModuleFileNameA(NULL,buffer,buffer_size);
    
    executable_path=buffer;
    #endif
    
    #ifdef UNIX_PLATFORM
    buffer_size=readlink("/proc/self/exe",buffer,buffer_size);
    
    std::string exec_string;
    exec_string.resize(buffer_size);
    
    for(unsigned int i=0;i<buffer_size;i++)
        exec_string[i]=buffer[i];
    
    executable_path=exec_string;
    #endif
    
    if(executable_path.has_filename())
        executable_path.remove_filename();
}

void PathManager::retrieve_user_profile_path()
{
    #ifdef _WIN32
    char buffer[10000];
    unsigned long buffer_length=10000;
    
    void *process_token;
    
    OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&process_token);
    GetUserProfileDirectoryA(process_token,buffer,&buffer_length);
    
    user_profile_path=buffer;
    
    user_profile_path/="AppData";
    user_profile_path/="Local";
    #endif
    
    #ifdef UNIX_PLATFORM
    char *home_c_str=std::getenv("XDG_CONFIG_HOME");
    
    if(home_c_str!=NULL) user_profile_path=home_c_str;
    else
    {
        home_c_str=std::getenv("HOME");
        user_profile_path=home_c_str;
        user_profile_path/=".config";
    }
    #endif
    
    user_profile_path/="Aether/";
    user_profile_materials=user_profile_path/"materials_library";
}

std::filesystem::path to_relative_file(std::filesystem::path file,
                                       std::filesystem::path base)
{
    std::filesystem::path fname=file.filename();
    
    file=std::filesystem::absolute(file);
    base=std::filesystem::absolute(base);
    
    file.remove_filename();
    base.remove_filename();
    
    return std::filesystem::relative(file,base)/fname;
}
