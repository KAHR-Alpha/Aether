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

#include <filehdl.h>

#include <gui_gl_fd.h>

//###################
//   GL_2D_display
//###################

GL_2D_display::GL_2D_display(wxWindow *parent)
    :wxGLCanvas(parent,wxID_ANY,NULL),
     waiting_bitmap_refresh(false), busy(false), gl_ok(false), N1(100), N2(100), map(nullptr)
{
    glcnt=new wxGLContext(this);
    Show(true);
    
    timer=new wxTimer(this);
    Bind(wxEVT_TIMER,&GL_2D_display::evt_timed_refresh,this);
    Bind(wxEVT_SIZE,&GL_2D_display::evt_resize,this);
    timer->Start(1000/60);
}

GL_2D_display::~GL_2D_display()
{
    timer->Stop();
    delete glcnt;
}

void GL_2D_display::base_render()
{
    if(IsShown())
    {
        if(waiting_bitmap_refresh) refresh_from_bitmap();
        
        SetCurrent(*glcnt);
        if(!busy)
        {
            glClearColor(0.0, 0.0, 0.0, 0.0);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            
            render();
            
            SwapBuffers();
        }
    }
}

void GL_2D_display::init_opengl()
{
    SetCurrent(*glcnt);
    
    if(gl3wInit())
        wxMessageBox("Error: Could not initialize OpenGL");
    if(!gl3wIsSupported(4,3))
        wxMessageBox("Error: OpenGL 4.3 not supported.\n Update your graphics drivers.");;
    
    //
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    glDisable(GL_CULL_FACE);
    
    // Vao
    
    glGenVertexArrays(1,&vao);
    glBindVertexArray(vao);
    
    glGenBuffers(1,&vertex_buff);
    glGenBuffers(1,&uv_buff);
    glGenBuffers(1,&index_buff);
    
    v_arr=new GLfloat[16];
    
    v_arr[0]=-1.0; v_arr[1]=-1.0; v_arr[2]=0;
    v_arr[3]= 1.0;
    
    v_arr[4]=+1.0; v_arr[5]=-1.0; v_arr[6]=0;
    v_arr[7]= 1.0;
    
    v_arr[8]=+1.0; v_arr[9]=+1.0; v_arr[10]=0;
    v_arr[11]=1.0;
    
    v_arr[12]=-1.0; v_arr[13]=+1.0; v_arr[14]=0;
    v_arr[15]=1.0;
    
    glBindBuffer(GL_ARRAY_BUFFER,vertex_buff);
    glBufferData(GL_ARRAY_BUFFER,
                 16*sizeof(GLfloat),
                 (void*)v_arr,
                 GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,0);
    
    GLfloat *uv_arr=new GLfloat[8];
    
    uv_arr[0]=0.0;
    uv_arr[1]=0.0;
    uv_arr[2]=1.0;
    uv_arr[3]=0.0;
    uv_arr[4]=1.0;
    uv_arr[5]=1.0;
    uv_arr[6]=0.0;
    uv_arr[7]=1.0;
    
    glBindBuffer(GL_ARRAY_BUFFER,uv_buff);
    glBufferData(GL_ARRAY_BUFFER,
                 8*sizeof(GLfloat),
                 (void*)uv_arr,
                 GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,0);
    
    GLuint *index_arr=new GLuint[6];
    
    index_arr[0]=0;
    index_arr[1]=1;
    index_arr[2]=2;
    index_arr[3]=0;
    index_arr[4]=2;
    index_arr[5]=3;
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,index_buff);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 6*sizeof(GLuint),
                 (void*)index_arr,
                 GL_STATIC_DRAW);
    
    glGenTextures(1,&texture_rgb);
    glBindTexture(GL_TEXTURE_2D,texture_rgb);
    glTexStorage2D(GL_TEXTURE_2D,1,GL_RGB32F,N1,N2);
    
    GLfloat *map=new GLfloat[3*N1*N2];
    
    int i,j;
    
    for(i=0;i<N1;i++) for(j=0;j<N2;j++)
    {
        map[3*(i+N1*j)+0]=0.0;
        map[3*(i+N1*j)+1]=0.0;
        map[3*(i+N1*j)+2]=0.0;
    }
    
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,N1,N2,GL_RGB,GL_FLOAT,map);
    
    delete[] uv_arr;
    delete[] index_arr;
    delete[] map;
    
    // Programs 
    
    prog_planes=Glite::create_program(PathManager::locate_resource("resources/glsl/planemap_vshader.glsl"),
                                      PathManager::locate_resource("resources/glsl/planemap_fshader.glsl"));
    
    uni_proj_planes=glGetUniformLocation(prog_planes,"proj_mat");
    
    glGenSamplers(1,&sampler);
    glBindSampler(0,sampler);
    
    glSamplerParameteri(sampler,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glSamplerParameteri(sampler,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    
    glSamplerParameteri(sampler,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
}

void GL_2D_display::focus()
{
    SetCurrent(*glcnt);
}

void GL_2D_display::render()
{
    int Sx=GetSize().x;
    int Sy=GetSize().y;
    
    float x_factor=1.0,y_factor=1.0;
    
    if(static_cast<double>(Sy)/Sx<=static_cast<double>(N2)/N1) 
        x_factor=static_cast<float>(N1)/Sx/(static_cast<double>(N2)/Sy);
    else
        y_factor=static_cast<double>(N2)/Sy/(static_cast<double>(N1)/Sx);
    
    glUseProgram(prog_planes);
    
    GLfloat proj_gl[16];
    
    proj_gl[0 ]=x_factor;
    proj_gl[1 ]=0;
    proj_gl[2 ]=0;
    proj_gl[3 ]=0;
    
    proj_gl[4 ]=0;
    proj_gl[5 ]=y_factor;
    proj_gl[6 ]=0;
    proj_gl[7 ]=0;
    
    proj_gl[8 ]=0;
    proj_gl[9 ]=0;
    proj_gl[10]=1;
    proj_gl[11]=0;
    
    proj_gl[12]=0;
    proj_gl[13]=0;
    proj_gl[14]=0;
    proj_gl[15]=1;
    
    glUniformMatrix4fv(uni_proj_planes,1,0,proj_gl);
    
    glBindVertexArray(vao);
    
    glBindTexture(GL_TEXTURE_2D,texture_rgb);
    
    glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);
}

void GL_2D_display::reallocate(int N1_,int N2_)
{
    N1=N1_;
    N2=N2_;
    
    glDeleteTextures(1,&texture_rgb);
    glGenTextures(1,&texture_rgb);
    glBindTexture(GL_TEXTURE_2D,texture_rgb);
    
    glTexStorage2D(GL_TEXTURE_2D,1,GL_RGB32F,N1,N2);
}

void GL_2D_display::refresh_from_bitmap()
{
    if(gl_ok)
    {
        focus();
    //    std::unique_lock<std::mutex> lock(bitmap.get_mutex());
        
        int i,j;
        int Nx=bitmap.width;
        int Ny=bitmap.height;
        
        if(N1!=Nx || N2!=Ny)
        {
            if(map!=nullptr) delete[] map;
            map=new GLfloat[3*Nx*Ny];
        }
        
        for(i=0;i<Nx;i++)
        {
            for(j=0;j<Ny;j++)
            {
                map[3*(i+Nx*j)+0]=bitmap.M(i,Ny-1-j,2)/255.0;
                map[3*(i+Nx*j)+1]=bitmap.M(i,Ny-1-j,1)/255.0;
                map[3*(i+Nx*j)+2]=bitmap.M(i,Ny-1-j,0)/255.0;
            }
        }
        
        set_map(Nx,Ny,map);
        
        waiting_bitmap_refresh=false;
    }
    else waiting_bitmap_refresh=true;
}

void GL_2D_display::evt_resize(wxSizeEvent &event)
{
    if(gl_ok)
    {
        SetCurrent(*glcnt);
        
        int Sx=GetSize().x;
        int Sy=GetSize().y;
        
        glViewport(0, 0, (GLint)Sx, (GLint)Sy);
    }
}

void GL_2D_display::set_map(int N1_,int N2_,GLfloat *map)
{
    if(N1!=N1_ || N2!=N2_) reallocate(N1_,N2_);
    
    glBindTexture(GL_TEXTURE_2D,texture_rgb);
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,N1,N2,GL_RGB,GL_FLOAT,map);
}

