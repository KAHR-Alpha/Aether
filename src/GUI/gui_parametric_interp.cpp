/*Copyright 2008-2021 - Loïc Le Cunff

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
#include <gui_parametric_interp.h>

#include <wx/splitter.h>

#include <fstream>

extern std::ofstream plog;

enum
{
    MENU_LOAD,
    MENU_EXIT
};

void generate_test_data();

PInterFrame::PInterFrame(wxString const &title)
    :BaseFrame(title),
     timed_refresh(false)
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
    Bind(wxEVT_MENU,&PInterFrame::evt_menu,this);
    
    splitter->SplitVertically(ctrl_panel,graph,300);
    splitter->SetMinimumPaneSize(250);
    
//    generate_test_data();
    
    timer=new wxTimer(this);
    Bind(wxEVT_TIMER,&PInterFrame::evt_timed_refresh,this);;
    
    timer->Start(100);
}

PInterFrame::~PInterFrame()
{
    timer->Stop();
}

void PInterFrame::evt_menu(wxCommandEvent &event)
{
    if(event.GetId()==MENU_EXIT) Close();
    else if(event.GetId()==MENU_LOAD)
    {
        wxFileName fname=wxFileSelector("Load data..",
                                    wxEmptyString,wxEmptyString,
                                    wxEmptyString,wxEmptyString,
                                    wxFD_OPEN);
    
        if(fname.Exists()) load(fname.GetFullName().ToStdString());
        
//        load("test_data");
    }
}

void PInterFrame::evt_slider(wxCommandEvent &event)
{
    int i=event.GetId();
    
    int loc=params_slider[i]->GetValue();
    
    double x=loc/10000.0;
    
    double p_min=params[i][0];
    double p_max=params[i][params[i].size()-1];
    
    double val=p_min*(1.0-x)+p_max*x;
    
    params_text[i]->ChangeValue(std::to_string(val));
    
    timed_refresh=true;
}

void PInterFrame::evt_text(wxCommandEvent &event)
{
    int i=event.GetId();
    
    double val=std::stod(params_text[i]->GetValue().ToStdString());
    
    double p_min=params[i][0];
    double p_max=params[i][params[i].size()-1];
    
    if(val<p_min) { val=p_min; params_text[i]->ChangeValue(std::to_string(val)); }
    else if(val>p_max) { val=p_max; params_text[i]->ChangeValue(std::to_string(val)); }
    
    double x=(val-p_min)/(p_max-p_min);
    
    params_slider[i]->SetValue(nearest_integer(x*10000));
    
    timed_refresh=true;
}

void PInterFrame::evt_timed_refresh(wxTimerEvent &event)
{
    if(timed_refresh) interpolate_data();
    
    timed_refresh=false;
}

void PInterFrame::from_linear(std::vector<int> &index_,int i)
{
//    i/=Nx;
    
    for(int j=0;j<Np;j++)
    {
        index_[j]=i%steps[j];
        i-=index_[j];
        i/=steps[j];
    }
}

void PInterFrame::interpolate_data()
{
    int i,j;
    
    for(i=0;i<Np;i++)
    {
        double val=std::stod(params_text[i]->GetValue().ToStdString());
        
        vector_locate_linear(current_index[i],uv[i][1],params[i],val);
        
        uv[i][0]=1.0-uv[i][1];
    }
    
    for(j=0;j<N_data;j++)
    {
        for(i=0;i<Nx;i++) disp_y[j][i]=0;
        
        for(unsigned int k=0;k<combinations.size();k++)
        {
            double uv_prod=1;
            
            for(i=0;i<Np;i++)
            {
                index[i]=current_index[i]+combinations[k][i];
                uv_prod*=uv[i][combinations[k][i]];
            }
            
            int local_index=to_linear(index);
            
            for(i=0;i<Nx;i++)
                disp_y[j][i]+=uv_prod*data[j][local_index+i];
        }
    }
    
    graph->autoscale();
}

int int_pow(int a,int b);

void PInterFrame::load(std::string const &fname)
{
    int i,j,k;
    
    // Cleaning
    
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
    current_index.resize(Np);
    uv.resize(Np);
    
    for(i=0;i<Np;i++)
    {
        steps[i]=std::stoi(buf_split[i+2]);
        params[i].resize(steps[i]);
        uv[i].resize(2);
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
    
    // Params combinations
    
    uv.resize(Np);
    combinations.resize(int_pow(2,Np));
    
    for(i=0;i<int_pow(2,Np);i++)
    {
        combinations[i].resize(Np);
        
        for(j=0;j<Np;j++)
            combinations[i][j]=static_cast<int>((i&int_pow(2,j))!=0);
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
    
    for(i=0;i<Np;i++)
    {
        wxStaticBoxSizer *sizer=new wxStaticBoxSizer(wxHORIZONTAL,ctrl_panel,wxString(std::to_string(i+1)));
        
        params_text[i]=new wxTextCtrl(ctrl_panel,i,std::to_string(params[i][0]),
                                      wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER);
        params_slider[i]=new wxSlider(ctrl_panel,i,0,0,10000);
        
        sizer->Add(params_text[i],wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
        sizer->Add(params_slider[i],wxSizerFlags(1).Align(wxALIGN_CENTER_VERTICAL));
        
        ctrl_sizer->Add(sizer,wxSizerFlags().Expand());
        
        params_text[i]->Bind(wxEVT_TEXT_ENTER,&PInterFrame::evt_text,this);
        params_slider[i]->Bind(wxEVT_SLIDER,&PInterFrame::evt_slider,this);
    }
    
    ctrl_panel->Layout();
    ctrl_panel->Refresh();
    
    for(i=0;i<Np;i++) index[i]=0;
    
    j=to_linear(index);
    
    for(i=0;i<Nx;i++)
    {
        for(k=0;k<N_data;k++) disp_y[k][i]=data[k][i+j];
    }
    
    for(k=0;k<N_data;k++) graph->add_data(&x_data,&disp_y[k],randp(0.8),randp(0.8),randp(0.8));
    
    graph->autoscale();
}

int PInterFrame::to_linear(std::vector<int> const &index_)
{
    int out=index_[Np-1];
    
    for(int i=Np-2;i>=0;i--)
        out=index_[i]+steps[i]*out;
    
    return Nx*out;
}
