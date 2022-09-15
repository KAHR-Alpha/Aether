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

#include <gui_gl_graph.h>

//###################
//   GL_2D_display
//###################

GL_2D_graph::GL_2D_graph(wxWindow *parent)
    :wxGLCanvas(parent,wxID_ANY,NULL),
     data_min(0), data_max(0),
     gradient(0),
     busy(false), gl_ok(false), show_colorbar(false),
     N1(100), N2(100), map(nullptr)
{
    glcnt=new wxGLContext(this);
    Show(true);
    
    timer=new wxTimer(this);
    Bind(wxEVT_TIMER,&GL_2D_graph::evt_initialization_check,this);
    Bind(wxEVT_SIZE,&GL_2D_graph::evt_resize,this);
    timer->Start(1000/60);
}

GL_2D_graph::~GL_2D_graph()
{
    timer->Stop();
    delete glcnt;
}

void GL_2D_graph::base_render()
{
    if(IsShown())
    {
        SetCurrent(*glcnt);
        if(!busy)
        {
            int Sx=GetSize().x;
            int Sy=GetSize().y;
            
            glViewport(0, 0, (GLint)Sx, (GLint)Sy);
        
            glClearColor(0.0, 0.0, 0.0, 0.0);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            
            render();
            
            SwapBuffers();
        }
    }
}

void GL_2D_graph::enable_colorbar(bool value)
{
    show_colorbar=value;
}

void GL_2D_graph::evt_initialization_check(wxTimerEvent &event)
{
    if(!gl_ok)
    {
        if(IsShown())
        {
            init_opengl();
            gl_ok=true;
        }
    }
    else
    {
        timer->Stop();
        base_render();
    }
}

void GL_2D_graph::evt_paint_refresh(wxPaintEvent &event)
{
    if(gl_ok) base_render();
}

void GL_2D_graph::evt_resize(wxSizeEvent &event)
{
    if(gl_ok)
    {
        SetCurrent(*glcnt);
        
        int Sx=GetSize().x;
        int Sy=GetSize().y;
        
        glViewport(0, 0, (GLint)Sx, (GLint)Sy);
    }
}

void GL_2D_graph::focus()
{
    SetCurrent(*glcnt);
}

void GL_2D_graph::init_opengl()
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
    
    // Map Vao
    
    glGenVertexArrays(1,&vao);
    glBindVertexArray(vao);
    
    glGenBuffers(1,&vertex_buff);
    glGenBuffers(1,&uv_buff);
    glGenBuffers(1,&index_buff);
    
    v_arr=new GLfloat[16];
    
    Glite::set_vector(v_arr,0 ,0,0,0);
    Glite::set_vector(v_arr,4 ,1,0,0);
    Glite::set_vector(v_arr,8 ,1,1,0);
    Glite::set_vector(v_arr,12,0,1,0);
    
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
    
    // Programs 
    
    prog_planes=Glite::create_program(PathManager::locate_resource("resources/glsl/graph/data_vshader.glsl"),
                                      PathManager::locate_resource("resources/glsl/graph/data_fshader.glsl"));
    
    uni_proj_planes=glGetUniformLocation(prog_planes,"proj_mat");
    
    glGenSamplers(1,&sampler);
    glBindSampler(0,sampler);
    
    glSamplerParameteri(sampler,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glSamplerParameteri(sampler,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    
    glSamplerParameteri(sampler,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    
    // Colorbar initialization
    
    glGenVertexArrays(1,&vao_colorbar);
    glBindVertexArray(vao_colorbar);
    
    GLuint vbuff_colorbar,
           ibuff_colorbar;
    glGenBuffers(1,&vbuff_colorbar);
    glGenBuffers(1,&ibuff_colorbar);
    
    GLfloat *colorbar_varr=new GLfloat[16];
    
    Glite::set_vector(colorbar_varr,0 ,0,0,0);
    Glite::set_vector(colorbar_varr,4 ,1,0,0);
    Glite::set_vector(colorbar_varr,8 ,1,1,0);
    Glite::set_vector(colorbar_varr,12,0,1,0);
    
    glBindBuffer(GL_ARRAY_BUFFER,vbuff_colorbar);
    glBufferData(GL_ARRAY_BUFFER,
                 16*sizeof(GLfloat),
                 (void*)colorbar_varr,
                 GL_STATIC_DRAW);
                 
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ibuff_colorbar);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 6*sizeof(GLuint),
                 (void*)index_arr,
                 GL_STATIC_DRAW);
    
    program_colorbar=Glite::create_program(PathManager::locate_resource("resources/glsl/graph/colorbar_vshader.glsl"),
                                           PathManager::locate_resource("resources/glsl/graph/colorbar_fshader.glsl"));
    
    gl_text.initialize();
    
    delete[] colorbar_varr;
    delete[] index_arr;
    delete[] map;
    delete[] uv_arr;
}

std::string trailing_zeros_rmv(std::string const &str)
{
    std::string::size_type P=str.find('.');
    
    if(P!=std::string::npos)
    {
        std::string::size_type i=0;
        
        bool only_zeros=true;
        for(i=P+1;i<str.size();i++)
        {
            if(str[i]>'0' && str[i]<='9') only_zeros=false;
            if(str[i]<'0' || str[i]>'9') break;
        }
        
        if(only_zeros)
        {
            if(i==0 || i==str.size()) return str.substr(0,P);
            else return str.substr(0,P)+str.substr(i);
        }
        else return str;
    }
    else return str;
}

