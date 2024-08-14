/*Copyright 2008-2024 - Loïc Le Cunff

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#include <gl_utils.h>
#include <logger.h>

extern std::ofstream plog;

namespace Glite
{
    Camera::Camera()
        :clip_min(0.1), clip_max(100.0)
    {
    }
    
    void Camera::clip(double clip_min_i,double clip_max_i)
    {
        clip_min=clip_min_i;
        clip_max=clip_max_i;
    }
    
    void Camera::look_at(Vector3 const &V)
    {
        Vector3 up(0,0,1.0);
        
        Cw=V-Co;
        Cw.normalize();
        
        Cu.crossprod(Cw,up);
        Cu.normalize();
        
        Cv.crossprod(Cu,Cw);
        Cv.normalize();
        
        update_proj_mat();
    }
    
    void Camera::set_location(double x,double y,double z)
    {
        Co(x,y,z);
        
        update_loc_mat();
    }
    
    void Camera::set_location(Vector3 const &V)
    {
        Co=V;
        
        update_loc_mat();
    }
    
    void Camera::update_loc_mat()
    {
        C_dsp(0,0)=1.0; C_dsp(0,1)=0;   C_dsp(0,2)=0;   C_dsp(0,3)=-Co.x;
        C_dsp(1,0)=0;   C_dsp(1,1)=1.0; C_dsp(1,2)=0;   C_dsp(1,3)=-Co.y;
        C_dsp(2,0)=0;   C_dsp(2,1)=0;   C_dsp(2,2)=1.0; C_dsp(2,3)=-Co.z;
        C_dsp(3,0)=0;   C_dsp(3,1)=0;   C_dsp(3,2)=0;   C_dsp(3,3)=1.0;
    }
    
    void Camera::update_proj_angles(double Ch_ang,double Cv_ang)
    {
        h_ang=Ch_ang;
        v_ang=Cv_ang;
        
        update_proj_mat();
    }
    
    void Camera::update_proj_mat()
    {
        double np=clip_min,fp=clip_max;
        double tan_h=std::tan(h_ang/2.0);
        double tan_v=std::tan(v_ang/2.0);
        
        rot_mat(0,0)=Cu.x; rot_mat(0,1)=Cv.x; rot_mat(0,2)=Cw.x; rot_mat(0,3)=0;
        rot_mat(1,0)=Cu.y; rot_mat(1,1)=Cv.y; rot_mat(1,2)=Cw.y; rot_mat(1,3)=0;
        rot_mat(2,0)=Cu.z; rot_mat(2,1)=Cv.z; rot_mat(2,2)=Cw.z; rot_mat(2,3)=0;
        rot_mat(3,0)=0;    rot_mat(3,1)=0;    rot_mat(3,2)=0;    rot_mat(3,3)=1.0;
        
        proj_mat(0,0)=1.0/tan_h; proj_mat(0,1)=0;
        proj_mat(1,0)=0;         proj_mat(1,1)=1.0/tan_v;
        proj_mat(2,0)=0;         proj_mat(2,1)=0;
        proj_mat(3,0)=0;         proj_mat(3,1)=0;
        
        proj_mat(0,2)=0;               proj_mat(0,3)=0;
        proj_mat(1,2)=0;               proj_mat(1,3)=0;
        proj_mat(2,2)=(np+fp)/(fp-np); proj_mat(2,3)=-2.0*np*fp/(fp-np);
        proj_mat(3,2)=1.0;             proj_mat(3,3)=0;
        
        proj_mat=proj_mat*(rot_mat.inverse()*C_dsp);
        
        proj_gl[0]=proj_mat(0,0);
        proj_gl[1]=proj_mat(1,0);
        proj_gl[2]=proj_mat(2,0);
        proj_gl[3]=proj_mat(3,0);
        proj_gl[4]=proj_mat(0,1);
        proj_gl[5]=proj_mat(1,1);
        proj_gl[6]=proj_mat(2,1);
        proj_gl[7]=proj_mat(3,1);
        proj_gl[8]=proj_mat(0,2);
        proj_gl[9]=proj_mat(1,2);
        proj_gl[10]=proj_mat(2,2);
        proj_gl[11]=proj_mat(3,2);
        proj_gl[12]=proj_mat(0,3);
        proj_gl[13]=proj_mat(1,3);
        proj_gl[14]=proj_mat(2,3);
        proj_gl[15]=proj_mat(3,3);
    }
    
    GLuint create_program(std::filesystem::path const &v_fname,
                          std::filesystem::path const &f_fname)
    {
        GLuint v_shad,f_shad;
        
        v_shad=glCreateShader(GL_VERTEX_SHADER);
        f_shad=glCreateShader(GL_FRAGMENT_SHADER);
        
        Glite::load_shader_file(v_shad,v_fname);
        Glite::load_shader_file(f_shad,f_fname);
        
        glCompileShader(v_shad);
        glCompileShader(f_shad);
        
        Glite::check_shader_compilation(v_shad);
        Glite::check_shader_compilation(f_shad);
        
        GLuint prog=glCreateProgram();
        glAttachShader(prog,v_shad);
        glAttachShader(prog,f_shad);
        glLinkProgram(prog);
        
        glDetachShader(prog,v_shad);
        glDetachShader(prog,f_shad);
        
        glDeleteShader(v_shad);
        glDeleteShader(f_shad);
        
        return prog;
    }
    
    void check_shader_compilation(GLuint shader)
    {
        GLint shad_comp;
        glGetShaderiv(shader,GL_COMPILE_STATUS,&shad_comp);
        
        if(shad_comp==GL_FALSE)
        {
            Plog::print("Compilation error\n");
            
            GLint shad_log_l;
            glGetShaderiv(shader,GL_INFO_LOG_LENGTH,&shad_log_l);
            
            GLchar *log=new GLchar[shad_log_l];
            
            glGetShaderInfoLog(shader,shad_log_l*2,&shad_log_l,log);
            
            for(int i=0;i<shad_log_l;i++)
                Plog::print(char(log[i]));
            
            delete[] log;
            
            Plog::print("Press enter to continue\n");
            std::cin.get();
        }
    }
    
    void load_shader_file(GLuint shader,std::filesystem::path const &fname)
    {
        unsigned int i,j;
        
        std::ifstream file(fname,std::ios::in|std::ios::binary);
        
        std::string fcont;
        std::getline(file,fcont,'\0');
        
        file.close();
        
        int L=fcont.size()+1;
        
        for(i=0;i<fcont.size();i++)
            if(fcont[i]=='\r') L--;
        
        GLchar **fcont_C=new char*[1];
        fcont_C[0]=new char[L];
        
        j=0;
        for(i=0;i<fcont.size();i++)
        {
            if(fcont[i]!='\r')
            {
                fcont_C[0][j]=fcont[i];
                j++;
            }
        }
        
        fcont_C[0][j]='\0';
        
        glShaderSource(shader,1,fcont_C,NULL);
        
        delete[] fcont_C[0];
        delete[] fcont_C;
    }
    
    void make_block(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,
                    double lx,double ly,double lz,
                    double off_x,double off_y,double off_z)
    {
        V_arr.resize(24);
        F_arr.resize(12);
        
        double x1=off_x*lx;
        double x2=x1+lx;
        
        double y1=off_y*ly;
        double y2=y1+ly;
        
        double z1=off_z*lz;
        double z2=z1+lz;
        
        V_arr[0].loc(x1,y1,z1); V_arr[0].norm(0,-1,0);
        V_arr[1].loc(x2,y1,z1); V_arr[1].norm(0,-1,0);
        V_arr[2].loc(x2,y1,z2); V_arr[2].norm(0,-1,0);
        V_arr[3].loc(x1,y1,z2); V_arr[3].norm(0,-1,0);
        
        F_arr[0].V1=0; F_arr[0].V2=1; F_arr[0].V3=2;
        F_arr[1].V1=0; F_arr[1].V2=2; F_arr[1].V3=3;
        
        V_arr[4].loc(x2,y1,z1); V_arr[4].norm(1,0,0);
        V_arr[5].loc(x2,y2,z1); V_arr[5].norm(1,0,0);
        V_arr[6].loc(x2,y1,z2); V_arr[6].norm(1,0,0);
        V_arr[7].loc(x2,y2,z2); V_arr[7].norm(1,0,0);
        
        F_arr[2].V1=4; F_arr[2].V2=5; F_arr[2].V3=6;
        F_arr[3].V1=6; F_arr[3].V2=5; F_arr[3].V3=7;
        
        V_arr[ 8].loc(x1,y2,z1); V_arr[ 8].norm(0,1,0);
        V_arr[ 9].loc(x2,y2,z1); V_arr[ 9].norm(0,1,0);
        V_arr[10].loc(x2,y2,z2); V_arr[10].norm(0,1,0);
        V_arr[11].loc(x1,y2,z2); V_arr[11].norm(0,1,0);
        
        F_arr[4].V1=8; F_arr[4].V2=9; F_arr[4].V3=10;
        F_arr[5].V1=8; F_arr[5].V2=10; F_arr[5].V3=11;
        
        V_arr[12].loc(x1,y1,z1); V_arr[12].norm(-1,0,0);
        V_arr[13].loc(x1,y2,z1); V_arr[13].norm(-1,0,0);
        V_arr[14].loc(x1,y1,z2); V_arr[14].norm(-1,0,0);
        V_arr[15].loc(x1,y2,z2); V_arr[15].norm(-1,0,0);
        
        F_arr[6].V1=12; F_arr[6].V2=13; F_arr[6].V3=14;
        F_arr[7].V1=14; F_arr[7].V2=13; F_arr[7].V3=15;
        
        V_arr[16].loc(x1,y1,z1); V_arr[16].norm(0,0,-1);
        V_arr[17].loc(z2,y1,z1); V_arr[17].norm(0,0,-1);
        V_arr[18].loc(x2,y2,z1); V_arr[18].norm(0,0,-1);
        V_arr[19].loc(x1,y2,z1); V_arr[19].norm(0,0,-1);
        
        F_arr[8].V1=16; F_arr[8].V2=17; F_arr[8].V3=18;
        F_arr[9].V1=16; F_arr[9].V2=18; F_arr[9].V3=19;
        
        V_arr[20].loc(x1,y1,z2); V_arr[20].norm(0,0,1);
        V_arr[21].loc(x2,y1,z2); V_arr[21].norm(0,0,1);
        V_arr[22].loc(x2,y2,z2); V_arr[22].norm(0,0,1);
        V_arr[23].loc(x1,y2,z2); V_arr[23].norm(0,0,1);
        
        F_arr[10].V1=20; F_arr[10].V2=21; F_arr[10].V3=22;
        F_arr[11].V1=20; F_arr[11].V2=22; F_arr[11].V3=23;
    }
    
    void make_block_wires(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,
                          double lx,double ly,double lz,
                          double off_x,double off_y,double off_z)
    {
        V_arr.resize(8);
        F_arr.resize(12);
        
        double x1=off_x*lx;
        double x2=x1+lx;
        
        double y1=off_y*ly;
        double y2=y1+ly;
        
        double z1=off_z*lz;
        double z2=z1+lz;
        
        double sr=std::sqrt(1.0/3.0);
        
        V_arr[0].loc(x1,y1,z1); V_arr[0].norm(-sr,-sr,-sr);
        V_arr[1].loc(x2,y1,z1); V_arr[1].norm(+sr,-sr,-sr);
        V_arr[2].loc(x2,y2,z1); V_arr[2].norm(+sr,+sr,-sr);
        V_arr[3].loc(x1,y2,z1); V_arr[3].norm(-sr,+sr,-sr);
        V_arr[4].loc(x1,y1,z2); V_arr[4].norm(-sr,-sr,+sr);
        V_arr[5].loc(x2,y1,z2); V_arr[5].norm(+sr,-sr,+sr);
        V_arr[6].loc(x2,y2,z2); V_arr[6].norm(+sr,+sr,+sr);
        V_arr[7].loc(x1,y2,z2); V_arr[7].norm(-sr,+sr,+sr);
        
        F_arr[0].V1=0; F_arr[0].V2=1; F_arr[0].V3=1;
        F_arr[1].V1=1; F_arr[1].V2=2; F_arr[1].V3=2;
        F_arr[2].V1=2; F_arr[2].V2=3; F_arr[2].V3=3;
        F_arr[3].V1=3; F_arr[3].V2=0; F_arr[3].V3=0;
        
        F_arr[4].V1=0; F_arr[4].V2=4; F_arr[4].V3=4;
        F_arr[5].V1=1; F_arr[5].V2=5; F_arr[5].V3=5;
        F_arr[6].V1=2; F_arr[6].V2=6; F_arr[6].V3=6;
        F_arr[7].V1=3; F_arr[7].V2=7; F_arr[7].V3=7;
        
        F_arr[ 8].V1=4; F_arr[ 8].V2=5; F_arr[ 8].V3=5;
        F_arr[ 9].V1=5; F_arr[ 9].V2=6; F_arr[ 9].V3=6;
        F_arr[10].V1=6; F_arr[10].V2=7; F_arr[10].V3=7;
        F_arr[11].V1=7; F_arr[11].V2=4; F_arr[11].V3=4;
    }
    
    void make_unitary_block(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr)
    {
        V_arr.resize(24);
        F_arr.resize(12);
        
        V_arr[0].loc(0,0,0); V_arr[0].norm(0,-1,0);
        V_arr[1].loc(1,0,0); V_arr[1].norm(0,-1,0);
        V_arr[2].loc(1,0,1); V_arr[2].norm(0,-1,0);
        V_arr[3].loc(0,0,1); V_arr[3].norm(0,-1,0);
        
        F_arr[0].V1=0; F_arr[0].V2=1; F_arr[0].V3=2;
        F_arr[1].V1=0; F_arr[1].V2=2; F_arr[1].V3=3;
        
        V_arr[4].loc(1,0,0); V_arr[4].norm(1,0,0);
        V_arr[5].loc(1,1,0); V_arr[5].norm(1,0,0);
        V_arr[6].loc(1,0,1); V_arr[6].norm(1,0,0);
        V_arr[7].loc(1,1,1); V_arr[7].norm(1,0,0);
        
        F_arr[2].V1=4; F_arr[2].V2=5; F_arr[2].V3=6;
        F_arr[3].V1=6; F_arr[3].V2=5; F_arr[3].V3=7;
        
        V_arr[ 8].loc(0,1,0); V_arr[ 8].norm(0,1,0);
        V_arr[ 9].loc(1,1,0); V_arr[ 9].norm(0,1,0);
        V_arr[10].loc(1,1,1); V_arr[10].norm(0,1,0);
        V_arr[11].loc(0,1,1); V_arr[11].norm(0,1,0);
        
        F_arr[4].V1=8; F_arr[4].V2=9; F_arr[4].V3=10;
        F_arr[5].V1=8; F_arr[5].V2=10; F_arr[5].V3=11;
        
        V_arr[12].loc(0,0,0); V_arr[12].norm(-1,0,0);
        V_arr[13].loc(0,1,0); V_arr[13].norm(-1,0,0);
        V_arr[14].loc(0,0,1); V_arr[14].norm(-1,0,0);
        V_arr[15].loc(0,1,1); V_arr[15].norm(-1,0,0);
        
        F_arr[6].V1=12; F_arr[6].V2=13; F_arr[6].V3=14;
        F_arr[7].V1=14; F_arr[7].V2=13; F_arr[7].V3=15;
        
        V_arr[16].loc(0,0,0); V_arr[16].norm(0,0,-1);
        V_arr[17].loc(1,0,0); V_arr[17].norm(0,0,-1);
        V_arr[18].loc(1,1,0); V_arr[18].norm(0,0,-1);
        V_arr[19].loc(0,1,0); V_arr[19].norm(0,0,-1);
        
        F_arr[8].V1=16; F_arr[8].V2=17; F_arr[8].V3=18;
        F_arr[9].V1=16; F_arr[9].V2=18; F_arr[9].V3=19;
        
        V_arr[20].loc(0,0,1); V_arr[20].norm(0,0,1);
        V_arr[21].loc(1,0,1); V_arr[21].norm(0,0,1);
        V_arr[22].loc(1,1,1); V_arr[22].norm(0,0,1);
        V_arr[23].loc(0,1,1); V_arr[23].norm(0,0,1);
        
        F_arr[10].V1=20; F_arr[10].V2=21; F_arr[10].V3=22;
        F_arr[11].V1=20; F_arr[11].V2=22; F_arr[11].V3=23;
    }
    
    void make_unitary_block_wires(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr)
    {
        V_arr.resize(8);
        F_arr.resize(12);
        
        double sr=std::sqrt(1.0/3.0);
        
        V_arr[0].loc(0,0,0); V_arr[0].norm(-sr,-sr,-sr);
        V_arr[1].loc(1,0,0); V_arr[1].norm(+sr,-sr,-sr);
        V_arr[2].loc(1,1,0); V_arr[2].norm(+sr,+sr,-sr);
        V_arr[3].loc(0,1,0); V_arr[3].norm(-sr,+sr,-sr);
        V_arr[4].loc(0,0,1); V_arr[4].norm(-sr,-sr,+sr);
        V_arr[5].loc(1,0,1); V_arr[5].norm(+sr,-sr,+sr);
        V_arr[6].loc(1,1,1); V_arr[6].norm(+sr,+sr,+sr);
        V_arr[7].loc(0,1,1); V_arr[7].norm(-sr,+sr,+sr);
        
        F_arr[0].V1=0; F_arr[0].V2=1;
        F_arr[1].V1=1; F_arr[1].V2=2;
        F_arr[2].V1=2; F_arr[2].V2=3;
        F_arr[3].V1=3; F_arr[3].V2=0;
        
        F_arr[4].V1=0; F_arr[4].V2=4;
        F_arr[5].V1=1; F_arr[5].V2=5;
        F_arr[6].V1=2; F_arr[6].V2=6;
        F_arr[7].V1=3; F_arr[7].V2=7;
        
        F_arr[ 8].V1=4; F_arr[ 8].V2=5;
        F_arr[ 9].V1=5; F_arr[ 9].V2=6;
        F_arr[10].V1=6; F_arr[10].V2=7;
        F_arr[11].V1=7; F_arr[11].V2=4;
    }
    
    void make_unitary_cone(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,int disc)
    {
        int i,v_offset,f_offset;
        
        V_arr.resize(3*disc+1);
        F_arr.resize(3*disc);
        
        // Bottom
        
        V_arr[0].loc=Vector3(0,0,0);
        V_arr[0].norm=Vector3(0,0,-1.0);
        
        v_offset=1;
        
        for(i=0;i<disc;i++)
        {
            double ang=i*(2.0*Pi/disc);
            
            V_arr[i+v_offset].loc=Vector3(std::cos(ang),std::sin(ang),0);
            V_arr[i+v_offset].norm=Vector3(0,0,-1.0);
        }
        
        for(i=0;i<disc;i++)
        {
            F_arr[i].V1=0;
            F_arr[i].V2=1+i;
            F_arr[i].V3=2+i;
        }
        
        F_arr[disc-1].V3=1;
        
        // Cone
        
        v_offset=1+disc;
        f_offset=disc;
        
        for(i=0;i<disc;i++)
        {
            double ang=i*(2.0*Pi/disc);
            double ang_th=Pi/4.0;
            
            Vector3 tmp_norm(std::cos(ang)*std::sin(ang_th),
                             std::sin(ang)*std::sin(ang_th),
                             std::cos(ang_th));
            
            V_arr[i+v_offset].loc=Vector3(std::cos(ang),std::sin(ang),0);
            V_arr[i+v_offset].norm=tmp_norm;
            
            V_arr[i+v_offset+disc].loc=Vector3(0,0,1.0);
            V_arr[i+v_offset+disc].norm=tmp_norm;
        }
        
        for(i=0;i<disc;i++)
        {
            F_arr[i+f_offset].V1=v_offset+i;
            F_arr[i+f_offset].V2=v_offset+i+1;
            F_arr[i+f_offset].V3=v_offset+i+disc;
            
            F_arr[i+f_offset+disc].V1=v_offset+i+1;
            F_arr[i+f_offset+disc].V2=v_offset+i+disc+1;
            F_arr[i+f_offset+disc].V3=v_offset+i+disc;
        }
        
        F_arr[f_offset+disc-1].V2=v_offset;
        
        F_arr[f_offset+2*disc-1].V1=v_offset;
        F_arr[f_offset+2*disc-1].V2=v_offset+disc+1;
    }
    
    void make_unitary_cone_wires(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,int disc)
    {
        int i;
        
        V_arr.resize(2+disc);
        F_arr.resize(3*disc);
        
        // Ring
        
        for(i=0;i<disc;i++)
        {
            double ang=i*(2.0*Pi/disc);
            
            V_arr[i].loc(std::cos(ang),std::sin(ang),0);
            V_arr[i].norm=V_arr[i].loc-Vector3(0,0,0.5);
            V_arr[i].norm.normalize();
        }
        
        // Poles
        
        V_arr[disc  ].loc(0,0,0); V_arr[disc  ].norm(0,0,-1);
        V_arr[disc+1].loc(0,0,1); V_arr[disc+1].norm(0,0,1);
        
        // Faces
        
        for(i=0;i<disc;i++)
        {
            F_arr[i+0*disc].V1=i; F_arr[i+0*disc].V2=i+1;
            F_arr[i+1*disc].V1=i; F_arr[i+1*disc].V2=disc;
            F_arr[i+2*disc].V1=i; F_arr[i+2*disc].V2=disc+1;
        }
        
        F_arr[disc-1].V2=0;
    }
    
    
    CSphere_Indexer::CSphere_Indexer(int N_)
        :N(N_), Nm(N-1)
    {
        Da=Pi/2.0/N;
        
        int c=N*N;
        
        offset_1=(N+1)*(N+1);
        offset_2=offset_1+c;
        offset_3=offset_2+c;
        offset_4=offset_3+c;
        offset_5=offset_4+c;
    }
    
    int CSphere_Indexer::get_total() { return offset_5+(N-1)*(N-1); }
    
    int CSphere_Indexer::index_0(int i,int j) { return i+j*(N+1); }
    
    int CSphere_Indexer::index_1(int i,int j)
    {
        if(i==N)
        {
            if(j==N) return index_0(N,0);
            else return index_2(0,j);
        }
        else if(j==N) return index_0(i,0);
        else return offset_1+i+j*N;
    }
    
    int CSphere_Indexer::index_2(int i,int j)
    {
        if(i==N)
        {
            if(j==N) return index_0(N,N);
            else return index_3(0,j);
        }
        else if(j==N) return index_0(N,i);
        else return offset_2+i+j*N;
    }
    
    int CSphere_Indexer::index_3(int i,int j)
    {
        if(i==N)
        {
            if(j==N) return index_0(0,N);
            else return index_4(0,j);
        }
        else if(j==N) return index_0(N-i,N);
        else return offset_3+i+j*N;
    }
    
    int CSphere_Indexer::index_4(int i,int j)
    {
        if(i==N)
        {
            if(j==N) return index_0(0,0);
            else return index_1(0,j);
        }
        else if(j==N) return index_0(0,N-i);
        else return offset_4+i+j*N;
    }
    
    int CSphere_Indexer::index_5(int i,int j)
    {
             if(i==0) return index_2(N-j,0);
        else if(i==N) return index_4(j,0);
        else if(j==0) return index_3(i,0);
        else if(j==N) return index_1(N-i,0);
        else return offset_5+i-1+(j-1)*Nm;
    }
    
    Vector3 CSphere_Indexer::vertex_location(int i,int j,int f)
    {
        Vector3 out;
        
        double am=-Pi/4.0+i*Da;
        double bm=-Pi/4.0+j*Da;
        
        Vector3 V1(-std::sin(am),std::cos(am),0),
                V2(-std::sin(bm),0,std::cos(bm)),
                V3;
        
        V3.crossprod(V1,V2);
        V3.normalize();
        
        double x=V3.x;
        double y=V3.y;
        double z=V3.z;
        
             if(f==0) out(-z,y,x);
        else if(f==1) out(x,y,z);
        else if(f==2) out(-y,x,z);
        else if(f==3) out(-x,-y,z);
        else if(f==4) out(y,-x,z);
        else if(f==5) out(z,-y,-x);
        
        return out;
    }
    
    void make_unitary_csphere(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,int disc)
    {
        int i,j;
        
        CSphere_Indexer ind(disc);
        
        int Nv=ind.get_total();
        int Nf=6*2*disc*disc;
        
        V_arr.resize(Nv);
        F_arr.resize(Nf);
        
        for(i=0;i<=disc;i++) 
            for(j=0;j<=disc;j++) V_arr[ind.index_0(i,j)].loc=ind.vertex_location(i,j,0);
        for(i=0;i<=disc;i++) 
            for(j=0;j<=disc;j++) V_arr[ind.index_1(i,j)].loc=ind.vertex_location(i,j,1);
        for(i=0;i<=disc;i++) 
            for(j=0;j<=disc;j++) V_arr[ind.index_2(i,j)].loc=ind.vertex_location(i,j,2);
        for(i=0;i<=disc;i++) 
            for(j=0;j<=disc;j++) V_arr[ind.index_3(i,j)].loc=ind.vertex_location(i,j,3);
        for(i=0;i<=disc;i++) 
            for(j=0;j<=disc;j++) V_arr[ind.index_4(i,j)].loc=ind.vertex_location(i,j,4);
        for(i=0;i<=disc;i++) 
            for(j=0;j<=disc;j++) V_arr[ind.index_5(i,j)].loc=ind.vertex_location(i,j,5);
        
        for(i=0;i<Nv;i++) V_arr[i].norm=V_arr[i].loc;
        
        for(int f=0;f<6;f++)
        {
            int (CSphere_Indexer::*t_ind)(int,int);
            
                 if(f==0) t_ind=&CSphere_Indexer::index_0;
            else if(f==1) t_ind=&CSphere_Indexer::index_1;
            else if(f==2) t_ind=&CSphere_Indexer::index_2;
            else if(f==3) t_ind=&CSphere_Indexer::index_3;
            else if(f==4) t_ind=&CSphere_Indexer::index_4;
            else if(f==5) t_ind=&CSphere_Indexer::index_5;
            
            int f_offset=2*f*disc*disc;
            
            for(i=0;i<disc;i++) for(j=0;j<disc;j++)
            {
                Face &F1=F_arr[2*i+0+2*j*disc+f_offset];
                Face &F2=F_arr[2*i+1+2*j*disc+f_offset]; 
                
                F1.V1=(ind.*t_ind)(i,j);
                F1.V2=(ind.*t_ind)(i+1,j);
                F1.V3=(ind.*t_ind)(i,j+1);
                
                F2.V1=(ind.*t_ind)(i+1,j);
                F2.V2=(ind.*t_ind)(i+1,j+1);
                F2.V3=(ind.*t_ind)(i,j+1);
            }
        }
    }
    
    void make_unitary_csphere_wires(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,int disc)
    {
        int i,j;
        
        CSphere_Indexer ind(disc);
        
        int Nv=ind.get_total();
        int Ne=12*disc*disc;
        
        V_arr.resize(Nv);
        F_arr.resize(Ne);
        
        for(i=0;i<=disc;i++) 
            for(j=0;j<=disc;j++) V_arr[ind.index_0(i,j)].loc=ind.vertex_location(i,j,0);
        for(i=0;i<=disc;i++) 
            for(j=0;j<=disc;j++) V_arr[ind.index_1(i,j)].loc=ind.vertex_location(i,j,1);
        for(i=0;i<=disc;i++) 
            for(j=0;j<=disc;j++) V_arr[ind.index_2(i,j)].loc=ind.vertex_location(i,j,2);
        for(i=0;i<=disc;i++) 
            for(j=0;j<=disc;j++) V_arr[ind.index_3(i,j)].loc=ind.vertex_location(i,j,3);
        for(i=0;i<=disc;i++) 
            for(j=0;j<=disc;j++) V_arr[ind.index_4(i,j)].loc=ind.vertex_location(i,j,4);
        for(i=0;i<=disc;i++) 
            for(j=0;j<=disc;j++) V_arr[ind.index_5(i,j)].loc=ind.vertex_location(i,j,5);
        
        for(i=0;i<Nv;i++) V_arr[i].norm=V_arr[i].loc;
        
        for(i=0;i<disc;i++) for(j=0;j<disc;j++)
        {
            Face &F1=F_arr[2*i+0+2*j*disc];
            Face &F2=F_arr[2*i+1+2*j*disc];
            
            F1.V1=ind.index_0(i,j);
            F1.V2=ind.index_0(i+1,j);
            
            F2.V1=ind.index_0(i,j);
            F2.V2=ind.index_0(i,j+1);
        }
        
        int f_offset=2*disc*disc;
        
        for(i=0;i<disc;i++)
        {
            Face &F1=F_arr[i+f_offset];
            
            F1.V1=ind.index_0(i,disc);
            F1.V2=ind.index_0(i+1,disc);
        }
        
        f_offset+=disc;
        
        for(j=0;j<disc;j++)
        {
            Face &F1=F_arr[j+f_offset];
            
            F1.V1=ind.index_0(disc,j);
            F1.V2=ind.index_0(disc,j+1);
        }
        
        f_offset=2*disc*(disc+1);
        
        for(int f=1;f<5;f++)
        {
            int (CSphere_Indexer::*t_ind)(int,int);
            
                 if(f==1) t_ind=&CSphere_Indexer::index_1;
            else if(f==2) t_ind=&CSphere_Indexer::index_2;
            else if(f==3) t_ind=&CSphere_Indexer::index_3;
            else if(f==4) t_ind=&CSphere_Indexer::index_4;
                        
            for(i=0;i<disc;i++) for(j=0;j<disc;j++)
            {
                Face &F1=F_arr[2*i+0+2*j*disc+f_offset];
                Face &F2=F_arr[2*i+1+2*j*disc+f_offset]; 
                
                F1.V1=(ind.*t_ind)(i,j);
                F1.V2=(ind.*t_ind)(i+1,j);
                
                F2.V1=(ind.*t_ind)(i,j);
                F2.V2=(ind.*t_ind)(i,j+1);
            }
            
            f_offset+=2*disc*disc;
        }
        
        for(i=1;i<disc;i++)
        {
            for(j=0;j<disc;j++)
            {
                Face &F1=F_arr[j+f_offset];
                
                F1.V1=ind.index_5(i,j);
                F1.V2=ind.index_5(i,j+1);
            }
            
            f_offset+=disc;
        }
        
        for(j=1;j<disc;j++)
        {
            for(i=0;i<disc;i++)
            {
                Face &F1=F_arr[i+f_offset];
                
                F1.V1=ind.index_5(i,j);
                F1.V2=ind.index_5(i+1,j);
            }
            
            f_offset+=disc;
        }
    }
    
    void make_unitary_cylinder(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,int disc)
    {
        int i,v_offset,f_offset;
        
        V_arr.resize(4*disc+2);
        F_arr.resize(4*disc);
        
        // Bottom
        
        V_arr[0].loc=Vector3(0,0,0);
        V_arr[0].norm=Vector3(0,0,-1.0);
        
        v_offset=1;
        
        for(i=0;i<disc;i++)
        {
            double ang=i*(2.0*Pi/disc);
            
            V_arr[i+v_offset].loc=Vector3(std::cos(ang),std::sin(ang),0);
            V_arr[i+v_offset].norm=Vector3(0,0,-1.0);
        }
        
        for(i=0;i<disc;i++)
        {
            F_arr[i].V1=0;
            F_arr[i].V2=1+i;
            F_arr[i].V3=2+i;
        }
        
        F_arr[disc-1].V3=1;
        
        // Side
        
        v_offset=1+disc;
        f_offset=disc;
        
        for(i=0;i<disc;i++)
        {
            double ang=i*(2.0*Pi/disc);
            double ang_th=Pi/4.0;
            
            Vector3 tmp_norm(std::cos(ang)*std::sin(ang_th),
                             std::sin(ang)*std::sin(ang_th),
                             std::cos(ang_th));
            
            V_arr[i+v_offset].loc(std::cos(ang),std::sin(ang),0);
            V_arr[i+v_offset].norm=tmp_norm;
            
            V_arr[i+v_offset+disc].loc(std::cos(ang),std::sin(ang),1.0);
            V_arr[i+v_offset+disc].norm=tmp_norm;
        }
        
        for(i=0;i<disc;i++)
        {
            F_arr[i+f_offset].V1=v_offset+i;
            F_arr[i+f_offset].V2=v_offset+i+1;
            F_arr[i+f_offset].V3=v_offset+i+disc;
            
            F_arr[i+f_offset+disc].V1=v_offset+i+1;
            F_arr[i+f_offset+disc].V2=v_offset+i+disc+1;
            F_arr[i+f_offset+disc].V3=v_offset+i+disc;
        }
        
        F_arr[f_offset+disc-1].V2=v_offset;
        
        F_arr[f_offset+2*disc-1].V1=v_offset;
        F_arr[f_offset+2*disc-1].V2=v_offset+disc+1;
        
        // Top
        
        v_offset=v_offset+2*disc;
        f_offset=f_offset+2*disc;
        
        V_arr[v_offset].loc=Vector3(0,0,1.0);
        V_arr[v_offset].norm=Vector3(0,0,1.0);
        
        v_offset+=1;
        
        for(i=0;i<disc;i++)
        {
            double ang=i*(2.0*Pi/disc);
            
            V_arr[i+v_offset].loc=Vector3(std::cos(ang),std::sin(ang),1.0);
            V_arr[i+v_offset].norm=Vector3(0,0,1.0);
        }
        
        for(i=0;i<disc;i++)
        {
            F_arr[i+f_offset].V1=v_offset-1;
            F_arr[i+f_offset].V2=v_offset+i;
            F_arr[i+f_offset].V3=v_offset+i+1;
        }
        
        F_arr[f_offset+disc-1].V3=v_offset;
    }
    
    void make_unitary_cylinder_wires(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,int disc)
    {
        int i;
        
        V_arr.resize(2+2*disc);
        F_arr.resize(5*disc);
        
        // Bottom Ring
        
        for(i=0;i<disc;i++)
        {
            double ang=i*(2.0*Pi/disc);
            
            V_arr[i].loc(std::cos(ang),std::sin(ang),0);
            V_arr[i].norm=V_arr[i].loc-Vector3(0,0,0.5);
            V_arr[i].norm.normalize();
        }
        
        // Top Ring
        
        for(i=0;i<disc;i++)
        {
            double ang=i*(2.0*Pi/disc);
            
            V_arr[i+disc].loc(std::cos(ang),std::sin(ang),1.0);
            V_arr[i+disc].norm=V_arr[i+disc].loc-Vector3(0,0,0.5);
            V_arr[i+disc].norm.normalize();
        }
        
        // Poles
        
        V_arr[2*disc  ].loc(0,0,0); V_arr[2*disc  ].norm(0,0,-1);
        V_arr[2*disc+1].loc(0,0,1); V_arr[2*disc+1].norm(0,0,1);
        
        // Faces
        
        for(i=0;i<disc;i++)
        {
            F_arr[i+0*disc].V1=i;      F_arr[i+0*disc].V2=i+1;
            F_arr[i+1*disc].V1=i+disc; F_arr[i+1*disc].V2=i+1+disc;
            F_arr[i+2*disc].V1=i;      F_arr[i+2*disc].V2=i+disc;
            F_arr[i+3*disc].V1=i;      F_arr[i+3*disc].V2=2*disc;
            F_arr[i+4*disc].V1=i+disc; F_arr[i+4*disc].V2=2*disc+1;
        }
        
        F_arr[disc-1].V2=0;
        F_arr[2*disc-1].V2=disc;
    }
    
    void make_unitary_disk(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr,int disc)
    {
        int i,v_offset;
        
        V_arr.resize(disc+1);
        F_arr.resize(disc);
        
        // Bottom
        
        V_arr[0].loc=Vector3(0,0,0);
        V_arr[0].norm=Vector3(0,0,1.0);
        
        v_offset=1;
        
        for(i=0;i<disc;i++)
        {
            double ang=i*(2.0*Pi/disc);
            
            V_arr[i+v_offset].loc=Vector3(std::cos(ang),std::sin(ang),0);
            V_arr[i+v_offset].norm=Vector3(0,0,1.0);
        }
        
        for(i=0;i<disc;i++)
        {
            F_arr[i].V1=0;
            F_arr[i].V2=1+i;
            F_arr[i].V3=2+i;
        }
        
        F_arr[disc-1].V3=1;
    }
    
    void make_unitary_square(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr)
    {
        V_arr.resize(4);
        F_arr.resize(2);
        
        V_arr[0].loc(0,0,0); V_arr[0].norm(0,0,1);
        V_arr[1].loc(1,0,0); V_arr[1].norm(0,0,1);
        V_arr[2].loc(1,1,0); V_arr[2].norm(0,0,1);
        V_arr[3].loc(0,1,0); V_arr[3].norm(0,0,1);
        
        F_arr[0].V1=0; F_arr[0].V2=1; F_arr[0].V3=2;
        F_arr[1].V1=0; F_arr[1].V2=2; F_arr[1].V3=3;
    }
    
    void make_unitary_square_wires(std::vector<Vertex> &V_arr,std::vector<Face> &F_arr)
    {
        V_arr.resize(4);
        F_arr.resize(4);
        
        V_arr[0].loc(0,0,0); V_arr[0].norm(0,0,1);
        V_arr[1].loc(1,0,0); V_arr[1].norm(0,0,1);
        V_arr[2].loc(1,1,0); V_arr[2].norm(0,0,1);
        V_arr[3].loc(0,1,0); V_arr[3].norm(0,0,1);
        
        F_arr[0].V1=0; F_arr[0].V2=1;
        F_arr[1].V1=1; F_arr[1].V2=2;
        F_arr[2].V1=2; F_arr[2].V2=3;
        F_arr[3].V1=3; F_arr[3].V2=0;
    }
    
    void set_vector(GLfloat *glarray,int i,double x,double y,double z,double w)
    {
        glarray[i+0]=x; glarray[i+1]=y; glarray[i+2]=z; glarray[i+3]=w;
    }
}
