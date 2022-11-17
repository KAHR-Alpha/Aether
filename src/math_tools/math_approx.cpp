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

#include <math_approx.h>

extern const double Pi;

Cspline::Cspline()
    :Np(0),
     end_mode(CSPLINE_NATURAL),
     offset_x(0), offset_y(0),
     scale_x(1.0), scale_y(1.0)
{
}

Cspline::Cspline(Cspline const &spline)
    :Np(spline.Np),
     end_mode(spline.end_mode),
     offset_x(spline.offset_x),
     offset_y(spline.offset_y),
     scale_x(spline.scale_x),
     scale_y(spline.scale_y),
     xp(spline.xp),
     yp(spline.yp)
{
    coeffs.init(spline.coeffs.L1(),spline.coeffs.L2());
    
    coeffs=spline.coeffs;
}

Cspline::Cspline(std::vector<double> const &xp_,std::vector<double> const &yp_,int end_mode_)
    :Np(xp_.size()),
     end_mode(end_mode_),
     xp(xp_),
     yp(yp_),
     coeffs(4,Np-1,0)
{
    calc_coeffs();
}

void Cspline::calc_coeffs()
{
    reorder();
    rescale();
    
    int i,j;
    int Ns=Np-1;
    int Ns3=3*Ns;
    
    Eigen::MatrixXd A(Ns3,Ns3);
    Eigen::VectorXd B(Ns3);
    
    for(i=0;i<Ns3;i++)
    {
        for(j=0;j<Ns3;j++) A(i,j)=0;
        B(i)=0;
    }
    
    double xd=0;
    double xd2=0;
    
    //second condition on y
    
    j=0;
    
    for(i=0;i<Ns;i++)
    {
        xd=xp[i+1]-xp[i];
        xd2=xd*xd;
        
        A(j,3*i+0)=xd2*xd;
        A(j,3*i+1)=xd2;
        A(j,3*i+2)=xd;
        
        B[j]=yp[i+1]-yp[i];
        
        j+=1;
    }
    
    //condition on derivatives
    
    for(i=1;i<Np-1;i++)
    {
        xd=xp[i]-xp[i-1];
        
        A(j,3*(i-1)+0)=3.0*xd*xd;
        A(j,3*(i-1)+1)=2*xd;
        A(j,3*(i-1)+2)=1;
        
        A(j,3*(i-1)+5)=-1;
        
        j+=1;
    }
    
    //condition on second derivatives
    
    for(i=1;i<Np-1;i++)
    {
        xd=xp[i]-xp[i-1];
        
        A(j,3*(i-1)+0)=6.0*xd;
        A(j,3*(i-1)+1)=2.0;
        
        A(j,3*(i-1)+4)=-2.0;
        
        j+=1;
    }
    
    //boundary conditions
    
    if(end_mode==CSPLINE_NATURAL)
    {
        A(j,1)=2.0; j+=1;
        
        xd=xp[Ns]-xp[Ns-1];
        A(j,3*(Ns-1)+0)=6.0*xd;
        A(j,3*(Ns-1)+1)=2.0;
    }
    else if(end_mode==CSPLINE_NAK)
    {
        A(j,0)=6.0;
        A(j,3)=-6.0;
        j+=1;
        
        A(j,3*(Ns-1))=6.0;
        A(j,3*(Ns-2))=-6.0;
        j+=1;
    }
    
    //compute the coeffcients
    
    Eigen::VectorXd Vc=A.fullPivLu().solve(B);
    
    for(i=0;i<Ns;i++)
    {
        coeffs(0,i)=Vc(3*i+0);
        coeffs(1,i)=Vc(3*i+1);
        coeffs(2,i)=Vc(3*i+2);
        coeffs(3,i)=yp[i]; //first condition on y
    }
}

double Cspline::eval(double const &x_) const
{
    int i;
    
    double x=(x_-offset_x)/scale_x;
    
    double seg_ind=0;
    
    //find which segment to consider    
    if(x>=xp[0] && x<=xp[Np-1])
    {
        for(i=0;i<Np-1;i++)
        {
            if(x>=xp[i] && x<=xp[i+1])
            {
                seg_ind=i;
                break;
            }
        }
    }
    else if(x<xp[0]) seg_ind=0;
    else if(x>xp[Np-1]) seg_ind=Np-2;
    
    i=seg_ind;
        
    double xd=x-xp[i];
    
    double out=((coeffs(0,i)*xd+coeffs(1,i))*xd+coeffs(2,i))*xd+coeffs(3,i);
    
    return scale_y*out+offset_y;
}

