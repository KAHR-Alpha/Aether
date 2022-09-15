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

#include <bitmap3.h>

#include <algorithm>

Bitmap::Bitmap()
    :width(0), height(0)
{
    mag[0]='B';
    mag[1]='M';
    app=0;
    start=54;
    sizeh=40;
    col=1;
    comp=0;
    bpp=24;
    resh=2834;
    resv=2834;
    colp=0;
    colimp=0;
    
    set_size(10,10);
}

Bitmap::Bitmap(int x,int y)
    :width(0), height(0)
{
    mag[0]='B';
    mag[1]='M';
    app=0;
    start=54;
    sizeh=40;
    col=1;
    comp=0;
    bpp=24;
    resh=2834;
    resv=2834;
    colp=0;
    colimp=0;
    
    set_size(x,y);
}

void Bitmap::degra(int i,int j,double pos,double min,double max)
{
    double rr=0,rg=0,rb=0;
    double R;

    R=(max-min)/5.0;
    
    double pos2=pos-min;

    if(pos>=min && pos<(R+min))
    {
        rb=1.0;
        rg=0;
        rr=1.0-pos2/R;
    }
    else if(pos>=(R+min) && pos<(2*R+min))
    {
        rb=1.0;
        rg=(pos2-R)/R;
        rr=0;
    }
    else if(pos>=(2*R+min) && pos<(3*R+min))
    {
         rb=1.0-(pos2-2*R)/R;
         rg=1.0;
         rr=0;
    }
    else if(pos>=(3*R+min) && pos<(4*R+min))
    {
         rb=0;
         rg=1.0;
         rr=(pos2-3*R)/R;
    }
    else if(pos>=(4*R+min) && pos<(5*R+min))
    {
         rb=0;
         rg=1.0-(pos2-4*R)/R;
         rr=1.0;
    }
    else if(pos==max)
    {
         rb=0;
         rg=0;
         rr=1.0;
    }

    //return r;
    set(i,j,rr,rg,rb);
}

void Bitmap::degra_bw(int i,int j,double pos,double min,double max)
{
    double rr=0,rg=0,rb=0;
    
    double pos2=(pos-min)/(max-min);
    rb=rg=rr=pos2;
    
    set(i,j,rr,rg,rb);
}

void Bitmap::degra_circ(int i,int j,double pos,double min,double max)
{
    double rr=0,rg=0,rb=0;
    
    double pos6=6.0*(pos-min)/(max-min);

    if(pos6>=0 && pos6<1.0)
    {
        rb=1.0;
        rg=0;
        rr=1.0-pos6;
    }
    else if(pos6>=1.0 && pos6<2.0)
    {
        rb=1.0;
        rg=pos6-1.0;
        rr=0;
    }
    else if(pos6>=2.0 && pos6<3.0)
    {
         rb=1.0-pos6+2.0;
         rg=1.0;
         rr=0;
    }
    else if(pos6>=3.0 && pos6<4.0)
    {
         rb=0;
         rg=1.0;
         rr=pos6-3.0;
    }
    else if(pos6>=4.0 && pos6<5.0)
    {
         rb=0;
         rg=1.0-pos6+4.0;
         rr=1.0;
    }
    else if(pos6>=5.0 && pos6<6.0)
    {
         rb=pos6-5.0;
         rg=0;
         rr=1.0;
    }
    else if(pos6>=6.0)
    {
         rb=1.0;
         rg=0;
         rr=1.0;
    }
    
    set(i,j,rr,rg,rb);
}

/*void Bitmap::degra_circ(int i,int j,double pos,double min,double max)
{
    double rr=0,rg=0,rb=0;
    double R;

    R=(max-min)/6.0;
    
    double pos2=(pos-min)/R;

    if(pos>=min && pos<(R+min))
    {
        rb=1.0;
        rg=0;
        rr=1.0-pos2;
    }
    else if(pos>=(R+min) && pos<(2*R+min))
    {
        rb=1.0;
        rg=pos2-1.0;
        rr=0;
    }
    else if(pos>=(2*R+min) && pos<(3*R+min))
    {
         rb=1.0-pos2+2.0;
         rg=1.0;
         rr=0;
    }
    else if(pos>=(3*R+min) && pos<(4*R+min))
    {
         rb=0;
         rg=1.0;
         rr=pos2-3.0;
    }
    else if(pos>=(4*R+min) && pos<(5*R+min))
    {
         rb=0;
         rg=1.0-pos2+4.0;
         rr=1.0;
    }
    else if(pos>=(5*R+min) && pos<(6*R+min))
    {
         rb=pos2-5.0;
         rg=0;
         rr=1.0;
    }
    else if(pos==max)
    {
         rb=1.0;
         rg=0;
         rr=1.0;
    }

    //return r;
    set(i,j,rr,rg,rb);
}*/

