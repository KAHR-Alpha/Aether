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

#include <geometry.h>
#include <sensors.h>
#include <bitmap3.h>

extern const double Pi;
extern const Imdouble Im;
extern std::ofstream plog;

//####################
//     DiffSensor
//####################

DiffSensor::DiffSensor(int type_,
                       int x1_,int x2_,
                       int y1_,int y2_,
                       int z1_,int z2_)
    :SensorFieldHolder(type_,x1_,x2_,y1_,y2_,z1_,z2_,true),
     n_index(1.0)
{
}

DiffSensor::~DiffSensor()
{
}

void DiffSensor::link(FDTD const &fdtd)
{
    SensorFieldHolder::link(fdtd);
    
    n_index=fdtd.get_index(x1,y1,z1);
    
    beta_x.init(Nl,0);
    beta_y.init(Nl,0);
    
    for(int l=0;l<Nl;l++)
    {
        beta_x[l]=fdtd.get_kx(lambda[l]);
        beta_y[l]=fdtd.get_ky(lambda[l]);
    }
}
        
void DiffSensor::treat()
{
    int i,j,l,p,q;
    
    double L1=span1*Dx;
    double L2=span2*Dy;
    
    Imdouble ax=0,
             ay=0,
             az=0;
    
    Imdouble coeff=0;
    
    std::string fname=name;
    fname.append("_difford");
    
    chk_var(fname);
    
    std::ofstream file(fname,std::ios::out|std::ios::trunc);
    
    int pmin_sp=std::numeric_limits<int>::max();
    int pmax_sp=std::numeric_limits<int>::min();
    int qmin_sp=std::numeric_limits<int>::max();
    int qmax_sp=std::numeric_limits<int>::min();
    
    for(l=0;l<Nl;l++)
    {
        double k0=2.0*Pi/lambda[l];
        
        int pmin=static_cast<int>(-L1*(n_index/lambda[l]+beta_x[l]/(2.0*Pi)));
        int pmax=static_cast<int>(+L1*(n_index/lambda[l]-beta_x[l]/(2.0*Pi)));
        
        int qmin=static_cast<int>(-L2*(n_index/lambda[l]+beta_y[l]/(2.0*Pi)));
        int qmax=static_cast<int>(+L2*(n_index/lambda[l]-beta_y[l]/(2.0*Pi)));
        
        pmin_sp=std::min(pmin_sp,pmin);
        pmax_sp=std::max(pmax_sp,pmax);
        
        qmin_sp=std::min(qmin_sp,qmin);
        qmax_sp=std::max(qmax_sp,qmax);
    }
    
    for(l=0;l<Nl;l++)
    {
        double k0=2.0*Pi/lambda[l];
        double kn=k0*n_index;
        double w=2.0*Pi*c_light/lambda[l];
        
        int pmin=static_cast<int>(-L1*(n_index/lambda[l]+beta_x[l]/(2.0*Pi)));
        int pmax=static_cast<int>(+L1*(n_index/lambda[l]-beta_x[l]/(2.0*Pi)));
        
        int qmin=static_cast<int>(-L2*(n_index/lambda[l]+beta_y[l]/(2.0*Pi)));
        int qmax=static_cast<int>(+L2*(n_index/lambda[l]-beta_y[l]/(2.0*Pi)));
        
        Angle inc_th,inc_phi;
        
        double b2=beta_x[l]*beta_x[l]+beta_y[l]*beta_y[l];
        
        if(b2>kn*kn) inc_th=Degree(90);
        else inc_th=std::asin(std::sqrt(b2)/kn);
        
        inc_phi=std::atan2(beta_y[l],beta_x[l]);
        
        std::stringstream out;
        
        out<<lambda[l]<<" ";
        out<<inc_th.degree()<<" ";
        out<<inc_phi.degree()<<" ";
        out<<pmin<<" ";
        out<<pmax<<" ";
        out<<qmin<<" ";
        out<<qmax<<" ";
        
        double F_tot=0;
        
//        for(p=pmin;p<=pmax;p++){ for(q=qmin;q<=qmax;q++)
        for(p=pmin_sp;p<=pmax_sp;p++){ for(q=qmin_sp;q<=qmax_sp;q++)
        {
            ax=0,ay=0,az=0;
            
            double k1=beta_x[l]+2.0*p*Pi/L1;
            double k2=beta_y[l]+2.0*q*Pi/L2;
            
            double k3s=kn*kn-k1*k1-k2*k2;
            
            double F_pq=0;
            
//            double phi=std::atan2(k2,k1)*180.0/Pi;
//            double theta=90;
            
            if(k3s>0)
            {
//                theta=90.0-std::atan2(std::sqrt(k3s),
//                                      std::sqrt(k1*k1+k2*k2))*180.0/Pi;
                
                double k1d=k1*Dx;
                double k2d=k2*Dy;
                
                for(j=0;j<span2;j++)
                {
                    for(i=0;i<span1;i++)
                    {
                        coeff=std::exp(-(k1d*i+k2d*j)*Im);
                        
                        ax+=sp_Ex(i,j,l)*coeff;
                        ay+=sp_Ey(i,j,l)*coeff;
                        az+=sp_Ez(i,j,l)*coeff;
                    }
                }
        
                if(type==NORMAL_X || type==NORMAL_XM)
                {
                    ax*=Dy*Dz;
                    ay*=Dy*Dz;
                    az*=Dy*Dz;
                }
                else if(type==NORMAL_Y || type==NORMAL_YM)
                {
                    ax*=Dx*Dz;
                    ay*=Dx*Dz;
                    az*=Dx*Dz;
                }
                else if(type==NORMAL_Z || type==NORMAL_ZM)
                {
                    ax*=Dx*Dy;
                    ay*=Dx*Dy;
                    az*=Dx*Dy;
                }
                
                ax/=L1*L2;
                ay/=L1*L2;
                az/=L1*L2;
                
                ImVector3 a_pq(ax,ay,az);
                ImVector3 k_pq(k1,k2,std::sqrt(k3s));
                
                if(type==NORMAL_ZM) k_pq.z=-k_pq.z;
                
                ImVector3 CC=crossprod(a_pq,crossprod(k_pq,a_pq.conj()));
                
                F_pq=L1*L2*CC.z.real()/(2.0*w*mu0);
                if(type==NORMAL_ZM) F_pq=-F_pq;
            }
            
            out<<p<<" ";
            out<<q<<" ";
//            out<<theta<<" ";
//            out<<phi<<" ";
            out<<F_pq<<" ";
            
            F_tot+=F_pq;
        }}
        
        file<<out.str()<<" "<<F_tot<<std::endl;
    }
}