void GL_2D_display::evt_timed_refresh(wxTimerEvent &event)
{
    if(!gl_ok)
    {
        if(IsShown())
        {
            init_opengl();
            gl_ok=true;
        }
    }
    else base_render();
}

void GL_2D_display::evt_paint_refresh(wxPaintEvent &event)
{
    if(gl_ok) base_render();
}

//################
//   GL_3D_Base
//################

GL_3D_Base::GL_3D_Base(wxWindow *parent)
    :wxGLCanvas(parent,wxID_ANY,NULL),
     busy(false),
     gl_ok(false),
     mouse_x(0), mouse_xp(0),
     mouse_y(0), mouse_yp(0),
     cam_r(2.0), cam_ph(0), cam_th(Pi/2.0),
     cam_t(0,0,0)
{
    update_camera();
    camera.clip(0.01,100);
    
    glcnt=new wxGLContext(this);
    SetCurrent(*glcnt);
    Show(true);
    
    timer=new wxTimer(this);
    
//    if(gl3wInit())
//        wxMessageBox("Error: Could not initialize OpenGL");
//    if(!gl3wIsSupported(4,3))
//        wxMessageBox("Error: OpenGL 4.3 not supported.\n Update your graphics drivers.");
//    
//    //
//    
//    Bind(wxEVT_SIZE,&GL_3D_Base::resize,this);
//    Bind(wxEVT_MOTION,&GL_3D_Base::evt_mouse_motion,this);
//    Bind(wxEVT_MOUSEWHEEL,&GL_3D_Base::evt_mouse_wheel,this);    
    Bind(wxEVT_TIMER,&GL_3D_Base::timed_refresh,this);
    timer->Start(1000/60);
    
//    Bind(wxEVT_CREATE,&GL_3D_Base::evt_shown,this);
    
//    glEnable(GL_DEPTH_TEST);
//    glDepthFunc(GL_LESS);
//    
//    glDisable(GL_CULL_FACE);
}

GL_3D_Base::~GL_3D_Base()
{
    timer->Stop();
    delete glcnt;
}

