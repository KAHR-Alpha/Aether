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

#include <string_tools.h>
#include <spectral_color.h>

#include <gui_parametric_visualizer.h>

#include <wx/splitter.h>

#include <fstream>

extern std::ofstream plog;

enum
{
    MENU_LOAD,
    MENU_EXIT
};

void generate_test_data()
{
    int i,j,N=1001;
    
    std::vector<double> x(N);
    
    for(i=0;i<N;i++) x[i]=-10.0+20.0*i/(N-1.0);
    
    int N1=20;
    int N2=20;
    
    std::vector<double> param_1(N1),param_2(N2);
    
    for(i=0;i<N1;i++) param_1[i]=-5+10*i/(N1-1.0);
    for(i=0;i<N2;i++) param_2[i]=2.0*i/(N2-1);
    
    std::ofstream file("test_data",std::ios::trunc|std::ios::out);
    
    file<<N<<" 2 "<<N1<<" "<<N2<<'\n';
    
    for(i=0;i<N;i++)
    {
        file<<x[i];
        
        if(i<N-1) file<<" "; else file<<'\n';
    }
    
    for(i=0;i<N1;i++)
    {
        file<<param_1[i];
        
        if(i<N1-1) file<<" "; else file<<'\n';
    }
    
    for(i=0;i<N2;i++)
    {
        file<<param_2[i];
        
        if(i<N2-1) file<<" "; else file<<'\n';
    }
    
    for(j=0;j<N2;j++)
    {
        for(i=0;i<N1;i++)
        {
            for(int k=0;k<N;k++)
                file<<param_2[j]*std::exp(-std::pow((x[k]-param_1[i])/2.0,2.0))<<" "
                    <<0.5*param_2[j]*std::exp(-std::pow((x[k]-param_1[i])/2.0,2.0))*(x[k]-param_1[i])<<'\n';
        }
    }
}

PVisuFrame::PVisuFrame(wxString const &title)
    :BaseFrame(title)
{
    wxSplitterWindow *splitter=new wxSplitterWindow(this);
    
    ctrl_panel=new wxScrolledWindow(splitter);
    graph=new Graph(splitter);
    
    ctrl_sizer=new wxBoxSizer(wxVERTICAL);
    ctrl_panel->SetSizer(ctrl_sizer);
    
    // Menu
    
    wxMenuBar *menu_bar=new wxMenuBar();
    
    wxMenu *file_menu=new wxMenu();
    file_menu->Append(MENU_LOAD,"Load");
    file_menu->AppendSeparator();
    file_menu->Append(MENU_EXIT,"Exit");
    
    menu_bar->Append(file_menu,"File");
    
    SetMenuBar(menu_bar);
    Bind(wxEVT_MENU,&PVisuFrame::evt_menu,this);
    
    splitter->SplitVertically(ctrl_panel,graph,300);
    splitter->SetMinimumPaneSize(250);
    
//    generate_test_data();
}

PVisuFrame::~PVisuFrame()
{
}

void PVisuFrame::evt_menu(wxCommandEvent &event)
{
    if(event.GetId()==MENU_EXIT) Close();
    else if(event.GetId()==MENU_LOAD)
    {
        wxFileName fname=wxFileSelector("Load data..",
                                    wxEmptyString,wxEmptyString,
                                    wxEmptyString,wxEmptyString,
                                    wxFD_OPEN);
    
        if(fname.Exists()) load(fname.GetFullPath().ToStdString());
        
//        load("test_data");
    }
}

void PVisuFrame::evt_slider(wxCommandEvent &event)
{
    int i=event.GetId();
    
    int loc=params_slider[i]->GetValue();
    
    params_text[i]->ChangeValue(std::to_string(params[i][loc]));
    
    update_data();
}

void PVisuFrame::from_linear(std::vector<int> &index_,int i)
{
//    i/=Nx;
    
    for(int j=0;j<Np;j++)
    {
        index_[j]=i%steps[j];
        i-=index_[j];
        i/=steps[j];
    }
}

int int_pow(int a,int b)
{
    int r=1;
    for(int i=0;i<b;i++) r*=a;
    return r;
}

