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

#include <filehdl.h>
#include <geometry.h>
#include <lua_base.h>
#include <spectral_color.h>
#include <string_tools.h>

#include <gui_graph.h>
#include <gui_rsc.h>
#include <gui_samples_explorer.h>

#include <wx/artprov.h>
#include <wx/dir.h>
#include <wx/dirdlg.h>
#include <wx/progdlg.h>
#include <wx/splitter.h>

enum
{
    BOOL_AND=0,
    BOOL_OR,
    BOOL_NOT
};

wxDEFINE_EVENT(EVT_REFRESH_BOOLEAN,wxCommandEvent);
wxDEFINE_EVENT(EVT_SAMPLE_PANELS,wxCommandEvent);

class TagsDialog : public wxDialog
{
    public:
        bool &show_tagless;
        BooleanTree &filter_tree;
        
        wxCheckBox *tagless_ctrl;
        
        wxScrolledWindow *boolean_container;
        BooleanPanel *boolean_panel;
        
        TagsDialog(std::vector<std::string> const &tags_,
                   bool &show_tagless_,
                   BooleanTree &filter_tree_)
            :wxDialog(0,wxID_ANY,"Display tags",
                      wxGetApp().default_dialog_origin(),
                      wxGetApp().default_dialog_size()),
             show_tagless(show_tagless_),
             filter_tree(filter_tree_)
        {
            wxBoxSizer *sizer=new wxBoxSizer(wxVERTICAL);
            
            tagless_ctrl=new wxCheckBox(this,wxID_ANY," Show tagless");
            tagless_ctrl->SetValue(show_tagless);
            
            sizer->Add(tagless_ctrl,wxSizerFlags().Border(wxALL,2));
            
            boolean_container=new wxScrolledWindow(this);
            boolean_container->SetScrollRate(10,10);
            boolean_container->SetWindowStyle(wxBORDER_SUNKEN);
            
            wxBoxSizer *boolean_sizer=new wxBoxSizer(wxVERTICAL);
            
            boolean_container->SetSizer(boolean_sizer);
            
            boolean_panel=new BooleanPanel(boolean_container,BOOL_AND,tags_);
            if(!filter_tree.is_empty()) boolean_panel->from_tree(filter_tree);
            
            boolean_sizer->Add(boolean_panel);
            
            sizer->Add(boolean_container,wxSizerFlags(1).Expand());
            
            Bind(EVT_REFRESH_BOOLEAN,&TagsDialog::evt_boolean_change,this);
            Bind(wxEVT_CLOSE_WINDOW,&TagsDialog::evt_close,this);
            
            SetSizer(sizer);
            
            ShowModal();
        }
        
        void evt_boolean_change(wxCommandEvent &event)
        {
            boolean_container->Layout();
            boolean_container->FitInside();
            
            Refresh();
        }
        
        void evt_close(wxCloseEvent &event)
        {
            show_tagless=tagless_ctrl->GetValue();
            
            filter_tree.clear_tree();
            boolean_panel->to_tree(filter_tree);
            
            event.Skip();
        }
};

//#################
//   BooleanTree
//#################

void BooleanTree::clear_tree()
{
    for(unsigned int i=0;i<leaves.size();i++)
        delete leaves[i];
    
    tags.clear();
    leaves.clear();
}

bool BooleanTree::evaluate(std::vector<std::string> const &base_tags,
                           std::vector<bool> const &base_values)
{
    std::size_t i;
    
    bool R=true;
    
    if(type==BOOL_AND)
    {
        for(i=0;i<tags.size();i++)
        {
            int N=vector_locate(base_tags,tags[i]);
            R=R && base_values[N];
        }
        for(i=0;i<leaves.size();i++)
        {
            R=R && leaves[i]->evaluate(base_tags,base_values);
        }
    }
    else if(type==BOOL_OR)
    {
        R=false;
        if(tags.size()+leaves.size()==0) R=true;
        
        for(i=0;i<tags.size();i++)
        {
            int N=vector_locate(base_tags,tags[i]);
            R=R || base_values[N];
        }
        for(i=0;i<leaves.size();i++)
        {
            R=R || leaves[i]->evaluate(base_tags,base_values);
        }
    }
    else if(type==BOOL_NOT)
    {
        if(tags.size()>0)
        {
            int N=vector_locate(base_tags,tags[0]);
            R=!(base_values[N]);
        }
        else if(leaves.size()>0)
        {
            R=!(leaves[0]->evaluate(base_tags,base_values));
        }
    }
    
    return R;
}

bool BooleanTree::is_empty()
{
    if(tags.size()+leaves.size()==0) return true;
    else return false;
}

//##################
//   BooleanPanel
//##################