std::size_t Cspline::get_N() const { return xp.size(); }

double Cspline::get_x_base(std::size_t const &i) const
{
    return xp[i]*scale_x+offset_x;
}

double Cspline::get_y_base(std::size_t const &i) const
{
    return yp[i]*scale_y+offset_y;
}

void Cspline::init(std::vector<double> const &xp_,std::vector<double> const &yp_,int end_mode_)
{
    Np=xp_.size();
    end_mode=end_mode_;
    
    coeffs.init(4,Np-1,0);
    
    xp=xp_;
    yp=yp_;
    
    calc_coeffs();
}

bool Cspline::is_empty() const
{
    return Np==0;
}

void Cspline::show() const
{
    std::cout<<"Np: "<<Np<<"\n";
    std::cout<<"end_mod: "<<end_mode<<"\n";
    std::cout<<"offset_x: "<<offset_x<<"\n";
    std::cout<<"offset_y: "<<offset_y<<"\n";
    std::cout<<"scale_x: "<<scale_x<<"\n";
    std::cout<<"scale_y: "<<scale_y<<"\n";
    
    std::cout<<"xp:\n";
    for(std::size_t i=0;i<xp.size();i++)
        std::cout<<xp[i]<<" ";
    std::cout<<"\nyp:\n";
    for(std::size_t i=0;i<yp.size();i++)
        std::cout<<yp[i]<<" ";
    std::cout<<"\ncoeffs:\n";
    for(int i=0;i<coeffs.L1();i++)
    {
        for(int j=0;j<coeffs.L2();j++)
            std::cout<<coeffs(i,j)<<" ";
        std::cout<<"\n";
    }
}

double Cspline::operator () (double const &x) const { return eval(x); }

void Cspline::operator = (Cspline const &spline)
{
    Np=spline.Np;
    end_mode=spline.end_mode;
    
    offset_x=spline.offset_x;
    offset_y=spline.offset_y;
    scale_x=spline.scale_x;
    scale_y=spline.scale_y;
    
    xp=spline.xp;
    yp=spline.yp;
    coeffs.init(spline.coeffs.L1(),spline.coeffs.L2());
    
    xp=spline.xp;
    yp=spline.yp;
    coeffs=spline.coeffs;
}

bool Cspline::operator == (Cspline const &spline)
{
    if(Np!=spline.Np ||
       end_mode!=spline.end_mode ||
       offset_x!=spline.offset_x ||
       offset_y!=spline.offset_y ||
       scale_x!=spline.scale_x ||
       scale_y!=spline.scale_y ||
       !(xp==spline.xp) ||
       !(yp==spline.yp) ||
       !(coeffs==spline.coeffs)) return false;
    
    return true;
}

void Cspline::reorder()
{
    if(xp.size()>1 && xp[1]<xp[0])
    {
        std::size_t N=xp.size();
        
        for(std::size_t i=0;i<N/2;i++)
        {
            std::swap(xp[i],xp[N-1-i]);
            std::swap(yp[i],yp[N-1-i]);
        }
    }
}

void Cspline::rescale()
{
    double x_min=vector_min(xp);
    double x_max=vector_max(xp);
    
    double y_min=vector_min(yp);
    double y_max=vector_max(yp);
    
    offset_x=x_min;
    offset_y=y_min;
    
    scale_x=x_max-x_min;
    scale_y=y_max-y_min;
    
    for(int l=0;l<Np;l++)
    {
        xp[l]=(xp[l]-offset_x)/scale_x;
        yp[l]=(yp[l]-offset_y)/scale_y;
    }
}

//####################
//    Cspline 2D
//####################

Cspline_2fct::Cspline_2fct(std::vector<double> const &t,
                           std::vector<double> const &x,
                           std::vector<double> const &y,
                           int end_mode)
    :x_spline(t,x,end_mode),
     y_spline(t,y,end_mode)
{
}

void Cspline_2fct::eval(double t,double &x,double &y)
{
    x=x_spline.eval(t);
    y=y_spline.eval(t);
}
