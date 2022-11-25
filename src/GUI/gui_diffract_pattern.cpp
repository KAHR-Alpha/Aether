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

extern const double Pi;
extern const Imdouble Im;

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
    
    input_scale=new LengthSelector(ctrl_panel,"Aperture x scale",1e-6,true);
    
    ctrl_sizer->Add(input_scale,default_ctrl_sizer_flags);
    
    // - Screen
    
    screen_distance=new LengthSelector(ctrl_panel,"Screen distance",1,true);
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
    int i,j;
    
    Imdouble R=0;
    
    for(j=0;j<Ny_d;j++)
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

void DiffPatternFrame::evt_compute(wxCommandEvent &event)
{
    int i,j,ik,jk;
    
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
                x_end=std::min(x_end,i);
            }
        }
    }
    
    Dx=input_scale->get_length()/(x_end-x_start+1.0);
    
    double Dk=2.0*Pi/input_scale->get_length();
    
    double k0=2.0*Pi*c_light/500e-9;
    
    chk_var(10.0*Dk);
    chk_var(k0);
    
    for(i=0;i<Nx_s;i++)
    {
        for(j=0;j<Ny_s;j++)
        {
            double kx=20.0*(i-Nx_s/2)/(Nx_s/2)*Dk;
            double ky=20.0*(j-Ny_s/2)/(Ny_s/2)*Dk;
            
            Imdouble F=compute_data_spectrum(kx,ky);
            
//            chk_var(F);
//            screen_data(i,j)=std::log(std::abs(F));
            screen_data(i,j)=std::abs(F);
            
//            if(!compressed_data_end[j].empty())
//            {
//                screen_data(i,j)=compressed_data_end[j][0];
//            }
        }
    }
    
    G2_to_degra(screen_data,"test.png");
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