void Bitmap::G2degra(Grid2<double> &G2,std::string K)
{
    int i,j;
    
    double min=G2.min();
    double max=G2.max();

    int Nx=std::max(width,G2.L1());
    int Ny=std::max(height,G2.L2());
        
    for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++)
    {
        degra(i,j,G2(i,j),min,max);
    }}
    
    write(K);
}

void Bitmap::G2degra_fixed(Grid2<double> &G2,std::string K)
{
    int i,j;
    
    double min=0;
    double max=1;

    int Nx=std::max(width,G2.L1());
    int Ny=std::max(height,G2.L2());
        
    for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++)
    {
        degra(i,j,G2(i,j),min,max);
    }}
    
    write(K);
}

void Bitmap::G2degra(Grid2<double> &G2,std::string k1,int t,std::string k2)
{
    int i,j;
    
    std::stringstream K;
    K<<k1<<t<<k2;
    
    double max=-1000;
    double min=1000;

    int Nx=std::max(width,G2.L1());
    int Ny=std::max(height,G2.L2());
    
    for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++)
    {
        min=std::min(min,G2(i,j));
        max=std::max(max,G2(i,j));
    }}
    
    for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++)
    {
        degra(i,j,G2(i,j),min,max);
    }}
    
    write(K.str());
}

void Bitmap::G2degraM(Grid2<double> &G2,std::string k1,int t,std::string k2,double min,double max)
{
    int i,j;
    
    std::stringstream K;
    K<<k1<<t<<k2;
    
    int Nx=std::max(width,G2.L1());
    int Ny=std::max(height,G2.L2());
    
    for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++)
    {
        degra(i,j,G2(i,j),min,max);
    }}
    
    write(K.str());
}

void Bitmap::G2BW(Grid2<double> &G2,std::string K)
{
    int i,j;
    
    int Nx=std::max(width,G2.L1());
    int Ny=std::max(height,G2.L2());
    
    double min=G2.min();
    double max=G2.max();
    
    double tmp;
    for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++)
    {
        tmp=(G2(i,j)-min)/(max-min);
        set(i,j,tmp,tmp,tmp);
    }}
    
    write(K);
}

void Bitmap::G2BW(Grid2<double> &G2,std::string k1,int t,std::string k2)
{
    int i,j;
    
    std::stringstream K;
    K<<k1<<t<<k2;
    
    double min=G2.min();
    double max=G2.max();

    int Nx=std::max(width,G2.L1());
    int Ny=std::max(height,G2.L2());
        
    double tmp;
    for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++)
    {
        tmp=(G2(i,j)-min)/(max-min);
        set(i,j,tmp,tmp,tmp);
    }}
    
    write(K.str());
}

void Bitmap::G2WB(Grid2<double> &G2,std::string K)
{
    int i,j;
    
    double min=G2.min();
    double max=G2.max();

    int Nx=std::max(width,G2.L1());
    int Ny=std::max(height,G2.L2());
        
    double tmp;
    for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++)
    {
        tmp=1.0-(G2(i,j)-min)/(max-min);
        set(i,j,tmp,tmp,tmp);
    }}
    
    write(K);
}

void Bitmap::G2WB(Grid2<double> &G2,std::string k1,int t,std::string k2)
{
    int i,j;
    
    std::stringstream K;
    K<<k1<<t<<k2;
    
    double min=G2.min();
    double max=G2.max();

    int Nx=std::max(width,G2.L1());
    int Ny=std::max(height,G2.L2());
        
    double tmp;
    for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++)
    {
        tmp=1.0-(G2(i,j)-min)/(max-min);
        set(i,j,tmp,tmp,tmp);
    }}
    
    write(K.str());
}

void Bitmap::G3col(Grid3<double> &G3,std::string K)
{
    int i,j;

    int Nx=std::max(width,G3.L1());
    int Ny=std::max(height,G3.L2());
    
    double R,G,B;
    
    for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++)
    {
        R=G3(i,j,0);
        G=G3(i,j,1);
        B=G3(i,j,2);
        
        R=std::clamp(R,0.0,1.0);
        G=std::clamp(G,0.0,1.0);
        B=std::clamp(B,0.0,1.0);
        
        set(i,j,R,G,B);
    }}
    
    write(K);
}

