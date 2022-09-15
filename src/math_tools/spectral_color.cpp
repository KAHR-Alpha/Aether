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

#include <spectral_color.h>
#include <math_approx.h>

extern std::ofstream plog;

double CIE31_base[81][4]={{380e-9,0.001368,0.000039,0.006450},
                          {385e-9,0.002236,0.000064,0.010550},
                          {390e-9,0.004243,0.000120,0.020050},
                          {395e-9,0.007650,0.000217,0.036210},
                          {400e-9,0.014310,0.000396,0.067850},
                          {405e-9,0.023190,0.000640,0.110200},
                          {410e-9,0.043510,0.001210,0.207400},
                          {415e-9,0.077630,0.002180,0.371300},
                          {420e-9,0.134380,0.004000,0.645600},
                          {425e-9,0.214770,0.007300,1.039050},
                          {430e-9,0.283900,0.011600,1.385600},
                          {435e-9,0.328500,0.016840,1.622960},
                          {440e-9,0.348280,0.023000,1.747060},
                          {445e-9,0.348060,0.029800,1.782600},
                          {450e-9,0.336200,0.038000,1.772110},
                          {455e-9,0.318700,0.048000,1.744100},
                          {460e-9,0.290800,0.060000,1.669200},
                          {465e-9,0.251100,0.073900,1.528100},
                          {470e-9,0.195360,0.090980,1.287640},
                          {475e-9,0.142100,0.112600,1.041900},
                          {480e-9,0.095640,0.139020,0.812950},
                          {485e-9,0.057950,0.169300,0.616200},
                          {490e-9,0.032010,0.208020,0.465180},
                          {495e-9,0.014700,0.258600,0.353300},
                          {500e-9,0.004900,0.323000,0.272000},
                          {505e-9,0.002400,0.407300,0.212300},
                          {510e-9,0.009300,0.503000,0.158200},
                          {515e-9,0.029100,0.608200,0.111700},
                          {520e-9,0.063270,0.710000,0.078250},
                          {525e-9,0.109600,0.793200,0.057250},
                          {530e-9,0.165500,0.862000,0.042160},
                          {535e-9,0.225750,0.914850,0.029840},
                          {540e-9,0.290400,0.954000,0.020300},
                          {545e-9,0.359700,0.980300,0.013400},
                          {550e-9,0.433450,0.994950,0.008750},
                          {555e-9,0.512050,1.000000,0.005750},
                          {560e-9,0.594500,0.995000,0.003900},
                          {565e-9,0.678400,0.978600,0.002750},
                          {570e-9,0.762100,0.952000,0.002100},
                          {575e-9,0.842500,0.915400,0.001800},
                          {580e-9,0.916300,0.870000,0.001650},
                          {585e-9,0.978600,0.816300,0.001400},
                          {590e-9,1.026300,0.757000,0.001100},
                          {595e-9,1.056700,0.694900,0.001000},
                          {600e-9,1.062200,0.631000,0.000800},
                          {605e-9,1.045600,0.566800,0.000600},
                          {610e-9,1.002600,0.503000,0.000340},
                          {615e-9,0.938400,0.441200,0.000240},
                          {620e-9,0.854450,0.381000,0.000190},
                          {625e-9,0.751400,0.321000,0.000100},
                          {630e-9,0.642400,0.265000,0.000050},
                          {635e-9,0.541900,0.217000,0.000030},
                          {640e-9,0.447900,0.175000,0.000020},
                          {645e-9,0.360800,0.138200,0.000010},
                          {650e-9,0.283500,0.107000,0.000000},
                          {655e-9,0.218700,0.081600,0.000000},
                          {660e-9,0.164900,0.061000,0.000000},
                          {665e-9,0.121200,0.044580,0.000000},
                          {670e-9,0.087400,0.032000,0.000000},
                          {675e-9,0.063600,0.023200,0.000000},
                          {680e-9,0.046770,0.017000,0.000000},
                          {685e-9,0.032900,0.011920,0.000000},
                          {690e-9,0.022700,0.008210,0.000000},
                          {695e-9,0.015840,0.005723,0.000000},
                          {700e-9,0.011359,0.004102,0.000000},
                          {705e-9,0.008111,0.002929,0.000000},
                          {710e-9,0.005790,0.002091,0.000000},
                          {715e-9,0.004109,0.001484,0.000000},
                          {720e-9,0.002899,0.001047,0.000000},
                          {725e-9,0.002049,0.000740,0.000000},
                          {730e-9,0.001440,0.000520,0.000000},
                          {735e-9,0.001000,0.000361,0.000000},
                          {740e-9,0.000690,0.000249,0.000000},
                          {745e-9,0.000476,0.000172,0.000000},
                          {750e-9,0.000332,0.000120,0.000000},
                          {755e-9,0.000235,0.000085,0.000000},
                          {760e-9,0.000166,0.000060,0.000000},
                          {765e-9,0.000117,0.000042,0.000000},
                          {770e-9,0.000083,0.000030,0.000000},
                          {775e-9,0.000059,0.000021,0.000000},
                          {780e-9,0.000042,0.000015,0.000000}};

