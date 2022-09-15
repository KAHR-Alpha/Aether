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

#ifndef AETHER_H_INCLUDED
#define AETHER_H_INCLUDED

#include <wx/wx.h>

#ifndef LIMULE_MODE
    class MainFrame;
    
    
    class Aether: public wxApp
    {
        private:
            bool main_frame;
            int NSFrames;
            MainFrame *mfr;
            
        public:
            wxPoint default_dialog_origin();
            wxSize default_dialog_size();
            void frame_closed();
            void main_frame_closed();
            void new_frame();
            void restore_main_frame();
            
            virtual bool OnInit();
            virtual int OnExit();
    };

    wxDECLARE_APP(Aether);
#else
    class LimuleApp: public wxApp
    {
        public:
            wxPoint default_dialog_origin();
            wxSize default_dialog_size();
            void frame_closed();
            void main_frame_closed();
            void new_frame();
            void screen_size(int &sx,int &sy);
            
            virtual bool OnInit();
            virtual int OnExit();
    };

    wxDECLARE_APP(LimuleApp);
#endif

#endif // AETHER_H_INCLUDED