void get_max_orders(std::string const &diff_fname,int &pmin,int &pmax,int &qmin,int &qmax)
{
    int l;
    int Nl=fcountlines(diff_fname);
    
    std::string buf;
    
    std::ifstream diff_file(diff_fname,std::ios::in|std::ios::binary);
    
    double tmp;
    int t_pmin,t_pmax;
    int t_qmin,t_qmax;
    
    std::getline(diff_file,buf);
    std::stringstream strm(buf);
    
    strm>>tmp;
    strm>>pmin; strm>>pmax;
    strm>>qmin; strm>>qmax;
    
    for(l=1;l<Nl;l++)
    {        
        std::getline(diff_file,buf);
        strm.str(buf);
        
        strm>>tmp; strm>>tmp; strm>>tmp;
        strm>>t_pmin; strm>>t_pmax;
        strm>>t_qmin; strm>>t_qmax;
        
        pmin=std::min(pmin,t_pmin);
        pmax=std::max(pmax,t_pmax);
        qmin=std::min(qmin,t_qmin);
        qmax=std::max(qmax,t_qmax);
    }
}

std::string diffract_average_files(std::vector<std::string> const &fnames,std::string const &out_fname_)
{
    int f,l,p,q;
    int Nf=fnames.size();
    
    if(Nf<1)
    {
        std::cout<<"Diffraction files averaging error"<<std::endl;
        std::exit(0);
    }
    
    std::string out_fname=out_fname_;
    
    if(out_fname.size()==0)
    {
        out_fname="buf/diffract_tmp_";
        out_fname.append(std::to_string(randi()));
    }
    
    std::vector<std::ifstream> files(Nf);
    
    for(f=0;f<Nf;f++)
    {
        files[f].open(fnames[f],std::ios::in|std::ios::binary);
        if(!files[f].is_open())
        {
            std::cout<<"Couldn't open "<<fnames[f]<<std::endl;
            std::exit(0);
        }
    }
    
    int Nl=fcountlines(fnames[0]);
    
    for(f=0;f<Nf;f++)
    {
        if(Nl!=fcountlines(fnames[f]))
        {
            std::cout<<"Invalid file "<<fnames[f]<<std::endl;
            std::exit(0);
        }
    }
    
    int pmin=0,pmax=0;
    int qmin=0,qmax=0;
    
    double val,val_buf;
    
    std::string buf1,buf2,buf3,buf4;
    
    std::ofstream f_out(out_fname,std::ios::out|std::ios::binary|std::ios::trunc);
    
    for(l=0;l<Nl;l++)
    {
        for(f=0;f<Nf;f++)
        {
            files[f]>>buf1;
//            files[f]>>buf2;
//            files[f]>>buf3;
            
            files[f]>>pmin;
            files[f]>>pmax;
            files[f]>>qmin;
            files[f]>>qmax;
        }
        
        f_out<<buf1<<" ";
//        f_out<<buf2<<" ";
//        f_out<<buf3<<" ";
        
        f_out<<pmin<<" ";
        f_out<<pmax<<" ";
        f_out<<qmin<<" ";
        f_out<<qmax<<" ";
        
        for(p=pmin;p<=pmax;p++){ for(q=qmin;q<=qmax;q++)
        {
            for(f=0;f<Nf;f++)
            {
                files[f]>>buf1;
                files[f]>>buf2;
                files[f]>>buf3;
                files[f]>>buf4;
            }
                    
            f_out<<buf1<<" ";
            f_out<<buf2<<" ";
            f_out<<buf3<<" ";
            f_out<<buf4<<" ";
            
            val=0;
            
            for(f=0;f<Nf;f++)
            {
                files[f]>>val_buf;
                val+=val_buf;
            }
            
            f_out<<val/static_cast<double>(Nf)<<" ";
        }}
        
        val=0;
        
        for(f=0;f<Nf;f++)
        {
            files[f]>>val_buf;
            val+=val_buf;
        }
        
        f_out<<val/static_cast<double>(Nf);
        f_out<<std::endl;
    }
    
    return out_fname;
}

