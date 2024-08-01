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

#ifndef LOGGER_H_INCLUDED
#define LOGGER_H_INCLUDED

#include <filesystem>
#include <fstream>
#include <iostream>

enum class LogType
{
    FATAL,
    WARNING
};

class Plog
{
    public:
        static void flush()
        {
            std::cout<<std::flush;
            p_file<<std::flush;
        }

        static void init(std::filesystem::path const &file_path);
        
        template<typename... T>
        static void print(T const &... args)
        {
            (std::cout<<...<<args);
            (p_file<<...<<args);
        }
        
        template<typename... T>
        static void print(LogType log_type, T const &... args)
        {
            if(log_type==LogType::FATAL)
            {
                std::cout<<"! ";
                p_file<<"! ";
                
                (std::cout<<...<<args);
                (p_file<<...<<args);
            }
            else if(log_type==LogType::WARNING)
            {
                std::cout<<"w ";
                p_file<<"w ";
                
                (std::cout<<...<<args);
                (p_file<<...<<args);
            }
        }
        
    private:
        static std::ofstream p_file;
        
};

#endif // LOGGER_H_INCLUDED
