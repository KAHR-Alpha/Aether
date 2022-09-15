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

#include <foctree.h>

FOctree_node::FOctree_node(int level_,int max_level_,
                           double fx1_,double fx2_,
                           double fy1_,double fy2_,
                           double fz1_,double fz2_)
    :bottom_node(false),
     level(level_),
     max_level(max_level_),
     fx1(fx1_), fx2(fx2_),
     fy1(fy1_), fy2(fy2_),
     fz1(fz1_), fz2(fz2_)
{
    if(level==max_level) bottom_node=true;
    
    for(int i=0;i<8;i++) children[i]=nullptr;
}

FOctree_node::~FOctree_node()
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

void FOctree_node::finalize()
{
    int i,j;
    
    if(bottom_node)
    {
        Nf=flist.size();
        gflist.init(Nf,0);
        
        std::list<int>::const_iterator iterator;
        
        j=0;
        for(iterator=flist.begin();iterator!=flist.end();iterator++)
        {
            gflist[j]=*iterator;
            j++;
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

FOctree::FOctree()
    :max_level(5),
     fx1(-1), fx2(1),
     fy1(-1), fy2(1),
     fz1(-1), fz2(1)
{
    for(int i=0;i<8;i++) children[i]=nullptr;
}

FOctree::FOctree(int max_level_,
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

FOctree::~FOctree()
{
    clear_tree();
}

void FOctree::clear_tree()
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

void FOctree::finalize()
{
    int i;
    
    for(i=0;i<8;i++) children[i]->finalize();
}

void FOctree::set_params(int max_level_,
                         double fx1_,double fx2_,
                         double fy1_,double fy2_,
                         double fz1_,double fz2_)
{
    max_level=max_level_;
    fx1=fx1_; fx2=fx2_;
    fy1=fy1_; fy2=fy2_;
    fz1=fz1_; fz2=fz2_;
}