int orders_normalize_FID=0;

int get_ON_ID()
{
    int out=orders_normalize_FID;
    orders_normalize_FID++;
    return out;
}

std::string diffract_orders_normalize(std::string const &diff_fname,std::string out_fname)
{
    int l,p,q;
    
    if(out_fname.size()==0)
    {
        out_fname="buf/diffract_tmp_";
        out_fname.append(std::to_string(get_ON_ID()));
    }
    
    std::ifstream diff_file(diff_fname,std::ios::in|std::ios::binary);
    
    if(!diff_file.is_open())
    {
        std::cout<<"Couldn't load "<<diff_fname<<std::endl;
        std::exit(0);
    }
    
    std::ofstream out_file(out_fname,std::ios::out|std::ios::binary|std::ios::trunc);
    
    int Nl=fcountlines(diff_fname);
    
    int pmin_f,pmax_f;
    int qmin_f,qmax_f;
    
    get_max_orders(diff_fname,pmin_f,pmax_f,qmin_f,qmax_f);
    
    int pmin,pmax;
    int qmin,qmax;
    double F_pq,F_tot;
    std::string buf;
    
    for(l=0;l<Nl;l++)
    {
        diff_file>>buf;  out_file<<buf<<" ";
        diff_file>>buf;  out_file<<buf<<" ";
        diff_file>>buf;  out_file<<buf<<" ";
        diff_file>>pmin; out_file<<pmin_f<<" ";
        diff_file>>pmax; out_file<<pmax_f<<" ";
        diff_file>>qmin; out_file<<qmin_f<<" ";
        diff_file>>qmax; out_file<<qmax_f<<" ";
        
        for(p=pmin_f;p<=pmax_f;p++){ for(q=qmin_f;q<=qmax_f;q++)
        {
            if(is_in(p,pmin,pmax) && is_in(q,qmin,qmax))
            {
                diff_file>>buf; out_file<<buf<<" ";
                diff_file>>buf; out_file<<buf<<" ";
                diff_file>>buf; out_file<<buf<<" ";
                diff_file>>buf; out_file<<buf<<" ";
                
                diff_file>>F_pq; out_file<<F_pq<<" ";
            }
            else
            {
                out_file<<p<<" ";
                out_file<<q<<" ";
                out_file<<90<<" ";
                out_file<<std::atan2(q,p)*180.0/Pi<<" ";
                
                out_file<<0<<" ";
            }
        }}
        
        diff_file>>F_tot; out_file<<F_tot;
        out_file<<std::endl;
    }
    
    return out_fname;
}

