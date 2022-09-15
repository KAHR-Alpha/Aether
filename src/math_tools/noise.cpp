/**Copyright (c) <2010-2019> <Loïc LE CUNFF>

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
   distribution.*/

#include <noise.h>
#include <geometry.h>

//#############
//   Perlin2
//#############

Perlin2::Perlin2()
    :gsize(256)
{
    int i,j;
    data.init(gsize,gsize,Vector2());
    
    ProgDisp dsp(gsize,"Generating Perlin 2D");
    
    for(i=0;i<gsize;i++)
    {
        for(j=0;j<gsize;j++)
        {
            data(i,j).randnorm();
        }
        ++dsp;
    }
    
    std::cout<<"Done"<<std::endl<<std::endl;
}

double Perlin2::antiblobby(double x,double y,double sc,double amp,int rec)
{
    double r=0;
    double ampb=1;
    double scb=sc;
    
    for(int i=0;i<=rec;i++)
    {
        r+=std::abs((*this)(x,y,scb,ampb));
        scb/=2.0;
        ampb/=2.0;
    }
    
    return 1-r*amp/2.0;
}

double Perlin2::antilightning(double x,double y,double sc,double amp,int rec)
{
    double r=0;
    double ampb=1;
    double scb=sc;
    
    for(int i=0;i<=rec;i++)
    {
        r+=(*this)(x,y,scb,ampb);
        scb/=2.0;
        ampb/=2.0;
    }
    
    return std::fabs(r*amp/2.0);
}

double Perlin2::blobby(double x,double y,double sc,double amp,int rec)
{
    double r=0;
    double ampb=1;
    double scb=sc;
    
    for(int i=0;i<=rec;i++)
    {
        r+=std::abs((*this)(x,y,scb,ampb));
        scb/=2.0;
        ampb/=2.0;
    }
    
    return r*amp/2.0;
}

double Perlin2::cloud(double x,double y,double sc,double amp,int rec)
{
    double r=0;
    double ampb=1;
    double scb=sc;
    
    for(int i=0;i<=rec;i++)
    {
        r+=(*this)(x,y,scb,ampb);
        scb/=2.0;
        ampb/=2.0;
    }
    
    return r*amp/2.0;
}

double Perlin2::lightning(double x,double y,double sc,double amp,int rec)
{
    double r=0;
    double ampb=1;
    double scb=sc;
    
    for(int i=0;i<=rec;i++)
    {
        r+=(*this)(x,y,scb,ampb);
        scb/=2.0;
        ampb/=2.0;
    }
    
    return amp-std::abs(r*amp/2.0);
}

double Perlin2::operator() (double x,double y,double sc,double amp)
{
    int i;
    
    double length=sc*gsize;
    
    double xb=gsize*(x/length-std::floor(x/length));
    double yb=gsize*(y/length-std::floor(y/length));

    int xi=int(xb);
    int yi=int(yb);

    double u=xb-xi;
    double v=yb-yi;
    
    Grid1<Vector2> vb(4,Vector2(0,0));
    vb[0].x=u;     vb[0].y=v;       //0,0
    vb[1].x=u-1.0; vb[1].y=v;       //1,0
    vb[2].x=u-1.0; vb[2].y=v-1.0;   //1,1
    vb[3].x=u;     vb[3].y=v-1.0;   //0,1
    
    /*vb[0].x=1.0-u; vb[0].y=1.0-v;   //0,0
    vb[1].x=u;     vb[1].y=1.0-v;   //1,0
    vb[2].x=u;     vb[2].y=v;       //1,1
    vb[3].x=1.0-u;  vb[3].y=v;       //0,1*/
    
    int xc[4]={0,1,1,0};
    int yc[4]={0,0,1,1};
    
    double coeff,coeff1,coeff2;
    double r=0;
    
    for(i=0;i<4;i++)
    {
        if((xi+xc[i])<=(gsize-1)) xc[i]+=xi;
        else xc[i]=0;
        if((yi+yc[i])<=(gsize-1)) yc[i]+=yi;
        else yc[i]=0;
    }
    
    using std::pow;
    
    for(i=0;i<4;i++)
    {
        u=1.0-std::fabs(vb[i].x);
        v=1.0-std::fabs(vb[i].y);
        //coeff=(3.0*std::pow(u,2)-2.0*std::pow(u,3))*
        //      (3.0*std::pow(v,2)-2.0*std::pow(v,3));
        
        //u=vb[i].x;
        //v=vb[i].y;
        
        //coeff=(6.0*pow(u,5)-15.0*pow(u,4)+10.0*pow(u,3))*
        //      (6.0*pow(v,5)-15.0*pow(v,4)+10.0*pow(v,3));
        
        coeff1=((6.0*u-15.0)*u+10.0)*u*u*u;
        coeff2=((6.0*v-15.0)*v+10.0)*v*v*v;
        coeff=coeff1*coeff2;

        r+=coeff*(vb[i].x*data(xc[i],yc[i]).x+vb[i].y*data(xc[i],yc[i]).y);
    }
    
    return amp*r;
}

//#############
//   Perlin3
//#############

