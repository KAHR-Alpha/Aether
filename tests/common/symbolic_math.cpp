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

bool trigonometric_functions()
{
    SymNode x("x=0");

    // Sine

    x.set_expression("x=cos(0)");

    // Cosine



    return true;
}

int symbolic_math(int argc,char *argv[])
{
    return !(   basic_operations()
             && sub_expressions()
             && special_values()
             && trigonometric_functions());
}