double D65_base[81]={49.975500,
                     52.311800,
                     54.648200,
                     68.701500,
                     82.754900,
                     87.120400,
                     91.486000,
                     92.458900,
                     93.431800,
                     90.057000,
                     86.682300,
                     95.773600,
                     104.865000,
                     110.936000,
                     117.008000,
                     117.410000,
                     117.812000,
                     116.336000,
                     114.861000,
                     115.392000,
                     115.923000,
                     112.367000,
                     108.811000,
                     109.082000,
                     109.354000,
                     108.578000,
                     107.802000,
                     106.296000,
                     104.790000,
                     106.239000,
                     107.689000,
                     106.047000,
                     104.405000,
                     104.225000,
                     104.046000,
                     102.023000,
                     100.000000,
                     98.167100,
                     96.334200,
                     96.061100,
                     95.788000,
                     92.236800,
                     88.685600,
                     89.345900,
                     90.006200,
                     89.802600,
                     89.599100,
                     88.648900,
                     87.698700,
                     85.493600,
                     83.288600,
                     83.493900,
                     83.699200,
                     81.863000,
                     80.026800,
                     80.120700,
                     80.214600,
                     81.246200,
                     82.277800,
                     80.281000,
                     78.284200,
                     74.002700,
                     69.721300,
                     70.665200,
                     71.609100,
                     72.979000,
                     74.349000,
                     67.976500,
                     61.604000,
                     65.744800,
                     69.885600,
                     72.486300,
                     75.087000,
                     69.339800,
                     63.592700,
                     55.005400,
                     46.418200,
                     56.611800,
                     66.805400,
                     65.094100,
                     63.382800};

double FL2_base[81]={1.18,
                     1.48,
                     1.84,
                     2.15,
                     2.44,
                     15.69,
                     3.85,
                     3.74,
                     4.19,
                     4.62,
                     5.06,
                     34.98,
                     11.81,
                     6.27,
                     6.63,
                     6.93,
                     7.19,
                     7.40,
                     7.54,
                     7.62,
                     7.65,
                     7.62,
                     7.62,
                     7.45,
                     7.28,
                     7.15,
                     7.05,
                     7.04,
                     7.16,
                     7.47,
                     8.04,
                     8.88,
                     10.01,
                     24.88,
                     16.64,
                     14.59,
                     16.16,
                     17.56,
                     18.62,
                     21.47,
                     22.79,
                     19.29,
                     18.66,
                     17.73,
                     16.54,
                     15.21,
                     13.80,
                     12.36,
                     10.95,
                     9.65,
                     8.40,
                     7.32,
                     6.31,
                     5.43,
                     4.68,
                     4.02,
                     3.45,
                     2.96,
                     2.55,
                     2.19,
                     1.89,
                     1.64,
                     1.53,
                     1.27,
                     1.10,
                     0.99,
                     0.88,
                     0.76,
                     0.68,
                     0.61,
                     0.56,
                     0.54,
                     0.51,
                     0.47,
                     0.47,
                     0.43,
                     0.46,
                     0.47,
                     0.40,
                     0.33,
                     0.27};

