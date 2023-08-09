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

#include <gui_diffract_pattern.h>

#include <wx/splitter.h>

extern const Imdouble Im;
extern std::ofstream plog;

DiffPatternFrame::DiffPatternFrame(wxString const &title)
    :BaseFrame(title)
{
    wxSplitterWindow *splitter=new wxSplitterWindow(this);
    
    // Controls
    
    ctrl_panel=new wxScrolledWindow(splitter);
    
    wxBoxSizer *ctrl_sizer=new wxBoxSizer(wxVERTICAL);
    wxSizerFlags default_ctrl_sizer_flags=wxSizerFlags().Expand().Border(wxALL,2);
    
    // - Input
    
    input_data=new FileSelector(ctrl_panel,"Input data");
    input_data->Bind(EVT_FILE_SELECTOR,&DiffPatternFrame::evt_new_data,this);
    
    ctrl_sizer->Add(input_data,default_ctrl_sizer_flags);
    
    // - Spectrum
    
    lambda=new WavelengthSelector(ctrl_panel,"Wavelength",500e-9);
    
    ctrl_sizer->Add(lambda,default_ctrl_sizer_flags);
    
    // - Scale
    
    input_scale=new LengthSelector(ctrl_panel,"Aperture x scale",10e-6,true);
    
    ctrl_sizer->Add(input_scale,default_ctrl_sizer_flags);
    
    // - Screen
    
    screen_distance=new LengthSelector(ctrl_panel,"Screen distance",0.1,true);
    screen_x_res=new NamedTextCtrl<int>(ctrl_panel,"Screen X resolution",100,true);
    screen_y_res=new NamedTextCtrl<int>(ctrl_panel,"Screen Y resolution",100,true);
    
    ctrl_sizer->Add(screen_distance,default_ctrl_sizer_flags);
    ctrl_sizer->Add(screen_x_res,default_ctrl_sizer_flags);
    ctrl_sizer->Add(screen_y_res,default_ctrl_sizer_flags);
    
    // - Compute
    
    wxButton *compute_btn=new wxButton(ctrl_panel,wxID_ANY,"Compute");
    compute_btn->Bind(wxEVT_BUTTON,&DiffPatternFrame::evt_compute,this);
    ctrl_sizer->Add(compute_btn,default_ctrl_sizer_flags);
    
    // - Controls wrapping up
    
    ctrl_panel->SetSizer(ctrl_sizer);
    ctrl_panel->FitInside();
    ctrl_panel->SetScrollRate(10,10);
    
    
    // Display
    
    wxNotebook *display_book=new wxNotebook(splitter,wxID_ANY);
    
    input_display=new GL_2D_display(display_book);
    output_display=new GL_2D_display(display_book);
    
    display_book->AddPage(input_display,"Input image");
    display_book->AddPage(output_display,"Output image");
    
    // Wrapping up
    
    splitter->SplitVertically(ctrl_panel,display_book,250);
    splitter->SetMinimumPaneSize(250);
}

DiffPatternFrame::~DiffPatternFrame()
{
}

Imdouble DiffPatternFrame::compute_data_spectrum(double kx,double ky)
{
    Imdouble R=0;
    
    for(int j=0;j<Ny_d;j++)
    {
        Imdouble kx_buffer=0;
        
        for(std::size_t i=0;i<compressed_data_start[j].size();i++)
        {
            if(kx==0) kx_buffer+=Dx*(compressed_data_end[j][i]-compressed_data_start[j][i]);
            else
            {
                kx_buffer+=( std::exp(-Dx*(compressed_data_end[j][i]-0.5)*kx*Im)
                            -std::exp(-Dx*(compressed_data_start[j][i]-0.5)*kx*Im))/kx*Im;
            }
        }
        
        R+=kx_buffer*std::exp(-Dx*j*ky*Im)*Dx;
    }
    
    return R;
}

