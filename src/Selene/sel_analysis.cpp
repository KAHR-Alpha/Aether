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

#include <logger.h>
#include <selene.h>

namespace Sel
{

RayCounter::RayCounter()
    :owner(false),
     object(nullptr),
     N_faces(0),
     computation_type(RC_COUNTING),
     spectral_mode(SP_FULL),
     empty_sensor(false),
     has_lambda(false),
     has_source(false),
     has_path(false),
     has_generation(false),
     has_phase(false),
     has_polarization(false)
{
}

double RayCounter::compute_angular_spread()
{
    double x_avg=0,
           y_avg=0,
           z_avg=0,
           x_stddev=0,
           y_stddev=0,
           z_stddev=0;
    
    std::size_t N=obj_dir.size();
    
    for(std::size_t i=0;i<N;i++)
    {
        x_avg+=obj_dir[i].x;
        y_avg+=obj_dir[i].y;
        z_avg+=obj_dir[i].z;
    }
    
    x_avg/=N;
    y_avg/=N;
    z_avg/=N;
    
    for(std::size_t i=0;i<N;i++)
    {
        double xs=obj_dir[i].x-x_avg;
        double ys=obj_dir[i].y-y_avg;
        double zs=obj_dir[i].z-z_avg;
        
        x_stddev+=xs*xs;
        y_stddev+=ys*ys;
        z_stddev+=zs*zs;
    }
    
    x_stddev/=N;
    y_stddev/=N;
    z_stddev/=N;
    
    return std::sqrt(x_stddev)+std::sqrt(y_stddev)+std::sqrt(z_stddev);
}


double RayCounter::compute_hit_count()
{
    return obj_inter.size();
}


double RayCounter::compute_spatial_spread()
{
    double x_avg=0,
           y_avg=0,
           z_avg=0,
           x_stddev=0,
           y_stddev=0,
           z_stddev=0;
    
    std::size_t N=obj_dir.size();
    
    for(std::size_t i=0;i<N;i++)
    {
        x_avg+=obj_inter[i].x;
        y_avg+=obj_inter[i].y;
        z_avg+=obj_inter[i].z;
    }
    
    x_avg/=N;
    y_avg/=N;
    z_avg/=N;
    
    for(std::size_t i=0;i<N;i++)
    {
        double xs=obj_inter[i].x-x_avg;
        double ys=obj_inter[i].y-y_avg;
        double zs=obj_inter[i].z-z_avg;
        
        x_stddev+=xs*xs;
        y_stddev+=ys*ys;
        z_stddev+=zs*zs;
    }
    
    x_stddev/=N;
    y_stddev/=N;
    z_stddev/=N;
    
    return std::sqrt(x_stddev)+std::sqrt(y_stddev)+std::sqrt(z_stddev);
}

void RayCounter::initialize()
{
    if(loader.file_ok==false)
    {
        empty_sensor=true;
        return;
    }
    
    // Power unit
    
    std::string sensor_values=loader.header[1];
    
    std::vector<std::string> sv_split;
    split_string(sv_split,sensor_values);
    
    ray_unit=std::stod(sv_split[sv_split.size()-1]);
    chk_var(ray_unit);
    
    // Sensor type
    
    std::string sensor_content_linear=loader.header[2];
    
    std::vector<std::string> sensor_content;
    split_string(sensor_content,sensor_content_linear);
    
    int column_offset=0;
    
    if(vector_contains(sensor_content,std::string("wavelength")))
    {
        has_lambda=true;
        lambda_column=column_offset;
        column_offset++;
    }
    if(vector_contains(sensor_content,std::string("source")))
    {
        has_source=true;
        source_column=column_offset;
        column_offset++;
    }
    if(vector_contains(sensor_content,std::string("path")))
    {
        has_path=true;
        path_column=column_offset;
        column_offset++;
    }
    if(vector_contains(sensor_content,std::string("generation")))
    {
        has_generation=true;
        generation_column=column_offset;
        column_offset++;
    }
    if(vector_contains(sensor_content,std::string("length"))) column_offset++;
    if(vector_contains(sensor_content,std::string("phase")))
    {
        has_phase=true;
        phase_column=column_offset;
        column_offset++;
    }
    if(vector_contains(sensor_content,std::string("world_intersection"))) column_offset+=3;
    if(vector_contains(sensor_content,std::string("world_direction"))) column_offset+=3;
    if(vector_contains(sensor_content,std::string("world_polarization"))) column_offset+=3;
    if(vector_contains(sensor_content,std::string("obj_intersection")))
    {
        obj_inter_column=column_offset;
        column_offset+=3;
    }
    else
    {
        Plog::print(LogType::FATAL, "Invalid ray file ", sensor_fname, "\n");
        Plog::print(LogType::FATAL, "Missing object intersection points\nAborting...\n");
        std::exit(EXIT_FAILURE);
    }
    if(vector_contains(sensor_content,std::string("obj_direction")))
    {
        obj_dir_column=column_offset;
        column_offset+=3;
    }
    if(vector_contains(sensor_content,std::string("obj_polarization")))
    {
        has_polarization=true;
        obj_polar_column=column_offset;
        column_offset+=3;
    }
    if(vector_contains(sensor_content,std::string("obj_face")))
    {
        face_column=column_offset;
    }
    else
    {
        Plog::print(LogType::FATAL, "Invalid ray file ", sensor_fname, "\n");
        Plog::print(LogType::FATAL, "Missing face intersections\nAborting...\n");
        std::exit(EXIT_FAILURE);
    }
    
    N_faces=object->get_N_faces();
    
    bins.resize(N_faces);
    
    Du.resize(N_faces);
    Dv.resize(N_faces);
    
    Nu.resize(N_faces);
    Nv.resize(N_faces);
    
    for(int i=0;i<N_faces;i++)
    {
        object->default_N_uv(Nu[i],Nv[i],i);
        
        Du[i]=1.0/Nu[i];
        Dv[i]=1.0/Nv[i];
        
        bins[i].init(Nu[i],Nv[i]);
    }
    
    // Loading
    
    int Nl=loader.Nl;
    
    if(has_lambda) lambda.resize(Nl);
    if(has_source) source.resize(Nl);
    if(has_path) path.resize(Nl);
    if(has_generation) generation.resize(Nl);
    if(has_phase) phase.resize(Nl);
    if(has_polarization) obj_polarization.resize(Nl);
    
    obj_inter.resize(Nl);
    obj_dir.resize(Nl);
    face.resize(Nl);
    
    std::vector<double> data;
    
    lambda_max=0;
    lambda_min=std::numeric_limits<double>::max();
    
    for(int i=0;i<Nl;i++)
    {
        loader.load_seq(data);
        
        if(has_lambda)
        {
            lambda[i]=data[lambda_column];
            
            lambda_min=std::min(lambda_min,lambda[i]);
            lambda_max=std::max(lambda_max,lambda[i]);
        }
        
        if(has_polarization)
        {
            obj_polarization[i].x=data[obj_polar_column+0];
            obj_polarization[i].y=data[obj_polar_column+1];
            obj_polarization[i].z=data[obj_polar_column+2];
        }
        
        obj_inter[i].x=data[obj_inter_column+0];
        obj_inter[i].y=data[obj_inter_column+1];
        obj_inter[i].z=data[obj_inter_column+2];
        
        obj_dir[i].x=data[obj_dir_column+0];
        obj_dir[i].y=data[obj_dir_column+1];
        obj_dir[i].z=data[obj_dir_column+2];
        
        face[i]=data[face_column];
    }
}

void RayCounter::set_sensor(Sel::Object *object_)
{
    object=object_;
    sensor_fname=object->get_sensor_file_path();
    loader.initialize(sensor_fname.generic_string());
    
    initialize();
}

void RayCounter::set_sensor(std::filesystem::path const &sensor_file)
{
    sensor_fname=sensor_file;
    
    if(object==nullptr)
    {
        owner=true;
        object=new Sel::Object;
    }
    
    // Reading the object geometry from the sensor file
    
    loader.initialize(sensor_fname.generic_string());
    
    std::string object_header=loader.header[0];
    chk_var(object_header);
    
    std::vector<std::string> header_split;
    split_string(header_split,object_header);
    
    std::string object_type=header_split[0];
    std::vector<double> parameters(header_split.size()-1);
    
    for(std::size_t i=1;i<header_split.size();i++)
        parameters[i-1]=std::stod(header_split[i]);
    
    // Setting the object with the proper geometry
    
    if(owner)
    {
             if(object_type=="box") object->set_box(parameters[0],parameters[1],parameters[2]);
        else if(object_type=="cylinder") object->set_cylinder_volume(parameters[0],parameters[1],parameters[2]);
        else if(object_type=="disk") object->set_disk(parameters[0],parameters[1]);
        else if(object_type=="lens") object->set_lens(parameters[0],parameters[1],parameters[2],parameters[3]);
        else if(object_type=="mesh") ;
        else if(object_type=="rectangle") object->set_rectangle(parameters[0],parameters[1]);
        else if(object_type=="parabola") object->set_parabola(parameters[0],parameters[1],parameters[2]);
        else if(object_type=="sphere") object->set_sphere(parameters[0],parameters[1]);
        else if(object_type=="spherical_patch") object->set_spherical_patch(parameters[0],parameters[1]);
        else
        {
            std::cerr<<"Ray analysis error, couldn't set up the object type "<<object_type<<"\nAborting...\n";
            std::exit(EXIT_FAILURE);
        }
    }
    
    initialize();
}

void RayCounter::reallocate()
{
    for(int i=0;i<N_faces;i++)
    {
        if(   Nu[i]!=bins[i].L1()
           || Nv[i]!=bins[i].L2())
        {
            bins[i].init(Nu[i],Nv[i]);
            
            Du[i]=1.0/Nu[i];
            Dv[i]=1.0/Nv[i];
        }
    }
}

void RayCounter::update()
{
    reallocate();
    
    for(int i=0;i<N_faces;i++)
        bins[i]=0;
    
    double unit=1.0;
    if(computation_type==RC_POWER) unit=ray_unit;
    
    for(std::size_t i=0;i<obj_inter.size();i++)
    {
        if(has_lambda && spectral_mode!=SP_FULL)
        {
            if(lambda[i]<lambda_min || lambda[i]>lambda_max) continue;
        }
        
        double u,v;
        
        object->xyz_to_uv(u,v,face[i],
                          obj_inter[i].x,
                          obj_inter[i].y,
                          obj_inter[i].z);
        
        int m=u/Du[face[i]];
        int n=v/Dv[face[i]];
        
        Grid2<double> &fbins=bins[face[i]];
        
        if(m>=0 && m<Nu[face[i]] && n>=0 && n<Nv[face[i]])
        {
            fbins(m,n)+=unit;
        }
    }
}

void RayCounter::update_from_file()
{
    reallocate();
    
    for(int i=0;i<N_faces;i++)
        bins[i]=0;
    
    int Nl=loader.Nl;
    
    std::vector<double> data;
    
    for(int i=0;i<Nl;i++)
    {
        loader.load_seq(data);
        
        double x=data[obj_inter_column+0];
        double y=data[obj_inter_column+1];
        double z=data[obj_inter_column+2];
        
        int face=data[face_column];
        
        double u,v;
        
        object->xyz_to_uv(u,v,face,x,y,z);
        
        int m=u/Du[face];
        int n=v/Dv[face];
        
        Grid2<double> &fbins=bins[face];
        
        if(m>=0 && m<Nu[face] && n>=0 && n<Nv[face])
        {
            fbins(m,n)++;
        }
    }
}
    
}
