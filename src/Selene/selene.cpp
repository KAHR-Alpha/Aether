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

#include <filesystem>
#include <list>

#include <filehdl.h>
#include <selene.h>
#include <mesh_tools.h>

namespace Sel
{
    
void generate_intersection(RayInter &inter,SelRay const &ray,double t,int face_hit,int obj_ID)
{
    Vector3 V=ray.start+t*ray.dir;
    
//    inter.does_intersect=true;
    inter.obj_x=V.x;
    inter.obj_y=V.y;
    inter.obj_z=V.z;
    inter.t=t;
    inter.face=face_hit;
    inter.object=obj_ID;
}

//#################
//   OptimTarget
//#################


double OptimTarget::evaluate() const
{
    double score=0;
    
    RayCounter counter;
    counter.set_sensor(sensor);
    
    if(counter.empty_sensor) return 1e300;
    
    if(goal==OptimGoal::MAXIMIZE_HIT_COUNT)
    {
        score=-counter.compute_hit_count();
    }
    else if(goal==OptimGoal::MINIMIZE_ANGULAR_SPREAD)
    {
        score=counter.compute_angular_spread();
    }
    else if(goal==OptimGoal::MINIMIZE_SPATIAL_SPREAD)
    {
        score=counter.compute_spatial_spread();
    }
    else if(goal==OptimGoal::TARGET_HIT_COUNT)
    {
        score=std::abs(counter.compute_hit_count()-target_value);
    }
    
    return score*weight;
}


//############
//   Selene
//############


Selene::Selene()
    :Nobj(0),
     Nlight(0),
     render_number(0),
     Nr_bounces(200),
     Nr_disp(1000),
     Nr_tot(10000),
     fetch_family(0),
     N_fetched_families(0),
     ray_family_counter(0),
     fetched_source(0)
{
}

Selene::~Selene()
{
}

std::ostream& operator << (std::ostream &strm,RayPath const &ray_path)
{
    strm<<"complete: "<<ray_path.complete<<std::endl;
//    strm<<"intersect: "<<ray_path.intersection.does_intersect<<std::endl;
    strm<<"intersect: "<<ray_path.does_intersect<<std::endl;
    strm<<"obj x: "<<ray_path.intersection.obj_x<<std::endl;
    strm<<"obj y: "<<ray_path.intersection.obj_y<<std::endl;
    strm<<"obj z: "<<ray_path.intersection.obj_z<<std::endl;
    strm<<"t: "<<ray_path.intersection.t<<std::endl;
    strm<<"obj inter: "<<ray_path.intersection.object<<std::endl;
    strm<<"face inter: "<<ray_path.intersection.face<<std::endl;
    strm<<"obj ignore: "<<ray_path.obj_last_intersection_f<<std::endl;
    strm<<"face ignore: "<<ray_path.face_last_intersect<<std::endl;
    
    return strm;
}

void Selene::add_light(Light *src)
{
    light_arr.push_back(src);
    Nlight+=1;
}

void Selene::add_object(Object *obj)
{
    obj->obj_ID=Nobj;
    obj_arr.push_back(obj);
    Nobj+=1;
}

void Selene::fetch_ray(SelRay const &ray)
{
    unsigned int source=ray.source_ID;
    
    if(source!=fetched_source)
    {
        N_fetched_families=0;
        fetched_source=source;
    }
    
    if(N_fetched_families<=light_Nr_disp[source] && ray.family>fetch_family)
    {
        fetch_family=ray.family;
        N_fetched_families++;
    }
    
    if(ray.family<=fetch_family)
    {
        gen_ftc.push_back(ray.generation);
        lambda_ftc.push_back(ray.lambda);
        xs_ftc.push_back(ray.prev_start.x);
        ys_ftc.push_back(ray.prev_start.y);
        zs_ftc.push_back(ray.prev_start.z);
        xe_ftc.push_back(ray.start.x);
        ye_ftc.push_back(ray.start.y);
        ze_ftc.push_back(ray.start.z);
        lost_ftc.push_back(false);
    }
}

void Selene::fetch_ray_lost(SelRay const &ray)
{
    unsigned int source=ray.source_ID;
    
    if(source!=fetched_source)
    {
        N_fetched_families=0;
        fetched_source=source;
    }
    
    if(N_fetched_families<=light_Nr_disp[source] && ray.family>fetch_family)
    {
        fetch_family=ray.family;
        N_fetched_families++;
    }
    
    if(ray.family<=fetch_family)
    {
        gen_ftc.push_back(ray.generation);
        lambda_ftc.push_back(ray.lambda);
        xs_ftc.push_back(ray.start.x);
        ys_ftc.push_back(ray.start.y);
        zs_ftc.push_back(ray.start.z);
        xe_ftc.push_back(ray.start.x+1.0*ray.dir.x);
        ye_ftc.push_back(ray.start.y+1.0*ray.dir.y);
        ze_ftc.push_back(ray.start.z+1.0*ray.dir.z);
        lost_ftc.push_back(true);
    }
}

void Selene::render()
{
    Timer timer;
    
    timer.tic();
    
    trace_calls=0;
    
    if(Nlight==0) return;
    
    if(!output_directory.empty())
        std::filesystem::create_directories(output_directory);
    
    int i;
    
    // Lights initialization
    
    for(i=0;i<Nlight;i++) light_arr[i]->bootstrap();
    
    // - Powers renomalization
    
    double total_power=0;
    for(i=0;i<Nlight;i++) total_power+=light_arr[i]->get_power();
    
    ray_power=total_power/Nr_tot;
    
    int run_Nr_tot=0;
    
    light_N_rays.resize(Nlight);
    light_Nr_disp.resize(Nlight);
    for(i=0;i<Nlight;i++)
    {
        light_N_rays[i]=std::max(static_cast<int>(light_arr[i]->get_power()/ray_power),1);
        run_Nr_tot+=light_N_rays[i];
        
        light_Nr_disp[i]=std::floor(Nr_disp*light_arr[i]->get_power()/total_power);
    }
    
    // Objects initialization
    
    for(i=0;i<Nobj;i++)
        obj_arr[i]->bootstrap(output_directory,ray_power,Nr_bounces);
    
    // Rendering
    
    Nr_cast=0;
    reset_fetcher();
    for(i=0;i<Nlight;i++) light_arr[i]->reset_ray_counter();
    
    RayPath ray_path;
    
    ray_path=request_job();
    Nr_cast++;
    
    while(ray_path.complete==false && Nr_cast<=run_Nr_tot)
    {
        request_raytrace(ray_path);
        
        RayInter &inter=ray_path.intersection;
        
//        if(inter.does_intersect==true)
        if(ray_path.does_intersect==true)
        {
            obj_arr[inter.object]->process_intersection(ray_path);
            fetch_ray(ray_path.ray);
        }
        else
        {
            if(ray_path.ray.generation!=0) fetch_ray_lost(ray_path.ray);
            ray_path.complete=true;
        }
        
        ray_path.ray.generation++;
        
        if(ray_path.complete==true)
        {
            ray_path=request_job();
            Nr_cast++;
        }
    }
    
    for(i=0;i<Nobj;i++) obj_arr[i]->cleanup();
    
     std::ofstream file(output_directory / ("selene_fetcher_" + std::to_string(render_number) + ".txt"),
                        std::ios::out|std::ios::trunc);
    
    int Nftc=gen_ftc.size();
    
    for(int i=0;i<Nftc;i++)
    {
        file<<gen_ftc[i]<<" ";
        file<<lambda_ftc[i]<<" ";
        file<<xs_ftc[i]<<" ";
        file<<ys_ftc[i]<<" ";
        file<<zs_ftc[i]<<" ";
        file<<xe_ftc[i]<<" ";
        file<<ye_ftc[i]<<" ";
        file<<ze_ftc[i]<<" ";
        file<<lost_ftc[i]<<" ";
        file<<std::endl;
    }
    
    file.close();
    
    file.open(output_directory / ("selene_render_"+std::to_string(render_number)),
              std::ios::out|std::ios::trunc);
              
    file<<"total_rays("<<run_Nr_tot<<")\n\n";
    
    for(int i=0;i<Nlight;i++)
        file<<"source_power("<<std::to_string(i)<<","<<std::to_string(light_arr[i]->power)<<")\n";
    file<<"\n";
    
    for(int i=0;i<Nlight;i++)
        file<<"source_N_rays("<<std::to_string(i)<<","<<std::to_string(light_N_rays[i])<<")\n";
    file<<"\n";
    
    file<<"unit_ray_power("<<ray_power<<")\n";
    
    render_number++;
    chk_var(trace_calls);
        
    timer.toc();
    
    chk_var(timer()/trace_calls);
}

void Selene::render(int Nr_disp_,int Nr_tot_)
{
    Nr_disp=Nr_disp_;
    Nr_tot=Nr_tot_;
    
    render();
}

RayPath Selene::request_job()
{
    RayPath job_out;
    job_out.complete=true;
    
    unsigned int sum=0;
    
    for(int i=0;i<Nlight;i++)
    {
        sum+=light_N_rays[i];
        
        if(Nr_cast<sum)
        {
            light_arr[i]->get_ray(job_out.ray);
            
            job_out.ray.source_ID=i;
            job_out.complete=false;
            
            break;
        }
    }
    
    job_out.ray.family=ray_family_counter;
    ray_family_counter++;
    
    return job_out;
}

void Selene::request_raytrace(RayPath &ray_path)
{
    trace_calls++;
    
    int i;
    
    intersection_buffer.clear();
    
    int const &obj_last_intersection_f=ray_path.obj_last_intersection_f;
    int const &face_last_intersect=ray_path.face_last_intersect;
    
    if(obj_last_intersection_f>=0)
    {
        for(i=0;i<obj_last_intersection_f;i++)
            obj_arr[i]->intersect(ray_path.ray,intersection_buffer);
        
        obj_arr[obj_last_intersection_f]->intersect(ray_path.ray,intersection_buffer,face_last_intersect);
        
        for(i=obj_last_intersection_f+1;i<Nobj;i++)
            obj_arr[i]->intersect(ray_path.ray,intersection_buffer);
    }
    else
    {
        for(i=0;i<Nobj;i++)
            obj_arr[i]->intersect(ray_path.ray,intersection_buffer);
    }
    
    double t_min=1e100;
    
    ray_path.does_intersect=false;
    
    for(unsigned int i=0;i<intersection_buffer.size();i++)
    {
        RayInter &itmp=intersection_buffer[i];
        
//        if(itmp.does_intersect==true)
//        {
            if(itmp.t<t_min)
            {
                t_min=itmp.t;
                ray_path.does_intersect=true;
                ray_path.intersection=itmp;
            }
//        }
    }
}

void Selene::reset_fetcher()
{
    gen_ftc.clear();
    lambda_ftc.clear();
    xs_ftc.clear();
    ys_ftc.clear();
    zs_ftc.clear();
    xe_ftc.clear();
    ye_ftc.clear();
    ze_ftc.clear();
    lost_ftc.clear();
    N_fetched_families=0;
    ray_family_counter=0;
    fetched_source=0;
}

void Selene::set_max_ray_bounces(int N) { Nr_bounces=N; }
void Selene::set_N_rays_disp(int Nr_disp_) { Nr_disp=Nr_disp_; }
void Selene::set_N_rays_total(int Nr_tot_) { Nr_tot=Nr_tot_; }

void Selene::set_output_directory(std::filesystem::path const &output_directory_)
{
    output_directory=output_directory_;
}
    

//####################

void lens_geometry(double &A,double &B,double &r_max,double &th_1,double &th_2,
                   double thickness,double r_max_,double r1,double r2)
{
    A=-thickness/2.0+r1;
    B=thickness/2.0+r2;
    
    double inter_x=(r1*r1-r2*r2-A*A+B*B)/(2.0*(B-A));
    double inter_z=r1*r1-(inter_x-A)*(inter_x-A);
    
    bool in=true;
    
    if(   inter_z<0
       || sgn(inter_x-A)!=sgn(-r1)
       || sgn(inter_x-B)!=sgn(-r2)) in=false;
    
    if(in) inter_z=std::sqrt(inter_z);
    else inter_z=r_max_;
    
    r_max=var_min(r_max_,std::abs(r1),std::abs(r2),inter_z);
    
    th_1=std::asin(std::abs(r_max/r1));
    th_2=std::asin(std::abs(r_max/r2));
}

}