Imdouble integral_2D_constant_approx(Grid2<Imdouble> const &G,
                                     double x_min,double x_max,
                                     double y_min,double y_max)
{
    Imdouble R=0;
    
    int i,j,Nx=G.L1(),Ny=G.L2();
        
    R+=(G(0,0)+G(Nx-1,0)+G(Nx-1,Ny-1)+G(0,Ny-1))/4.0;
    
    for(i=1;i<Nx-1;i++)
    {
        R+=G(i,0)/2.0;
        R+=G(i,Ny-1)/2.0;
    }
    
    for(j=1;i<Ny-1;j++)
    {
        R+=G(0,j)/2.0;
        R+=G(Nx-1,j)/2.0;
    }
    
    
    for(j=1;j<Ny-1;j++)
    {
        for(i=1;i<Nx-1;i++) R+=G(i,j);
    }
    
    double Dx=(x_max-x_min)/(Nx-1.0);
    double Dy=(y_max-y_min)/(Ny-1.0);
        
    R*=Dx*Dy;
    
    return R;
}

Imdouble integral_abs_2D_constant_approx(Grid2<Imdouble> const &G,
                                         double x_min,double x_max,
                                         double y_min,double y_max)
{
    using std::abs;
    
    Imdouble R=0;
    
    int i,j,Nx=G.L1(),Ny=G.L2();
        
    R+=(abs(G(0,0))+abs(G(Nx-1,0))+abs(G(Nx-1,Ny-1))+abs(G(0,Ny-1)))/4.0;
    
    for(i=1;i<Nx-1;i++)
    {
        R+=abs(G(i,0))/2.0;
        R+=abs(G(i,Ny-1))/2.0;
    }
    
    for(j=1;i<Ny-1;j++)
    {
        R+=abs(G(0,j))/2.0;
        R+=abs(G(Nx-1,j))/2.0;
    }
    
    
    for(j=1;j<Ny-1;j++)
    {
        for(i=1;i<Nx-1;i++) R+=abs(G(i,j));
    }
    
    double Dx=(x_max-x_min)/(Nx-1.0);
    double Dy=(y_max-y_min)/(Ny-1.0);
        
    R*=Dx*Dy;
    
    return R;
}

extern "C"
{
    #include <fftw3.h>
}

void G2_FFT(Grid2<Imdouble> &out_data,Grid2<Imdouble> const &in_data,int sign)
{
    fftw_complex *in,*out;
    fftw_plan plan;
    
    int Nx=in_data.L1();
    int Ny=in_data.L2();
    
    in=(fftw_complex*)fftw_malloc(Nx*Ny*sizeof(fftw_complex));
    out=(fftw_complex*)fftw_malloc(Nx*Ny*sizeof(fftw_complex));
    
    if(sign<0) plan=fftw_plan_dft_2d(Nx,Ny,in,out,FFTW_FORWARD,FFTW_ESTIMATE);
    else plan=fftw_plan_dft_2d(Nx,Ny,in,out,FFTW_BACKWARD,FFTW_ESTIMATE);
    
    for(int i=0;i<Nx;i++)
    {
        for(int j=0;j<Ny;j++)
        {
            int index=j+i*Ny;
            
            in[index][0]=in_data(i,j).real();
            in[index][1]=in_data(i,j).imag();
        }
    }
    
    fftw_execute(plan);
    
    fftw_free(in);
    
    // Output
    
    out_data.init(Nx,Ny);
    
    for(int i=0;i<Nx;i++)
    {
        for(int j=0;j<Ny;j++)
        {
            int index=j+i*Ny;
            
            out_data(i,j)=out[index][0]+
                          out[index][1]*Im;
        }
    }
    
    fftw_free(out);
    
    fftw_destroy_plan(plan);
}

void DiffPatternFrame::evt_compute(wxCommandEvent &event)
{
    evt_compute_double_FFT();
//    evt_compute_fraunhofer();
//    evt_compute_FT_growth();
}