void GL_3D_Base::base_render()
{
    if(IsShown())
    {
        SetCurrent(*glcnt);
        if(!busy)
        {
            glClearColor(0.0, 0.0, 0.0, 0.0);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            
            camera.look_at(cam_t);
            
            render();
            
            SwapBuffers();
        }
    }
}

void GL_3D_Base::evt_mouse_motion(wxMouseEvent &event)
{
    mouse_xp=mouse_x;
    mouse_yp=mouse_y;
    
    mouse_x=event.GetX();
    mouse_y=event.GetY();
    
    if(!evt_mouse_motion_intercept(event))
    {
        if(event.Dragging())
        {
            if(event.LeftIsDown())
            {
                double dx=(mouse_x-mouse_xp)/(GetSize().x+0.0);
                double dy=(mouse_y-mouse_yp)/(GetSize().y+0.0);
                
                if(event.ShiftDown())
                {
                    cam_t+=cam_r*(-camera.Cu*dx+camera.Cv*dy);
                }
                else
                {
                    using std::sin;
                    using std::cos;
                    
                    double dph=2.0*dx*camera.h_ang;
                    double dth=2.0*dy*camera.v_ang;
                    
                    cam_ph-=dph;
                    cam_th-=dth;
                    
                    if(cam_ph<-Pi) cam_ph+=2.0*Pi;
                    if(cam_ph>+Pi) cam_ph-=2.0*Pi;
                    if(cam_th<Pi*0.0001) cam_th=Pi*0.0001;
                    if(cam_th>Pi*0.9999) cam_th=Pi*0.9999;
                }
                    
                update_camera();
            }
            else if(event.RightIsDown())
            {
                
            }
        }
    }
    
    event.Skip();
}

bool GL_3D_Base::evt_mouse_motion_intercept(wxMouseEvent &event)
{
    return false;
}

void GL_3D_Base::evt_mouse_wheel(wxMouseEvent &event)
{
    if(!evt_mouse_wheel_intercept(event))
    {
        if(event.GetWheelRotation()>0) cam_r/=1.05;
        else if(event.GetWheelRotation()<0) cam_r*=1.05;
        
        update_camera();
    }
}

bool GL_3D_Base::evt_mouse_wheel_intercept(wxMouseEvent &event)
{
    return false;
}

void GL_3D_Base::focus()
{
    SetCurrent(*glcnt);
}

void GL_3D_Base::init_opengl()
{
}

void GL_3D_Base::render()
{
}

void GL_3D_Base::update_viewport()
{
    int Sx=GetSize().x;
    int Sy=GetSize().y;
    
    double Cv_ang=90*Pi/180.0;
    double Ch_ang=2.0*std::atan(std::tan(Cv_ang/2.0)*(Sx+0.0)/(Sy+0.0));
    
    camera.update_proj_angles(Ch_ang,Cv_ang);
    
    glViewport(0, 0, (GLint)Sx, (GLint)Sy);
}

void GL_3D_Base::resize(wxSizeEvent &event)
{
    SetCurrent(*glcnt);
    
    update_viewport();
}

void GL_3D_Base::set_camera_direction(Vector3 const &dir_)
{
    Vector3 dir=-dir_;
    cam_th=std::acos(dir.z/dir.norm());
    cam_ph=std::atan2(dir.y,dir.x);
    
    update_camera();
}

void GL_3D_Base::set_camera_target(Vector3 const &target_)
{
    cam_t=target_;
    
    update_camera();
}

void GL_3D_Base::set_camera_radius(double radius)
{
    cam_r=radius;
    
    update_camera();
}

void GL_3D_Base::timed_refresh(wxTimerEvent &event)
{
    if(!gl_ok)
    {
        if(IsShown())
        {
            SetCurrent(*glcnt);
            
            if(gl3wInit())
                wxMessageBox("Error: Could not initialize OpenGL");
            if(!gl3wIsSupported(4,3))
                wxMessageBox("Error: OpenGL 4.3 not supported.\n Update your graphics drivers.");
            
            //
            
            Bind(wxEVT_SIZE,&GL_3D_Base::resize,this);
            Bind(wxEVT_MOTION,&GL_3D_Base::evt_mouse_motion,this);
            Bind(wxEVT_MOUSEWHEEL,&GL_3D_Base::evt_mouse_wheel,this);
            
            update_viewport();
            
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
            
            glDisable(GL_CULL_FACE);
                        
            init_opengl();
            
            gl_ok=true;
        }
    }
    else base_render();
}

void GL_3D_Base::paint_refresh(wxPaintEvent &event)
{
    base_render();
}

void GL_3D_Base::update_camera()
{
    camera.set_location(cam_t.x+cam_r*cos(cam_ph)*sin(cam_th),
                        cam_t.y+cam_r*sin(cam_ph)*sin(cam_th),
                        cam_t.z+cam_r*cos(cam_th));
}

//###############
//  Basic_VAO
//###############

Basic_VAO::Basic_VAO()
    :display(false), wireframe(false), ID(0),
     scale(1.0),
     color(1.0,1.0,1.0)
{
}