void PVisuFrame::load(std::string const &fname)
{
    chk_var(fname);
    int i,j,k;
    
    // Cleaning
    
    ctrl_panel->DestroyChildren();
    graph->clear_graph();
    
    // Loading
    
    std::vector<std::string> fraw;
    
    {
        std::ifstream file(fname,std::ios::in);
        
        while(!file.eof())
        {
            std::string str;
            std::getline(file,str,'\n');
            
            clean_data_string(str);
            if(str.size()!=0 && !contains_non_numerics(str))
            {
                fraw.push_back(str);
            }
        }
    }
    
    // Header
    
    std::vector<std::string> buf_split;
    
    split_string(buf_split,fraw[0]);
    
    Nx=std::stoi(buf_split[0]);
    N_data=std::stoi(buf_split[1]);
    
    Np=buf_split.size()-2;
    
    // Allocation
    
    x_data.resize(Nx);
    steps.resize(Np);
    params.resize(Np);
    index.resize(Np);
    
    for(i=0;i<Np;i++)
    {
        steps[i]=std::stoi(buf_split[i+2]);
        params[i].resize(steps[i]);
    }
    
    // Xdata and parameters reading
    
    std::stringstream strm(fraw[1]);
    
    for(i=0;i<Nx;i++) strm>>x_data[i];
    
    for(j=0;j<Np;j++)
    {
        std::stringstream strm(fraw[2+j]);
        strm.clear();
        
        for(i=0;i<steps[j];i++) strm>>params[j][i];
    }
    
    // Data allocation
    
    int steps_p=1;
    for(i=0;i<Np;i++) steps_p*=steps[i];
    
    data.resize(N_data);
    disp_y.resize(N_data);
    
    for(i=0;i<N_data;i++)
    {
        data[i].resize(Nx*steps_p);
        disp_y[i].resize(Nx);
    }
    
    // File to data
    
    for(i=0;i<steps_p;i++)
    {
        from_linear(index,i);
        
        for(j=0;j<Nx;j++)
        {
            split_string(buf_split,fraw[i*Nx+j+2+Np]);
            
            for(k=0;k<N_data;k++)
            {
                data[k][i*Nx+j]=std::stod(buf_split[k]);
            }
        }
    }
    
    // UI
    
    params_text.resize(Np);
    params_slider.resize(Np);
    
    wxPanel *sub_panel=new wxPanel(ctrl_panel);
    wxBoxSizer *sub_sizer=new wxBoxSizer(wxVERTICAL);
    
    for(i=0;i<Np;i++)
    {
        wxStaticBoxSizer *ctrl_sizer_box=new wxStaticBoxSizer(wxHORIZONTAL,sub_panel,wxString(std::to_string(i+1)));
        
        params_text[i]=new wxTextCtrl(sub_panel,i,std::to_string(params[i][0]),
                                      wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER);
        params_slider[i]=new wxSlider(sub_panel,i,0,0,steps[i]-1);
        params_slider[i]->Bind(wxEVT_SLIDER,&PVisuFrame::evt_slider,this);
        
        params_text[i]->SetEditable(false);
        
        ctrl_sizer_box->Add(params_text[i],wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
        ctrl_sizer_box->Add(params_slider[i],wxSizerFlags(1).Align(wxALIGN_CENTER_VERTICAL));
        
        sub_sizer->Add(ctrl_sizer_box,wxSizerFlags().Expand());
    }
    
    sub_panel->SetSizer(sub_sizer);
    ctrl_sizer->Add(sub_panel,wxSizerFlags().Expand());
    
    ctrl_panel->Layout();
    ctrl_panel->Refresh();
    
    for(i=0;i<Np;i++) index[i]=0;
    
    j=to_linear(index);
    
    for(i=0;i<Nx;i++)
    {
        for(k=0;k<N_data;k++) disp_y[k][i]=data[k][i+j];
    }
    
    DistinctColorsSampler sampler(1.0,1.0,1.0);
        
    for(k=0;k<N_data;k++)
    {
        double R,G,B;
        sampler(R,G,B);
        
        graph->add_external_data(&x_data,&disp_y[k],R,G,B);
    }
    
    graph->autoscale();
}

int PVisuFrame::to_linear(std::vector<int> const &index_)
{
    int out=index_[Np-1];
    
    for(int i=Np-2;i>=0;i--)
        out=index_[i]+steps[i]*out;
    
    return Nx*out;
}

void PVisuFrame::update_data()
{
    int i,j;
    
    for(i=0;i<Np;i++)
        index[i]=params_slider[i]->GetValue();
    
    int local_index=to_linear(index);
        
    for(j=0;j<N_data;j++)
    {
        for(i=0;i<Nx;i++)
            disp_y[j][i]=data[j][local_index+i];
    }
    
    graph->autoscale();
}