//###########################
//   DistinctColorsSampler
//###########################

DistinctColorsSampler::DistinctColorsSampler(double R,double G,double B)
{
    L.push_back(0);
    a.push_back(0);
    b.push_back(0);
    
    sRGB_to_Lab(L[0],a[0],b[0],R,G,B);
}
        
void DistinctColorsSampler::operator() (double &R_,double &G_,double &B_)
{
    int N=100000;
    
    double D_max=0;
    
    double L_push,a_push,b_push;
    
    N=50;
    
//    for(int i=0;i<N;i++)
    for(int i=0;i<N;i++) for(int j=0;j<N;j++) for(int k=0;k<N;k++)
    {
        double R=rand();
        double G=rand();
        double B=rand();
        
        R=i/(N-1.0);
        G=j/(N-1.0);
        B=k/(N-1.0);
        
        double D=std::numeric_limits<double>::max();
        
        double t_L,t_a,t_b;
        sRGB_to_Lab(t_L,t_a,t_b,R,G,B);
        
        for(std::size_t l=0;l<L.size();l++)
        {
            D=std::min(D,(t_L-L[l])*(t_L-L[l])+
                         (t_a-a[l])*(t_a-a[l])+
                         (t_b-b[l])*(t_b-b[l]));
        }
        
        if(D>D_max)
        {
            D_max=D;
            
            R_=R;
            G_=G;
            B_=B;
            
            L_push=t_L;
            a_push=t_a;
            b_push=t_b;
        }
    }
    
    L.push_back(L_push);
    a.push_back(a_push);
    b.push_back(b_push);
}


////////////////////

class CIE31
{
    public:
        Cspline x_spline,y_spline,z_spline;
        
        CIE31()
        {

            std::vector<double> lambda(81),xg(81),yg(81),zg(81);
            
            for(int i=0;i<81;i++)
            {
                lambda[i]=CIE31_base[i][0];
                xg[i]=CIE31_base[i][1];
                yg[i]=CIE31_base[i][2];
                zg[i]=CIE31_base[i][3];
            }
            
            x_spline.init(lambda,xg);
            y_spline.init(lambda,yg);
            z_spline.init(lambda,zg);
        }
        
        double x(double lambda)
        {
            if(lambda>=380e-9 && lambda<=780e-9) return x_spline(lambda);
            else return 0;
        }
        
        double y(double lambda)
        {
            if(lambda>=380e-9 && lambda<=780e-9) return y_spline(lambda);
            else return 0;
        }
        
        double z(double lambda)
        {
            if(lambda>=380e-9 && lambda<=780e-9) return z_spline(lambda);
            else return 0;
        }
};

CIE31 cie31;

class Illuminant
{
    public:
        Cspline val_spline;
        double X,Y,Z;
        
        Illuminant(double *val)
            :X(0), Y(0), Z(0)
        {
            std::vector<double> lambda(81),val_s(81);
            
            for(int i=0;i<81;i++)
            {
                lambda[i]=(380+5*i)*1e-9;
                
                val_s[i]=val[i];
                
                X+=val[i]*cie31.x(lambda[i]);
                Y+=val[i]*cie31.y(lambda[i]);
                Z+=val[i]*cie31.z(lambda[i]);
            }
            
            val_spline.init(lambda,val_s);
            
            X*=100.0/Y;
            Z*=100.0/Y;
            Y=100.0;
        }
        
        double operator () (double lambda)
        {
            if(lambda>=380e-9 && lambda<=780e-9) return val_spline(lambda);
            else return 0;
        }
};

Illuminant d65(D65_base);
Illuminant fl2(FL2_base);

