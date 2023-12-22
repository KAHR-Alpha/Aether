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

#ifndef VORONOI_H_INCLUDED
#define VORONOI_H_INCLUDED

#include <bitmap3.h>
#include <polygons_2D.h>

class FortuneVoronoi
{
    class Site
    {
        public:
            double x,y;
    };

    class Arc
    {
        public:
            int site,event_ID;
    };

    class Breakpoint
    {
        public:
            bool pos_value;
            int site_prev,site_next;
    };
    
    class Event
    {
        public:
            int site,ID;
            double x,y;
            
            bool circle_event;
            double circ_x,circ_y;
            int circ_ps,circ_ns;
            
            bool operator < (Event const &event)
            {
                if(y==event.y)
                     return x<event.x;
                else return !(y<event.y);
            }
    };

    class EventQueue
    {
        public:
            int ID_counter;
            std::vector<Event> events;
            
            EventQueue()
                :ID_counter(0)
            {}
            
            EventQueue(int N_sites)
                :ID_counter(0), events(N_sites)
            {}
            
            int add_circle_event(Event &c_event);
            void forget_event(int ID);
            bool is_empty();
            Event next_event();
            void resize(int N_sites);
            void set_site_event(int i,double x,double y);
            void sort_site_events();
    };
    
    class BoxCross
    {
        public:
            int poly;
            Vector3 P,v;
            
            BoxCross(int poly_,Vector3 const &P_,Vector3 const &v_)
                :poly(poly_), P(P_), v(v_)
            {}
            
            BoxCross(BoxCross const &B)
                :poly(B.poly), P(B.P), v(B.v)
            {}
    };
    
    public:
        int current_vertex;
        double Dx,Dy;
        double bx_min,bx_max,by_min,by_max;
        std::vector<Arc> arcs;
        std::vector<Breakpoint> points;
        std::vector<Site> sites;
        
        std::vector<Vertex> *v_arr;
        std::vector<Polygon2D> *polys;
        
        Bitmap *img;
        
        EventQueue queue;
        
        void boundary_intersection(double x_min,double x_max,double y_min,double y_max);
        void boundary_vertex_generation(std::vector<BoxCross> const &box_cross);
        double breakpoint_x(int i,double y);
        double breakpoints_span(Site const &S1,Site const &S2,Site const &S3,double l,bool pos_val_1,bool pos_val_2);
        bool closing_breakpoints(Site const &S1,Site const &S2,Site const &S3,double l,bool pos_val_1,bool pos_val_2,double delta);
        int event_to_arc(int event_ID);
        void generate(std::vector<Vertex> *v_arr,
                      std::vector<Polygon2D> *polys,
                      std::vector<Vector3> const &sites,
                      double x_min,double x_max,
                      double y_min,double y_max);
        double parabola_intersection(Site const &A,Site const &B,double l,bool pos_value);
        void predict_circle_event(Arc &arc,Breakpoint const &P1,Breakpoint const &P2,double l,
                                  int gen_s1,int gen_s2,int gen_s3);
        void print(int Nx,std::string const &fname);
        void print_parabolas(int Nx,std::string const &fname,double y);
        void process_circle_event(Event const &event);
        void process_site_event(Event const &event);
};

void poisson_disc_sampling(std::vector<Vector3> &P,int N,int Nt,double x_min,double x_max,double y_min,double y_max);
void poisson_disc_sampling(std::vector<Vector3> &P,int N,int Nt,double x_min,double x_max,double y_min,double y_max,int Na);
void poisson_disc_sampling(std::vector<Vector3> &P,int N,int Nt,double x_min,double x_max,double y_min,double y_max,int Na,
                           std::vector<Vector3> const &forbidden_sites,double forbidden_rad);
void poisson_disc_sampling(std::vector<Vector3> &P,int N,int Nt,double x_min,double x_max,double y_min,double y_max,int Na,
                           Grid2<double> const &density_map);

#endif // VORONOI_H_INCLUDED