std::string diffract_power_normalize(std::string const &diff_fname,std::string const &base_fname,std::string out_fname)
{
    int l,p,q;
    
    if(out_fname.size()==0)
    {
        out_fname="buf/diffract_tmp_";
        out_fname.append(std::to_string(randi()));
    }
    
    std::ifstream diff_file(diff_fname,std::ios::in|std::ios::binary);
    std::ifstream base_file(base_fname,std::ios::in|std::ios::binary);
    
    if(!diff_file.is_open())
    {
        std::cout<<"Couldn't open "<<diff_fname<<std::endl;
        std::exit(0);
    }
    
    if(!base_file.is_open())
    {
        std::cout<<"Couldn't open "<<base_fname<<std::endl;
        std::exit(0);
    }
    
    int Nl=fcountlines(diff_fname);
    
    if(Nl!=fcountlines(base_fname))
    {
        std::cout<<"Files mismatch: "<<diff_fname<<" , "<<base_fname<<std::endl;
        std::exit(0);
    }
    
    int pmin=0,pmax=0;
    int qmin=0,qmax=0;
    
    double val,baseline;
    
    std::string buf1,buf2,buf3,buf4;
    
    std::ofstream f_out(out_fname,std::ios::out|std::ios::binary|std::ios::trunc);
    
    for(l=0;l<Nl;l++)
    {
        diff_file>>buf1;
//        diff_file>>buf2;
//        diff_file>>buf3;
        
        diff_file>>pmin;
        diff_file>>pmax;
        diff_file>>qmin;
        diff_file>>qmax;
        
        f_out<<buf1<<" ";
//        f_out<<buf2<<" ";
//        f_out<<buf3<<" ";
        
        f_out<<pmin<<" ";
        f_out<<pmax<<" ";
        f_out<<qmin<<" ";
        f_out<<qmax<<" ";
        
        base_file>>buf1;
        base_file>>baseline;
        
        for(p=pmin;p<=pmax;p++){ for(q=qmin;q<=qmax;q++)
        {
            diff_file>>buf1;
            diff_file>>buf2;
            diff_file>>buf3;
            diff_file>>buf4;
            
            f_out<<buf1<<" ";
            f_out<<buf2<<" ";
            f_out<<buf3<<" ";
            f_out<<buf4<<" ";
            
            diff_file>>val;
            f_out<<val/baseline<<" ";
        }}
    
        diff_file>>val;
        f_out<<val/baseline<<std::endl;
    }
    
    return out_fname;
}