Perlin3::Perlin3(bool init_noise)
    :gsize(64), vb(8), data(gsize,gsize,gsize,Vector3(0,0,0))
{
    set_threads_number(1);
    
    if(init_noise) init();
}

double Perlin3::antilightning(double x,double y,double z,double sc,double amp,int rec,int thID)
{
    double r=0;
    double ampb=1;
    double scb=sc;
    
    for(int i=0;i<=rec;i++)
    {
        r+=(*this)(x,y,z,scb,ampb,thID);
        scb/=2.0;
        ampb/=2.0;
    }
    
    return std::fabs(r*amp/2.0);
}

double Perlin3::cloud(double x,double y,double z,double sc,double amp,int rec,int thID)
{
    double r=0;
    double ampb=1;
    double scb=sc;
    
    for(int i=0;i<=rec;i++)
    {
        r+=(*this)(x,y,z,scb,ampb,thID);
        scb/=2.0;
        ampb/=2.0;
    }
    
    return r*amp/2.0;
}

void Perlin3::init()
{
    int i,j,k;
    
    for(i=0;i<gsize;i++)
    {
        for(j=0;j<gsize;j++)
        {
            for(k=0;k<gsize;k++)
            {
                data(i,j,k).rand_sph();
            }
        }
    }
}

double Perlin3::lightning(double x,double y,double z,double sc,double amp,int rec,int thID)
{
    double r=0;
    double ampb=1;
    double scb=sc;
    
    for(int i=0;i<=rec;i++)
    {
        r+=(*this)(x,y,z,scb,ampb,thID);
        scb/=2.0;
        ampb/=2.0;
    }
    
    return amp-std::abs(r*amp/2.0);
}

double Perlin3::operator() (double x,double y,double z,double sc,double amp,int thID)
{
    int i;
    
    double length=sc*gsize;
    
    double xb=gsize*(x/length-std::floor(x/length));
    double yb=gsize*(y/length-std::floor(y/length));
    double zb=gsize*(z/length-std::floor(z/length));
    
    int xi=static_cast<int>(xb);
    int yi=static_cast<int>(yb);
    int zi=static_cast<int>(zb);
    
    double u=xb-xi;
    double v=yb-yi;
    double w=zb-zi;
    
    std::vector<int> &xct=xc[thID];
    std::vector<int> &yct=yc[thID];
    std::vector<int> &zct=zc[thID];
    std::vector<Vector3> &vbt=vb[thID];
    
    vbt[0].x=u;     vbt[0].y=v;     vbt[0].z=w;    
    vbt[1].x=u-1.0; vbt[1].y=v;     vbt[1].z=w;    
    vbt[2].x=u-1.0; vbt[2].y=v-1.0; vbt[2].z=w;    
    vbt[3].x=u;     vbt[3].y=v-1.0; vbt[3].z=w;    
    vbt[4].x=u;     vbt[4].y=v;     vbt[4].z=w-1.0;
    vbt[5].x=u-1.0; vbt[5].y=v;     vbt[5].z=w-1.0;
    vbt[6].x=u-1.0; vbt[6].y=v-1.0; vbt[6].z=w-1.0;
    vbt[7].x=u;     vbt[7].y=v-1.0; vbt[7].z=w-1.0;
    
    xct[0]=0; yct[0]=0; zct[0]=0;
    xct[1]=1; yct[1]=0; zct[1]=0;
    xct[2]=1; yct[2]=1; zct[2]=0;
    xct[3]=0; yct[3]=1; zct[3]=0;
    xct[4]=0; yct[4]=0; zct[4]=1;
    xct[5]=1; yct[5]=0; zct[5]=1;
    xct[6]=1; yct[6]=1; zct[6]=1;
    xct[7]=0; yct[7]=1; zct[7]=1;
    
    double coeff,coeff1,coeff2,coeff3;
    double r=0;
    
    for(i=0;i<8;i++)
    {
        if((xi+xct[i])<gsize) xct[i]+=xi;
        else xct[i]=0;
        if((yi+yct[i])<gsize) yct[i]+=yi;
        else yct[i]=0;
        if((zi+zct[i])<gsize) zct[i]+=zi;
        else zct[i]=0;
    }
    
    using std::pow;
    
    for(i=0;i<8;i++)
    {
        u=1.0-std::fabs(vbt[i].x);
        v=1.0-std::fabs(vbt[i].y);
        w=1.0-std::fabs(vbt[i].z);
        
        coeff1=((6.0*u-15.0)*u+10.0)*u*u*u;
        coeff2=((6.0*v-15.0)*v+10.0)*v*v*v;
        coeff3=((6.0*w-15.0)*w+10.0)*w*w*w;
        coeff=coeff1*coeff2*coeff3;
        
        r+=coeff*scalar_prod(vbt[i],data(xct[i],yct[i],zct[i]));
    }
    
    return amp*r;
}

void Perlin3::set_threads_number(int Nthr_)
{
    Nthr=Nthr_;
    
    xc.resize(Nthr);
    yc.resize(Nthr);
    zc.resize(Nthr);
    vb.resize(Nthr);
    
    for(int i=0;i<Nthr;i++)
    {
        xc[i].resize(8);
        yc[i].resize(8);
        zc[i].resize(8);
        vb[i].resize(8);
    }
}