void DiffPatternFrame::evt_compute_fraunhofer()
{
    int i,j;
    //int ik,jk;
    
    Nx_s=screen_x_res->get_value();
    Ny_s=screen_x_res->get_value();
    
    screen_data.init(Nx_s,Ny_s,0);
    
    Nx_d=data.L1();
    Ny_d=data.L2();
    
    std::vector<double> data_ky(Nx_d);
    
    int x_start=Nx_d-1;
    int x_end=0;
    
    int y_start=Ny_d-1;
    int y_end=0;
    
    for(i=0;i<Nx_d;i++)
    {
        for(j=0;j<Ny_d;j++)
        {
            if(data(i,j)!=0)
            {
                x_start=std::min(x_start,i);
                y_start=std::min(y_start,j);
                
                x_end=std::max(x_end,i);
                y_end=std::max(y_end,j);
            }
        }
    }
    
    x_end+=1;
    y_end+=1;
    
    int x_span=x_end-x_start;
    int y_span=y_end-y_start;
    
    Dx=input_scale->get_length()/x_span;
    
    int span_max=std::max(x_span,y_span);
    
    int extra=3;
    
    int Nkx=2*extra*span_max+x_span;
    int Nky=2*extra*span_max+y_span;
    
    Grid2<Imdouble> fft_in_data(Nkx,Nky,0),
                    fft_out_data;
                    
    for(i=x_start;i<x_end;i++)
    {
        for(j=y_start;j<y_end;j++)
        {
            fft_in_data(i-x_start+extra*span_max,j-y_start+extra*span_max)=data(i,j);
        }
    }
    
    //double k0=2.0*Pi/500e-9;
    
    G2_FFT(fft_out_data,fft_in_data,-1);
    
    // Shift
    
    for(i=0;i<Nkx;i++)
    {
        for(j=0;j<Nky;j++)
        {
            fft_in_data((i+Nkx/2)%Nkx,(j+Nky/2)%Nky)=fft_out_data(i,j);
        }
    }
    
    fft_out_data=fft_in_data;
    
    // Propagator
    
//    double z=screen_distance->get_length();
//    
//    for(i=0;i<Nkx;i++)
//    {
//        for(j=0;j<Nky;j++)
//        {
//            double kx=2.0*Pi*(i-Nkx/2)/(Dx*Nkx);
//            double ky=2.0*Pi*(j-Nky/2)/(Dx*Nky);
//            
//            double sq_arg=k0*k0-kx*kx-ky*ky;
//            
//            fft_in_data(i,j)*=std::exp(std::sqrt(static_cast<Imdouble>(sq_arg))*Im*z);
//            
//        }
//    }
    
    // Reverse shift
    
//    for(i=0;i<Nkx;i++)
//    {
//        for(j=0;j<Nky;j++)
//        {
//            fft_out_data(i,j)=fft_in_data((i+Nkx/2)%Nkx,(j+Nky/2)%Nky);
//        }
//    }
//    
//    G2_FFT(fft_out_data,fft_in_data,+1);
    
    Grid2<double> FT_im(Nkx,Nky);
    
    for(i=0;i<Nkx;i++)
    {
        for(j=0;j<Nky;j++)
        {
            FT_im(i,j)=std::abs(fft_out_data(i,j));
//            FT_im(i,j)=fft_out_data(i,j).real();
//            FT_im(i,j)=std::log10(std::abs(fft_out_data(i,j)));
//            chk_var(FT_im(i,j));
        }
    }
    
    G2_to_BW(FT_im,"test2.png");
//    std::exit(0);
}

