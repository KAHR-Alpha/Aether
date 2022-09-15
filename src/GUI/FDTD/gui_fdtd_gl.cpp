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

#include <gui_fdtd.h>

FDTD_GL::FDTD_GL(wxWindow *parent)
    :GL_FD_Base(parent)
{
   
}

void FDTD_GL::add_sensor(Sensor_generator const &generator)
{
    sensor_xm.push_back(generator.x1r);
    sensor_xp.push_back(generator.x2r);
    
    sensor_ym.push_back(generator.y1r);
    sensor_yp.push_back(generator.y2r);
    
    sensor_zm.push_back(generator.z1r);
    sensor_zp.push_back(generator.z2r);
}

void FDTD_GL::add_source(Source_generator const &generator)
{
    source_xm.push_back(generator.x1r);
    source_xp.push_back(generator.x2r);
    
    source_ym.push_back(generator.y1r);
    source_yp.push_back(generator.y2r);
    
    source_zm.push_back(generator.z1r);
    source_zp.push_back(generator.z2r);
    
    source_orientation.push_back(generator.orientation);
}

void FDTD_GL::delete_sensor(int sensor_ID)
{
    std::vector<double>::const_iterator it;
    
    it=sensor_xm.begin()+sensor_ID; sensor_xm.erase(it);
    it=sensor_xp.begin()+sensor_ID; sensor_xp.erase(it);
    
    it=sensor_ym.begin()+sensor_ID; sensor_ym.erase(it);
    it=sensor_yp.begin()+sensor_ID; sensor_yp.erase(it);
    
    it=sensor_zm.begin()+sensor_ID; sensor_zm.erase(it);
    it=sensor_zp.begin()+sensor_ID; sensor_zp.erase(it);
}

void FDTD_GL::delete_source(int source_ID)
{
    std::vector<double>::const_iterator it;
    std::vector<int>::const_iterator it2;
    
    it=source_xm.begin()+source_ID; source_xm.erase(it);
    it=source_xp.begin()+source_ID; source_xp.erase(it);
    
    it=source_ym.begin()+source_ID; source_ym.erase(it);
    it=source_yp.begin()+source_ID; source_yp.erase(it);
    
    it=source_zm.begin()+source_ID; source_zm.erase(it);
    it=source_zp.begin()+source_ID; source_zp.erase(it);
    
    it2=source_orientation.begin()+source_ID;
    source_orientation.erase(it2);
}

void FDTD_GL::forget_sensors()
{
    sensor_xm.clear(); sensor_xp.clear();
    sensor_ym.clear(); sensor_yp.clear();
    sensor_zm.clear(); sensor_zp.clear();
}

void FDTD_GL::forget_sources()
{
    source_xm.clear(); source_xp.clear();
    source_ym.clear(); source_yp.clear();
    source_zm.clear(); source_zp.clear();
    source_orientation.clear();
}

void FDTD_GL::init_opengl()
{
    GL_FD_Base::init_opengl();
    
    sensor_vao.init_opengl();
    sensor_vao.set_mesh_solid("block",0);
    sensor_vao.set_mesh_wireframe("block",0);
    sensor_vao.set_shading(Vector3(0.5,0.5,1.0),true,true);
    
    source_vao.init_opengl();
    source_vao.set_mesh_solid("sphere",3);
    source_vao.set_mesh_wireframe("sphere",3);
//    source_vao.set_mesh_solid("block",3);
//    source_vao.set_mesh_wireframe("block",3);
    source_vao.set_shading(Vector3(1.0,1.0,0),true,true);
}

void FDTD_GL::render()
{
    GL_FD_Base::render();
    
    sensor_vao.set_world(lx,ly,lz,scale);
    source_vao.set_world(lx,ly,lz,scale);
    
    glUseProgram(prog_wires);
    
    glLineWidth(3.0);
    glUniformMatrix4fv(10,1,0,camera.proj_gl);
    
    for(unsigned int i=0;i<sensor_xm.size();i++)
    {
        sensor_vao.set_matrix(Vector3(sensor_xm[i],sensor_ym[i],sensor_zm[i]),
                              Vector3(sensor_xp[i]-sensor_xm[i],0,0),
                              Vector3(0,sensor_yp[i]-sensor_ym[i],0),
                              Vector3(0,0,sensor_zp[i]-sensor_zm[i]));
        
        sensor_vao.draw_wireframe();
    }
    
    double max_l=var_max(lx/scale,ly/scale,lz/scale);
    
    for(unsigned int i=0;i<source_xm.size();i++)
    {
        Vector3 Va(0.01*max_l,0,0);
        Vector3 Vb(0,0.01*max_l,0);
        Vector3 Vc(0,0,0.01*max_l);
        
             if(source_orientation[i]==NORMAL_X || source_orientation[i]==NORMAL_XM) Va*=8.0;
        else if(source_orientation[i]==NORMAL_Y || source_orientation[i]==NORMAL_YM) Vb*=8.0;
        else if(source_orientation[i]==NORMAL_Z || source_orientation[i]==NORMAL_ZM) Vc*=8.0;
        
        source_vao.set_matrix(Vector3(source_xm[i],source_ym[i],source_zm[i]),Va,Vb,Vc);
        
        source_vao.draw_wireframe();
    }
    
    glLineWidth(1.0);
}

void FDTD_GL::update_sensor(int sensor_ID,Sensor_generator const &generator)
{
    sensor_xm[sensor_ID]=generator.x1r;
    sensor_xp[sensor_ID]=generator.x2r;
    
    sensor_ym[sensor_ID]=generator.y1r;
    sensor_yp[sensor_ID]=generator.y2r;
    
    sensor_zm[sensor_ID]=generator.z1r;
    sensor_zp[sensor_ID]=generator.z2r;
}

void FDTD_GL::update_source(int source_ID,Source_generator const &generator)
{
    source_xm[source_ID]=generator.x1r;
    source_xp[source_ID]=generator.x2r;
    
    source_ym[source_ID]=generator.y1r;
    source_yp[source_ID]=generator.y2r;
    
    source_zm[source_ID]=generator.z1r;
    source_zp[source_ID]=generator.z2r;
    
    source_orientation[source_ID]=generator.orientation;
}
