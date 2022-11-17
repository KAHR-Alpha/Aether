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

#include <phys_tools.h>

#include <gui_material_editor_panels.h>

#include <wx/grid.h>

extern const Imdouble Im;

//###################
//   MatGUI Panels
//###################

wxDEFINE_EVENT(EVT_DELETE_CAUCHY,wxCommandEvent);
wxDEFINE_EVENT(EVT_DELETE_CRITPOINT,wxCommandEvent);
wxDEFINE_EVENT(EVT_DELETE_DEBYE,wxCommandEvent);
wxDEFINE_EVENT(EVT_DELETE_DRUDE,wxCommandEvent);
wxDEFINE_EVENT(EVT_DELETE_LORENTZ,wxCommandEvent);
wxDEFINE_EVENT(EVT_DELETE_SELLMEIER,wxCommandEvent);
wxDEFINE_EVENT(EVT_DELETE_SPLINE,wxCommandEvent);

namespace MatGUI
{
    // SubmodelPanel
    
    SubmodelPanel::SubmodelPanel(wxWindow *parent)
        :PanelsListBase(parent)
    {
        up_btn->Hide();
        down_btn->Hide();
        
        double R=randp(0.2);
        double G=randp(0.2);
        double B=randp(0.2);
        
        SetBackgroundColour(wxColour(255*(0.8+R),
                                     255*(0.8+G),
                                     255*(0.8+B)));
        
        Bind(EVT_NAMEDTXTCTRL,&SubmodelPanel::evt_modification,this);
        Bind(EVT_WAVELENGTH_SELECTOR,&SubmodelPanel::evt_modification,this);
    }
    
    void SubmodelPanel::apoptose(wxCommandEvent &event)
    {
        signal_type();
    }
    
    void SubmodelPanel::evt_modification(wxCommandEvent &event)
    {
        gui_to_mat();
        
        event.Skip();
    }
    
    void SubmodelPanel::gui_to_mat() {}
    
    void SubmodelPanel::signal_type() {}
    
    // EpsInfPanel
    
    EpsInfPanel::EpsInfPanel(wxWindow *parent,double *eps_inf_)
        :SubmodelPanel(parent),
         mat_eps_inf(eps_inf_)
    {
        eps_inf=new NamedTextCtrl<double>(this,"Eps Infinity",*mat_eps_inf,true);
        
        kill->Hide();
        
        sizer->Add(eps_inf,wxSizerFlags().Expand());
    }
    
    void EpsInfPanel::gui_to_mat()
    {
        *mat_eps_inf=eps_inf->get_value();
    }
    
    void EpsInfPanel::lock()
    {
        eps_inf->lock();
        PanelsListBase::lock();
    }
    
    void EpsInfPanel::unlock()
    {
        eps_inf->unlock();
        PanelsListBase::unlock();
    }
    
    // CritpointPanel
    
    CritpointPanel::CritpointPanel(wxWindow *parent,CritpointModel *critpoint_,int ID_)
        :SubmodelPanel(parent),
         ID(ID_), mat_critpoint(critpoint_)
    {
        set_title("Critpoint "+std::to_string(ID));
        
        A=new NamedTextCtrl<double>(this,"A: ",mat_critpoint->A,true);
        O=new WavelengthSelector(this,"O: ",rad_Hz_to_m(mat_critpoint->O));
        P=new NamedTextCtrl<double>(this,"P: ",mat_critpoint->P,true);
        G=new WavelengthSelector(this,"G: ",rad_Hz_to_m(mat_critpoint->G));
        
        O->change_unit("rad Hz");
        G->change_unit("rad Hz");
                
        sizer->Add(A,wxSizerFlags().Expand());
        sizer->Add(O,wxSizerFlags().Expand());
        sizer->Add(P,wxSizerFlags().Expand());
        sizer->Add(G,wxSizerFlags().Expand());
    }
    
    void CritpointPanel::gui_to_mat()
    {
        mat_critpoint->set(A->get_value(),
                           m_to_rad_Hz(O->get_lambda()),
                           P->get_value(),
                           m_to_rad_Hz(G->get_lambda()));
    }
    
    void CritpointPanel::lock()
    {
        A->lock();
        P->lock();
        O->lock();
        G->lock();
        
        PanelsListBase::lock();
    }
    