void Basic_VAO::draw()
{
    if(!display) return;
    
    if(!wireframe)
    {
        glBindVertexArray(vao);
        
        glVertexAttrib4f(2,color.x,color.y,color.z,1.0);
        glVertexAttrib4f(3,A.x,A.y,A.z,0);
        glVertexAttrib4f(4,B.x,B.y,B.z,0);
        glVertexAttrib4f(5,C.x,C.y,C.z,0);
        glVertexAttrib4f(6,O.x,O.y,O.z,1.0);
        
        glDrawElements(GL_TRIANGLES,3*Nf,GL_UNSIGNED_INT,0);
    }
}

void Basic_VAO::draw_wireframe()
{
    if(!display) return;
    
    if(wireframe)
    {
        glBindVertexArray(vao_w);
        
        glVertexAttrib4f(2,color.x,color.y,color.z,1.0);
        glVertexAttrib4f(3,A.x,A.y,A.z,0);
        glVertexAttrib4f(4,B.x,B.y,B.z,0);
        glVertexAttrib4f(5,C.x,C.y,C.z,0);
        glVertexAttrib4f(6,O.x,O.y,O.z,1.0);
        
        glDrawElements(GL_LINES,2*Ne,GL_UNSIGNED_INT,0);
    }
}

void Basic_VAO::init_opengl()
{
    glGenVertexArrays(1,&vao);
    glGenBuffers(1,&v_buff);
    glGenBuffers(1,&vn_buff);
    glGenBuffers(1,&index_buff);
    
    glGenVertexArrays(1,&vao_w);
    glGenBuffers(1,&v_buff_w);
    glGenBuffers(1,&vn_buff_w);
    glGenBuffers(1,&index_buff_w);
}

void Basic_VAO::rescale_vectors()
{
    O=scale*uO-Vector3(lx/2.0,ly/2.0,lz/2.0);
    A=scale*uA;
    B=scale*uB;
    C=scale*uC;
}

void Basic_VAO::set_matrix(Vector3 const &O_,
                           Vector3 const &A_,
                           Vector3 const &B_,
                           Vector3 const &C_)
{
    uO=O_;
    uA=A_;
    uB=B_;
    uC=C_;
    
    rescale_vectors();
}

void Basic_VAO::set_mesh(std::string mesh_type,int disc)
{
    set_mesh_solid(mesh_type,disc);
    set_mesh_wireframe(mesh_type,disc);
    
    display=true;
}

void Basic_VAO::set_mesh_solid(std::string mesh_type,int disc)
{
    int i;
    std::vector<Vertex> V_arr;
    std::vector<Face> F_arr;
    
         if(mesh_type=="block") Glite::make_unitary_block(V_arr,F_arr);
    else if(mesh_type=="cone") Glite::make_unitary_cone(V_arr,F_arr,disc);
    else if(mesh_type=="cylinder") Glite::make_unitary_cylinder(V_arr,F_arr,disc);
    else if(mesh_type=="sphere") Glite::make_unitary_csphere(V_arr,F_arr,disc);
    else return;
    
    int Nv=V_arr.size();
    Nf=F_arr.size();
    
    GLfloat *v_arr_buff=new GLfloat[4*Nv];
    GLfloat *vn_arr_buff=new GLfloat[4*Nv];
    GLuint *ind_arr_buff=new GLuint[3*Nf];
    
    for(i=0;i<Nv;i++)
    {
        v_arr_buff[4*i+0]=V_arr[i].loc.x;
        v_arr_buff[4*i+1]=V_arr[i].loc.y;
        v_arr_buff[4*i+2]=V_arr[i].loc.z;
        v_arr_buff[4*i+3]=1.0;
                
        vn_arr_buff[4*i+0]=V_arr[i].norm.x;
        vn_arr_buff[4*i+1]=V_arr[i].norm.y;
        vn_arr_buff[4*i+2]=V_arr[i].norm.z;
        vn_arr_buff[4*i+3]=0;
    }
    
    for(i=0;i<Nf;i++)
    {
        ind_arr_buff[3*i+0]=F_arr[i].V1;
        ind_arr_buff[3*i+1]=F_arr[i].V2;
        ind_arr_buff[3*i+2]=F_arr[i].V3;
    }
    
    // Solid VAO
    
    glBindVertexArray(vao);
    
    glBindBuffer(GL_ARRAY_BUFFER,v_buff);
    glBufferData(GL_ARRAY_BUFFER,
                 4*Nv*sizeof(GLfloat),
                 (void*)v_arr_buff,
                 GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,0);
    
    glBindBuffer(GL_ARRAY_BUFFER,vn_buff);
    glBufferData(GL_ARRAY_BUFFER,
                 4*Nv*sizeof(GLfloat),
                 (void*)vn_arr_buff,
                 GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,0,0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,index_buff);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 3*Nf*sizeof(GLuint),
                 (void*)ind_arr_buff,
                 GL_STATIC_DRAW);
    
    delete[] v_arr_buff;
    delete[] vn_arr_buff;
    delete[] ind_arr_buff;
}

