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

#include <octree.h>

Octree_node::Octree_node(int level_,int max_level_,
                         double fx1_,double fx2_,
                         double fy1_,double fy2_,
                         double fz1_,double fz2_)
    :bottom_node(false),
     level(level_),
     max_level(max_level_),
     fx1(fx1_), fx2(fx2_),
     fy1(fy1_), fy2(fy2_),
     fz1(fz1_), fz2(fz2_),
     N_elem(0)
{
    if(level==max_level) bottom_node=true;
    
    for(int i=0;i<8;i++) children[i]=nullptr;
}

Octree_node::~Octree_node()
{
    for(int i=0;i<8;i++)
    {
        if(children[i]!=nullptr)
        {
            delete children[i];
            children[i]=nullptr;
        }
    }
}

void Octree_node::finalize()
{
    int i;
    
    if(bottom_node)
    {
        N_elem = flist.size();
        gflist.resize(N_elem);
        
        std::list<int>::const_iterator iterator;
        
        i = 0;
        
        for(int val : flist)
        {
            gflist[i] = val;
            i++;
        }
        
        flist.clear();
    }
    else
    {
        for(i=0;i<8;i++) children[i]->finalize();
    }
}

//################
//################

Octree::Octree()
    :max_level(5),
     fx1(-1), fx2(1),
     fy1(-1), fy2(1),
     fz1(-1), fz2(1)
{
    for(int i=0;i<8;i++) children[i]=nullptr;
}

Octree::Octree(int max_level_,
               double fx1_,double fx2_,
               double fy1_,double fy2_,
               double fz1_,double fz2_)
    :max_level(max_level_),
     fx1(fx1_), fx2(fx2_),
     fy1(fy1_), fy2(fy2_),
     fz1(fz1_), fz2(fz2_)
{
    for(int i=0;i<8;i++) children[i]=nullptr;
}

Octree::~Octree()
{
    clear_tree();
}

void Octree::clear_tree()
{
    for(int i=0;i<8;i++)
    {
        if(children[i]!=nullptr)
        {
            delete children[i];
            children[i]=nullptr;
        }
    }
}

void Octree::finalize()
{
    int i;
    
    for(i=0;i<8;i++) children[i]->finalize();
}

void Octree::set_params(int max_level_,
                         double fx1_,double fx2_,
                         double fy1_,double fy2_,
                         double fz1_,double fz2_)
{
    max_level=max_level_;
    fx1=fx1_; fx2=fx2_;
    fy1=fy1_; fy2=fy2_;
    fz1=fz1_; fz2=fz2_;
}