    void CritpointPanel::signal_type()
    {
        wxCommandEvent event(EVT_DELETE_CRITPOINT);
        
        event.SetId(ID);
        wxPostEvent(this,event);
    }
    
    void CritpointPanel::unlock()
    {
        A->unlock();
        P->unlock();
        O->unlock();
        G->unlock();
        
        PanelsListBase::unlock();
    }
    
    // CauchyPanel
    
    CauchyPanel::CauchyPanel(wxWindow *parent,std::vector<double> *coeffs_,int ID_)
        :SubmodelPanel(parent),
         ID(ID_), mat_coeffs(coeffs_)
    {
        set_title("Cauchy "+std::to_string(ID));
        
        std::size_t N=mat_coeffs->size();
        
        // Order
        
        order=new NamedTextCtrl<int>(this,"Order",N,true);
        order->Bind(EVT_NAMEDTXTCTRL,&CauchyPanel::evt_order,this);
        
        sizer->Add(order,wxSizerFlags().Expand());
        
        // Coefficients
        
        coeffs.resize(N);
        
        for(std::size_t i=0;i<N;i++)
        {
            coeffs[i]=new NamedTextCtrl(this,"C"+std::to_string(i)+": ",(*mat_coeffs)[i],true);
            sizer->Add(coeffs[i],wxSizerFlags().Expand());
        }
    }
    
    void CauchyPanel::evt_order(wxCommandEvent &event)
    {
        int N=order->get_value();
        int base_N=mat_coeffs->size();
        
        if(N<1) N=1;
        
        if(N<base_N)
        {
            for(int i=base_N-1;i>=N;i--) coeffs[i]->Destroy();
            
            coeffs.resize(N);
        }
        
        mat_coeffs->resize(N);
        
        for(int i=base_N;i<N;i++)
        {
            mat_coeffs->push_back(0);
            
            NamedTextCtrl<double> *new_control=new NamedTextCtrl<double>(this,"C"+std::to_string(i)+": ",0,true);
            sizer->Add(new_control,wxSizerFlags().Expand());
            
            coeffs.push_back(new_control);
        }
        
        Layout();
        
        event.Skip();
    }
    
    void CauchyPanel::gui_to_mat()
    {
        mat_coeffs->resize(coeffs.size());
        
        for(std::size_t i=0;i<coeffs.size();i++)
        {
            (*mat_coeffs)[i]=coeffs[i]->get_value();
        }
    }
    
    void CauchyPanel::lock()
    {
        order->lock();
        
        for(std::size_t i=0;i<coeffs.size();i++)
            coeffs[i]->lock();
            
        PanelsListBase::lock();
    }
    
    void CauchyPanel::signal_type()
    {
        wxCommandEvent event(EVT_DELETE_CAUCHY);
        
        event.SetId(ID);
        wxPostEvent(this,event);
    }
    
    void CauchyPanel::unlock()
    {
        order->unlock();
        
        for(std::size_t i=0;i<coeffs.size();i++)
            coeffs[i]->unlock();
            
        PanelsListBase::unlock();
    }
    
    // DebyePanel
    
    DebyePanel::DebyePanel(wxWindow *parent,DebyeModel *debye_,int ID_)
        :SubmodelPanel(parent),
         ID(ID_), mat_debye(debye_)
    {
        set_title("Debye "+std::to_string(ID));
        
        ds=new NamedTextCtrl(this,"ds",mat_debye->ds,true);
        t0=new NamedTextCtrl(this,"t0",mat_debye->t0,true);
                
        sizer->Add(ds,wxSizerFlags().Expand());
        sizer->Add(t0,wxSizerFlags().Expand());
    }
    
    void DebyePanel::gui_to_mat()
    {
        mat_debye->set(ds->get_value(),t0->get_value());
    }
    
    void DebyePanel::lock()
    {
        ds->lock();
        t0->lock();
        
        PanelsListBase::lock();
    }
    
    void DebyePanel::signal_type()
    {
        wxCommandEvent event(EVT_DELETE_DEBYE);
        
        event.SetId(ID);
        wxPostEvent(this,event);
    }
    
    void DebyePanel::unlock()
    {
        ds->unlock();
        t0->unlock();
        
        PanelsListBase::unlock();
    }
    
    // DrudePanel
    