void Basic_VAO::set_mesh_solid(std::vector<Vertex> const &V_arr,std::vector<Face> const &F_arr)
{
    int i;
    
    int Nv=V_arr.size();
    Nf=F_arr.size();
    
    GLfloat *v_arr_buff=new GLfloat[4*Nv];
    GLfloat *vn_arr_buff=new GLfloat[4*Nv];
    GLuint *ind_arr_buff=new GLuint[3*Nf];
    
    for(i=0;i<Nv;i++)
    {
        v_arr_buff[4*i+0]=V_arr[i].loc.x;
        v_arr_buff[4*i+1]=V_arr[i].loc.y;
        v_arr_buff[4*i+2]=V_arr[i].loc.z;
        v_arr_buff[4*i+3]=1.0;
                
        vn_arr_buff[4*i+0]=V_arr[i].norm.x;
        vn_arr_buff[4*i+1]=V_arr[i].norm.y;
        vn_arr_buff[4*i+2]=V_arr[i].norm.z;
        vn_arr_buff[4*i+3]=0;
    }
    
    for(i=0;i<Nf;i++)
    {
        ind_arr_buff[3*i+0]=F_arr[i].V1;
        ind_arr_buff[3*i+1]=F_arr[i].V2;
        ind_arr_buff[3*i+2]=F_arr[i].V3;
    }
    
    // Solid VAO
    
    glBindVertexArray(vao);
    
    glBindBuffer(GL_ARRAY_BUFFER,v_buff);
    glBufferData(GL_ARRAY_BUFFER,
                 4*Nv*sizeof(GLfloat),
                 (void*)v_arr_buff,
                 GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,0);
    
    glBindBuffer(GL_ARRAY_BUFFER,vn_buff);
    glBufferData(GL_ARRAY_BUFFER,
                 4*Nv*sizeof(GLfloat),
                 (void*)vn_arr_buff,
                 GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,0,0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,index_buff);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 3*Nf*sizeof(GLuint),
                 (void*)ind_arr_buff,
                 GL_STATIC_DRAW);
    
    delete[] v_arr_buff;
    delete[] vn_arr_buff;
    delete[] ind_arr_buff;
}

void Basic_VAO::set_mesh_wireframe(std::string mesh_type,int disc)
{
    int i;
    std::vector<Vertex> V_arr;
    std::vector<Face> F_arr;
    
         if(mesh_type=="block") Glite::make_unitary_block_wires(V_arr,F_arr);
    else if(mesh_type=="cone") Glite::make_unitary_cone_wires(V_arr,F_arr,disc);
    else if(mesh_type=="cylinder") Glite::make_unitary_cylinder_wires(V_arr,F_arr,disc);
    else if(mesh_type=="sphere") Glite::make_unitary_csphere_wires(V_arr,F_arr,disc);
    else return;
    
    int Nv=V_arr.size();
    Ne=F_arr.size();
    
    GLfloat *v_arr_buff=new GLfloat[4*Nv];
    GLfloat *vn_arr_buff=new GLfloat[4*Nv];
    GLuint *ind_arr_buff=new GLuint[2*Ne];
    
    for(i=0;i<Nv;i++)
    {
        v_arr_buff[4*i+0]=V_arr[i].loc.x;
        v_arr_buff[4*i+1]=V_arr[i].loc.y;
        v_arr_buff[4*i+2]=V_arr[i].loc.z;
        v_arr_buff[4*i+3]=1.0;
                
        vn_arr_buff[4*i+0]=V_arr[i].norm.x;
        vn_arr_buff[4*i+1]=V_arr[i].norm.y;
        vn_arr_buff[4*i+2]=V_arr[i].norm.z;
        vn_arr_buff[4*i+3]=0;
    }
    
    for(i=0;i<Ne;i++)
    {
        ind_arr_buff[2*i+0]=F_arr[i].V1;
        ind_arr_buff[2*i+1]=F_arr[i].V2;
    }
    
    // Wires VAO
    
    glBindVertexArray(vao_w);
    
    glBindBuffer(GL_ARRAY_BUFFER,v_buff_w);
    glBufferData(GL_ARRAY_BUFFER,
                 4*Nv*sizeof(GLfloat),
                 (void*)v_arr_buff,
                 GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,0);
    
    glBindBuffer(GL_ARRAY_BUFFER,vn_buff_w);
    glBufferData(GL_ARRAY_BUFFER,
                 4*Nv*sizeof(GLfloat),
                 (void*)vn_arr_buff,
                 GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,0,0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,index_buff_w);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 2*Ne*sizeof(GLuint),
                 (void*)ind_arr_buff,
                 GL_STATIC_DRAW);
    
    delete[] v_arr_buff;
    delete[] vn_arr_buff;
    delete[] ind_arr_buff;
}