std::mutex& Bitmap::get_mutex() { return modification_mutex; }

void Bitmap::operator ()(int x,int y,double r,double g,double b)
{
    M(x,height-1-y,2)=static_cast<unsigned char>(r*255.0);
    M(x,height-1-y,1)=static_cast<unsigned char>(g*255.0);
    M(x,height-1-y,0)=static_cast<unsigned char>(b*255.0);
}

void Bitmap::set(int x,int y,double r,double g,double b)
{
    M(x,height-1-y,2)=static_cast<unsigned char>(r*255.0);
    M(x,height-1-y,1)=static_cast<unsigned char>(g*255.0);
    M(x,height-1-y,0)=static_cast<unsigned char>(b*255.0);
}

void Bitmap::set_full(double r,double g,double b)
{
    int i,j;
    
    unsigned char bc=b*255;
    unsigned char gc=g*255;
    unsigned char rc=r*255;
    
    for(i=0;i<width;i++){ for(j=0;j<height;j++)
    {
        M(i,j,0)=bc;
        M(i,j,1)=gc;
        M(i,j,2)=rc;
    }}
}

void Bitmap::set_size(int w,int h)
{
    if(width!=w || height!=h)
    {
        width=w;
        height=h;
        
        if (width/4.0==int(width/4.0)) rank=0;
        else if ((width+1)/4.0==int((width+1)/4.0)) rank=3;
        else if ((width+2)/4.0==int((width+2)/4.0)) rank=2;
        else if ((width+3)/4.0==int((width+3)/4.0)) rank=1;
        
        sizeraw=width*height*bpp/8+height*rank+2;
        size=sizeraw+start;
        
        M.init(width,height,3,0);
    }
//    else M=0;
}

#include <cstring>

void Bitmap::write(std::string const &fname)
{
    if(fname.find(".bmp")!=std::string::npos)
    {
        int i,j,k;

        std::ofstream file;
        file.open(fname,std::ios::out|std::ios::binary|std::ios::trunc);

        file<<mag[0]<<mag[1];
        file<<int_to_hex(size,4);
        file<<int_to_hex(app,4);
        file<<int_to_hex(start,4);
        file<<int_to_hex(sizeh,4);
        file<<int_to_hex(width,4);
        file<<int_to_hex(height,4);
        file<<int_to_hex(col,2);
        file<<int_to_hex(bpp,2);
        file<<int_to_hex(comp,4);
        file<<int_to_hex(sizeraw,4);
        file<<int_to_hex(resh,4);
        file<<int_to_hex(resv,4);
        file<<int_to_hex(colp,4);
        file<<int_to_hex(colimp,4);

        for(j=0;j<height;j++)
        {
            for(i=0;i<width;i++)
            {
                for(k=0;k<3;k++)
                {
                    file<<M(i,height-1-j,k);
                }
            }
            file<<int_to_hex(0,rank);
        }
        file<<int_to_hex(0,2);

        file.close();
    }
    else if(fname.find(".png")!=std::string::npos)
    {
        int i,j;
        
        png_image img;
        
        std::memset(&img,0,sizeof(png_image));
        
        img.opaque=NULL;
        img.version=PNG_IMAGE_VERSION;
        img.format=PNG_FORMAT_RGB;
        img.width=width;
        img.height=height;
        
        png_bytep buffer=(png_bytep)malloc(PNG_IMAGE_SIZE(img));
        
        for(j=0;j<height;j++)
        {
            for(i=0;i<width;i++)
            {
                buffer[0+3*(i+width*(height-1-j))]=M(i,height-1-j,2);
                buffer[1+3*(i+width*(height-1-j))]=M(i,height-1-j,1);
                buffer[2+3*(i+width*(height-1-j))]=M(i,height-1-j,0);
            }
        }
        
        png_image_write_to_file(&img,fname.c_str(),0,buffer,0,NULL);
        
        free(buffer);
    }
}

//##########

