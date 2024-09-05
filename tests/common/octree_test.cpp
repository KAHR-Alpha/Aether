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

struct TestElement
{
    double x, y, z;
};

struct TestRule
{
    bool operator() (TestElement const &E,
                     double xm, double xp,
                     double ym, double yp,
                     double zm, double zp) const
    {
        if(   E.x < xm || E.x > xp
           || E.y < ym || E.y > yp
           || E.z < zm || E.z > zp)
        {
            return false;
        }

        return true;
    }
};

bool basic_fill_test()
{
    Octree octree(5,
                  -1.0, 1.0,
                  -1.0, 1.0,
                  -1.0, 1.0);

    std::vector<TestElement> elements;

    double x[8], y[8], z[8];
    
    x[0] = -0.5;   y[0] = -0.5;   z[0] = -0.5;
    x[1] = +0.5;   y[1] = -0.5;   z[1] = -0.5;
    x[2] = +0.5;   y[2] = +0.5;   z[2] = -0.5;
    x[3] = -0.5;   y[3] = +0.5;   z[3] = -0.5;
    x[4] = -0.5;   y[4] = -0.5;   z[4] = +0.5;
    x[5] = +0.5;   y[5] = -0.5;   z[5] = +0.5;
    x[6] = +0.5;   y[6] = +0.5;   z[6] = +0.5;
    x[7] = -0.5;   y[7] = +0.5;   z[7] = +0.5;

    for(int i=0; i<8; i++)
    {
        elements.push_back({ x[i], y[i], z[i]});
    }
    
    TestRule generation_rule;

    octree.generate_tree(elements, generation_rule);

    bool fail = false;
    std::vector<int> output;

    for(int i=0; i<8; i++)
    {
        if(   output.size() < 1
           || output.front() != 0)
        {
            fail |= true;
            std::cout<<"Basic octree fill failure on point "<<i<<"\n";
        }
    }

    return fail;
}

int octree_test(int argc, char* argv[])
{
    return !basic_fill_test();
}