    DrudePanel::DrudePanel(wxWindow *parent,DrudeModel *drude_,int ID_)
        :SubmodelPanel(parent),
         ID(ID_), mat_drude(drude_)
    {
        set_title("Drude "+std::to_string(ID));
        
        wd=new WavelengthSelector(this,"wd: ",rad_Hz_to_m(mat_drude->wd));
        g=new WavelengthSelector(this,"g: ",rad_Hz_to_m(mat_drude->g));
        
        wd->change_unit("rad Hz");
        g->change_unit("rad Hz");
                
        sizer->Add(wd,wxSizerFlags().Expand());
        sizer->Add(g,wxSizerFlags().Expand());
    }
    
    void DrudePanel::gui_to_mat()
    {
        mat_drude->set(m_to_rad_Hz(wd->get_lambda()),
                       m_to_rad_Hz(g->get_lambda()));
    }
    
    void DrudePanel::lock()
    {
        wd->lock();
        g->lock();
        
        PanelsListBase::lock();
    }
    
    void DrudePanel::signal_type()
    {
        wxCommandEvent event(EVT_DELETE_DRUDE);
        
        event.SetId(ID);
        wxPostEvent(this,event);
    }
    
    void DrudePanel::unlock()
    {
        wd->unlock();
        g->unlock();
        
        PanelsListBase::unlock();
    }
    
    // LorentzPanel
    
    LorentzPanel::LorentzPanel(wxWindow *parent,LorentzModel *lorentz_,int ID_)
        :SubmodelPanel(parent),
         ID(ID_), mat_lorentz(lorentz_)
    {
        set_title("Lorentz "+std::to_string(ID));
        
        A=new NamedTextCtrl<double>(this,"A: ",mat_lorentz->A,true);
        O=new WavelengthSelector(this,"O: ",rad_Hz_to_m(mat_lorentz->O));
        G=new WavelengthSelector(this,"G: ",rad_Hz_to_m(mat_lorentz->G));
        
        O->change_unit("rad Hz");
        G->change_unit("rad Hz");
                
        sizer->Add(A,wxSizerFlags().Expand());
        sizer->Add(O,wxSizerFlags().Expand());
        sizer->Add(G,wxSizerFlags().Expand());
    }
    
    void LorentzPanel::gui_to_mat()
    {
        mat_lorentz->set(A->get_value(),
                         m_to_rad_Hz(O->get_lambda()),
                         m_to_rad_Hz(G->get_lambda()));
    }
    
    void LorentzPanel::lock()
    {
        A->lock();
        O->lock();
        G->lock();
        
        PanelsListBase::lock();
    }
    
    void LorentzPanel::signal_type()
    {
        wxCommandEvent event(EVT_DELETE_LORENTZ);
        
        event.SetId(ID);
        wxPostEvent(this,event);
    }
    
    void LorentzPanel::unlock()
    {
        A->unlock();
        O->unlock();
        G->unlock();
        
        PanelsListBase::unlock();
    }
    
    // SellmeierPanel
    
    SellmeierPanel::SellmeierPanel(wxWindow *parent,double *B_,double *C_,int ID_)
        :SubmodelPanel(parent),
         ID(ID_), mat_B(B_), mat_C(C_)
    {
        set_title("Sellmeier "+std::to_string(ID));
        
        B=new NamedTextCtrl(this,"B: ",*mat_B,true);
        C=new WavelengthSelector(this,"C: ",*mat_C);
        
        sizer->Add(B,wxSizerFlags().Expand());
        sizer->Add(C,wxSizerFlags().Expand());
    }
    
    void SellmeierPanel::gui_to_mat()
    {
        *mat_B=B->get_value();
        *mat_C=C->get_lambda();
    }
    
    void SellmeierPanel::lock()
    {
        B->lock();
        C->lock();
        
        PanelsListBase::lock();
    }
    
    void SellmeierPanel::signal_type()
    {
        wxCommandEvent event(EVT_DELETE_SELLMEIER);
        
        event.SetId(ID);
        wxPostEvent(this,event);
    }
    
    void SellmeierPanel::unlock()
    {
        B->unlock();
        C->unlock();
        
        PanelsListBase::unlock();
    }
    
    // DataPanelDialog
    
    class DataPanelDialog: public wxDialog
    {
        public:
            std::vector<double> &lambda,&data_r,&data_i;
            char &index_type;
            
