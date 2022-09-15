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

#ifndef GUI_OPTICAL_FIBERS_H_INCLUDED
#define GUI_OPTICAL_FIBERS_H_INCLUDED

#include <phys_tools.h>
#include <optical_fibers.h>

#include <gui.h>
#include <gui_gl_fd.h>
#include <gui_material.h>

#include <gl_utils.h>

#include <GL/gl3w.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <wx/wx.h>
#include <wx/splitter.h>

class OptFibersReportDialog: public wxDialog
{
    public:
        OptFibersReportDialog(wxPoint const &coord,wxSize const &size,wxString const &report);
};

class OptFibersFrame: public BaseFrame
{
    public:
        SpectrumSelector *spectrum;
        MiniMaterialSelector *core_mat,*cladding_mat;
        LengthSelector *radius;
        
        NamedTextCtrl<int> *min_order,*max_order;
        NamedTextCtrl<double> *precision;
        wxButton *report_btn,*export_btn;
        
        Graph *graph;
        
        std::vector<std::vector<OpticalFiberMode>> modes_raw; 
        
        std::vector<std::vector<double>> modes_TE,modes_TE_l;
        std::vector<std::vector<double>> modes_TM,modes_TM_l;
        std::vector<std::vector<double>> modes_TEM,modes_TEM_l;
        std::vector<std::vector<int>> modes_TEM_orders;
        
        OptFibersFrame(wxString const &title);
        
        void clear_data();
        void evt_compute(wxCommandEvent &event);
        void evt_export(wxCommandEvent &event);
        void evt_report(wxCommandEvent &event);
};

#endif // GUI_OPTICAL_FIBERS_H_INCLUDED