BooleanPanel::BooleanPanel(wxWindow *parent_,int type_,
                           std::vector<std::string> const &full_tags_)
    :wxPanel(parent_), type(type_), full_tags(full_tags_),
     tag_ID_counter(0), op_ID_counter(0)
{
    wxBoxSizer *sizer=new wxBoxSizer(wxVERTICAL);
    
    SetSizer(sizer);
    
    // Header
    
    wxBoxSizer *header_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    // - Type Selector
    
    wxStaticBoxSizer *type_sizer=new wxStaticBoxSizer(wxVERTICAL,this,"Type");
    
    type_ctrl=new wxChoice(this,wxID_ANY);
    type_ctrl->Append("AND");
    type_ctrl->Append("OR");
    type_ctrl->Append("NOT");
    
         if(type==BOOL_AND) type_ctrl->SetSelection(0);
    else if(type==BOOL_OR) type_ctrl->SetSelection(1);
    else if(type==BOOL_NOT) type_ctrl->SetSelection(2);
    
    type_sizer->Add(type_ctrl);
    
    type_ctrl->Bind(wxEVT_CHOICE,&BooleanPanel::evt_type,this);
    
    header_sizer->Add(type_sizer,wxSizerFlags().Border(wxALL,2));
    
    // - Elements addition
    
    wxStaticBoxSizer *type_add_sizer=new wxStaticBoxSizer(wxHORIZONTAL,this,"Add Element");
    
    add_choice=new wxChoice(this,wxID_ANY);
    add_choice->Append("Tag");
    add_choice->Append("----");
    add_choice->Append("AND");
    add_choice->Append("OR");
    add_choice->Append("NOT");
    add_choice->SetSelection(0);
    
    wxButton *add_btn=new wxButton(this,wxID_ANY,"Add");
    add_btn->Bind(wxEVT_BUTTON,&BooleanPanel::evt_add_element,this);
    
    type_add_sizer->Add(add_choice,wxSizerFlags().Border(wxALL,2));
    type_add_sizer->Add(add_btn,wxSizerFlags().Border(wxALL,2));
    
    header_sizer->Add(type_add_sizer,wxSizerFlags().Border(wxALL,2));
    sizer->Add(header_sizer);
    
    // Elements
    
    tags_sizer=new wxBoxSizer(wxVERTICAL);
    ops_sizer=new wxBoxSizer(wxVERTICAL);
    
    sizer->Add(tags_sizer);
    sizer->Add(ops_sizer);
    
    Vector3 color;
    color.rand_sph();
    
    color.x=std::abs(color.x);
    color.y=std::abs(color.y);
    color.z=std::abs(color.z);
    
    double min_val=color.least_dominant_val();
    double max_val=color.most_dominant_val();
    
    color.x=0.8+0.2*(color.x-min_val)/(max_val-min_val);
    color.y=0.8+0.2*(color.y-min_val)/(max_val-min_val);
    color.z=0.8+0.2*(color.z-min_val)/(max_val-min_val);
    
    color*=255.0;
    
    SetBackgroundColour(wxColor(color.x,color.y,color.z));
}