void diffract_renorm(std::string const &diff_fname,std::string const &base_fname,std::string const &out_fname)
{
    int l,p,q;
    
    std::ifstream diff_file(diff_fname,std::ios::in|std::ios::binary);
    std::ifstream base_file(base_fname,std::ios::in|std::ios::binary);
    
    if(!diff_file.is_open()) { std::cout<<"Can't open file: "<<diff_fname<<std::endl; return; }
    if(!base_file.is_open()) { std::cout<<"Can't open file: "<<base_fname<<std::endl; return; }
    
    int Nl=fcountlines(diff_fname);
    if(Nl!=fcountlines(base_fname)) { std::cout<<"Number mismatch"<<std::endl; return; }
    
    std::ofstream out_file(out_fname,std::ios::out|std::ios::binary|std::ios::trunc);
    
    int pmin_f,pmax_f;
    int qmin_f,qmax_f;
    
    get_max_orders(diff_fname,pmin_f,pmax_f,qmin_f,qmax_f);
    
    chk_var(pmin_f);
    chk_var(pmax_f);
    chk_var(qmin_f);
    chk_var(qmax_f);
    
    int pmin,pmax;
    int qmin,qmax;
    double tmp,baseline,F_pq,F_tot;
    std::string buf;
    
    for(l=0;l<Nl;l++)
    {
        base_file>>tmp;
        base_file>>baseline;
        
        diff_file>>buf;  out_file<<buf<<" ";
        diff_file>>pmin; out_file<<pmin_f<<" ";
        diff_file>>pmax; out_file<<pmax_f<<" ";
        diff_file>>qmin; out_file<<qmin_f<<" ";
        diff_file>>qmax; out_file<<qmax_f<<" ";
        
        for(p=pmin_f;p<=pmax_f;p++){ for(q=qmin_f;q<=qmax_f;q++)
        {
            if(is_in(p,pmin,pmax) && is_in(q,qmin,qmax))
            {
                diff_file>>buf; out_file<<buf<<" ";
                diff_file>>buf; out_file<<buf<<" ";
                diff_file>>buf; out_file<<buf<<" ";
                diff_file>>buf; out_file<<buf<<" ";
                
                diff_file>>F_pq; out_file<<F_pq/baseline<<" ";
            }
            else
            {
                out_file<<p<<" ";
                out_file<<q<<" ";
                out_file<<90<<" ";
                out_file<<std::atan2(q,p)*180.0/Pi<<" ";
                
                out_file<<0<<" ";
            }
        }}
        
        diff_file>>F_tot; out_file<<F_tot/baseline;
        out_file<<std::endl;
    }
}