            wxChoice *type;
            wxGrid *data;
            
            wxMenu popup;
            int popup_target;
            
            class Converter
            {
                public:
                    std::stringstream buffer;
                    
                    std::string operator () (double const &value)
                    {
                        buffer.str("");
                        buffer<<value;
                        return buffer.str();
                    }
            };
            
            Converter cnv;
            
            enum
            {
                MENU_DELETE,
                MENU_INSERT
            };
            
            DataPanelDialog(std::vector<double> *lambda_,
                            std::vector<double> *data_r_,
                            std::vector<double> *data_i_,char *index_type_)
                :wxDialog(nullptr,wxID_ANY,"Spline data",
                          wxGetApp().default_dialog_origin()),
                 lambda(*lambda_),
                 data_r(*data_r_),
                 data_i(*data_i_),
                 index_type(*index_type_)
                 
            {
                wxBoxSizer *sizer=new wxBoxSizer(wxVERTICAL);
                
                // Type
                
                wxBoxSizer *type_sizer=new wxBoxSizer(wxHORIZONTAL);
                
                type=new wxChoice(this,wxID_ANY);
                type->Append("Permittivity");
                type->Append("Index");
                
                type->SetSelection(index_type);
                type->Bind(wxEVT_CHOICE,&DataPanelDialog::evt_type,this);
                
                type_sizer->Add(new wxStaticText(this,wxID_ANY,"Data type: "),wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
                type_sizer->Add(type);
                
                sizer->Add(type_sizer);
                
                // Datasheet
                
                int N=lambda.size();
                
                data=new wxGrid(this,wxID_ANY);
                data->Bind(wxEVT_GRID_CELL_CHANGING,&DataPanelDialog::evt_cell,this);
                data->Bind(wxEVT_GRID_LABEL_RIGHT_CLICK,&DataPanelDialog::evt_grid_label_rclick,this);
                
                data->CreateGrid(N,3);
                
                data->SetColLabelValue(0,"Lambda");
                determine_labels();
                fill_table();
                
                sizer->Add(data,wxSizerFlags(1).Expand());
                
                // Wrapping up
                
                popup.Append(MENU_DELETE,"Delete");
                popup.Append(MENU_INSERT,"Insert");
                
                Bind(wxEVT_MENU,&DataPanelDialog::evt_menu,this);
                
                SetSizer(sizer);
            }
            
            void determine_labels()
            {
                if(index_type)
                {
                    data->SetColLabelValue(1,"real(n)");
                    data->SetColLabelValue(2,"imag(n)");
                }
                else
                {
                    data->SetColLabelValue(1,"real(eps)");
                    data->SetColLabelValue(2,"imag(eps)");
                }
            }
            
            void evt_cell(wxGridEvent &event)
            {
                std::size_t row=event.GetRow();
                std::size_t col=event.GetCol();
                
                double value=std::stod(event.GetString().ToStdString());
                
                if(col==0)
                {
                    lambda[row]=value;
                    
                    bool unsorted=false;
                    
                    if(row>0 && lambda[row-1]>lambda[row]) unsorted=true;
                    if(row+1<lambda.size() && lambda[row+1]<lambda[row]) unsorted=true;
                    
                    if(unsorted)
                    {
                        for(std::size_t i=0;i<lambda.size();i++)
                        {
                            for(std::size_t j=i+1;j<lambda.size();j++)
                            {
                                if(lambda[j]<lambda[i])
                                {
                                    std::swap(lambda[i],lambda[j]);
                                    std::swap(data_r[i],data_r[j]);
                                    std::swap(data_i[i],data_i[j]);
                                }
                            }
                        }
                        
                        event.Veto();
                        
                        data->ClearGrid();
                        fill_table();
                    }
                }
                else if(col==1) data_r[row]=value;
                else data_i[row]=value;
            }
            
            void evt_grid_label_rclick(wxGridEvent &event)
            {
                int row=event.GetRow();
                
                if(row>=0)
                {
                    popup_target=row;
                    
                    PopupMenu(&popup);
                }
            }
            
            void evt_menu(wxCommandEvent &event)
            {
                if(event.GetId()==MENU_DELETE)
                {
                    data->DeleteRows(popup_target);
                    
                    lambda.erase(lambda.begin()+popup_target);
                    data_r.erase(data_r.begin()+popup_target);
                    data_i.erase(data_i.begin()+popup_target);
                }
                else if(event.GetId()==MENU_INSERT)
                {
                    double new_lambda=0;
                    
                    if(popup_target>0)
                    {
                        new_lambda=0.5*(lambda[popup_target-1]+
                                        lambda[popup_target]);
                    }
                    
                    data->InsertRows(popup_target);
                    
                    data->SetCellValue(popup_target,0,cnv(new_lambda));
                    data->SetCellValue(popup_target,1,cnv(0.0));
                    data->SetCellValue(popup_target,2,cnv(0.0));
                    
                    lambda.insert(lambda.begin()+popup_target,new_lambda);
                    data_r.insert(data_r.begin()+popup_target,0);
                    data_i.insert(data_i.begin()+popup_target,0);
                }
            }
            
            void evt_type(wxCommandEvent &event)
            {
                int selection=type->GetSelection();
                
                if(selection!=index_type)
                {
                    index_type=selection;
                    
                    for(std::size_t i=0;i<data_r.size();i++)
                    {
                        Imdouble new_data=data_r[i]+data_i[i]*Im;
                        
                        if(index_type) new_data=std::sqrt(new_data);  // To index
                        else new_data=new_data*new_data;  // To permittivity
                        
                        data_r[i]=new_data.real();
                        data_i[i]=new_data.imag();
                        
                        data->SetCellValue(i,1,cnv(data_r[i]));
                        data->SetCellValue(i,2,cnv(data_i[i]));
                    }
                    
                    determine_labels();
                }
            }
            
            void fill_table()
            {
                for(std::size_t i=0;i<lambda.size();i++)
                {
                    data->SetCellValue(i,0,cnv(lambda[i]));
                    data->SetCellValue(i,1,cnv(data_r[i]));
                    data->SetCellValue(i,2,cnv(data_i[i]));
                }
            }
    };
    