void BooleanPanel::add_tag_panel()
{
    wxPanel *panel=new wxPanel(this);
    wxBoxSizer *sizer=new wxBoxSizer(wxHORIZONTAL);
    
    panel->SetSizer(sizer);
    
    // Kill Button
    
    std::string x_button_14=PathManager::locate_resource("resources/x_button_14.png").generic_string();
    
    wxButton *btn=new wxButton(panel,tag_ID_counter,wxEmptyString,wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    btn->SetBitmap(ImagesManager::get_bitmap(x_button_14));
    btn->Bind(wxEVT_BUTTON,&BooleanPanel::evt_delete_tag,this);
    
    sizer->Add(btn,wxSizerFlags().Border(wxALL,2));
    
    // Control
    
    wxChoice *choice=new wxChoice(panel,wxID_ANY);
    
    for(unsigned int i=0;i<full_tags.size();i++)
        choice->Append(full_tags[i]);
    choice->SetSelection(0);
    
    sizer->Add(choice,wxSizerFlags().Border(wxALL,2));
    
    // Logging
    
    tag_IDs.push_back(tag_ID_counter);
    tags.push_back(choice);
    tag_panels.push_back(panel);
    
    tag_ID_counter++;
    
    // Adding
    
    tags_sizer->Add(panel,wxSizerFlags().Border(wxALL,2));
}

void BooleanPanel::add_op_panel(int type_)
{
    wxPanel *panel=new wxPanel(this);
    wxBoxSizer *sizer=new wxBoxSizer(wxHORIZONTAL);
    
    panel->SetSizer(sizer);
    
    // Kill Button
    
    std::string x_button_14=PathManager::locate_resource("resources/x_button_14.png").generic_string();
    
    wxButton *btn=new wxButton(panel,op_ID_counter,wxEmptyString,wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    btn->SetBitmap(ImagesManager::get_bitmap(x_button_14));
    btn->Bind(wxEVT_BUTTON,&BooleanPanel::evt_delete_operation,this);
    
    sizer->Add(btn,wxSizerFlags().Border(wxALL,2));
    
    // Control
    
    BooleanPanel *op=new BooleanPanel(panel,type_,full_tags);
    
    sizer->Add(op,wxSizerFlags().Border(wxALL,2));
    
    // Logging
    
    op_IDs.push_back(op_ID_counter);
    ops.push_back(op);
    op_panels.push_back(panel);
    
    op_ID_counter++;
    
    // Adding
    
    ops_sizer->Add(panel,wxSizerFlags().Border(wxALL,2));
}

void BooleanPanel::evt_add_element(wxCommandEvent &event)
{
    int N=add_choice->GetSelection();
    
    if(N!=1 && !(type==BOOL_NOT && (tags.size()+ops.size())>=1))
    {
             if(N==0) add_tag_panel();
        else if(N==2) add_op_panel(BOOL_AND);
        else if(N==3) add_op_panel(BOOL_OR);
        else if(N==4) add_op_panel(BOOL_NOT);
        
        wxCommandEvent event_out(EVT_REFRESH_BOOLEAN);
        wxPostEvent(this,event_out);
    }    
    
    event.Skip();
}

void BooleanPanel::evt_delete_tag(wxCommandEvent &event)
{
    int N=event.GetId();
    
    N=vector_locate(tag_IDs,N);
    
    tag_panels[N]->Destroy();
    
    std::vector<int>::const_iterator it_IDs=tag_IDs.begin()+N;
    std::vector<wxChoice*>::const_iterator it_ctrl=tags.begin()+N;
    std::vector<wxPanel*>::const_iterator it_panels=tag_panels.begin()+N;
    
    tag_IDs.erase(it_IDs);
    tags.erase(it_ctrl);
    tag_panels.erase(it_panels);
    
    wxCommandEvent event_out(EVT_REFRESH_BOOLEAN);
    wxPostEvent(this,event_out);
}

void BooleanPanel::evt_delete_operation(wxCommandEvent &event)
{
    int N=event.GetId();
    
    N=vector_locate(op_IDs,N);
    
    op_panels[N]->Destroy();
    
    std::vector<int>::const_iterator it_IDs=op_IDs.begin()+N;
    std::vector<BooleanPanel*>::const_iterator it_ctrl=ops.begin()+N;
    std::vector<wxPanel*>::const_iterator it_panels=op_panels.begin()+N;
    
    op_IDs.erase(it_IDs);
    ops.erase(it_ctrl);
    op_panels.erase(it_panels);
    
    wxCommandEvent event_out(EVT_REFRESH_BOOLEAN);
    wxPostEvent(this,event_out);
}

void BooleanPanel::evt_type(wxCommandEvent &event)
{
    unsigned int i;
    int N=type_ctrl->GetSelection();
    
         if(N==0) type=BOOL_AND;
    else if(N==1) type=BOOL_OR;
    else if(N==2) type=BOOL_NOT;
    
    if(type==BOOL_NOT)
    {
        if(tags.size()>0)
        {
            for(i=1;i<tags.size();i++)
                tag_panels[i]->Destroy();
            
            tag_IDs.resize(1);
            tags.resize(1);
            tag_panels.resize(1);
            
            for(i=0;i<ops.size();i++)
                op_panels[i]->Destroy();
            
            op_IDs.clear();
            ops.clear();
            op_panels.clear();
        }
        else if(ops.size()>0)
        {
            for(i=1;i<ops.size();i++) 
                op_panels[i]->Destroy();
            
            op_IDs.resize(1);
            ops.resize(1);
            op_panels.resize(1);
        }
        
        wxCommandEvent event_out(EVT_REFRESH_BOOLEAN);
        wxPostEvent(this,event_out);
    }
    
    event.Skip();
}

void BooleanPanel::from_tree(BooleanTree &tree)
{
    type=tree.type;
    
         if(type==BOOL_AND) type_ctrl->SetSelection(0);
    else if(type==BOOL_OR) type_ctrl->SetSelection(1);
    else if(type==BOOL_NOT) type_ctrl->SetSelection(2);
    
    for(std::string tag : tree.tags)
    {
        add_tag_panel();
        
        int N=vector_locate(full_tags,tag);
        
        tags[tags.size()-1]->SetSelection(N);
    }
    
    for(BooleanTree *leave : tree.leaves)
    {
        add_op_panel(BOOL_AND);
        
        ops[ops.size()-1]->from_tree(*leave);
    }
}

void BooleanPanel::to_tree(BooleanTree &tree)
{
    tree.type=type;
    
    for(wxChoice *ctrl : tags)
    {
        tree.tags.push_back(full_tags[ctrl->GetSelection()]);
    }
    
    for(BooleanPanel *panel : ops)
    {
        BooleanTree *subtree=new BooleanTree;
        
        panel->to_tree(*subtree);
        
        tree.leaves.push_back(subtree);
    }
}

//#######################
//   SampleGraphDialog
//#######################

//class DistinctColorSampler
//{
//    public:
//        std::vector<double> r,g,b;
//        
//        DistinctColorSampler(double r_,double g_,double b_)
//        {
//            r.push_back(r_);
//            g.push_back(g_);
//            b.push_back(b_);
//        }
//        
//        void operator() (double &r_,double &g_,double &b_)
//        {
//            int N=1000;
//            
//            double D_max=0;
//            
//            for(int i=0;i<N;i++)
//            {
//                double r_t=rand();
//                double g_t=rand();
//                double b_t=rand();
//                
//                double D=std::numeric_limits<double>::max();
//                
//                for(std::size_t j=0;j<r.size();j++)
//                {
//                    D=std::min(D,(r_t-r[j])*(r_t-r[j])+
//                                 (g_t-g[j])*(g_t-g[j])+
//                                 (b_t-b[j])*(b_t-b[j]));
//                }
//                
//                if(D>D_max)
//                {
//                    D_max=D;
//                    
//                    r_=r_t;
//                    g_=g_t;
//                    b_=b_t;
//                }
//            }
//            
//            r.push_back(r_);
//            g.push_back(g_);
//            b.push_back(b_);
//        }
//};


SampleGraphDialog::SampleGraphDialog(wxString const &caption,
                                     std::vector<wxString> const &file_x,std::vector<int> const &ID_x,
                                     std::vector<wxString> const &file_y,std::vector<int> const &ID_y,
                                     std::array<double,4> const &axis)
    :wxFrame(0,wxID_ANY,"",wxGetApp().default_dialog_origin(),wxGetApp().default_dialog_size())
{
    wxPanel *panel=new wxPanel(this);
    
    wxBoxSizer *sizer=new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *display_sizer=new wxBoxSizer(wxVERTICAL);
    
    SetSizer(sizer);
    panel->SetSizer(display_sizer);
    
    // Graph
    
    std::size_t N=file_x.size();
    
    x.resize(N);
    y.resize(N);
    std::vector<std::vector<double>> f_data;
    
    for(std::size_t i=0;i<N;i++)
    {
        ascii_data_loader(file_x[i].ToStdString(),f_data);
        x[i]=f_data[ID_x[i]];
        
        ascii_data_loader(file_y[i].ToStdString(),f_data);
        y[i]=f_data[ID_y[i]];
    }
    
    Graph *graph=new Graph(panel);
    
    double r,g,b;
    DistinctColorsSampler sampler(1.0,1.0,1.0);
    
    for(std::size_t i=0;i<N;i++)
    {
        sampler(r,g,b);
        
        graph->add_external_data(&x[i],&y[i],r,g,b);
    }
    
    if(axis[0]==axis[1]) graph->autoscale();
    else graph->set_scale(axis[0],axis[1],axis[2],axis[3]);
    
    // Description
    
    wxTextCtrl *description=new wxTextCtrl(panel,wxID_ANY,caption,wxDefaultPosition,wxDefaultSize,wxTE_MULTILINE);
    description->SetEditable(false);
    description->SetMinClientSize(wxSize(-1,50));
    description->SetMaxClientSize(wxSize(-1,50));
    
    display_sizer->Add(graph,wxSizerFlags(1).Expand());
    display_sizer->Add(description,wxSizerFlags().Expand().Border(wxALL,5));
    
    sizer->Add(panel,wxSizerFlags(1).Expand());
    
    Show();
}

//#######################
//   SampleImageDialog
//#######################

#include <gui_gl.h>

SampleImageDialog::SampleImageDialog(wxString const &im_fname,std::string const &im_description)
    :wxFrame(0,wxID_ANY,im_fname,wxGetApp().default_dialog_origin(),wxGetApp().default_dialog_size())
{
    wxPanel *panel=new wxPanel(this);
    
    wxBoxSizer *sizer=new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *display_sizer=new wxBoxSizer(wxVERTICAL);
    
    SetSizer(sizer);
    panel->SetSizer(display_sizer);
    
    // Image
    
    wxImage wx_img;
    
    wx_img.LoadFile(im_fname);
    
    GL_2D_display *display=new GL_2D_display(panel);
    
    int Nx=wx_img.GetSize().x;
    int Ny=wx_img.GetSize().y;
    
    display->bitmap.set_size(Nx,Ny);
    
    for(int i=0;i<Nx;i++)
    {
        for(int j=0;j<Ny;j++)
        {
            double R=wx_img.GetRed(i,Ny-1-j)/255.0;
            double G=wx_img.GetGreen(i,Ny-1-j)/255.0;
            double B=wx_img.GetBlue(i,Ny-1-j)/255.0;
            
            display->bitmap.set(i,j,R,G,B);
        }
    }
    
    display->refresh_from_bitmap();
    
    // Description
    
    wxTextCtrl *description=new wxTextCtrl(panel,wxID_ANY,im_description,wxDefaultPosition,wxDefaultSize,wxTE_MULTILINE);
    description->SetEditable(false);
    description->SetMinClientSize(wxSize(-1,50));
    description->SetMaxClientSize(wxSize(-1,50));
    
    display_sizer->Add(display,wxSizerFlags(1).Expand());
    display_sizer->Add(description,wxSizerFlags().Expand().Border(wxALL,5));
    
    sizer->Add(panel,wxSizerFlags(1).Expand());
    
    Show();
}

//##################
//   SamplePanel
//##################

SamplePanel* get_panel_pointer(lua_State *L)
{
    lua_getglobal(L,"mother_class");
    return reinterpret_cast<SamplePanel*>(lua_touserdata(L,-1));
}

int lua_add_data(lua_State *L)
{
    int N=lua_gettop(L);
    
    SamplePanel *panel=get_panel_pointer(L);
    
    std::size_t G=panel->graph_caption.size();
    
    if(G>0)
    {
        G--;
        
        int ID_x,ID_y;
        std::string file_x,file_y;
        
        if(N==3)
        {
            file_x=file_y=lua_tostring(L,1);
            ID_x=lua_tointeger(L,2);
            ID_y=lua_tointeger(L,3);
        }
        else
        {
            file_x=lua_tostring(L,1);
            ID_x=lua_tointeger(L,2);
            
            file_y=lua_tostring(L,3);
            ID_y=lua_tointeger(L,4);
        }
        
        panel->graph_x_ID[G].push_back(ID_x);
        panel->graph_y_ID[G].push_back(ID_y);
        
        panel->graph_x_file[G].push_back(file_x);
        panel->graph_y_file[G].push_back(file_y);
    }
    
    return 0;
}

int lua_add_graph(lua_State *L)
{
    int N=lua_gettop(L);
    
    SamplePanel *panel=get_panel_pointer(L);
    
    std::string caption;
    if(N>0) caption=lua_tostring(L,1);
    
    panel->graph_caption.push_back(caption);
    
    std::size_t G=panel->graph_caption.size();
    
    panel->graph_x_ID.resize(G);
    panel->graph_y_ID.resize(G);
    
    panel->graph_x_file.resize(G);
    panel->graph_y_file.resize(G);
    
    panel->graph_axis.push_back({0,0,0,0});
    
    return 0;
}

int lua_add_image(lua_State *L)
{
    SamplePanel *panel=get_panel_pointer(L);
    
    panel->im_fname.push_back(lua_tostring(L,1));
    
    return 0;
}

int lua_image_description(lua_State *L)
{
    SamplePanel *panel=get_panel_pointer(L);
    
    panel->im_description.resize(panel->im_fname.size());
    panel->im_description[panel->im_description.size()-1]=lua_tostring(L,1);
    
    return 0;
}

int lua_set_axis(lua_State *L)
{
    if(lua_gettop(L)==0) return 0;
    
    SamplePanel *panel=get_panel_pointer(L);
    
    std::size_t N=panel->graph_axis.size();
    
    if(N>0)
    {
        std::array<double,4> &arr=panel->graph_axis[N-1];
        
        for(int i=0;i<4;i++)
            arr[i]=lua_tonumber(L,i+1);
    }
    
    return 0;
}

int lua_set_date(lua_State *L)
{
    SamplePanel *panel=get_panel_pointer(L);
    
    panel->date=lua_tostring(L,1);
    
    return 0;
}

int lua_set_description(lua_State *L)
{
    SamplePanel *panel=get_panel_pointer(L);
    
    panel->description=lua_tostring(L,1);
    
    return 0;
}

int lua_set_tags(lua_State *L)
{
    int N=lua_gettop(L);
    
    SamplePanel *panel=get_panel_pointer(L);
    
    for(int i=1;i<=N;i++)
        panel->tags.push_back(lua_tostring(L,i));
    
    return 0;
}

SamplePanel::SamplePanel(wxWindow *parent,wxString const &script_name)
    :wxPanel(parent), root(script_name)
{
    // Script loading
    
    lua_State *L=luaL_newstate();
    luaL_openlibs(L);
    
    lua_pushlightuserdata(L,reinterpret_cast<void*>(this));
    lua_setglobal(L,"mother_class");
    
    lua_register(L,"axis",lua_set_axis);
    lua_register(L,"add_data",lua_add_data);
    lua_register(L,"add_graph",lua_add_graph);
    lua_register(L,"add_image",lua_add_image);
    lua_register(L,"axis",lua_set_axis);
    lua_register(L,"date",lua_set_date);
    lua_register(L,"description",lua_set_description);
    lua_register(L,"image_description",lua_image_description);
    lua_register(L,"tags",lua_set_tags);
    
    luaL_loadfile(L,root.GetFullPath().ToStdString().c_str());
    lua_pcall(L,0,0,0);
    
    lua_close(L);
    
    // UI
    
    wxStaticBoxSizer *sizer=new wxStaticBoxSizer(wxHORIZONTAL,this,script_name);
    wxBoxSizer *data_sizer=new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *ctrl_sizer=new wxBoxSizer(wxVERTICAL);
    
    // Description
    
    wxFlexGridSizer *description_sizer=new wxFlexGridSizer(2);
    
    description_sizer->AddGrowableCol(1,1);
    
    // - Date
    
    date_title=new wxStaticText(this,wxID_ANY,"Date: ");
    
    date_ctrl=new wxTextCtrl(this,wxID_ANY,date);
    
    description_sizer->Add(date_title,wxSizerFlags().Align(wxALIGN_TOP));
    description_sizer->Add(date_ctrl,wxSizerFlags().Expand());
    
    // - Description
    
    description_title=new wxStaticText(this,wxID_ANY,"Description: ");
    description_ctrl=new wxTextCtrl(this,wxID_ANY,description,wxDefaultPosition,wxDefaultSize,wxTE_MULTILINE);
    description_ctrl->SetMinClientSize(wxSize(-1,50));
    description_ctrl->SetMaxClientSize(wxSize(-1,50));
    
    description_sizer->Add(description_title,wxSizerFlags().Align(wxALIGN_TOP));
    description_sizer->Add(description_ctrl,wxSizerFlags().Expand());
    
    // Tags
    
    tags_title=new wxStaticText(this,wxID_ANY,"Tags: ");
        
    std::stringstream strm;
    
    for(unsigned int i=0;i<tags.size();i++)
    {
        strm<<tags[i];
        if(i!=tags.size()-1) strm<<" , ";
    }
    
    tags_ctrl=new wxTextCtrl(this,wxID_ANY,strm.str());
    
    description_sizer->Add(tags_title,wxSizerFlags().Align(wxALIGN_TOP));
    description_sizer->Add(tags_ctrl,wxSizerFlags().Expand());
    
    data_sizer->Add(description_sizer,wxSizerFlags().Expand().Border(wxALL,5));
    
    // - Images
    
    im_panel=new wxScrolledWindow(this);
    im_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    int btn_id=0;
    im_fname_wx.resize(im_fname.size());
    
    for(unsigned int i=0;i<im_fname.size();i++)
    {
        im_fname_wx[i]=root.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR)+im_fname[i];
        
        wxImage img(im_fname_wx[i]);
        img.Rescale(std::floor(100.0*img.GetSize().x/img.GetSize().y),100);
        
        wxButton *btn=new wxButton(im_panel,btn_id,wxEmptyString,wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
        btn->Bind(wxEVT_BUTTON,&SamplePanel::evt_image_button,this);
        
        wxBitmap bmp(img);
        btn->SetBitmap(bmp);
        
        im_sizer->Add(btn,wxSizerFlags());
        
        if(im_description[i].size()>0)
            btn->SetToolTip(im_description[i]);
        
        btn_id++;
    }
    
    im_panel->SetSizer(im_sizer);
    im_panel->SetScrollRate(10,0);
    
    data_sizer->Add(im_panel,wxSizerFlags().Expand());
    
    // - Graphs
    
    gr_panel=new wxScrolledWindow(this);
    gr_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    btn_id=0;
    
    std::vector<std::vector<double>> data1,data2;
    
    wxMemoryDC dc;
        
    for(std::size_t i=0;i<graph_caption.size();i++)
    {
        for(std::size_t j=0;j<graph_x_file[i].size();j++)
        {
            graph_x_file[i][j]=root.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR)+graph_x_file[i][j];
            graph_y_file[i][j]=root.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR)+graph_y_file[i][j];
        }
        
        wxBitmap img(100,100);
        
        double x_min=graph_axis[i][0];
        double x_max=graph_axis[i][1];
        double y_min=graph_axis[i][2];
        double y_max=graph_axis[i][3];
        
        if(x_min==x_max)
        {
            x_min=y_min=std::numeric_limits<double>::max();
            x_max=y_max=std::numeric_limits<double>::lowest();
            
            for(std::size_t j=0;j<graph_x_file[i].size();j++)
            {
                ascii_data_loader(graph_x_file[i][j].ToStdString(),data1);
                ascii_data_loader(graph_y_file[i][j].ToStdString(),data2);
                
                std::vector<double> &x=data1[graph_x_ID[i][j]];
                std::vector<double> &y=data2[graph_y_ID[i][j]];
                
                for(std::size_t k=0;k<x.size();k++)
                {
                    x_min=std::min(x_min,x[k]);
                    x_max=std::max(x_max,x[k]);
                    
                    y_min=std::min(y_min,y[k]);
                    y_max=std::max(y_max,y[k]);
                }
            }
        }
        
        dc.SelectObject(img);
        
        dc.SetPen(*wxBLACK_PEN);
        dc.SetBackground(*wxWHITE_BRUSH);
        dc.Clear();
        
        double r,g,b;
        DistinctColorsSampler sampler(1.0,1.0,1.0);
        
        for(std::size_t j=0;j<graph_x_file[i].size();j++)
        {
            sampler(r,g,b);
            dc.SetPen(wxPen(wxColour(255*r,255*g,255*b)));
            
            if(x_min>=x_max || y_min>=y_max) continue;
            
            ascii_data_loader(graph_x_file[i][j].ToStdString(),data1);
            ascii_data_loader(graph_y_file[i][j].ToStdString(),data2);
            
            std::vector<double> &x=data1[graph_x_ID[i][j]];
            std::vector<double> &y=data2[graph_y_ID[i][j]];
            
            int l_prev=0;
            
            for(int k=0;k<100;k++)
            {
                double u=vector_interp_linear(x,y,x_min+(x_max-x_min)*k/99.0,false);
                
                int l=static_cast<int>(99.0*(u-y_min)/(y_max-y_min));
                
                if(k>0) dc.DrawLine(k-1,99-l_prev,k,99-l);
                
                l_prev=l;
            }
        }
        
        dc.SelectObject(wxNullBitmap);
        
        wxButton *btn=new wxButton(gr_panel,btn_id,wxEmptyString,wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
        btn->Bind(wxEVT_BUTTON,&SamplePanel::evt_graph_button,this);
        
        btn->SetBitmap(img);
        
        gr_sizer->Add(btn,wxSizerFlags());
        
        if(graph_caption[i].size()>0)
            btn->SetToolTip(graph_caption[i]);
        
        btn_id++;
    }
    
    gr_panel->SetSizer(gr_sizer);
    gr_panel->SetScrollRate(10,0);
    
    data_sizer->Add(gr_panel,wxSizerFlags().Expand());
    
    // Controls
    
    wxButton *dir_btn=new wxButton(this,wxID_ANY,wxEmptyString,wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    dir_btn->Bind(wxEVT_BUTTON,&SamplePanel::evt_open_directory,this);
    dir_btn->SetBitmap(wxArtProvider::GetBitmap(wxART_FOLDER_OPEN));
    
    edit_btn=new wxButton(this,wxID_ANY,wxEmptyString,wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    edit_btn->Bind(wxEVT_BUTTON,&SamplePanel::evt_edit,this);
    edit_btn->SetBitmap(wxArtProvider::GetBitmap(wxART_NEW));
    
    save_btn=new wxButton(this,wxID_ANY,wxEmptyString,wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    save_btn->Bind(wxEVT_BUTTON,&SamplePanel::evt_edit_save,this);
    save_btn->SetBitmap(wxArtProvider::GetBitmap(wxART_FLOPPY));
    
    cancel_btn=new wxButton(this,wxID_ANY,wxEmptyString,wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    cancel_btn->Bind(wxEVT_BUTTON,&SamplePanel::evt_edit_cancel,this);
    cancel_btn->SetBitmap(wxArtProvider::GetBitmap(wxART_UNDO));
    
    ctrl_sizer->Add(dir_btn);
    ctrl_sizer->Add(edit_btn);
    ctrl_sizer->Add(save_btn);
    ctrl_sizer->Add(cancel_btn);
    
    display_mode_switch();
    
    sizer->Add(data_sizer,wxSizerFlags(1));
    sizer->Add(ctrl_sizer);
    
    SetSizer(sizer);
}

void SamplePanel::display_mode_switch()
{
    date_ctrl->SetEditable(false);
    description_ctrl->SetEditable(false);
    tags_ctrl->SetEditable(false);
    
    if(date.size()==0)
    {
        date_title->Hide();
        date_ctrl->Hide();
    }
    
    if(description.size()==0)
    {
        description_title->Hide();
        description_ctrl->Hide();
    }
    
    if(tags.size()==0)
    {
        tags_title->Hide();
        tags_ctrl->Hide();
    }
    
    edit_btn->Show();
    save_btn->Hide();
    cancel_btn->Hide();
}

void SamplePanel::edit_mode_switch()
{
    date_title->Show();
    date_ctrl->Show();
    date_ctrl->SetEditable(true);
    
    description_title->Show();
    description_ctrl->Show();
    description_ctrl->SetEditable(true);
    
    tags_title->Show();
    tags_ctrl->Show();
    tags_ctrl->SetEditable(true);
    
    edit_btn->Hide();
    save_btn->Show();
    cancel_btn->Show();
}

void SamplePanel::evt_edit(wxCommandEvent &event)
{
    edit_mode_switch();
    
    wxCommandEvent event_out(EVT_SAMPLE_PANELS);
    wxPostEvent(this,event_out);
    
    event.Skip();
}

void SamplePanel::evt_edit_cancel(wxCommandEvent &event)
{
    date_ctrl->ChangeValue(date);
    description_ctrl->ChangeValue(description);
    update_tags_ctrl();
    
    display_mode_switch();
    
    wxCommandEvent event_out(EVT_SAMPLE_PANELS);
    wxPostEvent(this,event_out);
    
    event.Skip();
}

void SamplePanel::update_tags_ctrl()
{
    wxString str;
    
    for(std::size_t i=0;i<tags.size();i++)
    {
        str<<tags[i];
        if(i+1!=tags.size()) str<<" , ";
    }
    
    tags_ctrl->ChangeValue(str);
}

void SamplePanel::evt_edit_save(wxCommandEvent &event)
{
    date=date_ctrl->GetValue().ToStdString();
    description=description_ctrl->GetValue().ToStdString();
    description=replace_special_characters(description);
    
    std::string tmp_string=tags_ctrl->GetValue().ToStdString();
    tmp_string=remove_characters(tmp_string,' ');
    chk_var(tmp_string);
    
    if(tmp_string.size()>0) split_string(tags,tmp_string,',');
    
    rewrite_script();
    
    update_tags_ctrl();
    
    display_mode_switch();
    
    wxCommandEvent event_out(EVT_SAMPLE_PANELS);
    wxPostEvent(this,event_out);
    
    event.Skip();
}

void SamplePanel::evt_graph_button(wxCommandEvent &event)
{
    int i=event.GetId();
    
    SampleGraphDialog *dialog=new SampleGraphDialog(graph_caption[i],
                                                    graph_x_file[i],graph_x_ID[i],
                                                    graph_y_file[i],graph_y_ID[i],
                                                    graph_axis[i]);
    dialog->Show();
    
    event.Skip();
}

void SamplePanel::evt_image_button(wxCommandEvent &event)
{
    int i=event.GetId();
    
    SampleImageDialog *dialog=new SampleImageDialog(im_fname_wx[i],im_description[i]);
    dialog->Show();
    
    event.Skip();
}

void SamplePanel::evt_open_directory(wxCommandEvent &event)
{
    wxLaunchDefaultBrowser(root.GetPath());
    
    event.Skip();
}

bool SamplePanel::has_tag(std::string const &tag)
{
    return vector_contains(tags,tag);
}

void SamplePanel::rewrite_script()
{
    std::ofstream file(root.GetFullPath().ToStdString(),std::ios::out|std::ios::trunc);
    
    if(date.size()>0) file<<"date(\""<<date<<"\")\n\n";
    if(description.size()>0) file<<"description(\""<<description<<"\")\n\n";
    if(tags.size()>0)
    {
        file<<"tags(";
        
        for(std::size_t i=0;i<tags.size();i++)
        {
            file<<"\""<<tags[i]<<"\"";
            if(i+1!=tags.size()) file<<",";
        }
        
        file<<")\n\n";
    }
    if(im_fname.size()>0)
    {
        for(std::size_t i=0;i<im_fname.size();i++)
        {
            file<<"add_image(\""<<im_fname[i]<<"\")\n";
            file<<"image_description(\""<<im_description[i]<<"\")\n\n";
        }
    }
}

//##################
//   SamplesFrame
//##################

SamplesFrame::SamplesFrame(wxString const &title)
    :BaseFrame(title)
{
    wxSplitterWindow *splitter=new wxSplitterWindow(this);
    
    wxPanel *ctrl_panel=new wxPanel(splitter);
    display_panel=new wxScrolledWindow(splitter,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxALWAYS_SHOW_SB);
    
    // Ctrl Panel
    
    wxBoxSizer *ctrl_sizer=new wxBoxSizer(wxVERTICAL);
    
    // - Directory
    
    wxStaticBoxSizer *dir_name_sizer=new wxStaticBoxSizer(wxHORIZONTAL,ctrl_panel,"Directory");
    
    dir_name=new wxTextCtrl(ctrl_panel,wxID_ANY);
    dir_name->SetEditable(false);
    
    wxButton *dir_btn=new wxButton(ctrl_panel,wxID_ANY,"...",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    dir_btn->Bind(wxEVT_BUTTON,&SamplesFrame::evt_directory_name,this);
    
    dir_name_sizer->Add(dir_name,wxSizerFlags(1).Expand());
    dir_name_sizer->Add(dir_btn,wxSizerFlags().Expand());
    
    ctrl_sizer->Add(dir_name_sizer,wxSizerFlags().Expand().Border(wxALL,2));
    
    // - List
    
    wxStaticBoxSizer *dir_list_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Samples");
    
//    dir_list=new wxListBox(ctrl_panel,wxID_ANY,wxDefaultPosition,wxDefaultSize,0,0,wxLB_EXTENDED);
    dir_list=new wxListBox(ctrl_panel,wxID_ANY);
    
    dir_list_sizer->Add(dir_list,wxSizerFlags(1).Expand());
    
    ctrl_sizer->Add(dir_list_sizer,wxSizerFlags(1).Expand().Border(wxALL,2));
    
    // Tags
    
    wxButton *tags_btn=new wxButton(ctrl_panel,wxID_ANY,"Filter tags");
    tags_btn->Bind(wxEVT_BUTTON,&SamplesFrame::evt_tags,this);
    
    ctrl_sizer->Add(tags_btn,wxSizerFlags().Expand().Border(wxALL,2));
    
    ctrl_panel->SetSizer(ctrl_sizer);
    
    // Display Panel
    
    display_sizer=new wxBoxSizer(wxVERTICAL);
    
    display_panel->SetSizer(display_sizer);
    display_panel->SetScrollRate(0,10);
    
    //
    
    splitter->SplitVertically(ctrl_panel,display_panel,300);
    splitter->SetMinimumPaneSize(250);
    
    Bind(EVT_SAMPLE_PANELS,&SamplesFrame::evt_sample_change,this);
}

SamplesFrame::~SamplesFrame()
{
    filter_tree.clear_tree();
}

void SamplesFrame::evt_directory_name(wxCommandEvent &event)
{
    wxDirDialog dialog(0,"Select the sample directory");
    dialog.ShowModal();
    
    wxString dir=dialog.GetPath();
    
    if(dir.size()==0) return;
    
    // Clearning
    
    display_panel->DestroyChildren();
    
    files.Clear();
    tags.clear();
    filter_tree.clear_tree();
    samples.clear();
    
    dir_list->Clear();
    
    dir_name->SetValue(dir);
    
    int N=wxDir::GetAllFiles(dir,&files,"description.txt");
    
    // Creating samples
    
    samples.resize(N);
    wxProgressDialog *pdlg=new wxProgressDialog("","Building the samples database...",N-1);
    
    for(int i=0;i<N;i++)
    {
        samples[i]=new SamplePanel(display_panel,files[i]);
        display_sizer->Add(samples[i],wxSizerFlags().Expand());
        
        pdlg->Update(i);
    }
    
    pdlg->Destroy();
    display_panel->FitInside();
    
    // Updating list
    
    for(int i=0;i<N;i++)
    {
        int k=files[i].Find("description.txt");
        files[i]=files[i].SubString(0,k-2);
        files[i]=files[i].Mid(dir.Length()+1);
    }
    
    dir_list->InsertItems(files,0);
    
    // Updating known tags
    
    update_tags();
    
    show_tagless=true;
    
    event.Skip();
}

void SamplesFrame::evt_sample_change(wxCommandEvent &event)
{
    display_panel->Layout();
    display_panel->FitInside();
}

void SamplesFrame::evt_tags(wxCommandEvent &event)
{
    if(samples.size()==0) return;
    
    update_tags();
    
    TagsDialog dialog(tags,show_tagless,filter_tree);
    
    wxArrayString str_arr;
    std::vector<bool> base_values(tags.size());
    
    for(std::size_t i=0;i<samples.size();i++)
    {
        for(std::size_t j=0;j<base_values.size();j++)
            base_values[j]=false;
        
        bool valid_tags=false;
        
        if(samples[i]->tags.size()==0)
        {
            if(show_tagless) valid_tags=true;
        }
        else
        {        
            for(std::size_t j=0;j<tags.size();j++)
                if(samples[i]->has_tag(tags[j])) base_values[j]=true;
                
            valid_tags=filter_tree.evaluate(tags,base_values);
        }
        
        if(valid_tags)
        {
            samples[i]->Show();
            str_arr.push_back(files[i]);
        }
        else samples[i]->Hide();
    }
    
    dir_list->Clear();
    if(str_arr.size()>0)
        dir_list->InsertItems(str_arr,0);
    
    display_panel->FitInside();
    Refresh();
    
    event.Skip();
}

void SamplesFrame::update_tags()
{
    tags.clear();
    
    for(std::size_t i=0;i<samples.size();i++)
    {
        for(std::size_t j=0;j<samples[i]->tags.size();j++)
            vector_push_single<std::string>(tags,samples[i]->tags[j]);
    }
    
    alphabetical_reorder(tags);
}
