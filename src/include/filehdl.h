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

#ifndef FILEHDL_H
#define FILEHDL_H

#include <mathUT.h>
#include <phys_constants.h>

#include <filesystem>
#include <fstream>

class AsciiDataLoader
{
    public:
        bool file_ok;
        std::ifstream file;
        
        std::ifstream::pos_type data_start,data_read_position;
        int Nl,Nc,Nl_buffer;
        
        int buffer_position;
        std::vector<std::vector<double>> buffer;
        std::vector<std::string> header;
        
        AsciiDataLoader();
        AsciiDataLoader(std::string const &fname,double limit=50e6);
        
        void fill_buffer();
        void load_full(std::vector<std::vector<double>> &data_holder);
        void load_seq(std::vector<double> &data);
        void initialize(std::string const &fname,double limit=50e6);
};

class PathManager
{
    private:
        static void retrieve_executable_path();
        static void retrieve_user_profile_path();
    public:        
        static std::filesystem::path executable_path,
                                     pwd,
                                     resources_path,
                                     tmp_path,
                                     user_profile_path,
                                     user_profile_materials;
        
        static bool belongs_to_resources(std::filesystem::path path);
        static std::filesystem::path locate_file(std::filesystem::path const &path,
                                                 std::filesystem::path const &caller_path=pwd);
        static std::filesystem::path locate_resource(std::filesystem::path const &path);
        static std::filesystem::path to_default_path(std::filesystem::path const &path,
                                                     std::filesystem::path const &base_path=pwd);
        static std::filesystem::path to_temporary_path(std::filesystem::path const &path);
        static std::filesystem::path to_userprofile_path(std::filesystem::path const &path);
        static void initialize();
};

[[deprecated]] bool is_directory(std::string const &dir);
[[deprecated]] bool make_directory(std::string const &dir);
[[deprecated]] void proper_dirname_convertion(std::string &dir);
[[deprecated]] void proper_filename_convertion(std::string &dir);
std::filesystem::path to_relative_file(std::filesystem::path file,
                                       std::filesystem::path base);

#endif // FILEHDL_H