void Basic_VAO::set_mesh_wireframe(std::vector<Vertex> const &V_arr,std::vector<Face> const &F_arr)
{
    int i;
    
    int Nv=V_arr.size();
    Ne=F_arr.size();
    
    GLfloat *v_arr_buff=new GLfloat[4*Nv];
    GLfloat *vn_arr_buff=new GLfloat[4*Nv];
    GLuint *ind_arr_buff=new GLuint[2*Ne];
    
    for(i=0;i<Nv;i++)
    {
        v_arr_buff[4*i+0]=V_arr[i].loc.x;
        v_arr_buff[4*i+1]=V_arr[i].loc.y;
        v_arr_buff[4*i+2]=V_arr[i].loc.z;
        v_arr_buff[4*i+3]=1.0;
                
        vn_arr_buff[4*i+0]=V_arr[i].norm.x;
        vn_arr_buff[4*i+1]=V_arr[i].norm.y;
        vn_arr_buff[4*i+2]=V_arr[i].norm.z;
        vn_arr_buff[4*i+3]=0;
    }
    
    for(i=0;i<Ne;i++)
    {
        ind_arr_buff[2*i+0]=F_arr[i].V1;
        ind_arr_buff[2*i+1]=F_arr[i].V2;
    }
    
    // Wires VAO
    
    glBindVertexArray(vao_w);
    
    glBindBuffer(GL_ARRAY_BUFFER,v_buff_w);
    glBufferData(GL_ARRAY_BUFFER,
                 4*Nv*sizeof(GLfloat),
                 (void*)v_arr_buff,
                 GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,0);
    
    glBindBuffer(GL_ARRAY_BUFFER,vn_buff_w);
    glBufferData(GL_ARRAY_BUFFER,
                 4*Nv*sizeof(GLfloat),
                 (void*)vn_arr_buff,
                 GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,0,0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,index_buff_w);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 2*Ne*sizeof(GLuint),
                 (void*)ind_arr_buff,
                 GL_STATIC_DRAW);
    
    delete[] v_arr_buff;
    delete[] vn_arr_buff;
    delete[] ind_arr_buff;
}

void Basic_VAO::set_shading(Vector3 const &color_,bool wireframe_,bool display_)
{
    color=color_;
    wireframe=wireframe_;
    display=display_;
}

void Basic_VAO::set_world(double lx_,double ly_,double lz_,double scale_)
{
    lx=lx_;
    ly=ly_;
    lz=lz_;
    scale=scale_;
    
    rescale_vectors();
}

//###############
//  GL_FD_Base
//###############

GL_FD_Base::GL_FD_Base(wxWindow *parent)
    :GL_3D_Base(parent),
     Nx(60), Ny(60), Nz(60),
     Dx(5e-9), Dy(5e-9), Dz(5e-9), scale(1.0/(60*5e-9)),
     lx(1.0), ly(1.0), lz(1.0),
     pml_x(false), pml_y(false), pml_z(false),
     pml_vao(6), pad_vao(6)
{
}

void GL_FD_Base::forget_all_vaos()
{
    for(unsigned int i=0;i<vao.size();i++) delete vao[i];
    vao.clear();
}

void GL_FD_Base::forget_vao(std::vector<Basic_VAO*> const &vao_)
{
    for(unsigned int i=0;i<vao_.size();i++) forget_vao(vao_[i]);
}

void GL_FD_Base::forget_vao(Basic_VAO *vao_)
{
    if(vao.size()==0) return;
    
    unsigned int i,j;
    
    for(i=0;i<vao.size();i++)
    {
        if(vao[i]==vao_)
        {
            delete vao[i];
            for(j=i+1;j<vao.size();j++) vao[j-1]=vao[j];
            vao.pop_back();
            return;
        }
    }
}

void GL_FD_Base::init_opengl()
{
    focus();
    
    gxd=new Glite::LineGrid_VAO;
    gxu=new Glite::LineGrid_VAO;
    gyd=new Glite::LineGrid_VAO;
    gyu=new Glite::LineGrid_VAO;
    gzd=new Glite::LineGrid_VAO;
    gzu=new Glite::LineGrid_VAO;
    
    update_grid(Nx,Ny,Nz,Dx,Dy,Dz);
    
    chk_var(pml_vao.size());
    for(int i=0;i<6;i++)
    {
        pml_vao[i].init_opengl();
        pml_vao[i].set_mesh_wireframe("block",0);
        
        pad_vao[i].init_opengl();
        pad_vao[i].set_mesh_wireframe("block",0);
        
        pml_vao[i].set_matrix(Vector3(0),
                              Vector3(0),
                              Vector3(0),
                              Vector3(0));
        
        pml_vao[i].set_shading(Vector3(1.0,0.5,0),true,true);
        
        pad_vao[i].set_matrix(Vector3(0),
                              Vector3(0),
                              Vector3(0),
                              Vector3(0));
        
        pad_vao[i].set_shading(Vector3(0.5,0.5,0.5),true,true);
    }
    
    prog_grid=Glite::create_program(PathManager::locate_resource("resources/glsl/grid_vshader.glsl"),
                                    PathManager::locate_resource("resources/glsl/grid_fshader.glsl"));
    
    prog_solid=Glite::create_program(PathManager::locate_resource("resources/glsl/FD_vshader.glsl"),
                                     PathManager::locate_resource("resources/glsl/FD_solid_fshader.glsl"));
    
    prog_wires=Glite::create_program(PathManager::locate_resource("resources/glsl/FD_vshader.glsl"),
                                     PathManager::locate_resource("resources/glsl/FD_wires_fshader.glsl"));
    
//    glBindAttribLocation(prog_solid,3,"transf_matrix");
    
    uni_proj_grid=glGetUniformLocation(prog_grid,"proj_mat");
}

