#ifndef GUI_SEM_H_INCLUDED
#define GUI_SEM_H_INCLUDED

#include <phys_tools.h>

#include <gui.h>
#include <gui_gl_fd.h>
#include <gui_material.h>

#include <gl_utils.h>

#include <GL/gl3w.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <wx/wx.h>
#include <wx/splitter.h>

class Electron
{
    public:
        bool beam,show;
        double energy,initial_x;
        Vector3 loc,last_loc,dir,color;
        
        Electron()
            :show(false)
        {}
        
        Electron(Electron const &e)
            :beam(e.beam), show(e.show),
             energy(e.energy), initial_x(e.initial_x),
             loc(e.loc), last_loc(e.last_loc), dir(e.dir), color(e.color)
        {
        }
        
        void operator = (Electron const &e)
        {
            beam=e.beam;
            show=e.show;
            
            energy=e.energy;
            initial_x=e.initial_x;
            
            loc=e.loc;
            last_loc=e.last_loc;
            dir=e.dir;
            color=e.color;
        }
};

class GL_SEM_2D: public wxGLCanvas
{
    public:
        int Sx,Sy;
        double x_min,x_max;
        double scale;
        
        bool gl_ok;
        wxGLContext *glcnt;
        
        int Ns_profile;
        GLfloat *profile_vertex_buffer,
                *profile_color_buffer;
        GLuint prog_profile,
               vao_profile,
               gl_profile_vertex_buffer,
               gl_profile_color_buffer;
        
        int N_electrons;
        GLfloat *electrons_vertex_buffer,
                *electrons_color_buffer;
        GLuint prog_electrons,
               vao_electrons,
               gl_electrons_vertex_buffer,
               gl_electrons_color_buffer;
        
        GL_SEM_2D(wxWindow *parent);
        ~GL_SEM_2D();
        
        void evt_mouse(wxMouseEvent &event);
        void init_opengl(std::vector<double> const &profile_x,
                         std::vector<double> const &profile_z);
        void render();
        void update_viewport();
        void update_profile(std::vector<double> const &profile_x,
                            std::vector<double> const &profile_z,
                            std::vector<double> const &profile_color);
        void update_electrons(std::vector<Electron> const &electrons);
};

class SEM_2D_Frame: public BaseFrame
{
    public:
        int Nx;
        std::vector<int> bins,bins_buffer;
        std::vector<double> profile_x,profile_z,profile_color;
        
        bool scanning;
        wxToggleButton *scan_btn;
        std::vector<Electron> electrons;
        
        NamedTextCtrl<int> *N_electrons;
        NamedTextCtrl<double> *slope,*e_energy,*e_mfp,*SE_proportion;
        NamedTextCtrl<int> *scanning_time,*integration_time;
        
        double beam_x;
        SliderDisplay *beam_x_ctrl,*beam_width;
        
        GL_SEM_2D *gl;
        
        wxTimer *timer;
        
        long last_watch;
        wxStopWatch watch;
        
        SEM_2D_Frame(wxString const &title);
        ~SEM_2D_Frame();
        
        double profile(double x,double s);
        void evt_profile(wxCommandEvent &event);
        void evt_timer(wxTimerEvent &event);
        void update_electrons();
        void update_profile();
        
        
};

#endif // GUI_SEM_H_INCLUDED
