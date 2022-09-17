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

#include <selene.h>

namespace Sel
{

Light::Light()
    :spectrum_type(SPECTRUM_MONO),
     extent(EXTENT_POINT),
     polar_type(POLAR_UNSET),
     polar_vector(0,0,0),
     lambda_mono(550e-9),
     polymono_lambda(1,500e-9), polymono_weight(1,1.0),
     spectrum_shape(SPECTRUM_FLAT),
     lambda_min(370e-9), lambda_max(850e-9),
     planck_temperature(5500),
     power(1.0),
     extent_x(0),
     extent_y(0),
     extent_z(0),
     extent_d(0),
     amb_mat(nullptr),
     NRays_sent(0)
{
    type=SRC_POINT;
}

void Light::bootstrap()
{
    consolidate_position();
    
    polymono_weight_sum=0;
    for(std::size_t i=0;i<polymono_weight.size();i++)
        polymono_weight_sum+=polymono_weight[i];
    
    if(spectrum_type==SPECTRUM_POLY)
    {
        if(spectrum_shape==SPECTRUM_PLANCK)
        {
            double lambda_planck_max=h_planck*c_light/(4.965114231744276*k_boltzmann*planck_temperature);
            
            chk_var(lambda_planck_max);
            
            if(lambda_max<=lambda_planck_max)
                planck_max=planck_distribution_wavelength(lambda_max,planck_temperature);
            else if(lambda_min>=lambda_planck_max)
                planck_max=planck_distribution_wavelength(lambda_min,planck_temperature);
            else planck_max=planck_distribution_wavelength(lambda_planck_max,planck_temperature);
        }
        else if(spectrum_shape==SPECTRUM_FILE)
        {
            std::vector<std::vector<double>> data;
            ascii_data_loader(spectrum_file,data);
            
            spf_x=data[0];
            spf_y=data[1];
            
            lambda_min=spf_x[0];
            lambda_max=spf_x[0];
            file_max=spf_y[0];
            
            for(std::size_t i=0;i<spf_y.size();i++)
            {
                lambda_min=std::min(lambda_min,spf_x[i]);
                lambda_max=std::max(lambda_max,spf_x[i]);
                file_max=std::max(file_max,spf_y[i]);
            }
        }
    }
}

double Light::compute_wavelength()
{
         if(spectrum_type==SPECTRUM_MONO) return lambda_mono;
    else if(spectrum_type==SPECTRUM_POLY)
    {
        if(spectrum_shape==SPECTRUM_FLAT) return randp(lambda_min,lambda_max);
        else if(spectrum_shape==SPECTRUM_PLANCK)
        {
            double prob=randp();
            double lambda=randp(lambda_min,lambda_max);
            
            double run=true;
            
            while(run)
            {
                if(prob<=planck_distribution_wavelength(lambda,planck_temperature)/planck_max)
                {
                    run=false;
                    return lambda;
                }
                else
                {
                    prob=randp();
                    lambda=randp(lambda_min,lambda_max);
                }
            }
        }
        else if(spectrum_shape==SPECTRUM_FILE)
        {
            double prob=randp();
            double lambda=randp(lambda_min,lambda_max);
            
            double run=true;
            
            while(run)
            {
                if(prob<=vector_interp_linear(spf_x,spf_y,lambda,false)/file_max)
                {
                    run=false;
                    return lambda;
                }
                else
                {
                    prob=randp();
                    lambda=randp(lambda_min,lambda_max);
                }
            }
        }
    }
    else if(spectrum_type==SPECTRUM_POLYMONO)
    {
        double p=randp(polymono_weight_sum);
        
        int i=0;
        
        while(p-polymono_weight[i]>=0)
        {
            p-=polymono_weight[i];
            i++;
        }
        
        return polymono_lambda[i];
    }
    
    return lambda_min;
}


Vector3 Light::get_anchor(int anchor)
{
    return Vector3(0);
    
//    switch(type)
//    {
//        case OBJ_BOX: return box_anchor(anchor);
//        case OBJ_LENS: return lens_anchor(anchor);
//        case OBJ_PARABOLA: return parabola_anchor(anchor);
//        case OBJ_VOL_CONE: return cone_anchor(anchor);
//        case OBJ_VOL_CYLINDER: return cylinder_anchor(anchor);
//        default: return Vector3(0);
//    }
}

int Light::get_anchors_number()
{
    return 1;
    
//    switch(type)
//    {
//        case OBJ_BOX: return 15;
//        case OBJ_LENS: return 7;
//        case OBJ_PARABOLA: return 4;
//        case OBJ_VOL_CYLINDER: return 3;
//        case OBJ_VOL_CONE: return 3;
//        default: return 1;
//    }
}

std::string Light::get_anchor_name(int anchor)
{
    switch(type)
    {
        default: return "Center";
    }
}

std::string Light::get_anchor_script_name(int anchor)
{
    std::string prefix="";
    std::string anchor_name="";
    
    switch(type)
    {
        default:
            prefix="SEL_LIGHT_";
            anchor_name="CENTER";
    }
    
    anchor_name=prefix+anchor_name;
    for(std::size_t i=0;i<anchor_name.size();i++)
        if(anchor_name[i]!='_') anchor_name[i]=std::toupper(anchor_name[i]);
    
    return anchor_name;
}

double Light::get_power() { return power; }

void Light::get_ray(SelRay &ray)
{
    Vector3 ray_dir;
    
    ray.start=loc;
    ray.lambda=compute_wavelength();
    
    double x=0,y=0,z=0;
    double ex2=extent_x/2.0;
    double ey2=extent_y/2.0;
    double ez2=extent_z/2.0;
    double er=extent_d/2.0;
    
    switch(extent)
    {
        case EXTENT_CIRCLE:
            y=randp(-er,er);
            z=randp(-er,er);
            
            while(y*y+z*z>=er*er)
            {
                y=randp(-er,er);
                z=randp(-er,er);
            }
            break;
            
        case EXTENT_ELLIPSE:
            y=randp(-ey2,ey2);
            z=randp(-ez2,ez2);
            
            while(y*y/(ey2*ey2)+z*z/(ez2*ez2)>=1.0)
            {
                y=randp(-ey2,ey2);
                z=randp(-ez2,ez2);
            }
            break;
            
        case EXTENT_ELLIPSOID:
            x=randp(-ex2,ex2);
            y=randp(-ey2,ey2);
            z=randp(-ez2,ez2);
            
            while(x*x/(ex2*ex2)+y*y/(ey2*ey2)+z*z/(ez2*ez2)>=1.0)
            {
                x=randp(-ex2,ex2);
                y=randp(-ey2,ey2);
                z=randp(-ez2,ez2);
            }
            break;
            
        case EXTENT_RECTANGLE:
            y=randp(-ey2,ey2);
            z=randp(-ez2,ez2);
            break;
            
        case EXTENT_SPHERE:
            x=randp(-er,er);
            y=randp(-er,er);
            z=randp(-er,er);
            
            while(x*x+y*y+z*z>=er*er)
            {
                x=randp(-er,er);
                y=randp(-er,er);
                z=randp(-er,er);
            }
            break;
    }
    
    ray.start+=x*local_x+y*local_y+z*local_z;
    
    ray.pol.rand_sph();
    
         if(type==SRC_POINT) ray_dir.rand_sph();
    else if(type==SRC_POINT_PLANAR)
    {
        double th=randp(2.0*Pi);
        
        ray_dir=std::cos(th)*local_y+std::sin(th)*local_z;
    }
    else if(type==SRC_POINT_ZP)
    {
        ray_dir.rand_sph();
        if(ray_dir.z<0) ray_dir.z=-ray_dir.z;
    }
    else if(type==SRC_POINT_ZM)
    {
        ray_dir.rand_sph();
        if(ray_dir.z>0) ray_dir.z=-ray_dir.z;
    }
    else if(type==SRC_LAMBERTIAN)
    {
        Vector3 ld;
        bool c=true;
        
        while(c)
        {
            ld.rand_sph();
            if(ld.x<0) ld.x=-ld.x;
            if (randp(0, 1) < ld.x) c=false; // ld.x is the cosine of theta (elevation angle)
        }
        ray_dir=ld.x*local_x+ld.y*local_y+ld.z*local_z;
    }
    else if(type==SRC_PERFECT_BEAM)
    {
        ray_dir=local_x;
        ray.pol=local_z;
    }
    else if(type==SRC_USER_DEFINED)
    {
        ray_file.load_seq(user_ray_buffer);
        
        ray.lambda=user_ray_buffer[0];
        ray.phase=user_ray_buffer[1];
        ray.start=Vector3(user_ray_buffer[2],user_ray_buffer[3],user_ray_buffer[4]);
          ray_dir=Vector3(user_ray_buffer[5],user_ray_buffer[6],user_ray_buffer[7]);

        ray.start=loc+ray.start.x*local_x+ray.start.y*local_y+ray.start.z*local_z;
        ray_dir=ray_dir.x*local_x+ray_dir.y*local_y+ray_dir.z*local_z;
        
        if(user_ray_buffer.size()==11) // optional polarization
        {
            ray.pol=Vector3(user_ray_buffer[8],user_ray_buffer[9],user_ray_buffer[10]);
            ray.pol=to_global(ray.pol);
        }
    }
    
    ray.pol=ray.pol-ray_dir*scalar_prod(ray.pol,ray_dir);
    ray.pol.normalize();
    
    ray.set_dir(ray_dir);
    
    // Ambient refractive index
    
    if(amb_mat==nullptr) ray.n_ind=1.0;
    else ray.n_ind=amb_mat->get_n(m_to_rad_Hz(ray.lambda));
    
//    chk_var(ray.n_ind);
    
    NRays_sent++;
}

void Light::reset_ray_counter() { NRays_sent=0; }

void Light::set_power(double power_) { power=power_; }

void Light::set_spectrum_file(std::string const &fname)
{
    spectrum_file=fname;
    spectrum_type=SPECTRUM_POLY;
    spectrum_shape=SPECTRUM_FILE;
    
    std::vector<std::vector<double>> data;
    ascii_data_loader(fname,data);
    
    spf_x=data[0];
    spf_x=data[1];
}

void Light::set_spectrum_flat(double lambda_min_,double lambda_max_)
{
    spectrum_type=SPECTRUM_POLY;
    spectrum_shape=SPECTRUM_FLAT;
    
    lambda_min=lambda_min_;
    lambda_max=lambda_max_;
}

void Light::set_spectrum_planck(double lambda_min_,double lambda_max_,double T)
{
    spectrum_type=SPECTRUM_POLY;
    spectrum_shape=SPECTRUM_PLANCK;
    
    lambda_min=lambda_min_;
    lambda_max=lambda_max_;
    planck_temperature=T;
}

void Light::set_type(int type_) { type=type_; }

}
