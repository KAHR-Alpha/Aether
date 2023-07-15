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

#ifndef ENUM_TOOLS_H_INCLUDED
#define ENUM_TOOLS_H_INCLUDED

#include <tuple>

template<typename... T>
class AnyOf
{
    public:
        std::tuple<T...> data;
        
        AnyOf(T... args)
            :data(args...)
        {
        }
        
        template<typename E,std::size_t... I>
        bool double_equal_subcall(E const &val,std::index_sequence<I...>) const
        {
            return ((std::get<I>(data) == val) || ...);
        }
        
        template<typename E>
        bool operator == (E const &val)
        {
            std::size_t const N=std::tuple_size<decltype(data)>::value;
            auto sequence=std::make_index_sequence<N>();
            
            return double_equal_subcall(val,sequence);
        }
};

template<typename E,typename... T>
bool operator == (E const &val,AnyOf<T...> const &collection)
{
    std::size_t const N=std::tuple_size<decltype(collection.data)>::value;
    auto sequence=std::make_index_sequence<N>();
    
    return collection.double_equal_subcall(val,sequence);
}

#endif // ENUM_TOOLS_H_INCLUDED
