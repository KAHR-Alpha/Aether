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

#ifndef GUI_VECTOR3CTRL_H
#define GUI_VECTOR3CTRL_H

#include <geometry.h>
#include <gui.h>

wxDECLARE_EVENT(EVT_V3CTRL,wxCommandEvent);

class Vector3Ctrl: public wxPanel
{
    private:
        Vector3 data;
        
        wxTextCtrl *x_ctrl,*y_ctrl,*z_ctrl;
        
        void update_ctrl();
        void update_x(wxCommandEvent &event);
        void update_y(wxCommandEvent &event);
        void update_z(wxCommandEvent &event);
    public:
        Vector3Ctrl(wxWindow *parent,std::string name="");
        
        double GetX();
        double GetY();
        double GetZ();
        Vector3 GetValue();
        void SetValue(Vector3 const &V);
};

#endif // GUI_VECTOR3CTRL_H