void DiffPatternFrame::evt_compute_double_FFT()
{
    int i,j;
    //int ik,jk;
    
    Nx_s=screen_x_res->get_value();
    Ny_s=screen_x_res->get_value();
    
    screen_data.init(Nx_s,Ny_s,0);
    
    Nx_d=data.L1();
    Ny_d=data.L2();
    
    std::vector<double> data_ky(Nx_d);
    
    int x_start=Nx_d-1;
    int x_end=0;
    
    int y_start=Ny_d-1;
    int y_end=0;
    
    for(i=0;i<Nx_d;i++)
    {
        for(j=0;j<Ny_d;j++)
        {
            if(data(i,j)!=0)
            {
                x_start=std::min(x_start,i);
                y_start=std::min(y_start,j);
                
                x_end=std::max(x_end,i);
                y_end=std::max(y_end,j);
            }
        }
    }
    
    x_end+=1;
    y_end+=1;
    
    int x_span=x_end-x_start;
    int y_span=y_end-y_start;
    
    Dx=input_scale->get_length()/x_span;
    
    int span_max=std::max(x_span,y_span);
    
    int extra=5;
    
    int Nkx=2*extra*span_max+x_span;
    int Nky=2*extra*span_max+y_span;
    
    Grid2<Imdouble> fft_in_data(Nkx,Nky,0),
                    fft_out_data;
                    
    for(i=x_start;i<x_end;i++)
    {
        for(j=y_start;j<y_end;j++)
        {
            fft_in_data(i-x_start+extra*span_max,j-y_start+extra*span_max)=data(i,j);
        }
    }
    
    double k0=2.0*Pi/500e-9;
    
    G2_FFT(fft_out_data,fft_in_data,-1);
    
    // Shift
    
    for(i=0;i<Nkx;i++)
    {
        for(j=0;j<Nky;j++)
        {
            fft_in_data((i+Nkx/2)%Nkx,(j+Nky/2)%Nky)=fft_out_data(i,j);
        }
    }
    
    // Propagator
    
    double z=screen_distance->get_length();
    
    for(i=0;i<Nkx;i++)
    {
        for(j=0;j<Nky;j++)
        {
            double kx=2.0*Pi*(i-Nkx/2)/(Dx*Nkx);
            double ky=2.0*Pi*(j-Nky/2)/(Dx*Nky);
            
            double sq_arg=k0*k0-kx*kx-ky*ky;
            
            fft_in_data(i,j)*=std::exp(std::sqrt(static_cast<Imdouble>(sq_arg))*Im*z);
            
        }
    }
    
    // Reverse shift
    
    for(i=0;i<Nkx;i++)
    {
        for(j=0;j<Nky;j++)
        {
            fft_out_data(i,j)=fft_in_data((i+Nkx/2)%Nkx,(j+Nky/2)%Nky);
        }
    }
    
    G2_FFT(fft_out_data,fft_in_data,+1);
    
    Grid2<double> FT_im(Nkx,Nky);
    
    for(i=0;i<Nkx;i++)
    {
        for(j=0;j<Nky;j++)
        {
            FT_im(i,j)=std::sqrt((std::abs(fft_out_data(i,j))));
//            FT_im(i,j)=fft_out_data(i,j).real();
//            FT_im(i,j)=std::log10(std::abs(fft_out_data(i,j)));
//            chk_var(FT_im(i,j));
        }
    }
    
    G2_to_BW(FT_im,"test3.png");
//    std::exit(0);
}

