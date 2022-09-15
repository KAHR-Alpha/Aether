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

#include <gui_selene.h>
#include <aether.h>

namespace SelGUI
{

//###################
//   Polymono_item
//###################

class Polymono_item: public PanelsListBase
{
    public:
        WavelengthSelector *lambda;
        NamedTextCtrl<double> *weight;
        
        Polymono_item(wxWindow *parent,double lambda_,double weight_)
            :PanelsListBase(parent)
        {
            wxBoxSizer *sub_sizer=new wxBoxSizer(wxHORIZONTAL);
            
            lambda=new WavelengthSelector(this,"Wavelength",lambda_);
            weight=new NamedTextCtrl<double>(this,"Weight",weight_,true);
            
            sub_sizer->Add(lambda,wxSizerFlags(1).Expand());
            sub_sizer->Add(weight,wxSizerFlags(1).Expand());
            
            sizer->Add(sub_sizer,wxSizerFlags(1).Expand());
        }
        
        double get_lambda() { return lambda->get_lambda(); }
        double get_weight() { return weight->get_value(); }
};

//########################
//   SourceDialog
//########################

SourceDialog::SourceDialog(Sel::Light *light_,std::vector<Sel::Frame*> const &frames_,std::vector<Material*> &materials_)
    :FrameDialog(dynamic_cast<Sel::Frame*>(light_),frames_),
     materials(materials_),
     light(light_),
     file_x(2), file_y(2),
     spectrum_x(2), spectrum_y(2)
{
    wxSizerFlags std_flags=wxSizerFlags().Expand().Border(wxALL,2);
    
    power=new NamedTextCtrl(ctrl_panel,"Power",light->power,true);
    ctrl_sizer->Add(power,wxSizerFlags().Expand());
    
    // Material
    
    wxArrayString materials_str;
    for(std::size_t i=0;i<materials.size();i++) materials_str.Add(materials[i]->name);
    
    wxStaticBoxSizer *mat_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Ambient Material");
    ambient_material=new wxChoice(mat_sizer->GetStaticBox(),wxID_ANY);
    ambient_material->Set(materials_str);
    
    bool found=false;
    int k=vector_locate(found,materials,light->amb_mat);
    
    ambient_material->SetSelection(0);
    if(found) ambient_material->SetSelection(k);
    
    mat_sizer->Add(ambient_material,wxSizerFlags().Expand());
    ctrl_sizer->Add(mat_sizer,wxSizerFlags().Expand());
    
    // Controls
    
    wxStaticBoxSizer *extent_sizer=new wxStaticBoxSizer(wxVERTICAL,ctrl_panel,"Extent");
    
    extent=new wxChoice(extent_sizer->GetStaticBox(),wxID_ANY);
    extent->Append("Point");
    extent->Append("Circle");
    extent->Append("Ellipse");
    extent->Append("Ellipsoid");
    extent->Append("Rectangle");
    extent->Append("Sphere");
    
    int selection=0;
    
    switch(light->extent)
    {
        case Sel::EXTENT_CIRCLE: selection=1; break;
        case Sel::EXTENT_ELLIPSE: selection=2; break;
        case Sel::EXTENT_ELLIPSOID: selection=3; break;
        case Sel::EXTENT_RECTANGLE: selection=4; break;
        case Sel::EXTENT_SPHERE: selection=5; break;
    }
    
    extent->SetSelection(selection);
    extent->Bind(wxEVT_CHOICE,&SourceDialog::evt_extent,this);
    
    extent_x=new LengthSelector(extent_sizer->GetStaticBox(),"X: ",light->extent_x,false,"mm");
    extent_y=new LengthSelector(extent_sizer->GetStaticBox(),"Y: ",light->extent_y,false,"mm");
    extent_z=new LengthSelector(extent_sizer->GetStaticBox(),"Z: ",light->extent_z,false,"mm");
    extent_d=new LengthSelector(extent_sizer->GetStaticBox(),"D: ",light->extent_d,false,"mm");
    
    extent_sizer->Add(extent,wxSizerFlags().Expand());
    extent_sizer->Add(extent_x,wxSizerFlags().Expand());
    extent_sizer->Add(extent_y,wxSizerFlags().Expand());
    extent_sizer->Add(extent_z,wxSizerFlags().Expand());
    extent_sizer->Add(extent_d,wxSizerFlags().Expand());
    
    ctrl_sizer->Add(extent_sizer,std_flags);
    
    layout_extent();
    
    // Spectrum
    
    spectral_panel=new wxScrolledWindow(panels_book);
    wxBoxSizer *spectral_sizer=new wxBoxSizer(wxVERTICAL);
    
    wxString spectral_type_choices[]={"Monochromatic","Poly-Monochromatic","Polychromatic"};
    
    spectrum_type=new wxRadioBox(spectral_panel,wxID_ANY,"Spectrum Type",
                                 wxDefaultPosition,wxDefaultSize,
                                 3,spectral_type_choices);
    mono_lambda=new WavelengthSelector(spectral_panel,"Wavelength",light->lambda_mono);
    
    // - Polymonochromatic
    
    wxBoxSizer *polymono_sizer=new wxBoxSizer(wxHORIZONTAL);
    polymono_panel=new wxPanel(spectral_panel);
    polymono_list=new PanelsList<>(polymono_panel);
    wxButton *add_polymono_item_btn=new wxButton(polymono_panel,wxID_ANY,"Add");
    
    polymono_sizer->Add(polymono_list,wxSizerFlags(1).Expand());
    polymono_sizer->Add(add_polymono_item_btn,wxSizerFlags());
    
    polymono_panel->SetSizer(polymono_sizer);
    
    for(unsigned int i=0;i<light->polymono_lambda.size();i++)
        polymono_list->add_panel<Polymono_item>(light->polymono_lambda[i],light->polymono_weight[i]);
    
    // - Polychromatic
    
    polychromatic_panel=new wxPanel(spectral_panel);
    wxBoxSizer *polychromatic_sizer=new wxBoxSizer(wxVERTICAL);
    
    spectrum_shape=new wxChoice(polychromatic_panel,wxID_ANY);
    spectrum_shape->Append("Flat");
    spectrum_shape->Append("Planck");
    spectrum_shape->Append("File");
    
    spectrum_shape->Bind(wxEVT_CHOICE,&SourceDialog::evt_polychromatic_type,this);
    
    polychromatic_sizer->Add(spectrum_shape,wxSizerFlags().Expand());
    
    // -- Parameters
    
    wxBoxSizer *poly_params_sizer=new wxBoxSizer(wxHORIZONTAL);
    
    lambda_min=new WavelengthSelector(polychromatic_panel,"Lambda_min",light->lambda_min);
    lambda_max=new WavelengthSelector(polychromatic_panel,"Lambda_max",light->lambda_max);
    temperature=new NamedTextCtrl(polychromatic_panel,"Temperature (K)",light->planck_temperature,true);
    
    lambda_min->Bind(EVT_WAVELENGTH_SELECTOR,&SourceDialog::evt_graph,this);
    lambda_max->Bind(EVT_WAVELENGTH_SELECTOR,&SourceDialog::evt_graph,this);
    temperature->Bind(EVT_NAMEDTXTCTRL,&SourceDialog::evt_graph,this);
    
    poly_params_sizer->Add(lambda_min,wxSizerFlags(1).Expand());
    poly_params_sizer->Add(lambda_max,wxSizerFlags(1).Expand());
    poly_params_sizer->Add(temperature,wxSizerFlags().Expand());
    
    polychromatic_sizer->Add(poly_params_sizer,wxSizerFlags().Expand());
    
    // -- Spectrum File Controls
    
    file_panel=new wxPanel(polychromatic_panel);
    wxStaticBoxSizer *poly_file_sizer=new wxStaticBoxSizer(wxHORIZONTAL,file_panel,"File");
    
    file_ctrl=new wxTextCtrl(poly_file_sizer->GetStaticBox(),wxID_ANY,light->spectrum_file);
    file_ctrl->SetEditable(false);
    
    wxButton *poly_file_btn=new wxButton(poly_file_sizer->GetStaticBox(),wxID_ANY,"...",wxDefaultPosition,wxDefaultSize,wxBU_EXACTFIT);
    poly_file_btn->Bind(wxEVT_BUTTON,&SourceDialog::evt_polychromatic_file,this);
    
    poly_file_sizer->Add(file_ctrl,wxSizerFlags(1));
    poly_file_sizer->Add(poly_file_btn);
    
    file_panel->SetSizer(poly_file_sizer);
    polychromatic_sizer->Add(file_panel,wxSizerFlags().Expand());
    
    // -- Spectrum Graph
    
    sp_graph=new Graph(polychromatic_panel);
    sp_graph->SetMinSize(wxSize(1,400));
    sp_graph->add_data(&spectrum_x,&spectrum_y,1.0,0,0);
    
    polychromatic_sizer->Add(sp_graph,wxSizerFlags(1).Expand());
    
    switch(light->spectrum_shape)
    {
        case Sel::SPECTRUM_FLAT:
            spectrum_shape->SetSelection(0);
            break;
        case Sel::SPECTRUM_PLANCK:
            spectrum_shape->SetSelection(1);
            break;
        case Sel::SPECTRUM_FILE:
            spectrum_shape->SetSelection(2);
            break;
    }
    
    layout_polychromatic_parameters();
        
    polychromatic_panel->SetSizer(polychromatic_sizer);
    
    spectral_sizer->Add(spectrum_type,wxSizerFlags().Expand());
    spectral_sizer->Add(mono_lambda,wxSizerFlags().Expand());
    spectral_sizer->Add(polymono_panel,wxSizerFlags(1).Expand());
    spectral_sizer->Add(polychromatic_panel,wxSizerFlags().Expand());
    
    spectral_panel->SetSizer(spectral_sizer);
    
    switch(light->type)
    {
        case Sel::SRC_BEAM: layout_beam(); break;
        case Sel::SRC_CONE: layout_cone(); break;
        case Sel::SRC_POINT: layout_point(); break;
        case Sel::SRC_PERFECT_BEAM: layout_perfect_beam(); break;
    }
    
    if(light->spectrum_file!="") load_spectrum_file();
    else
    {
        file_x[0]=0; file_x[1]=1;
        file_y[0]=0; file_y[1]=1;
    }
    
    update_graph();
    
    // Spectrum type layout
    
    switch(light->spectrum_type)
    {
        case Sel::SPECTRUM_MONO:
            spectrum_type->SetSelection(0);
            layout_monochromatic();
            break;
        case Sel::SPECTRUM_POLYMONO:
            layout_poly_monochromatic();
            spectrum_type->SetSelection(1);
            break;
        case Sel::SPECTRUM_POLY:
            layout_polychromatic();
            spectrum_type->SetSelection(2);
            break;
    }
    
    spectral_panel->SetScrollRate(10,10);
    spectral_panel->FitInside();
    
    // Bindings
    
    add_polymono_item_btn->Bind(wxEVT_BUTTON,&SourceDialog::evt_add_polymono,this);
    polymono_list->Bind(EVT_PLIST_REMOVE,&SourceDialog::evt_delete_polymono,this);
    spectrum_type->Bind(wxEVT_RADIOBOX,&SourceDialog::evt_spectrum_type,this);
    
    panels_book->AddPage(spectral_panel,"Spectrum");
}

void SourceDialog::evt_add_polymono(wxCommandEvent &event)
{
    polymono_list->add_panel<Polymono_item>(500e-9,1.0);
    spectral_panel->Layout();
    
    spectral_panel->FitInside();
    spectral_panel->Layout();
    
    event.Skip();
}

void SourceDialog::evt_delete_polymono(wxCommandEvent &event)
{
    spectral_panel->FitInside();
    spectral_panel->Layout();
    
    event.Skip();
}

void SourceDialog::evt_extent(wxCommandEvent &event)
{
    layout_extent();
}

void SourceDialog::evt_graph(wxCommandEvent &event)
{
    update_graph();
}

void SourceDialog::evt_polychromatic_file(wxCommandEvent &event)
{
    wxString fname=wxFileSelectorEx("Select a file containing the spectrum...");
    
    if(fname.size()>0)
    {
        file_ctrl->ChangeValue(fname);
        load_spectrum_file();
    }
}

void SourceDialog::evt_polychromatic_type(wxCommandEvent &event)
{
    layout_polychromatic_parameters();
    update_graph();
}

void SourceDialog::evt_spectrum_type(wxCommandEvent &event)
{
    int selection=spectrum_type->GetSelection();
    
    if(selection==0) layout_monochromatic();
    else if(selection==1) layout_poly_monochromatic();
    else if(selection==2) layout_polychromatic();
    
    event.Skip();
}

void SourceDialog::layout_extent()
{
    extent_x->Hide();
    extent_y->Hide();
    extent_z->Hide();
    extent_d->Hide();
    
    switch(extent->GetSelection())
    {
        case 1: extent_d->Show(); break; // Circle
        case 2: extent_y->Show(); extent_z->Show(); break; // Ellipse
        case 3: extent_x->Show();  extent_y->Show(); extent_z->Show(); break; // Ellipsoid
        case 4: extent_y->Show(); extent_z->Show(); break; // Rectangle
        case 5: extent_d->Show(); break; // Sphere
    }
    
    ctrl_panel->FitInside();
    ctrl_panel->Layout();
}

void SourceDialog::layout_monochromatic()
{
    polymono_panel->Hide();
    polychromatic_panel->Hide();
    mono_lambda->Show();
    
    spectral_panel->FitInside();
    spectral_panel->Layout();
}

void SourceDialog::layout_poly_monochromatic()
{
    mono_lambda->Hide();
    polychromatic_panel->Hide();
    polymono_panel->Show();
    
    spectral_panel->FitInside();
    spectral_panel->Layout();
}

void SourceDialog::layout_polychromatic()
{
    mono_lambda->Hide();
    polymono_panel->Hide();
    polychromatic_panel->Show();
    
    spectral_panel->FitInside();
    spectral_panel->Layout();
}

void SourceDialog::layout_polychromatic_parameters()
{
    int selection=spectrum_shape->GetSelection();
    
    if(selection==0)
    {
        lambda_min->Show();
        lambda_max->Show();
        temperature->Hide();
        file_panel->Hide();
    }
    else if(selection==1)
    {
        lambda_min->Show();
        lambda_max->Show();
        temperature->Show();
        file_panel->Hide();
    }
    else if(selection==2)
    {
        lambda_min->Hide();
        lambda_max->Hide();
        temperature->Hide();
        file_panel->Show();
    }
    
    spectral_panel->Layout();
}

void SourceDialog::layout_cone()
{
}

void SourceDialog::layout_beam()
{
}

void SourceDialog::layout_perfect_beam()
{
}

void SourceDialog::layout_point()
{
}

void SourceDialog::layout_point_planar()
{
}

void SourceDialog::load_spectrum_file()
{
    std::vector<std::vector<double>> data;
    
    ascii_data_loader(file_ctrl->GetValue().ToStdString(),data);
    
    file_x=data[0];
    file_y=data[1];
    
    update_graph();
}

void SourceDialog::save_object()
{
    light->power=power->get_value();
    
    if(materials.size()>0)
        light->amb_mat=materials[ambient_material->GetSelection()];
    
    switch(extent->GetSelection())
    {
        case 0:
            light->extent=Sel::EXTENT_POINT;
            break;
        case 1:
            light->extent=Sel::EXTENT_CIRCLE;
            light->extent_d=extent_d->get_length();
            break;
        case 2:
            light->extent=Sel::EXTENT_ELLIPSE;
            light->extent_y=extent_y->get_length();
            light->extent_z=extent_z->get_length();
            break;
        case 3:
            light->extent=Sel::EXTENT_ELLIPSOID;
            light->extent_x=extent_x->get_length();
            light->extent_y=extent_y->get_length();
            light->extent_z=extent_z->get_length();
            break;
        case 4:
            light->extent=Sel::EXTENT_RECTANGLE;
            light->extent_y=extent_y->get_length();
            light->extent_z=extent_z->get_length();
            break;
        case 5:
            light->extent=Sel::EXTENT_SPHERE;
            light->extent_d=extent_d->get_length();
            break;
    }
    
    // Spectrum
    
    switch(spectrum_type->GetSelection())
    {
        case 0: light->spectrum_type=Sel::SPECTRUM_MONO; break;
        case 1: light->spectrum_type=Sel::SPECTRUM_POLYMONO; break;
        case 2: light->spectrum_type=Sel::SPECTRUM_POLY; break;
    }
    
    light->lambda_mono=mono_lambda->get_lambda();
    
    int N=polymono_list->get_size();
    
    light->polymono_lambda.resize(N);
    light->polymono_weight.resize(N);
    for(int i=0;i<N;i++)
    {
        Polymono_item *panel=dynamic_cast<Polymono_item*>(polymono_list->get_panel(i));
        light->polymono_lambda[i]=panel->get_lambda();
        light->polymono_weight[i]=panel->get_weight();
    }
    
    // Polychromatic
    
    if(light->spectrum_type==Sel::SPECTRUM_POLY)
    {
        switch(spectrum_shape->GetSelection())
        {
            case 0:
                light->set_spectrum_flat(lambda_min->get_lambda(),
                                         lambda_max->get_lambda());
                break;
            case 1:
                light->set_spectrum_planck(lambda_min->get_lambda(),
                                           lambda_max->get_lambda(),
                                           temperature->get_value());
                break;
            case 2:
                light->set_spectrum_file(to_generic_path(file_ctrl->GetValue().ToStdString()));
                break;
        }
    }
}

void SourceDialog::update_graph()
{
    int selection=spectrum_shape->GetSelection();
    
    if(selection==0)
    {
        spectrum_x.resize(2);
        spectrum_y.resize(2);
        
        spectrum_x[0]=lambda_min->get_lambda();
        spectrum_x[1]=lambda_max->get_lambda();
        spectrum_y[0]=1; spectrum_y[1]=1;
    }
    else if(selection==1)
    {
        spectrum_x.resize(200);
        spectrum_y.resize(200);
        
        double lmin=lambda_min->get_lambda();
        double lmax=lambda_max->get_lambda();
        double T=temperature->get_value();
        
        for(std::size_t i=0;i<spectrum_x.size();i++)
        {
            spectrum_x[i]=lmin+(lmax-lmin)*i/(spectrum_x.size()-1);
            spectrum_y[i]=planck_distribution_wavelength(spectrum_x[i],T);
        }
    }
    else if(selection==2)
    {
        spectrum_x=file_x;
        spectrum_y=file_y;
    }
            
    sp_graph->Refresh();
    sp_graph->autoscale();
}

}
