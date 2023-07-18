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

#ifndef GUI_SAMPLES_EXPLORER_H_INCLUDED
#define GUI_SAMPLES_EXPLORER_H_INCLUDED

#include <gui.h>

#include <wx/listbox.h>

#include <array>

class BooleanTree
{
    public:
        int type;
        std::vector<std::string> tags;
        std::vector<BooleanTree*> leaves;
        
        BooleanTree() {}
        ~BooleanTree() { clear_tree(); }
        
        void clear_tree();
        bool evaluate(std::vector<std::string> const &base_tags,
                      std::vector<bool> const &base_values);
        bool is_empty();
};

class BooleanPanel : public wxPanel
{
    public:
        int type;
        std::vector<std::string> full_tags;
        
        wxChoice *type_ctrl,*add_choice;
        
        int tag_ID_counter;
        std::vector<int> tag_IDs;
        std::vector<wxChoice*> tags;
        std::vector<wxPanel*> tag_panels;
        wxBoxSizer *tags_sizer;
        
        int op_ID_counter;
        std::vector<int> op_IDs;
        std::vector<BooleanPanel*> ops;
        std::vector<wxPanel*> op_panels;
        wxBoxSizer *ops_sizer;
        
        BooleanPanel(wxWindow *parent,int type,
                     std::vector<std::string> const &full_tags);
        
        void add_tag_panel();
        void add_op_panel(int type);
        void evt_add_element(wxCommandEvent &event);
        void evt_delete_operation(wxCommandEvent &event);
        void evt_delete_tag(wxCommandEvent &event);
        void evt_type(wxCommandEvent &event);
        void from_tree(BooleanTree &tree);
        void to_tree(BooleanTree &tree);
};

class SampleGraphDialog : public wxFrame
{
    public:
        std::vector<std::vector<double>> x,y;
        
        SampleGraphDialog(wxString const &caption,
                          std::vector<wxString> const &file_x,std::vector<int> const &ID_x,
                          std::vector<wxString> const &file_y,std::vector<int> const &ID_y,
                          std::array<double,4> const &axis);
};

class SampleImageDialog : public wxFrame
{
    public:
        
        SampleImageDialog(wxString const &im_fname,std::string const &im_description);
};

class SamplePanel : public wxPanel
{
    public:
        wxFileName root;
        std::string date,description;
        std::vector<std::string> tags;
        
        std::vector<std::string> im_fname,im_description;
        std::vector<wxString> im_fname_wx;
        
        std::vector<std::string> graph_caption;
        std::vector<std::vector<int>> graph_x_ID,graph_y_ID;
        std::vector<std::vector<wxString>> graph_x_file,graph_y_file;
        std::vector<std::array<double,4>> graph_axis;
        
        // Date
        
        wxStaticText *date_title;
        wxTextCtrl *date_ctrl;
        
        // Description
        
        wxStaticText *description_title;
        wxTextCtrl *description_ctrl;
        
        // Tags
        
        wxStaticText *tags_title;
        wxTextCtrl *tags_ctrl;
        
        // Images
        
        wxScrolledWindow *im_panel;
        wxBoxSizer *im_sizer;
        
        // Graphs
        
        wxScrolledWindow *gr_panel;
        wxBoxSizer *gr_sizer;
        
        // Buttons
        
        wxButton *edit_btn;
        wxButton *save_btn;
        wxButton *cancel_btn;
        
        SamplePanel(wxWindow *parent,
                    wxString const &script_name);
                    
        void display_mode_switch();
        void edit_mode_switch();
        void evt_edit(wxCommandEvent &event);
        void evt_edit_cancel(wxCommandEvent &event);
        void evt_edit_save(wxCommandEvent &event);
        void evt_graph_button(wxCommandEvent &event);
        void evt_image_button(wxCommandEvent &event);
        void evt_open_directory(wxCommandEvent &event);
        bool has_tag(std::string const &tag);
        void rewrite_script();
        void update_tags_ctrl();
};

class SamplesFrame : public BaseFrame
{
    public:
        wxTextCtrl *dir_name;
        wxListBox *dir_list;
        wxScrolledWindow *display_panel;
        wxBoxSizer *display_sizer;
        
        wxArrayString files;
        std::vector<std::string> tags;
        bool show_tagless;
        std::vector<SamplePanel*> samples;
        
        BooleanTree filter_tree;
        
        SamplesFrame(wxString const &title);
        ~SamplesFrame();
        
        void evt_directory_name(wxCommandEvent &event);
        void evt_sample_change(wxCommandEvent &event);
        void evt_tags(wxCommandEvent &event);
        void update_tags();
};

#endif // GUI_SAMPLES_EXPLORER_H_INCLUDED