void DiffPatternFrame::evt_compute_FT_growth()
{
    int i,j;
    //int ik,jk;
    
    Nx_s=screen_x_res->get_value();
    Ny_s=screen_x_res->get_value();
    
    screen_data.init(Nx_s,Ny_s,0);
    
    Nx_d=data.L1();
    Ny_d=data.L2();
    
    std::vector<double> data_ky(Nx_d);
    
    int x_start=Nx_d-1;
    int x_end=0;
    
    for(i=0;i<Nx_d;i++)
    {
        for(j=0;j<Ny_d;j++)
        {
            if(data(i,j)!=0)
            {
                x_start=std::min(x_start,i);
                x_end=std::max(x_end,i);
            }
        }
    }
    
    Dx=input_scale->get_length()/(x_end-x_start+1.0);
    
    double Dk=2.0*Pi/input_scale->get_length();
    
    double k0=2.0*Pi/500e-9;
    
    Dk=k0;
        
    chk_var(10.0*Dk);
    chk_var(k0);
    
    Imdouble I_target=0;
    
    for(i=0;i<Nx_s;i++)
    {
        for(j=0;j<Ny_s;j++)
        {
            double kx=-Dk+2.0*Dk*i/(Nx_s-1);
            double ky=-Dk+2.0*Dk*j/(Ny_s-1);
            
            Imdouble F=compute_data_spectrum(kx,ky);
            I_target+=F;
//            chk_var(F);
//            screen_data(i,j)=std::log(std::abs(F));
            screen_data(i,j)=std::abs(F);
            
//            if(!compressed_data_end[j].empty())
//            {
//                screen_data(i,j)=compressed_data_end[j][0];
//            }
        }
    }
    
    chk_var(std::abs(I_target));
    I_target*=4.0*Dk*Dk/Nx_s/Ny_s;
    chk_var(std::abs(I_target));
    
    int Nkx=101;
    int Nky=101;
    
    Grid2<Imdouble> FT(Nkx,Nky);
    
    double Dkx=Dk;
    double Dky=Dk;
    
    Imdouble FT_sum=0;
    
    for(i=0;i<Nkx;i++)
    {
        for(j=0;j<Nky;j++)
        {
            double kx=-Dkx+2.0*Dkx*i/(Nkx-1.0);
            double ky=-Dky+2.0*Dky*j/(Nky-1.0);
            
            FT(i,j)=compute_data_spectrum(kx,ky);
        }
    }
    
    FT_sum=integral_abs_2D_constant_approx(FT,-Dkx,Dkx,-Dky,Dky);
    
    G2_to_degra(screen_data,"test.png");
    
    bool unstable=true;
//    bool kx_grow=true;
//    bool ky_grow=true;
    
    int mode=0;
    //bool loop=false;
    
    double threshold=0.5e-3;
    
    while(unstable)
    {
        chk_var(mode);
        
//        double FT_sum_ext=0;
        Grid2<Imdouble> prev_FT=FT;
        
        Imdouble FT_sum_ext=0;
        
        if(mode==0)
        {
            FT.init(2*Nkx-1,2*Nky-1);
            
            for(i=0;i<Nkx;i++)
                for(j=0;j<Nky;j++)
                    FT(2*i,2*j)=prev_FT(i,j);
            
            Nkx=2.0*Nkx-1;
            Nky=2.0*Nky-1;
            
            for(i=0;i<Nkx;i++)
            {
                for(j=0;j<Nky;j++)
                {
                    if(i%2!=0 || j%2!=0)
                    {
                        double kx=-Dkx+2.0*Dkx*i/(Nkx-1.0);
                        double ky=-Dky+2.0*Dky*j/(Nky-1.0);
                        
                        FT(i,j)=compute_data_spectrum(kx,ky);
                    }
                }
            }
        }
        else if(mode==1)
        {mode=3;
//            threshold=1e-2;
//            
//            int Nkx_ext=std::floor(0.1*Nkx);
//            
//            FT.init(2*Nkx_ext+Nkx,Nky);
//            
//            for(i=0;i<Nkx;i++)
//                for(j=0;j<Nky;j++)
//                    FT(i+Nkx_ext,j)=prev_FT(i,j);
//            
//            double dk=2.0*Dkx/(Nkx-1.0);
//            Nkx=2*Nkx_ext+Nkx;
//                        
//            chk_var(Dkx);
//            
//            Dkx=(Nkx-1)*dk/2.0;
//            chk_var(dk);
//            chk_var(Dkx);
//            
//            ProgDisp dsp(Nkx*Nky);
//            
//            for(j=0;j<Nky;j++) for(i=0;i<Nkx;i++)
//            {
//                if(i<Nkx_ext || i>=Nkx-1-Nkx_ext)
//                {
//                    double kx=-Dkx+2.0*Dkx*i/(Nkx-1.0);
//                    double ky=-Dky+2.0*Dky*j/(Nky-1.0);
//                    
//                    FT(i,j)=compute_data_spectrum(kx,ky);
//                }
//                    ++dsp;
//            }
        }
        else if(mode==2)
        {mode=3;//std::exit(0);
//            ky_grow=true;
//            
//            double A=4.0*Dkx*Dky;   // Old area
//            double A_ext=4.0*Dk*Dkx;   // Extra area
//            
//            int Ns_ext=std::floor(Ns*A_ext/A);
//            
//            kx.resize(Ns+Ns_ext);
//            ky.resize(Ns+Ns_ext);
//            FT.resize(Ns+Ns_ext);
//            
//            for(i=Ns;i<Ns+Ns_ext;i++)
//            {
//                kx[i]=randp(-Dkx,Dkx);
//                
//                if(randp()>0.5) ky[i]=randp(Dky,Dky+Dk);
//                else            ky[i]=randp(-Dky-Dk,-Dky);
//                
//                FT[i]=compute_data_spectrum(kx[i],ky[i]);
////                FT_sum_ext+=std::abs(FT[i]);
//                FT_sum_ext+=FT[i];
//            }
//            
////            double I=FT_sum/Ns*A;
////            double I_ext=FT_sum_ext/Ns_ext*A_ext;
//            double I=std::abs(FT_sum)/Ns*A;
//            double I_ext=std::abs(FT_sum_ext)/Ns_ext*A_ext;
//            
//            if(I_ext/I<threshold) ky_grow=false;
//            
//            Dky+=Dk;
//            Ns+=Ns_ext;
//            
//            mode=1;
        }
            
        FT_sum_ext=integral_abs_2D_constant_approx(FT,-Dkx,Dkx,-Dky,Dky);
        
        double I=std::abs(FT_sum);
        double I_ext=std::abs(FT_sum_ext);
        
//        if(mode==0) I*=4;
        
        chk_var(I);
        chk_var(I_ext);
        chk_var(std::abs(I_target));
        chk_var(I_ext-I);
            
        if(std::abs((I-I_ext)/I)<threshold) mode++;
        
        if(mode>=3) unstable=false;
        
        chk_var(std::abs(FT_sum));
        FT_sum=FT_sum_ext;
        chk_var(std::abs(FT_sum));
//        std::system("pause");

        Grid2<double> FT_im(FT.L1(),FT.L2());
        
        for(i=0;i<FT.L1();i++) for(j=0;j<FT.L2();j++)
        {
    //        FT_im(i,j)=std::log10(std::abs(FT(i,j)));
            FT_im(i,j)=std::abs(FT(i,j));
        }
        
        G2_to_BW(FT_im,"test2.png");
    }
    
    std::exit(0);
}