    // DataPanel
    
    DataPanel::DataPanel(wxWindow *parent,int ID_,
                         std::vector<double> *lambda_,
                         std::vector<double> *data_r_,
                         std::vector<double> *data_i_,
                         char *index_type_,
                         Cspline *er_spline_,
                         Cspline *ei_spline_)
        :SubmodelPanel(parent),
         ID(ID_), lambda(lambda_),
         data_r(data_r_), data_i(data_i_),
         index_type(index_type_),
         er_spline(er_spline_),
         ei_spline(ei_spline_)
    {
        set_title("Spline "+std::to_string(ID));
        
        edit_btn=new wxButton(this,wxID_ANY,"Edit Data");
        edit_btn->Bind(wxEVT_BUTTON,&DataPanel::evt_edit,this);
        
        sizer->Add(edit_btn,wxSizerFlags().Expand());
    }
    
    void DataPanel::evt_edit(wxCommandEvent &event)
    {
        DataPanelDialog dialog(lambda,data_r,data_i,index_type);
        dialog.ShowModal();
        
        std::vector<double> w(lambda->size());
        
        for(std::size_t i=0;i<w.size();i++)
            w[i]=m_to_rad_Hz((*lambda)[i]);
        
        if(*index_type)
        {
            std::vector<double> er(lambda->size()),ei(lambda->size());
            
            for(std::size_t i=0;i<er.size();i++)
            {
                Imdouble eps=(*data_r)[i]+(*data_i)[i]*Im;
                eps=eps*eps;
                
                er[i]=eps.real();
                ei[i]=eps.imag();
            }
            
            er_spline->init(w,er);
            ei_spline->init(w,ei);
        }
        else
        {
            er_spline->init(w,*data_r);
            ei_spline->init(w,*data_i);
        }
                    
        wxCommandEvent event_out(EVT_NAMEDTXTCTRL);
        wxPostEvent(this,event_out); // Will be caught by the editor
    }
    
    void DataPanel::lock()
    {
        edit_btn->Disable();
        
        PanelsListBase::lock();
    }
    
    void DataPanel::signal_type()
    {
        wxCommandEvent event(EVT_DELETE_SPLINE);
        
        event.SetId(ID);
        wxPostEvent(this,event);
    }
    
    void DataPanel::unlock()
    {
        edit_btn->Enable();
        
        PanelsListBase::unlock();
    }
}