int degra(double pos,double max,int k)
{
    int r=0;
    double R;
    
    R=max/5.0;
    
    if(pos>=0 && pos<R)
    {
             if(k==0) r=255;
             if(k==1) r=0;
             if(k==2) r=int(255.0-255.0*pos/R);
    }
    else if(pos>=R && pos<2*R)
    {
         if(k==0) r=255;
         if(k==1) r=int(255.0*(pos-R)/R);
         if(k==2) r=0;
    }
    else if(pos>=2*R && pos<3*R)
    {
         if(k==0) r=int(255.0-255.0*(pos-2*R)/R);
         if(k==1) r=255;
         if(k==2) r=0;
    }
    else if(pos>=3*R && pos<4*R)
    {
         if(k==0) r=0;
         if(k==1) r=255;
         if(k==2) r=int(255.0*(pos-3*R)/R);
    }
    else if(pos>=4*R && pos<5*R)
    {
         if(k==0) r=0;
         if(k==1) r=int(255.0-255.0*(pos-4*R)/R);
         if(k==2) r=255;
    }
    else if(pos==max)
    {
         if(k==0) r=0;
         if(k==1) r=0;
         if(k==2) r=255;
    }
    
    return r;
}

void Bitmap::draw_circle(double x,double y,double R,double r,double g,double b)
{
    int i,j;
    
    int i1=static_cast<int>(x-R)+1;
    int i2=static_cast<int>(x+R);
    
    for(i=i1;i<=i2;i++)
    {
        double t=R*R-(i-x)*(i-x);
        
        j=static_cast<int>(0.5+y+std::sqrt(t));
        if(is_in_r(i,0,width) && is_in_r(j,0,height)) set(i,j,r,g,b);
        
        j=static_cast<int>(0.5+y-std::sqrt(t));
        if(is_in_r(i,0,width) && is_in_r(j,0,height)) set(i,j,r,g,b);
    }
    
    int j1=static_cast<int>(y-R)+1;
    int j2=static_cast<int>(y+R);
    
    for(j=j1;j<=j2;j++)
    {
        double t=R*R-(j-y)*(j-y);
        
        i=static_cast<int>(0.5+x+std::sqrt(t));
        if(is_in_r(i,0,width) && is_in_r(j,0,height)) set(i,j,r,g,b);
        
        i=static_cast<int>(0.5+x-std::sqrt(t));
        if(is_in_r(i,0,width) && is_in_r(j,0,height)) set(i,j,r,g,b);
    }
}

void Bitmap::draw_line(double x1,double y1,double x2,double y2,double r,double g,double b)
{
    int i,j;
    
    double x21=x2-x1;
    double y21=y2-y1;
    double mix=y2*x1-y1*x2;
    
    if(std::abs(x2-x1)>std::abs(y2-y1))
    {
        int i1=static_cast<int>(x1+0.5);
        int i2=static_cast<int>(x2+0.5);
        
        if(i1>i2) std::swap(i1,i2);
        
        for(i=i1;i<=i2;i++)
        {
            j=static_cast<int>(0.5+(y21*i-mix)/x21);
            
            if(is_in_r(i,0,width) && is_in_r(j,0,height)) set(i,j,r,g,b);
        }
    }
    else
    {
        int j1=static_cast<int>(y1+0.5);
        int j2=static_cast<int>(y2+0.5);
        
        if(j1>j2) std::swap(j1,j2);
        
        for(j=j1;j<=j2;j++)
        {
            i=static_cast<int>(0.5+(x21*j+mix)/y21);
            
            if(is_in_r(i,0,width) && is_in_r(j,0,height)) set(i,j,r,g,b);
        }
    }
}

void Bitmap::draw_line_radial(double i1,double j1,double rad,double th,double r,double g,double b)
{
    double i2=i1+rad*std::cos(th);
    double j2=j1+rad*std::sin(th);
    
    draw_line(i1,j1,i2,j2,r,g,b);
}

void G2_to_BW(Grid2<double> &G,std::string name)
{
    int Px=G.L1();
    int Py=G.L2();
    
    Bitmap result(Px,Py);
    result.G2BW(G,name);
}

void G2_to_degra(Grid2<double> &G,std::string name)
{
    int Px=G.L1();
    int Py=G.L2();
    
    Bitmap result(Px,Py);
    result.G2degra(G,name);
}

void G2_to_degra_fixed(Grid2<double> &G,std::string name)
{
    int Px=G.L1();
    int Py=G.L2();
    
    Bitmap result(Px,Py);
    result.G2degra_fixed(G,name);
}

void G2_to_WB(Grid2<double> &G,std::string name)
{
    int Px=G.L1();
    int Py=G.L2();
    
    Bitmap result(Px,Py);
    result.G2WB(G,name);
}

void G3_to_col(Grid3<double> &G,std::string name)
{
    int Px=G.L1();
    int Py=G.L2();
    
    Bitmap result(Px,Py);
    result.G3col(G,name);
}