void GL_FD_Base::recompute_pml_matrices()
{
    double px1=0,px2=0,px3=lx/scale,px4=lx/scale;
    double py1=0,py2=0,py3=ly/scale,py4=ly/scale;
    double pz1=0,pz2=0,pz3=lz/scale,pz4=lz/scale;
    
    if(pml_xm>0 || pml_xp>0)
    {
        px1=-(pml_xm+pad_xm)*Dx;
        px2=-pad_xm*Dx;
        
        px3=lx/scale+pad_xp*Dx;
        px4=lx/scale+(pml_xp+pad_xp)*Dx;
    }
    
    if(pml_ym>0 || pml_yp>0)
    {
        py1=-(pml_ym+pad_ym)*Dy;
        py2=-pad_ym*Dy;
        
        py3=ly/scale+pad_yp*Dy;
        py4=ly/scale+(pml_yp+pad_yp)*Dy;
    }
    
    if(pml_zm>0 || pml_zp>0)
    {
        pz1=-(pml_zm+pad_zm)*Dz;
        pz2=-pad_zm*Dz;
        
        pz3=lz/scale+pad_zp*Dz;
        pz4=lz/scale+(pml_zp+pad_zp)*Dz;
    }
    
//    chk_var(px1); chk_var(px2); chk_var(px3); chk_var(px4);
//    chk_var(py1); chk_var(py2); chk_var(py3); chk_var(py4);
//    chk_var(pz1); chk_var(pz2); chk_var(pz3); chk_var(pz4);
    
    pml_vao[0].set_matrix(Vector3(px1,py1,pz1),Vector3(px2-px1,0,0),Vector3(0,py4-py1,0),Vector3(0,0,pz4-pz1));
    pml_vao[1].set_matrix(Vector3(px3,py1,pz1),Vector3(px4-px3,0,0),Vector3(0,py4-py1,0),Vector3(0,0,pz4-pz1));
    
    pml_vao[2].set_matrix(Vector3(px1,py1,pz1),Vector3(px4-px1,0,0),Vector3(0,py2-py1,0),Vector3(0,0,pz4-pz1));
    pml_vao[3].set_matrix(Vector3(px1,py3,pz1),Vector3(px4-px1,0,0),Vector3(0,py4-py3,0),Vector3(0,0,pz4-pz1));
    
    pml_vao[4].set_matrix(Vector3(px1,py1,pz1),Vector3(px4-px1,0,0),Vector3(0,py4-py1,0),Vector3(0,0,pz2-pz1));
    pml_vao[5].set_matrix(Vector3(px1,py1,pz3),Vector3(px4-px1,0,0),Vector3(0,py4-py1,0),Vector3(0,0,pz4-pz3));
    
    pad_vao[0].set_matrix(Vector3(px2,py1,pz1),Vector3(0-px2,0,0),Vector3(0,py4-py1,0),Vector3(0,0,pz4-pz1));
    pad_vao[1].set_matrix(Vector3(lx/scale,py1,pz1),Vector3(px3-lx/scale,0,0),Vector3(0,py4-py1,0),Vector3(0,0,pz4-pz1));
    
    pad_vao[2].set_matrix(Vector3(px1,py1,pz1),Vector3(px4-px1,0,0),Vector3(0,py2-py1,0),Vector3(0,0,pz4-pz1));
    pad_vao[3].set_matrix(Vector3(px1,py3,pz1),Vector3(px4-px1,0,0),Vector3(0,py4-py3,0),Vector3(0,0,pz4-pz1));
//    
    pad_vao[4].set_matrix(Vector3(px1,py1,pz2),Vector3(px4-px1,0,0),Vector3(0,py4-py1,0),Vector3(0,0,0-pz2));
    pad_vao[5].set_matrix(Vector3(px1,py1,lz/scale),Vector3(px4-px1,0,0),Vector3(0,py4-py1,0),Vector3(0,0,pz3-lz/scale));
}

void GL_FD_Base::render()
{
    glUseProgram(prog_grid);
    glUniformMatrix4fv(uni_proj_grid,1,0,camera.proj_gl);
    
    if(camera.Co.x>-0.5*lx) gxd->draw();
    if(camera.Co.x<+0.5*lx) gxu->draw();
    if(camera.Co.y>-0.5*ly) gyd->draw();
    if(camera.Co.y<+0.5*ly) gyu->draw();
    if(camera.Co.z>-0.5*lz) gzd->draw();
    if(camera.Co.z<+0.5*lz) gzu->draw();
    
    glUseProgram(prog_wires);
    glUniformMatrix4fv(10,1,0,camera.proj_gl);
    
    bool draw_check[6];
    draw_check[0]=pml_x && (pml_xm!=0 || pml_xp!=0);
    draw_check[1]=pml_x && (pml_xm!=0 || pml_xp!=0);
    draw_check[2]=pml_y && (pml_ym!=0 || pml_yp!=0);
    draw_check[3]=pml_y && (pml_ym!=0 || pml_yp!=0);
    draw_check[4]=pml_z && (pml_zm!=0 || pml_zp!=0);
    draw_check[5]=pml_z && (pml_zm!=0 || pml_zp!=0);
    
    for(int i=0;i<6;i++)
        if(draw_check[i]) pad_vao[i].draw_wireframe();
        
    for(int i=0;i<6;i++)
        if(draw_check[i]) pml_vao[i].draw_wireframe();
    
    glUseProgram(prog_solid);
    
    Vector3 sun_dir(1.0,1.0,1.0);
//    sun_dir.normalize();
    sun_dir.set_spherical(1.0,Pi/4.0,-Pi/3.0);
    
    glUniformMatrix4fv(10,1,0,camera.proj_gl);
    glUniform4f(11,sun_dir.x,
                   sun_dir.y,
                   sun_dir.z,
                   0);
    
    for(unsigned int i=0;i<vao.size();i++) vao[i]->draw();
    
    glUseProgram(prog_wires);
    
    glLineWidth(3.0);
    
    glUniformMatrix4fv(10,1,0,camera.proj_gl);
    for(unsigned int i=0;i<vao.size();i++) vao[i]->draw_wireframe();
    
    glLineWidth(1.0);
}

