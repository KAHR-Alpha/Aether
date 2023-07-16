/*Copyright 2008-2023 - Loïc Le Cunff

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#include <gui.h>
#include <gui_curve_extractor.h>
#include <gui_diffract_orders.h>
#include <gui_diffract_pattern.h>
#include <gui_ellifr.h>
#include <gui_ellipso3.h>
#include <gui_fd_ms.h>
#include <gui_fdtd.h>
#include <gui_fdtd_structdesign.h>
#include <gui_fieldblock_explr.h>
#include <gui_fitter.h>
#include <gui_geometric_gradient.h>
#include <gui_gl_test.h>
#include <gui_layerfitter.h>
#include <gui_material.h>
#include <gui_matsfitter.h>
#include <gui_mie.h>
#include <gui_multilayers.h>
#include <gui_parametric_visualizer.h>
#include <gui_optical_fibers.h>
#include <gui_rsc.h>
#include <gui_samples_explorer.h>
#include <gui_selene.h>
#include <gui_selene_sensor.h>
#include <gui_sem.h>
#include <gui_spp.h>

#include <aether.h>

#include <wx/aboutdlg.h>
#include <wx/display.h>
#include <wx/strconv.h>

#include <ft2build.h>
#include FT_FREETYPE_H

extern std::ofstream plog;

wxIMPLEMENT_APP(Aether);

wxPoint Aether::default_dialog_origin()
{
//    int s=wxDisplay::GetFromWindow(GetTopWindow());
    int s=wxDisplay::GetFromWindow(wxGetActiveWindow());
    
    wxDisplay dsp(s);
    
//    int x=dsp.GetClientArea().width;
//    int y=dsp.GetClientArea().height;
    
    int x=dsp.GetGeometry().GetX();
    int y=dsp.GetGeometry().GetY();
    int w=dsp.GetGeometry().GetWidth();
    int h=dsp.GetGeometry().GetHeight();
    
    return wxPoint(x+0.1*std::abs(w),y+0.1*std::abs(h));
}

wxSize Aether::default_dialog_size()
{
//    int s=wxDisplay::GetFromWindow(GetTopWindow());
    int s=wxDisplay::GetFromWindow(wxGetActiveWindow());
    
    wxDisplay dsp(s);
    
    int x=dsp.GetClientArea().width;
    int y=dsp.GetClientArea().height;
    
    return wxSize(0.8*std::abs(x),0.8*std::abs(y));
}

void Aether::frame_closed()
{
    NSFrames--;
    
    #ifndef DEV_MODE
    if(NSFrames==0 && !main_frame)
    {
        mfr=new MainFrame("Aether", wxPoint(80, 80), wxSize(650,500) );
        main_frame=true;
    }
    #endif
}

void Aether::main_frame_closed()
{
    main_frame=false;
}

void Aether::new_frame() { NSFrames++; }

bool Aether::OnInit()
{
    PathManager::initialize();
    plog.open(PathManager::to_temporary_path("log.txt"),std::ios::out|std::ios::trunc);
    
    MaterialsLib::initialize();
    
    wxInitAllImageHandlers();
    
    main_frame=true;
    NSFrames=0;
    
    #ifndef DEV_MODE
    mfr=new MainFrame("Aether", wxPoint(80, 80), wxSize(650,500) );
    #else
//    Curve_Extract_Frame *mfr=new Curve_Extract_Frame("Aether");
//    DiffOrdersFrame *mfr=new DiffOrdersFrame("Aether");
//    DiffPatternFrame *mfr=new DiffPatternFrame("Aether");
//    FieldBlockExplorer *mfr=new FieldBlockExplorer("Aether");
//    FresnelFrame *mfr=new FresnelFrame("Aether");
//    EffModelFrame *mfr=new EffModelFrame("Aether");
//    GeoGradientFrame *mfr=new GeoGradientFrame("Aether");
//    MultilayerFrame *mfr=new MultilayerFrame("Aether");
//    SppFrame *mfr=new SppFrame("Aether");
//    MieTool *mfr=new MieTool("Aether");
    HAPSolverFrame *mfr=new HAPSolverFrame("Aether");
//    EMGeometry_Frame *mfr=new EMGeometry_Frame("Aether");
//    SamplesFrame *mfr=new SamplesFrame("Aether");
//    SelGUI::SeleneFrame *mfr=new SelGUI::SeleneFrame("Aether");
//    SelGUI::RayCounterFrame *mfr=new SelGUI::RayCounterFrame("Aether");
//    SEM_2D_Frame *mfr=new SEM_2D_Frame("Aether");
//    FDTD_Frame *mfr=new FDTD_Frame("Aether");
//    FitterFrame *mfr=new FitterFrame("Aether");
//    LayerFitter *mfr=new LayerFitter("Aether");
//    MaterialsEditor *mfr=new MaterialsEditor("Aether");
    mfr->Maximize();
    mfr->Show(true);
    
//    TestGLFrame *mfr1=new TestGLFrame("1");
//    TestGLFrame *mfr2=new TestGLFrame("2");
    #endif
    
    return true;
}

int Aether::OnExit()
{
    std::cout<<"Quitting"<<std::endl;
    std::cout<<main_frame<<std::endl;
    return wxApp::OnExit();
}

void Aether::restore_main_frame()
{
    #ifndef DEV_MODE
    if(!main_frame)
    {
        mfr=new MainFrame("Aether", wxPoint(80, 80), wxSize(650,500) );
        main_frame=true;
    }
    else
    {
        mfr->Restore();
        mfr->Raise();
    }
    #endif
}

//################
//   BaseFrame
//################

enum
{
    MENU_HELP_ABOUT=1000,
    MENU_HELP_BASE_WINDOW=1001
};

BaseFrame::BaseFrame(wxString const &title)
    :wxFrame(NULL,wxID_ANY,title,wxDefaultPosition,wxDefaultSize),
     help_menu_added(false)
{
    wxGetApp().new_frame();
}

BaseFrame::~BaseFrame()
{
    wxGetApp().frame_closed();
}

void BaseFrame::append_help_menu(wxMenuBar *menu_bar)
{
    if(!help_menu_added)
    {
        wxMenu *help_menu=new wxMenu;
        help_menu->Append(MENU_HELP_ABOUT,"About and Help");
        help_menu->AppendSeparator();
        help_menu->Append(MENU_HELP_BASE_WINDOW,"Show main window");
        
        menu_bar->Append(help_menu,"?");
        
        help_menu->Bind(wxEVT_MENU,&BaseFrame::evt_help_menu,this);
        
        help_menu_added=true;
    }
    else return;
}

class AboutDialog: public wxDialog
{
    public:
        AboutDialog()
            :wxDialog(NULL,wxID_ANY,"About Aether",
                      wxGetApp().default_dialog_origin(),
                      wxGetApp().default_dialog_size())
        {
            wxBoxSizer *sizer=new wxBoxSizer(wxHORIZONTAL);
            
            wxBitmap bmp=ImagesManager::get_bitmap(PathManager::locate_resource("resources/aether_logo_WB_grad_256.png").generic_string());
            wxGenericStaticBitmap *logo=new wxGenericStaticBitmap(this,wxID_ANY,bmp);
            
            sizer->Add(logo);
            
            wxNotebook *book=new wxNotebook(this,wxID_ANY);
            
            // Description
            
            wxScrolledWindow *description_panel=new wxScrolledWindow(book);
            wxBoxSizer *description_sizer=new wxBoxSizer(wxVERTICAL);
            
            std::stringstream descr;
            descr<<"The Aether project was originally a FDTD software written for the purposes of a PhD, within the L2n laboratory (formerly known as LNIO).\n";
            descr<<"After many years, it received many additional tools, a GUI interface and its own raytracing software.\n";
            descr<<"The hope is that this work will serve the scientific community, as well as teachers and students, and maybe some professionals even.\n";
            descr<<"It is by no mean perfect and remains a constant work in progress, but has been written to the best of our abilities.\n\n";
            descr<<"Find us on https://aether.utt.fr\n\n";
            descr<<"The following fundings have contributed directly and indirectly to the project:"<<std::endl<<std::endl;
            descr<<"The University of Technology of Troyes"<<std::endl;
            descr<<"The ANR MetaPhotonique project"<<std::endl;
            descr<<"A Région Champagne-Ardenne PhD grant"<<std::endl;
            descr<<"The Eurostars TurboPET project"<<std::endl;
            descr<<"The BISONS RAPID project"<<std::endl;
            descr<<"The ANR DECISIoN project"<<std::endl;
            descr<<"The SURYS society through the In-Fine joint laboratory"<<std::endl;
            descr<<std::endl<<"Build date: "<<__DATE__<<" "<<__TIME__<<std::endl;
            
            std::ifstream revision_file("revision",std::ios::in);
            
            if(revision_file.is_open())
            {
                std::string revision_number;
                std::getline(revision_file,revision_number);
                descr<<"Version: "<<revision_number<<std::endl;
            }
            else descr<<"Version: unknown"<<std::endl;
            
            wxTextCtrl *description=new wxTextCtrl(description_panel,0,descr.str(),
                                                   wxDefaultPosition,wxDefaultSize,
                                                   wxTE_READONLY|wxTE_RICH|wxTE_MULTILINE|wxTE_AUTO_URL);
            
            description_sizer->Add(description,wxSizerFlags(1).Expand());
    
            description_panel->SetSizer(description_sizer);
            description_panel->SetScrollRate(0,10);
            book->AddPage(description_panel,"Description");
            
            // License
            
            std::string license_str;
            std::ifstream license_file(PathManager::locate_resource("LICENSE"),std::ios::in);
            
            std::getline(license_file,license_str,'\0');
            
            wxScrolledWindow *license_panel=new wxScrolledWindow(book);
            wxBoxSizer *license_sizer=new wxBoxSizer(wxVERTICAL);
            
        
            wxTextCtrl *license=new wxTextCtrl(license_panel,1,license_str,
                                               wxDefaultPosition,wxDefaultSize,
                                               wxTE_READONLY|wxTE_RICH|wxTE_MULTILINE|wxTE_AUTO_URL);
            
            license_sizer->Add(license,wxSizerFlags(1).Expand());
            
            license_panel->SetSizer(license_sizer);
            license_panel->SetScrollRate(0,10);
            book->AddPage(license_panel,"License");
            
            // Contributors
            
            wxString contribs_str;
            wxFile contributors_file(PathManager::locate_resource("contributors").generic_string());
            contributors_file.ReadAll(&contribs_str);
                        
            wxScrolledWindow *contribs_panel=new wxScrolledWindow(book);
            wxBoxSizer *contribs_sizer=new wxBoxSizer(wxVERTICAL);
            
            wxTextCtrl *contribs=new wxTextCtrl(contribs_panel,2,contribs_str,
                                                wxDefaultPosition,wxDefaultSize,
                                                wxTE_READONLY|wxTE_RICH|wxTE_MULTILINE);
            
            contribs_sizer->Add(contribs,wxSizerFlags(1).Expand());
            
            contribs_panel->SetSizer(contribs_sizer);
            contribs_panel->SetScrollRate(0,10);
            book->AddPage(contribs_panel,"Contributors");
            
            // Help
            
            std::stringstream help_str;
            
            help_str<<"Need help or want to report a bug?\n\n";
            help_str<<"Have a look at the user manual at https://aether.utt.fr/manual\n\n";
            help_str<<"Chat with us on ircs://irc.libera.chat:6697/Aether\n\n";
            help_str<<"Or send an email to aether_project@utt.fr\n\n";
            
            wxScrolledWindow *help_panel=new wxScrolledWindow(book);
            wxBoxSizer *help_sizer=new wxBoxSizer(wxVERTICAL);
            
            wxTextCtrl *help=new wxTextCtrl(help_panel,3,help_str.str(),
                                            wxDefaultPosition,wxDefaultSize,
                                            wxTE_READONLY|wxTE_RICH|wxTE_MULTILINE|wxTE_AUTO_URL);
            
            help_sizer->Add(help,wxSizerFlags(1).Expand());
            
            help_panel->SetSizer(help_sizer);
            help_panel->SetScrollRate(0,10);
            book->AddPage(help_panel,"Help");
            
            sizer->Add(book,wxSizerFlags(1).Expand());
            
            SetSizer(sizer);
            
            Bind(wxEVT_TEXT_URL,&AboutDialog::evt_url,this);
            
            ShowModal();
        }
        
        void evt_url(wxTextUrlEvent &event)
        {
            wxMouseEvent sub_event=event.GetMouseEvent();
            
            if(sub_event.LeftDown())
            {
                if(event.GetId()==0) wxLaunchDefaultBrowser("https://aether.utt.fr/");
                else if(event.GetId()==1) wxLaunchDefaultBrowser("http://www.apache.org/licenses/");
                else if(event.GetId()==3) wxLaunchDefaultBrowser("https://aether.utt.fr/manual");
            }
        }
};

void BaseFrame::evt_help_menu(wxCommandEvent &event)
{
    int ID=event.GetId();
    
    if(ID==MENU_HELP_BASE_WINDOW) wxGetApp().restore_main_frame();
    else if(ID==MENU_HELP_ABOUT)
    {
        AboutDialog dialog{};
    }
    
    event.Skip();
}

void BaseFrame::evt_load_project(std::string const &extension,std::string const &description)
{
    std::stringstream strm;
    
    strm<<description<<" (*."<<extension<<")|*."<<extension;
    
    wxFileName fname=wxFileSelector("Load Project File...",
                                    wxEmptyString,wxEmptyString,
                                    extension,strm.str(),
                                    wxFD_OPEN);
    
    if(fname.Exists())
    {
        project_fname=fname;
        load_project(fname);
    }
}

void BaseFrame::evt_save_project(std::string const &extension,std::string const &description)
{
    if(project_fname.IsOk()) save_project(project_fname);
    else evt_save_project_as(extension,description);
}

void BaseFrame::evt_save_project_as(std::string const &extension_,std::string const &description)
{
    std::stringstream strm;
    
    strm<<description<<" (*."<<extension_<<")|*."<<extension_;
    
    wxFileName fname=wxFileSelector("Save Project As...",
                                    wxEmptyString,wxEmptyString,
                                    extension_,strm.str(),
                                    wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    
    if(!fname.IsOk()) return;
    else
    {
        wxString extension=fname.GetExt();
        
        if(extension!=extension_)
            wxMessageBox("Invalid file name, you might be overwriting something important, aborting","Warning!");
        else
        {
            project_fname=fname;
            save_project(fname);
        }
    }
}

void BaseFrame::load_project(wxFileName const &fname)
{
}

void BaseFrame::save_project(wxFileName const &fname)
{
}

//###############
//   MainFrame
//###############

char diff_orders_name[]="Diffraction orders";
char diff_pattern_name[]="Diffraction pattern";
char eff_model_name[]="Effective models";
char elli_name[]="Ellipsometry";
char elli3_name[]="Ellipsometry 3D";
char fdtd_name[]="FDTD";
char fbexplr[]="FieldBlock Explorer";
char fibers_name[]="Optical Fibers";
char fitter_name[]="Fitter";
char fresnel_name[]="Fresnel";
char gradient_name[]="Geometric Gradient";
char haps_name[]="Planar Modes Solver (Human-Assisted)";
char layer_fit[]="Layer Fitter";
char mats_fit[]="Materials Fitter";
char mat_explr_name[]="Materials Explorer";
char mie_name[]="Mie";
char multi_name[]="Multilayer";
char opt_fibers_name[]="Optical Fibers";
char param_visu_name[]="Parametric Visualizer";
char sample_exp_name[]="Samples Explorer";
char subdiff_name[]="Subdiffractive Limit";
char selene_name[]="Selene";
char selene_analysis_name[]="Selene";
char sem_2d_name[]="SEM 2D";
char spp_name[]="SPP";
char struct_des[]="Structure Design";

MainFrame::MainFrame(std::string title,wxPoint const &pos, wxSize const &size)
    :wxFrame(NULL,wxID_ANY,title,pos,size)
{
    wxPanel *base_panel=new wxPanel(this);
    wxBoxSizer *top_sizer=new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *tools_sizer=new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *col1_sizer=new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *col2_sizer=new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *col3_sizer=new wxBoxSizer(wxVERTICAL);
    
    wxStaticBoxSizer *fdtd_sizer=new wxStaticBoxSizer(wxVERTICAL,base_panel,wxString("Finite Difference"));
    wxStaticBoxSizer *selene_sizer=new wxStaticBoxSizer(wxVERTICAL,base_panel,wxString("Raytracing"));
    wxStaticBoxSizer *util_sizer=new wxStaticBoxSizer(wxVERTICAL,base_panel,wxString("Utilities"));
    wxStaticBoxSizer *class_sizer=new wxStaticBoxSizer(wxVERTICAL,base_panel,wxString("Classroom"));
    
    // Finite Difference
    
    wxButton *fdtd_btn=new wxButton(base_panel,wxID_ANY,"FDTD");
    fdtd_btn->Bind(wxEVT_BUTTON,&MainFrame::open_frame<FDTD_Frame,fdtd_name>,this);
    fdtd_sizer->Add(fdtd_btn,wxSizerFlags().Expand());
    
    wxButton *fdfd_btn=new wxButton(base_panel,wxID_ANY,"FDFD");
    fdtd_sizer->Add(fdfd_btn,wxSizerFlags().Expand());
    fdfd_btn->Disable();
    
    wxButton *struct_des_btn=new wxButton(base_panel,wxID_ANY,"Structure Design");
    struct_des_btn->Bind(wxEVT_BUTTON,&MainFrame::open_frame<EMGeometry_Frame,struct_des>,this);
    fdtd_sizer->Add(struct_des_btn,wxSizerFlags().Expand());
    
    wxButton *fbexplr_btn=new wxButton(base_panel,wxID_ANY,"Fieldblock Explorer");
    fbexplr_btn->Bind(wxEVT_BUTTON,&MainFrame::open_frame<FieldBlockExplorer,fbexplr>,this);
    fdtd_sizer->Add(fbexplr_btn,wxSizerFlags().Expand());
    
    // Utilities
    
    wxButton *diff_orders_btn=new wxButton(base_panel,wxID_ANY,"Diffraction Orders");
    diff_orders_btn->Bind(wxEVT_BUTTON,&MainFrame::open_frame<DiffOrdersFrame,diff_orders_name>,this);
    util_sizer->Add(diff_orders_btn,wxSizerFlags().Expand());
    
//    wxButton *diff_pattern_btn=new wxButton(base_panel,wxID_ANY,"Diffraction Pattern");
//    diff_pattern_btn->Bind(wxEVT_BUTTON,&MainFrame::open_frame<DiffPatternFrame,diff_pattern_name>,this);
//    util_sizer->Add(diff_pattern_btn,wxSizerFlags().Expand());
    
    
//    wxButton *eff_model_btn=new wxButton(base_panel,wxID_ANY,"Effective Models");
//    eff_model_btn->Bind(wxEVT_BUTTON,&MainFrame::open_frame<EffModelFrame,eff_model_name>,this);
//    util_sizer->Add(eff_model_btn,wxSizerFlags().Expand());
    
    wxButton *elli_btn=new wxButton(base_panel,wxID_ANY,"Ellipsometry 2D");
    elli_btn->Bind(wxEVT_BUTTON,&MainFrame::open_frame<ElliFrame,elli_name>,this);
    class_sizer->Add(elli_btn,wxSizerFlags().Expand());
    
    wxButton *elli_3d_btn=new wxButton(base_panel,wxID_ANY,"Ellipsometry 3D");
    elli_3d_btn->Bind(wxEVT_BUTTON,&MainFrame::open_frame<Ellipso3DFrame,elli3_name>,this);
    class_sizer->Add(elli_3d_btn,wxSizerFlags().Expand());
    
    wxButton *sem_2d_btn=new wxButton(base_panel,wxID_ANY,"SEM 2D");
    sem_2d_btn->Bind(wxEVT_BUTTON,&MainFrame::open_frame<SEM_2D_Frame,sem_2d_name>,this);
    class_sizer->Add(sem_2d_btn,wxSizerFlags().Expand());
        
//    wxButton *fresnel_tool_btn=new wxButton(base_panel,wxID_ANY,"Fresnel");
//    fresnel_tool_btn->Bind(wxEVT_BUTTON,&MainFrame::open_frame<FresnelFrame,fresnel_name>,this);
//    util_sizer->Add(fresnel_tool_btn,wxSizerFlags().Expand());
//    
//    #ifdef DISABLE_GUI_FRESNEL
//    fresnel_tool_btn->Disable();
//    #endif
    
    wxButton *fitter_btn=new wxButton(base_panel,wxID_ANY,"Fitter");
    fitter_btn->Bind(wxEVT_BUTTON,&MainFrame::open_frame<FitterFrame,fitter_name>,this);
    util_sizer->Add(fitter_btn,wxSizerFlags().Expand());
    
    wxButton *gradient_btn=new wxButton(base_panel,wxID_ANY,"Geometric Gradient");
    gradient_btn->Bind(wxEVT_BUTTON,&MainFrame::open_frame<GeoGradientFrame,gradient_name>,this);
    util_sizer->Add(gradient_btn,wxSizerFlags().Expand());
    
    wxButton *layer_fit_btn=new wxButton(base_panel,wxID_ANY,"Layer Fitter");
    layer_fit_btn->Bind(wxEVT_BUTTON,&MainFrame::open_frame<LayerFitter,layer_fit>,this);
    util_sizer->Add(layer_fit_btn,wxSizerFlags().Expand());
    
    wxButton *material_editor_btn=new wxButton(base_panel,wxID_ANY,"Materials Editor");
    material_editor_btn->Bind(wxEVT_BUTTON,&MainFrame::evt_open_materials_manager,this);
    util_sizer->Add(material_editor_btn,wxSizerFlags().Expand());
    
    wxButton *mats_fit_btn=new wxButton(base_panel,wxID_ANY,"Materials Fitter");
    mats_fit_btn->Bind(wxEVT_BUTTON,&MainFrame::open_frame<MatsFitter,mats_fit>,this);
    util_sizer->Add(mats_fit_btn,wxSizerFlags().Expand());
    
    wxButton *mie_tool_btn=new wxButton(base_panel,wxID_ANY,"Mie");
    mie_tool_btn->Bind(wxEVT_BUTTON,&MainFrame::open_frame<MieTool,mie_name>,this);
    util_sizer->Add(mie_tool_btn,wxSizerFlags().Expand());
    
    wxButton *multilayer_btn=new wxButton(base_panel,wxID_ANY,"Multilayers");
    multilayer_btn->Bind(wxEVT_BUTTON,&MainFrame::open_frame<MultilayerFrame,multi_name>,this);
    util_sizer->Add(multilayer_btn,wxSizerFlags().Expand());
    
    wxButton *optical_fibers_btn=new wxButton(base_panel,wxID_ANY,"Optical Fibers");
    optical_fibers_btn->Bind(wxEVT_BUTTON,&MainFrame::open_frame<OptFibersFrame,opt_fibers_name>,this);
    util_sizer->Add(optical_fibers_btn,wxSizerFlags().Expand());
    
    wxButton *param_visu_btn=new wxButton(base_panel,wxID_ANY,"Parametric Visualizer");
    param_visu_btn->Bind(wxEVT_BUTTON,&MainFrame::open_frame<PVisuFrame,param_visu_name>,this);
    util_sizer->Add(param_visu_btn,wxSizerFlags().Expand());
    
    wxButton *planar_solver_btn=new wxButton(base_panel,wxID_ANY,"Planar Modes Solver");
    util_sizer->Add(planar_solver_btn,wxSizerFlags().Expand());
    planar_solver_btn->Disable();
    
    wxButton *ha_planar_solver_btn=new wxButton(base_panel,wxID_ANY,"Planar Modes Solver (Human-Assisted)");
    ha_planar_solver_btn->Bind(wxEVT_BUTTON,&MainFrame::open_frame<HAPSolverFrame,haps_name>,this);
    util_sizer->Add(ha_planar_solver_btn,wxSizerFlags().Expand());
    
    wxButton *samples_btn=new wxButton(base_panel,wxID_ANY,"Samples Explorer");
    samples_btn->Bind(wxEVT_BUTTON,&MainFrame::open_frame<SamplesFrame,sample_exp_name>,this);
    util_sizer->Add(samples_btn,wxSizerFlags().Expand());
    
    wxButton *spp_btn=new wxButton(base_panel,wxID_ANY,"SPP");
    spp_btn->Bind(wxEVT_BUTTON,&MainFrame::open_frame<SppFrame,spp_name>,this);
    util_sizer->Add(spp_btn,wxSizerFlags().Expand());
    
    wxButton *subdiff_btn=new wxButton(base_panel,wxID_ANY,"Subdiffractive Limit");
    subdiff_btn->Bind(wxEVT_BUTTON,&MainFrame::open_frame<DiffOrdersSDFrame,subdiff_name>,this);
    util_sizer->Add(subdiff_btn,wxSizerFlags().Expand());
    
    // Selene
    
    wxButton *selene_btn=new wxButton(base_panel,wxID_ANY,"Selene");
    selene_btn->Bind(wxEVT_BUTTON,&MainFrame::open_frame<SelGUI::SeleneFrame,selene_name>,this);
    selene_sizer->Add(selene_btn,wxSizerFlags().Expand());
    
    wxButton *selene_analysis_btn=new wxButton(base_panel,wxID_ANY,"Analysis");
    selene_analysis_btn->Bind(wxEVT_BUTTON,&MainFrame::open_frame<SelGUI::RayCounterFrame,selene_analysis_name>,this);
    selene_sizer->Add(selene_analysis_btn,wxSizerFlags().Expand());
    
//    wxButton *lenses_btn=new wxButton(base_panel,wxID_ANY,"Lenses");
//    selene_sizer->Add(lenses_btn,wxSizerFlags().Expand());
//    lenses_btn->Disable();
    
    col1_sizer->Add(fdtd_sizer,wxSizerFlags().Expand().Border(wxALL,2));
    col1_sizer->Add(selene_sizer,wxSizerFlags().Expand().Border(wxALL,2));
    col2_sizer->Add(util_sizer,wxSizerFlags().Expand().Border(wxALL,2));
    
    col3_sizer->Add(class_sizer,wxSizerFlags().Expand().Border(wxALL,2));
    
    tools_sizer->Add(col1_sizer,wxSizerFlags(1).Expand());
    tools_sizer->Add(col2_sizer,wxSizerFlags(1).Expand());
    tools_sizer->Add(col3_sizer,wxSizerFlags(1).Expand());
    
    top_sizer->Add(tools_sizer,wxSizerFlags().Expand());
    
    wxButton *about_btn=new wxButton(base_panel,wxID_ANY,"About and Help");
    about_btn->Bind(wxEVT_BUTTON,&MainFrame::evt_about_help,this);
    
    top_sizer->Add(about_btn,wxSizerFlags().Align(wxALIGN_RIGHT).Border(wxALL,5));
    
    base_panel->SetSizerAndFit(top_sizer);
    
    this->Show(true);
}

MainFrame::~MainFrame()
{
    wxGetApp().main_frame_closed();
}

void MainFrame::evt_about_help(wxCommandEvent &event)
{
    AboutDialog dialog{};
}

void MainFrame::evt_open_materials_manager(wxCommandEvent &event)
{
    MaterialsEditor *frame=new MaterialsEditor("Materials Editor");
    frame->Show(true);
    frame->Maximize();
    
    Raise();
}

class TestFrame: public wxFrame
{
    public:
        
        TestFrame(wxString const &title,wxPoint const &pos,wxSize const &size)
            :wxFrame(NULL, wxID_ANY, title, pos, size)
        {
            wxBoxSizer *sizer=new wxBoxSizer(wxVERTICAL);
            
            wxButton *btn=new wxButton(this,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,wxBU_NOTEXT);
            btn->SetBackgroundColour(wxColour(255,0,0));
            btn->SetForegroundColour(wxColour(255,0,0));
            
            sizer->Add(btn,wxSizerFlags().Expand());
            
            SetSizer(sizer);
            
            this->Show(true);
        }
};

/*class TestGLFrame: public wxFrame
{
    public:
        
        wxGLCanvas *glcnv;
        wxGLContext *glcnt;
        
        TestGLFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
};*/