void spectrum_to_XYZ(std::vector<double> const &lambda,std::vector<double> const &spectrum,std::string const &source,
                     double &X,double &Y,double &Z)
{
    unsigned int i,N=lambda.size();
    
    X=Y=Z=0;
    double Yi=0;
    
    if(N<2) return;
    
    Illuminant *illuminant=&d65;
    
    if(source=="FL2") illuminant=&fl2;
    
    double dl;
    
    for(i=0;i<N;i++)
    {
        if(i==N-1) dl=lambda[i]-lambda[i-1];
        else dl=lambda[i+1]-lambda[i];
        
        double src=(*illuminant)(lambda[i]);
        
        X+=dl*cie31.x(lambda[i])*src*spectrum[i];
        Y+=dl*cie31.y(lambda[i])*src*spectrum[i];
        Z+=dl*cie31.z(lambda[i])*src*spectrum[i];
        
        Yi+=dl*cie31.y(lambda[i])*src;
    }
    
    double k_factor=100/Yi;
    
    X=k_factor*X;
    Y=k_factor*Y;
    Z=k_factor*Z;
}

void spectrum_to_sRGB(std::vector<double> const &lambda,std::vector<double> const &spectrum,std::string const &source,
                      double &R,double &G,double &B)
{
    double X,Y,Z;
    
    spectrum_to_XYZ(lambda,spectrum,source,X,Y,Z);
    XYZ_to_sRGB(X,Y,Z,R,G,B); 
}

double sRGB_inverse_linear_transform(double C)
{
    if(C<=0.0031308*12.92) return C/12.92;
    else
    {
        double a=0.055;
        
        return std::pow((C+a)/(1.0+a),2.4);
    }
}

double sRGB_linear_transform(double C)
{
    if(C<=0.0031308) return 12.92*C;
    else
    {
        double a=0.055; 
        
        return (1.0+a)*std::pow(C,1.0/2.4)-a;
    }
}

void sRGB_to_XYZ(double &X,double &Y,double &Z,double R,double G,double B)
{
    R=sRGB_inverse_linear_transform(R);
    G=sRGB_inverse_linear_transform(G);
    B=sRGB_inverse_linear_transform(B);
    
    X=0.4124564*R+0.3575761*G+0.1804375*B;
    Y=0.2126729*R+0.7151522*G+0.0721750*B;
    Z=0.0193339*R+0.1191920*G+0.9503041*B;
    
    X*=100.0;
    Y*=100.0;
    Z*=100.0;
}

void sRGB_to_Lab(double &L,double &a,double &b,double R,double G,double B)
{
    double X,Y,Z;
    
    sRGB_to_XYZ(X,Y,Z,R,G,B);
    XYZ_to_Lab(L,a,b,X,Y,Z);
}

double Lab_transform(double x)
{
    if(x>std::pow(24.0/116.0,3.0)) return std::pow(x,1.0/3.0);
    else return 841.0/108.0*x+16.0/116.0;
}

void XYZ_to_Lab(double &L,double &a,double &b,double X,double Y,double Z)
{
    X/=d65.X;
    Y/=d65.Y;
    Z/=d65.Z;
    
    L=116.0*Lab_transform(Y)-16.0;
	a=500.0*(Lab_transform(X)-Lab_transform(Y));
	b=200.0*(Lab_transform(Y)-Lab_transform(Z));
}

void XYZ_to_sRGB(double X,double Y,double Z,double &R,double &G,double &B)
{
    double Y65=100.0;
    
    double XN=X/Y65;
    double YN=Y/Y65;
    double ZN=Z/Y65;
    
    R=(+3.2406*XN-1.5372*YN-0.4986*ZN);
    G=(-0.9689*XN+1.8758*YN+0.0415*ZN);
    B=(+0.0557*XN-0.2040*YN+1.0570*ZN);
    
    R=std::clamp(R,0.0,1.0);
    G=std::clamp(G,0.0,1.0);
    B=std::clamp(B,0.0,1.0);
    
    R=sRGB_linear_transform(R);
    G=sRGB_linear_transform(G);
    B=sRGB_linear_transform(B);
}