void diffract_renorm(std::string const &diff_fname,std::vector<std::string> const &base_fname,std::string const &out_fname)
{
    int l,p,q;
    
    std::ifstream diff_file(diff_fname,std::ios::in|std::ios::binary);
    
    if(!diff_file.is_open()) { std::cout<<"Can't open file: "<<diff_fname<<std::endl; return; }
    
    std::vector<std::ifstream> base_file(base_fname.size());
    for(unsigned int i=0;i<base_fname.size();i++)
    {
        base_file[i].open(base_fname[i],std::ios::in|std::ios::binary);
        if(!base_file[i].is_open()) { std::cout<<"Can't open file: "<<base_fname[i]<<std::endl; return; }
    }
    
    int Nl=fcountlines(diff_fname);
    for(unsigned int i=0;i<base_fname.size();i++)
    {
        if(Nl!=fcountlines(base_fname[i])) { std::cout<<"Number mismatch "<<Nl<<" "<<base_fname[i]<<std::endl; return; }
    }
    
    std::ofstream out_file(out_fname,std::ios::out|std::ios::binary|std::ios::trunc);
    
    int pmin_f,pmax_f;
    int qmin_f,qmax_f;
    
    get_max_orders(diff_fname,pmin_f,pmax_f,qmin_f,qmax_f);
    
    chk_var(pmin_f);
    chk_var(pmax_f);
    chk_var(qmin_f);
    chk_var(qmax_f);
    
    int pmin,pmax;
    int qmin,qmax;
    double tmp,baseline,baseline_buf,F_pq,F_tot;
    std::string buf;
    
    for(l=0;l<Nl;l++)
    {
        baseline=0;
        
        for(unsigned int i=0;i<base_file.size();i++)
        {
            base_file[i]>>tmp;
            base_file[i]>>baseline_buf;
            
            baseline+=baseline_buf;
        }
        
        diff_file>>buf;  out_file<<buf<<" ";
        diff_file>>pmin; out_file<<pmin_f<<" ";
        diff_file>>pmax; out_file<<pmax_f<<" ";
        diff_file>>qmin; out_file<<qmin_f<<" ";
        diff_file>>qmax; out_file<<qmax_f<<" ";
        
        for(p=pmin_f;p<=pmax_f;p++){ for(q=qmin_f;q<=qmax_f;q++)
        {
            if(is_in(p,pmin,pmax) && is_in(q,qmin,qmax))
            {
                diff_file>>buf; out_file<<buf<<" ";
                diff_file>>buf; out_file<<buf<<" ";
                diff_file>>buf; out_file<<buf<<" ";
                diff_file>>buf; out_file<<buf<<" ";
                
                diff_file>>F_pq; out_file<<F_pq/baseline<<" ";
            }
            else
            {
                out_file<<p<<" ";
                out_file<<q<<" ";
                out_file<<90<<" ";
                out_file<<std::atan2(q,p)*180.0/Pi<<" ";
                
                out_file<<0<<" ";
            }
        }}
        
        diff_file>>F_tot; out_file<<F_tot/baseline;
        out_file<<std::endl;
    }
}

//####################
//   DiffSensor_FT
//####################