Basic_VAO* GL_FD_Base::request_vao()
{
    focus();
    
    Basic_VAO *new_vao=new Basic_VAO;
    
    new_vao->set_world(lx,ly,lz,scale);
    new_vao->init_opengl();
    
    vao.push_back(new_vao);
    
    return new_vao;
}

void GL_FD_Base::reset_pml_display()
{
    if(pml_xm!=0 || pml_xp!=0) pml_x=true;
    if(pml_ym!=0 || pml_yp!=0) pml_y=true;
    if(pml_zm!=0 || pml_zp!=0) pml_z=true;
}

void GL_FD_Base::set_pml(int pml_xm_,int pml_xp_,int pml_ym_,int pml_yp_,int pml_zm_,int pml_zp_,
                         int pad_xm_,int pad_xp_,int pad_ym_,int pad_yp_,int pad_zm_,int pad_zp_,
                         bool reset_display)
{
    pml_xm=pml_xm_; pml_xp=pml_xp_;
    pml_ym=pml_ym_; pml_yp=pml_yp_;
    pml_zm=pml_zm_; pml_zp=pml_zp_;
    
    pad_xm=pad_xm_; pad_xp=pad_xp_;
    pad_ym=pad_ym_; pad_yp=pad_yp_;
    pad_zm=pad_zm_; pad_zp=pad_zp_;
    
    if(reset_display) reset_pml_display();
    
    recompute_pml_matrices();
}

void GL_FD_Base::set_structure(Grid3<unsigned int> const &matsgrid,std::vector<Basic_VAO*> &disc_vao)
{
    forget_vao(disc_vao);
    disc_vao.clear();
    
    unsigned int i,N=matsgrid.max();
    
    std::vector<Vertex> V_arr;
    std::vector<Face> F_arr;
    
    for(i=0;i<=N;i++)
    {
        voxelize(V_arr,F_arr,matsgrid,i);
        
        if(F_arr.size()>0)
        {
            Basic_VAO *tmp_vao=request_vao();
            disc_vao.push_back(tmp_vao);
            
            tmp_vao->ID=i;
            tmp_vao->set_mesh_solid(V_arr,F_arr);
            
            voxelize_wireframe(V_arr,F_arr,matsgrid,i);
            tmp_vao->set_mesh_wireframe(V_arr,F_arr);
            
            tmp_vao->set_matrix(Vector3(0,0,0),
                                Vector3(lx/scale,0,0),
                                Vector3(0,ly/scale,0),
                                Vector3(0,0,lz/scale));
            
            double r=0,g=0,b=0;
            degra(i,N,r,g,b);
            
            tmp_vao->set_shading(Vector3(r,g,b),false,false);
            if(i>0) tmp_vao->display=true;
        }
    }
}

void GL_FD_Base::update_grid(int Nx_,int Ny_,int Nz_,
                             double Dx_,double Dy_,double Dz_)
{
    SetCurrent(*glcnt);
    
    Nx=Nx_;
    Ny=Ny_;
    Nz=Nz_;
    
    Dx=Dx_;
    Dy=Dy_;
    Dz=Dz_;
    
    chk_var(Nx);
    chk_var(Ny);
    chk_var(Nz);
    
    chk_var(Dx);
    chk_var(Dy);
    chk_var(Dz);
    
    
    
    lx=Nx*Dx;
    ly=Ny*Dy;
    lz=Nz*Dz;
    
    double l_max=var_max(lx,ly,lz);
    scale=1.0/l_max;
    
    lx*=scale;
    ly*=scale;
    lz*=scale;
    
    gxd->set_grid(Ny,Nz,Vector3(-0.5*lx,0,0),Vector3(0,ly,0),Vector3(0,0,lz));
    gxu->set_grid(Ny,Nz,Vector3(+0.5*lx,0,0),Vector3(0,ly,0),Vector3(0,0,lz));
    gyd->set_grid(Nx,Nz,Vector3(0,-0.5*ly,0),Vector3(lx,0,0),Vector3(0,0,lz));
    gyu->set_grid(Nx,Nz,Vector3(0,+0.5*ly,0),Vector3(lx,0,0),Vector3(0,0,lz));
    gzd->set_grid(Nx,Ny,Vector3(0,0,-0.5*lz),Vector3(lx,0,0),Vector3(0,ly,0));
    gzu->set_grid(Nx,Ny,Vector3(0,0,+0.5*lz),Vector3(lx,0,0),Vector3(0,ly,0));
    
    for(unsigned int i=0;i<6;i++) pml_vao[i].set_world(lx,ly,lz,scale);
    for(unsigned int i=0;i<6;i++) pad_vao[i].set_world(lx,ly,lz,scale);
    for(unsigned int i=0;i<vao.size();i++) vao[i]->set_world(lx,ly,lz,scale);
    
    recompute_pml_matrices();
}
