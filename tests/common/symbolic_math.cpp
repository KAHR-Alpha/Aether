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

#include <math_sym.h>

bool near(double val,double test,double delta=1e-6)
{
    if(std::abs(val-test)<delta) return true;
    
    return false;
}

bool basic_operations()
{
    SymLib lib;
    
    SymNode x("x=1",&lib);
    SymNode y("y=2",&lib);
    SymNode z("z=5",&lib);
    
    // Basic assignment
    
    double val=x.evaluate();
    
    if(val!=1) return false;
    
    // Addition
    
    x.set_expression("x=y+z");
    if(!near(x.evaluate(),7)) return false;
    std::cout<<"Addition validated\n";
    
    // Substration
    
    x.set_expression("x=y-z");
    if(!near(x.evaluate(),-3)) return false;
    std::cout<<"Substraction validated\n";
    
    // Multiplication
    
    x.set_expression("x=y*z");
    if(!near(x.evaluate(),10)) return false;
    std::cout<<"Multiplication validated\n";
    
    // Division
    
    x.set_expression("x=z/y");
    if(!near(x.evaluate(),2.5)) return false;
    std::cout<<"Division validated\n";
    
    return true; // Success
}

bool sub_expressions()
{
    SymLib lib;
    
    SymNode x("x=1",&lib);
    SymNode y("y=2",&lib);
    SymNode z("z=5",&lib);
    
    x.set_expression("x=(0)");
    if(!near(x.evaluate(),0))
    {
        std::cout<<"Basic subexpression failure\n";
        return false;
    }
    std::cout<<"Basic subexpression validated\n";
    
    x.set_expression("x=(0)+(1)+(2)");
    if(!near(x.evaluate(),3))
    {
        std::cout<<"Extended subexpression failure\n";
        return false;
    }
    std::cout<<"Extended subexpression validated\n";

    x.set_expression("x=0.5*(y+z)*(y/z)+(z-y)/2.0-4.0");
    if(!near(x.evaluate(),-1.1))
    {
        std::cout<<"Composition failure\n";
        return false;
    }
    std::cout<<"Composition validated\n";
    
    return true; // Success
}

bool special_values()
{
    SymLib lib;
    SymNode x("x=0",&lib);
    
    // Pi
    
    x.set_expression("x=pi");
    if(!near(x.evaluate(),Pi))
    {
        std::cout<<"Pi failed\n";
        return false;
    }
    std::cout<<"Pi validated\n";
    
    return true;
}

bool functions()
{
    SymLib lib;
    SymNode x("x=0",&lib);

    // Cosine

    int N=20;

    for(int i=0;i<N;i++)
    {
        double ang=2.0*Pi*i/(N-1.0);

        x.set_expression("x=cos("+std::to_string(ang)+")");
        if(!near(x.evaluate(),std::cos(ang)))
        {
            std::cout<<"Cos failed\n";
            return false;
        }
    }
    std::cout<<"Cos validated\n";

    // Sine

    for(int i=0;i<N;i++)
    {
        double ang=2.0*Pi*i/(N-1.0);

        x.set_expression("x=sin("+std::to_string(ang)+")");
        if(!near(x.evaluate(),std::sin(ang)))
        {
            std::cout<<"Sin failed\n";
            return false;
        }
    }
    std::cout<<"Sin validated\n";

    // Cos² + sin²

    for(int i=0;i<N;i++)
    {
        std::string ang=std::to_string(2.0*Pi*i/(N-1.0));

        x.set_expression("x=cos("+ang+")^2+sin("+ang+")^2");
        if(!near(x.evaluate(),1.0))
        {
            std::cout<<"Cos^2+sin^2 failed\n";
            return false;
        }
    }
    std::cout<<"Cos^2+sin^2 validated\n";

    // Cos² + sin²

    for(int i=0;i<N;i++)
    {
        std::string ang=std::to_string(2.0*Pi*i/(N-1.0));

        x.set_expression("x=cos(cos("+ang+"))");
        if(!near(x.evaluate(),std::cos(std::cos(std::stod(ang)))))
        {
            std::cout<<"Cos(cos) failed\n";
            return false;
        }
    }
    std::cout<<"Cos(cos) validated\n";

    // Cos Pi/2.0

    x.set_expression("x=cos(pi/2.0)");
    if(!near(x.evaluate(),0))
    {
        std::cout<<"Cos(pi/2.0) failed\n";
        return false;
    }
    std::cout<<"Cos(pi/2.0) validated\n";

    // Cos Pi

    x.set_expression("x=cos(pi)");
    if(!near(x.evaluate(),-1.0))
    {
        std::cout<<"Cos(pi) failed\n";
        return false;
    }
    std::cout<<"Cos(pi) validated\n";

    // Sin Pi/2.0

    x.set_expression("x=sin(pi/2.0)");
    if(!near(x.evaluate(),1.0))
    {
        std::cout<<"Sin(pi/2.0) failed\n";
        return false;
    }
    std::cout<<"Sin(pi/2.0) validated\n";

    // Sin Pi

    x.set_expression("x=sin(pi)");
    if(!near(x.evaluate(),0))
    {
        std::cout<<"Sin(pi) failed\n";
        return false;
    }
    std::cout<<"Sin(pi) validated\n";

    // Tan

    for(int i=0;i<N;i++)
    {
        std::string ang_str=std::to_string(2.0*Pi*i/(N-1.0));

        x.set_expression("x=tan("+ang_str+")");
        if(!near(x.evaluate(),std::tan(std::stod(ang_str))))
        {
            std::cout<<"Tan failed\n";
            return false;
        }
    }
    std::cout<<"Tan validated\n";

    // Acos

    for(int i=0;i<N;i++)
    {
        std::string val_str=std::to_string(-1.0+i*2.0/(N-1.0));

        x.set_expression("x=acos("+val_str+")");
        if(!near(x.evaluate(),std::acos(std::stod(val_str))))
        {
            std::cout<<"Acos failed\n";
            return false;
        }
    }
    std::cout<<"Acos validated\n";

    // Asin

    for(int i=0;i<N;i++)
    {
        std::string val_str=std::to_string(-1.0+i*2.0/(N-1.0));

        x.set_expression("x=asin("+val_str+")");
        if(!near(x.evaluate(),std::asin(std::stod(val_str))))
        {
            std::cout<<"Asin failed\n";
            return false;
        }
    }
    std::cout<<"Asin validated\n";

    // Atan

    for(int i=0;i<N;i++)
    {
        std::string val_str=std::to_string(-1.0+i*2.0/(N-1.0));

        x.set_expression("x=atan("+val_str+")");
        if(!near(x.evaluate(),std::atan(std::stod(val_str))))
        {
            std::cout<<"Atan failed\n";
            return false;
        }
    }
    std::cout<<"Atan validated\n";

    // Exp

    for(int i=0;i<N;i++)
    {
        std::string val_str=std::to_string(-3.0+i*6.0/(N-1.0));

        x.set_expression("x=exp("+val_str+")");
        if(!near(x.evaluate(),std::exp(std::stod(val_str))))
        {
            std::cout<<"Exp failed\n";
            return false;
        }
    }
    std::cout<<"Exp validated\n";

    return true;
}

int symbolic_math(int argc,char *argv[])
{
    return !(   basic_operations()
             && sub_expressions()
             && special_values()
             && functions());
}
