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

#ifndef ANISO_RETRIEVER_H
#define ANISO_RETRIEVER_H

#include <vector>

#include <berreman_strat.h>
#include <data_hdl.h>
#include <mathUT.h>

class AnisoRetriever;

enum
{
    ANRT_BASE_STEP,
    ANRT_SECOND_STEP,
    ANRT_FULL_STEP,
    ANRT_NULL,
    ANRT_SYM,
    ANRT_ADJ,
    ANRT_ASYM,
    ANRT_ISO,
    ANRT_W_EQ,
    ANRT_W_LIN,
    ANRT_W_QUAD
};


//Parameters handler
class AnisoRetriever
{
    private:
        int op_type,constraint,weight_mode;
        double n1,n3,h_slab;
        int Nl;
        double lambda_min,lambda_max;
        bool limit_ang;
        double ang_max;
        bool is_diagonal,field_rot;
        std::string data_fname,base_fname,out_fname;
        
        Grid1<double> lambda;
        Grid1<SpAng> sp_data_ref,sp_data_trans;
        
        double base_eps_max;
        double base_mu_max;
        
        bool flag_normal_inc;
        bool flag_opti_TE;
        bool flag_opti_TM;
        bool flag_opti_TEM;
        
        Grid2<bool> enable_eps_search;
        Grid2<bool> enable_mu_search;
        
        Grid3<double> base_eps_r_span,base_eps_i_span;
        Grid3<double> base_mu_r_span,base_mu_i_span;
        
        Grid3<Imdouble> eps,mu;
        Grid1<double> obj_func;
        
        bool flag_compare_norm;
        
        double compare_data(B_strat &strat,SpAng &sp_ref,SpAng &sp_trans);
        double compare_data_simp(B_strat &strat,SpAng &sp_ref,SpAng &sp_trans);
        double compare_data_norm(B_strat &strat,SpAng &sp_ref,SpAng &sp_trans);
                
        void generate_eps(Grid2<Imdouble> &eps,
                          Grid3<double> const &eps_r_span,Grid3<double> const &eps_i_span);
        void generate_mu(Grid2<Imdouble> &mu,
                         Grid3<double> const &mu_r_span,Grid3<double> const &mu_i_span);
                         
        double get_weight(double ang);
                         
        void init_base_step();
        void init_second_step();
        void init_full_step();
        
        void reset_spans();
        void init_search_span(double eps_coeff=300.0,double mu_coeff=100.0);
        void show_span();
        
        void search(Grid2<Imdouble> &eps,Grid2<Imdouble> &mu,double &obj_func_out,
                    Grid2<Imdouble> &base_eps,Grid2<Imdouble> &base_mu,
                    SpAng &sp_ref,SpAng &sp_trans,int it_level,bool follow=false);
        
        void search_cont(Grid2<Imdouble> &eps,Grid2<Imdouble> &mu,double &obj_func_out,
                         Grid2<Imdouble> &base_eps,Grid2<Imdouble> &base_mu,
                         SpAng &sp_ref,SpAng &sp_trans,double rad,double rad_min);
                    
        void gradient_descent(Grid2<Imdouble> &eps,Grid2<Imdouble> &mu,double &obj_func_out,
                              Grid2<Imdouble> &base_eps,Grid2<Imdouble> &base_mu,
                              SpAng &sp_ref,SpAng &sp_trans,int it_div,int it_mul,int max_sub,double disp_coeff=1e-4);
        
        void pretreat();

    public:
        AnisoRetriever();
        
        Grid1<double> const & get_lambda() const;
        int get_op_type() const;
        
        void set_angle_limit(double ang); //Angle in rad
        
        void set_base(std::string const &base_fname);
        void set_constraint(int constraint);
        void set_data(std::string const &data_fname);
        
        void set_operation_base();
        void set_operation_second();
        void set_operation_full();
        
        void disable_TE();
        void disable_TM();
        void disable_TEM();
        void enable_norm_inc();
        void set_diagonal();
        
        void set_output(std::string fname);
        
        void set_index_substrate(double n3);
        void set_index_superstrate(double n1);
        void set_spectrum(int Nl,double lambda_min,double lambda_max);
        void set_weight(int weight);
        void set_thickness(double h_slab);
        void run();
        
};

#endif // ANISO_RETRIEVER_H