void DiffPatternFrame::evt_new_data(wxCommandEvent &event)
{
    std::filesystem::path path=input_data->path;
    
    wxImage image;
    image.LoadFile(path.generic_string());
    
    chk_var(image.GetWidth());
    chk_var(image.GetHeight());
    
    int Nw=image.GetWidth();
    int Nh=image.GetHeight();
    
    Bitmap &bitmap=input_display->bitmap;
    
    Nx_d=Nw;
    Ny_d=Nh;
    
    data.init(Nx_d,Ny_d,0);
    bitmap.set_size(Nx_d,Ny_d);
    
    for(int i=0;i<Nx_d;i++)
    {
        for(int j=0;j<Ny_d;j++)
        {
            double R=image.GetRed(i,Nh-1-j)/255.0;
            double G=image.GetGreen(i,Nh-1-j)/255.0;
            double B=image.GetBlue(i,Nh-1-j)/255.0;
            
            bitmap(i,j,R,G,B);
            
            double val=(R+G+B)/3.0;
            
            if(val<0.5) val=0;
            else val=1.0;
            
            data(i,j)=val;
        }
    }
    
    input_display->refresh_from_bitmap();
    
    compressed_data_start.clear();
    compressed_data_end.clear();
    
    compressed_data_start.resize(Ny_d);
    compressed_data_end.resize(Ny_d);
    
    for(int j=0;j<Ny_d;j++)
    {
        bool apert=false;
        
        for(int i=0;i<Nx_d;i++)
        {
            if(apert==false)
            {
                if(data(i,j)!=0)
                {
                    compressed_data_start[j].push_back(i);
                    apert=true;
                }
            }
            else
            {
                if(data(i,j)==0)
                {
                    compressed_data_end[j].push_back(i);
                    apert=false;
                }
            }
        }
        
        if(apert==true) compressed_data_end[j].push_back(Nx_d);
    }
}