//DiffSensor_FT::DiffSensor_FT(int z1_i,double ref_index_i)
//    :ref_index(ref_index_i)
//{
//    z1=z1_i;
//    step=0;
//}
//
//void DiffSensor_FT::deep_feed(Grid3<double> const &Ex,Grid3<double> const &Ey,Grid3<double> const &Ez,
//                      Grid3<double> const &Hx,Grid3<double> const &Hy,Grid3<double> const &Hz)
//{
//    int i,j,l;
//    
//    for(l=0;l<Nl;l++)
//    {
//        double w=2.0*Pi*c_light/lambda[l];
//        Imdouble tf_coeff=std::exp(w*step*Dt*Im);
//        
//        for(i=0;i<Nx;i++){ for(j=0;j<Ny;j++)
//        {
//            acc_Ex(i,j,l)+=Ex(i,j,z1)*tf_coeff;
//            acc_Ey(i,j,l)+=Ey(i,j,z1)*tf_coeff;
//            acc_Ez(i,j,l)+=Ez(i,j,z1)*tf_coeff;
//        }}
//    }
//}
//
//
//
//void DiffSensor_FT::set_spectrum(double lambda_i)
//{
//    Sensor::set_spectrum(lambda_i);
//    
//    acc_Ex.init(Nx,Ny,1,0);
//    acc_Ey.init(Nx,Ny,1,0);
//    acc_Ez.init(Nx,Ny,1,0);
//}
//
//void DiffSensor_FT::set_spectrum(Grid1<double> const &lambda_i)
//{
//    Sensor::set_spectrum(lambda_i);
//    
//    acc_Ex.init(Nx,Ny,Nl,0);
//    acc_Ey.init(Nx,Ny,Nl,0);
//    acc_Ez.init(Nx,Ny,Nl,0);
//}
//
//void DiffSensor_FT::treat()
//{
//    int i,j,k,l;
//        
//    int Nx_sp=1000;
//    int Ny_sp=1000;
//    
//    for(l=0;l<Nl;l++)
//    {
//        std::cout<<"Computing Diffraction pattern "<<l<<"/"<<Nl<<std::endl;
//        double k0=2.0*Pi/lambda[l];
//        double kn=k0*ref_index;
//        
//        double kx,ky;
//        
//        double dkx=2.0*kn/(Nx_sp-1.0);
//        double dky=2.0*kn/(Ny_sp-1.0);
//        
//        Imdouble coeffx,coeffy;
//        
//        Grid2<Imdouble> tmptf_x(Nx,Ny_sp,0);
//        Grid2<Imdouble> tmptf_y(Nx,Ny_sp,0);
//        Grid2<Imdouble> tmptf_z(Nx,Ny_sp,0);
//        
//        Grid2<Imdouble> tf_x(Nx_sp,Ny_sp,0);
//        Grid2<Imdouble> tf_y(Nx_sp,Ny_sp,0);
//        Grid2<Imdouble> tf_z(Nx_sp,Ny_sp,0);
//        
//        Grid2<double> sp_x(Nx_sp,Ny_sp,0);
//        Grid2<double> sp_y(Nx_sp,Ny_sp,0);
//        Grid2<double> sp_z(Nx_sp,Ny_sp,0);
//        
//        for(k=0;k<Ny_sp;k++)
//        {
//            ky=-kn+k*dky;
//            
//            for(j=0;j<Ny;j++)
//            {
//                coeffy=std::exp(-ky*j*Dy*Im);
//                
//                for(i=0;i<Nx;i++)
//                {
//                    tmptf_x(i,k)+=acc_Ex(i,j,l)*coeffy;
//                    tmptf_y(i,k)+=acc_Ey(i,j,l)*coeffy;
//                    tmptf_z(i,k)+=acc_Ez(i,j,l)*coeffy;
//                }
//            }
//        }
//        
//        for(k=0;k<Nx_sp;k++)
//        {
//            kx=-kn+k*dkx;
//            
//            for(i=0;i<Nx;i++)
//            {
//                coeffx=std::exp(-kx*i*Dx*Im);
//                
//                for(j=0;j<Ny_sp;j++)
//                {
//                    tf_x(k,j)+=tmptf_x(i,j)*coeffx;
//                    tf_y(k,j)+=tmptf_y(i,j)*coeffx;
//                    tf_z(k,j)+=tmptf_z(i,j)*coeffx;
//                }
//            }
//        }
//        
//        int N=100;
//        N=static_cast<int>(100/ref_index);
//        double lx=Dx*Nx;
//        double ly=Dy*Ny;
//        
//        for(i=0;i<Nx_sp;i++)
//        {
//            kx=-kn+i*dkx;
//            
//            coeffx=(1.0-std::exp(-(N+1)*kx*lx*Im))/(1.0-std::exp(-kx*lx*Im));
//            
//            for(j=0;j<Ny_sp;j++)
//            {
//                ky=-kn+j*dky;
//                
//                coeffy=(1.0-std::exp(-(N+1)*ky*ly*Im))/(1.0-std::exp(-ky*ly*Im));
//                                
//                sp_x(i,j)=std::abs(tf_x(i,j)*coeffx*coeffy);
//                sp_y(i,j)=std::abs(tf_y(i,j)*coeffx*coeffy);
//                sp_z(i,j)=std::abs(tf_z(i,j)*coeffx*coeffy);
//            }
//        }
//        
//        std::stringstream fname_x,fname_y,fname_z;
//        fname_x<<"diffract_"<<name<<"_Ex_"<<static_cast<int>(lambda[l]/1e-9+0.5)<<".png";
//        fname_y<<"diffract_"<<name<<"_Ey_"<<static_cast<int>(lambda[l]/1e-9+0.5)<<".png";
//        fname_z<<"diffract_"<<name<<"_Ez_"<<static_cast<int>(lambda[l]/1e-9+0.5)<<".png";
//        
//        G2_to_degra(sp_x,fname_x.str());
//        G2_to_degra(sp_y,fname_y.str());
//        G2_to_degra(sp_z,fname_z.str());
//    }
//}
