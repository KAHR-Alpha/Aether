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

#include <gl_utils.h>

extern std::ofstream plog;
extern const double Pi;

namespace Glite
{
    LineGrid_VAO::LineGrid_VAO()
        :Nu(1), Nv(1), NVtx(8),
         P(0,0,0), U(1,0,0), V(0,1,0)
    {
        glGenVertexArrays(1,&vao);
        glGenBuffers(1,&v_buff);
    }
    
    void LineGrid_VAO::draw()
    {
        glBindVertexArray(vao);
        
        glVertexAttrib4f(1,P.x,P.y,P.z,0.0);
        
        glDrawArrays(GL_LINES,0,NVtx);
    }
    
    void LineGrid_VAO::set_grid(int Nu_,int Nv_,Vector3 const &P_,
                                                Vector3 const &U_,
                                                Vector3 const &V_)
    {
        int i;
        
        Nu=Nu_;
        Nv=Nv_;
        
        P=P_;
        U=U_;
        V=V_;
        
        glBindVertexArray(vao);
        
        int Nl1=Nu+1;
        int Nl2=Nv+1;
        
        int Nv1=2*(Nu+1);
        int Nv2=2*(Nv+1);
        NVtx=Nv1+Nv2;
        
        GLfloat *v_arr=new GLfloat[4*NVtx];
        
        for(i=0;i<Nl1;i++)
        {
            Vector3 Vd=-0.5*V;
            Vector3 Vu=+0.5*V;
            Vector3 Ut=(i/(Nl1-1.0)-0.5)*U;
            
            Vector3 Td=Vd+Ut;
            Vector3 Tu=Vu+Ut;
            
            v_arr[8*i+0]=Td.x;
            v_arr[8*i+1]=Td.y;
            v_arr[8*i+2]=Td.z;
            v_arr[8*i+3]=1.0;
            
            v_arr[8*i+4]=Tu.x;
            v_arr[8*i+5]=Tu.y;
            v_arr[8*i+6]=Tu.z;
            v_arr[8*i+7]=1.0;
        }
        
        for(i=0;i<Nl2;i++)
        {
            Vector3 Ud=-0.5*U;
            Vector3 Uu=+0.5*U;
            Vector3 Vt=(i/(Nl2-1.0)-0.5)*V;
            
            Vector3 Td=Ud+Vt;
            Vector3 Tu=Uu+Vt;
            
            v_arr[8*(i+Nl1)+0]=Td.x;
            v_arr[8*(i+Nl1)+1]=Td.y;
            v_arr[8*(i+Nl1)+2]=Td.z;
            v_arr[8*(i+Nl1)+3]=1.0;
            
            v_arr[8*(i+Nl1)+4]=Tu.x;
            v_arr[8*(i+Nl1)+5]=Tu.y;
            v_arr[8*(i+Nl1)+6]=Tu.z;
            v_arr[8*(i+Nl1)+7]=1.0;
        }
        
        glBindBuffer(GL_ARRAY_BUFFER,v_buff);
        
        glBufferData(GL_ARRAY_BUFFER,
                     4*NVtx*sizeof(GLfloat),
                     (void*)v_arr,
                     GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,0);
        
        delete[] v_arr;
    }
    
    void LineGrid_VAO::set_location(Vector3 const &P_i)
    {
        P=P_i;
    }
    
    //#################
    //      VAO
    //#################
    
    VAO::VAO()
        :x(0), y(0), z(0)
    {
        glGenVertexArrays(1,&vao);
        glGenBuffers(1,&v_buff);
        glGenBuffers(1,&vn_buff);
        glGenBuffers(1,&index_buff);
    }

    void VAO::draw()
    {
        glVertexAttrib4f(2,x,y,z,0);
        
        glBindVertexArray(vao);
        
        glDrawElements(GL_TRIANGLES,3*Nf,GL_UNSIGNED_INT,0);
    }

    void VAO::set_mesh(Grid1<Vertex> &V_arr_,Grid1<Face> &F_arr_)
    {
        int i;
        
        Nv=V_arr_.L1();
        Nf=F_arr_.L1();
        
        V_arr.init(Nv);
        F_arr.init(Nf);
        
        V_arr=V_arr_;
        F_arr=F_arr_;
        
        GLfloat *v_arr_buff=new GLfloat[4*Nv];
        GLfloat *vn_arr_buff=new GLfloat[4*Nv];
        GLuint *ind_arr_buff=new GLuint[3*(Nf)];
        
        for(i=0;i<Nv;i++)
        {
            v_arr_buff[4*i+0]=V_arr[i].loc.x;
            v_arr_buff[4*i+1]=V_arr[i].loc.y;
            v_arr_buff[4*i+2]=V_arr[i].loc.z;
            v_arr_buff[4*i+3]=1.0;
            
            double loc_norm=V_arr[i].loc.norm();
            
            vn_arr_buff[4*i+0]=V_arr[i].loc.x/loc_norm;
            vn_arr_buff[4*i+1]=V_arr[i].loc.y/loc_norm;
            vn_arr_buff[4*i+2]=V_arr[i].loc.z/loc_norm;
            vn_arr_buff[4*i+3]=0;
        }
        
        for(i=0;i<Nf;i++)
        {
            ind_arr_buff[3*i+0]=F_arr[i].V1;
            ind_arr_buff[3*i+1]=F_arr[i].V2;
            ind_arr_buff[3*i+2]=F_arr[i].V3;
        }
        
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

    void VAO::set_location(double x_,double y_,double z_)
    {
        x=x_;
        y=y_;
        z=z_;
    }
    
    void VAO::set_location(Vector3 const &V)
    {
        x=V.x;
        y=V.y;
        z=V.z;
    }
}