void GL_2D_graph::render()
{
    show_colorbar=true;
    
    int Sx=GetSize().x;
    int Sy=GetSize().y;
    
    gl_text.set_screen(Sx,Sy);
    
    float x_factor=1.0,y_factor=1.0;
    
    if(static_cast<double>(Sy)/Sx<=static_cast<double>(N2)/N1) 
        x_factor=static_cast<float>(N1)/Sx/(static_cast<double>(N2)/Sy);
    else
        y_factor=static_cast<double>(N2)/Sy/(static_cast<double>(N1)/Sx);
    
    int border_left=100;
    int border_right=50;
    int border_bottom=50;
    int border_top=50;
    
    if(show_colorbar) border_right=300;
    
    double x_shift=-1+2.0*border_left/(Sx+0.0);
    double y_shift=-1+2.0*border_bottom/(Sy+0.0);
    x_factor=2.0*(Sx-border_left-border_right)/(Sx+0.0);
    y_factor=2.0*(Sy-border_bottom-border_top)/(Sy+0.0);
    
    glUseProgram(prog_planes);
    
    GLfloat proj_gl[16];
    
    Glite::set_vector(proj_gl, 0,x_factor,0,0,0);
    Glite::set_vector(proj_gl, 4,0,y_factor,0,0);
    Glite::set_vector(proj_gl, 8,0,0,1,0);
    Glite::set_vector(proj_gl,12,x_shift,y_shift,0,1);
        
    glUniform1i(11,gradient);
    glUniformMatrix4fv(uni_proj_planes,1,0,proj_gl);
    
    glBindVertexArray(vao);
    glBindTexture(GL_TEXTURE_2D,texture_rgb);
    glBindSampler(0,sampler);
    
    glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);
    
    // Colorbar
    
    if(show_colorbar)
    {
        x_factor=2*50.0/Sx;
        x_shift=-1+2.0*(Sx-250.0)/Sx;
        y_factor=2.0*(Sy-2.0*border_bottom)/(Sy+0.0);
    
        Glite::set_vector(proj_gl, 0,x_factor,0,0,0);
        Glite::set_vector(proj_gl, 4,0,y_factor,0,0);
        Glite::set_vector(proj_gl, 8,0,0,1,0);
        Glite::set_vector(proj_gl,12,x_shift,y_shift,0,1);
        
        glUseProgram(program_colorbar);
        
        glUniform1i(11,gradient);
        glUniformMatrix4fv(10,1,0,proj_gl);
        
        glBindVertexArray(vao_colorbar);
        glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);
                
        double z_span=data_max-data_min;
        std::stringstream label;
        
        if(z_span>0)
        {
            int span_mag=std::floor(std::log10(z_span));
            double delta=std::pow(10,span_mag);
            
            if((z_span-delta)/delta<1.0)
            {
                span_mag--;
                delta=std::pow(10,span_mag);
            }
            
            double val=data_min;
            
            while(val<data_max)
            {
                double u=(val-data_min)/(data_max-data_min);
                double y_loc=border_bottom+u*(Sy-border_top-border_bottom);
                
                label.str("");
                label<<val;
                
                if(y_loc<Sy-border_top-30)
                    gl_text.request_text(Sx-175,y_loc,-0.1,label.str(),18,18,0,0);
                
                val+=delta;
            }
        }
        
        label.str("");
        label<<data_max;
                
        gl_text.request_text(Sx-175,Sy-border_top,-0.1,label.str(),18,18,0,0);
        
        gl_text.render();
    }
}

void GL_2D_graph::reallocate(int N1_,int N2_)
{
    N1=N1_;
    N2=N2_;
    
    glDeleteTextures(1,&texture_rgb);
    glGenTextures(1,&texture_rgb);
    glBindTexture(GL_TEXTURE_2D,texture_rgb);
    
    glTexStorage2D(GL_TEXTURE_2D,1,GL_RGB32F,N1,N2);
}

void GL_2D_graph::refresh_from_bitmap()
{
    if(gl_ok)
    {
        focus();
    //    std::unique_lock<std::mutex> lock(bitmap.get_mutex());
        
        int i,j;
        int Nx=data.L1();
        int Ny=data.L2();
        
        if(N1!=Nx || N2!=Ny)
        {
            if(map!=nullptr) delete[] map;
            map=new GLfloat[3*Nx*Ny];
        }
        
        data_min=std::min(0.0,data.min());
        data_max=data.max();
        
        for(i=0;i<Nx;i++)
        {
            for(j=0;j<Ny;j++)
            {
                double val=(data(i,Ny-1-j)-data_min)/(data_max-data_min);
                
                map[3*(i+Nx*j)+0]=val;
                map[3*(i+Nx*j)+1]=val;
                map[3*(i+Nx*j)+2]=val;
            }
        }
        
        set_map(Nx,Ny,map);
        
        base_render();
    }
}

void GL_2D_graph::set_gradient(int gradient_)
{
    gradient=gradient_;
}

void GL_2D_graph::set_map(int N1_,int N2_,GLfloat *map)
{
    if(N1!=N1_ || N2!=N2_) reallocate(N1_,N2_);
    
    glBindTexture(GL_TEXTURE_2D,texture_rgb);
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,N1,N2,GL_RGB,GL_FLOAT,map);
}
