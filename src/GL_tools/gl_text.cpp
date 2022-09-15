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

#include <bitmap3.h>
#include <filehdl.h>
#include <gl_utils.h>

namespace Glite
{
    Text::Text()
        :N_inst(0), N_max(1024),
         ID_buffer(N_max)
    {
        FT_Init_FreeType(&library);
        if(FT_New_Face(library,"C:/Windows/Fonts/Tahoma.ttf",0,&face))
            FT_New_Face(library,PathManager::locate_resource("resources/Inconsolata-Regular.ttf").generic_string().c_str(),0,&face);
        
        world_pos=new GLfloat[4*N_max];
        screen_offset=new GLfloat[2*N_max];
        screen_scale=new GLfloat[2*N_max];
        uv_offset=new GLfloat[2*N_max];
        uv_scale=new GLfloat[2*N_max];
    }
    
    Text::~Text()
    {
        delete[] world_pos;
        delete[] screen_offset;
        delete[] screen_scale;
        delete[] uv_offset;
        delete[] uv_scale;
    }
    
    void Text::initialize()
    {
        Glite::set_vector(screen_matrix,0,1,0,0,0);
        Glite::set_vector(screen_matrix,4,0,1,0,0);
        Glite::set_vector(screen_matrix,8,0,0,1,0);
        Glite::set_vector(screen_matrix,12,0,0,0,1);
        
        //Program
        
        prog=Glite::create_program(PathManager::locate_resource("resources/glsl/text_vshader.glsl"),
                                   PathManager::locate_resource("resources/glsl/text_fshader.glsl"));
        
        // Sampler
        
        glGenSamplers(1,&sampler);
        glBindSampler(0,sampler);
        
        glSamplerParameteri(sampler,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glSamplerParameteri(sampler,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        
        glSamplerParameteri(sampler,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
        glSamplerParameteri(sampler,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
        
        // Buffers
        
        GLuint mesh_buffer;
        GLfloat *data_v=new GLfloat[12];
        
        data_v[ 0]=0; data_v[ 1]=0;
        data_v[ 2]=1; data_v[ 3]=0;
        data_v[ 4]=0; data_v[ 5]=1;
        
        data_v[ 6]=1; data_v[ 7]=0;
        data_v[ 8]=1; data_v[ 9]=1;
        data_v[10]=0; data_v[11]=1;
        
        glGenBuffers(1,&mesh_buffer);
        glGenBuffers(1,&world_pos_buffer);
        glGenBuffers(1,&screen_offset_buffer);
        glGenBuffers(1,&screen_scale_buffer);
        glGenBuffers(1,&uv_offset_buffer);
        glGenBuffers(1,&uv_scale_buffer);
        
        glBindBuffer(GL_ARRAY_BUFFER,mesh_buffer);
        glBufferData(GL_ARRAY_BUFFER,12*sizeof(GLfloat),data_v,GL_STATIC_DRAW);
        
        delete[] data_v;
        
        glBindBuffer(GL_ARRAY_BUFFER,world_pos_buffer);
        glBufferData(GL_ARRAY_BUFFER,4*N_max*sizeof(GLfloat),NULL,GL_DYNAMIC_DRAW);
        
        glBindBuffer(GL_ARRAY_BUFFER,screen_offset_buffer);
        glBufferData(GL_ARRAY_BUFFER,2*N_max*sizeof(GLfloat),NULL,GL_DYNAMIC_DRAW);
        
        glBindBuffer(GL_ARRAY_BUFFER,screen_scale_buffer);
        glBufferData(GL_ARRAY_BUFFER,2*N_max*sizeof(GLfloat),NULL,GL_DYNAMIC_DRAW);
        
        glBindBuffer(GL_ARRAY_BUFFER,uv_offset_buffer);
        glBufferData(GL_ARRAY_BUFFER,2*N_max*sizeof(GLfloat),NULL,GL_DYNAMIC_DRAW);
        
        glBindBuffer(GL_ARRAY_BUFFER,uv_scale_buffer);
        glBufferData(GL_ARRAY_BUFFER,2*N_max*sizeof(GLfloat),NULL,GL_DYNAMIC_DRAW);
        
        // VAO
        
        glGenVertexArrays(1,&vao);
        glBindVertexArray(vao);
        
        glBindBuffer(GL_ARRAY_BUFFER,mesh_buffer);
        glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,0);
        
        glBindBuffer(GL_ARRAY_BUFFER,world_pos_buffer);
        glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,0,0);
        
        glBindBuffer(GL_ARRAY_BUFFER,screen_offset_buffer);
        glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,0,0);
        
        glBindBuffer(GL_ARRAY_BUFFER,screen_scale_buffer);
        glVertexAttribPointer(3,2,GL_FLOAT,GL_FALSE,0,0);
        
        glBindBuffer(GL_ARRAY_BUFFER,uv_offset_buffer);
        glVertexAttribPointer(4,2,GL_FLOAT,GL_FALSE,0,0);
        
        glBindBuffer(GL_ARRAY_BUFFER,uv_scale_buffer);
        glVertexAttribPointer(5,2,GL_FLOAT,GL_FALSE,0,0);
        
        glVertexAttribDivisor(1,1);
        glVertexAttribDivisor(2,1);
        glVertexAttribDivisor(3,1);
        glVertexAttribDivisor(4,1);
        glVertexAttribDivisor(5,1);
        
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);
        glEnableVertexAttribArray(4);
        glEnableVertexAttribArray(5);
        
        add_character('0',0x0030);
        add_character('1',0x0031);
        add_character('2',0x0032);
        add_character('3',0x0033);
        add_character('4',0x0034);
        add_character('5',0x0035);
        add_character('6',0x0036);
        add_character('7',0x0037);
        add_character('8',0x0038);
        add_character('9',0x0039);
        
        add_character('.',0x002E);
        add_character('+',0x002B);
        add_character(',',0x002C);
        add_character('-',0x002D);
        
        add_character(' ',0x0020);
        
        add_character('a',0x0061);
        add_character('b',0x0062);
        add_character('c',0x0063);
        add_character('d',0x0064);
        add_character('e',0x0065);
        add_character('f',0x0066);
        add_character('g',0x0067);
        add_character('h',0x0068);
        add_character('i',0x0069);
        add_character('j',0x006A);
        add_character('k',0x006B);
        add_character('l',0x006C);
        add_character('m',0x006D);
        add_character('n',0x006E);
        add_character('o',0x006F);
        add_character('p',0x0070);
        add_character('q',0x0071);
        add_character('r',0x0072);
        add_character('s',0x0073);
        add_character('t',0x0074);
        add_character('u',0x0075);
        add_character('v',0x0076);
        add_character('w',0x0077);
        add_character('x',0x0078);
        add_character('y',0x0079);
        add_character('z',0x007A);
        
        add_character('A',0x0041);
        add_character('B',0x0042);
        add_character('C',0x0043);
        add_character('D',0x0044);
        add_character('E',0x0045);
        add_character('F',0x0046);
        add_character('G',0x0047);
        add_character('H',0x0048);
        add_character('I',0x0049);
        add_character('J',0x004A);
        add_character('K',0x004B);
        add_character('L',0x004C);
        add_character('M',0x004D);
        add_character('N',0x004E);
        add_character('O',0x004F);
        add_character('P',0x0050);
        add_character('Q',0x0051);
        add_character('R',0x0052);
        add_character('S',0x0053);
        add_character('T',0x0054);
        add_character('U',0x0055);
        add_character('V',0x0056);
        add_character('W',0x0057);
        add_character('X',0x0058);
        add_character('Y',0x0059);
        add_character('Z',0x005A);
        
        compute_atlas();
        
        glEnable(GL_BLEND);
        
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);
        
        glEnable(GL_MULTISAMPLE);
    }
    
    void Text::add_character(char c,int code)
    {
        char_ID.push_back(c);
        char_code.push_back(code);
        
        glyph_sx.push_back(0);
        glyph_sy.push_back(0);
        glyph_offset_x.push_back(0);
        glyph_offset_y.push_back(0);
        glyph_advance.push_back(0);
        
        texture_u.push_back(0);
        texture_v.push_back(0);
        texture_us.push_back(1.0);
        texture_vs.push_back(1.0);
    }
    
    void Text::compute_atlas()
    {
        int i,j,l;
        
        int Nc=char_ID.size();
        
        int Nw=std::sqrt(Nc);
        int Nh=Nw;
        while(Nw*Nh<Nc) Nh++;
        
        int base_x=0;
        int base_y=0;
        
        FT_Set_Pixel_Sizes(face,0,32);
        
        GLfloat advance_max=0;
        
        for(l=0;l<Nc;l++)
        {
            FT_Load_Glyph(face,FT_Get_Char_Index(face,char_code[l]),FT_LOAD_RENDER);
            
            advance_max=std::max(advance_max,static_cast<GLfloat>(face->glyph->metrics.horiAdvance));
        }
        
        for(l=0;l<Nc;l++)
        {
            FT_Load_Glyph(face,FT_Get_Char_Index(face,char_code[l]),FT_LOAD_RENDER);
            
            FT_Glyph_Metrics &metrics=face->glyph->metrics;
            
            glyph_advance[l]=static_cast<GLfloat>(metrics.horiAdvance)/advance_max;
            glyph_offset_x[l]=static_cast<GLfloat>(metrics.horiBearingX)/advance_max;
            glyph_offset_y[l]=static_cast<GLfloat>(metrics.horiBearingY-metrics.height)/advance_max;
            glyph_sx[l]=static_cast<GLfloat>(metrics.width)/advance_max;
            glyph_sy[l]=static_cast<GLfloat>(metrics.height)/advance_max;
            
            base_x=std::max(base_x,int(face->glyph->advance.x/64));
            base_y=std::max(base_y,int(face->glyph->bitmap.rows));
        }
        
        int Nx=Nw*(1+base_x)-1;
        int Ny=Nh*(1+base_y)-1;
        
        GLfloat *map=new GLfloat[Nx*Ny];
        
        for(i=0;i<Nx*Ny;i++) map[i]=0;
        
        for(l=0;l<Nc;l++)
        {
            FT_Load_Glyph(face,FT_Get_Char_Index(face,char_code[l]),FT_LOAD_RENDER);
            
            int N1=face->glyph->bitmap.width;
            int N2=face->glyph->bitmap.rows;
            
            int offset_x=(l%Nw)*(1+base_x);
            int offset_y=(l/Nw)*(1+base_y);
            
            texture_u[l]=static_cast<GLfloat>(offset_x)/Nx;
            texture_v[l]=static_cast<GLfloat>(offset_y)/Ny;
            texture_us[l]=static_cast<GLfloat>(N1)/Nx;
            texture_vs[l]=static_cast<GLfloat>(N2)/Ny;
            
            for(i=0;i<N1;i++)
            {
                for(j=0;j<N2;j++)
                {
                    int i2=offset_x+i;
                    int j2=offset_y+j;
                    
                    map[i2+j2*Nx]=face->glyph->bitmap.buffer[i+(N2-j-1)*N1]/255.0;
                }
            }
        }
        
        glGenTextures(1,&map_tex);
        glBindTexture(GL_TEXTURE_2D,map_tex);
        
        glTexStorage2D(GL_TEXTURE_2D,1,GL_R32F,Nx,Ny);
        glTexSubImage2D(GL_TEXTURE_2D,0,0,0,Nx,Ny,GL_RED,GL_FLOAT,map);
        
        delete[] map;
    }
    
    int Text::get_ID(char c)
    {
        for(unsigned int i=0;i<char_ID.size();i++)
        {
            if(char_ID[i]==c) return i;
        }
        
        return 0;
    }
    
    void Text::render()
    {
        render(screen_matrix);
    }
    
    void Text::render(GLfloat *proj_matrix)
    {
        glBindBuffer(GL_ARRAY_BUFFER,world_pos_buffer);
        glBufferSubData(GL_ARRAY_BUFFER,0,4*N_inst*sizeof(GLfloat),world_pos);
        
        glBindBuffer(GL_ARRAY_BUFFER,screen_offset_buffer);
        glBufferSubData(GL_ARRAY_BUFFER,0,2*N_inst*sizeof(GLfloat),screen_offset);
        
        glBindBuffer(GL_ARRAY_BUFFER,screen_scale_buffer);
        glBufferSubData(GL_ARRAY_BUFFER,0,2*N_inst*sizeof(GLfloat),screen_scale);
        
        glBindBuffer(GL_ARRAY_BUFFER,uv_offset_buffer);
        glBufferSubData(GL_ARRAY_BUFFER,0,2*N_inst*sizeof(GLfloat),uv_offset);
        
        glBindBuffer(GL_ARRAY_BUFFER,uv_scale_buffer);
        glBufferSubData(GL_ARRAY_BUFFER,0,2*N_inst*sizeof(GLfloat),uv_scale);
        
        glUseProgram(prog);
        
        glBindVertexArray(vao);
        
        glBindTexture(GL_TEXTURE_2D,map_tex);
        glBindSampler(0,sampler);
        glUniformMatrix4fv(6,1,0,proj_matrix);
        glUniformMatrix4fv(7,1,0,screen_matrix);
        
        glDrawArraysInstanced(GL_TRIANGLES,0,6,N_inst);
        
        N_inst=0;
    }
    
    void Text::request_text(double x,double y,double z,
                            std::string const &text,
                            double scale_x ,double scale_y ,
                            double offset_x,double offset_y)
    {
        int ID;
        unsigned int n;
        
        double offset_sum=0;
        
        for(n=0;n<text.size();n++)
        {
            if(text[n]=='\n')
            {
                offset_sum=0;
                offset_y-=1.1*scale_y;
            }
            else
            {
                ID=get_ID(text[n]);
                
                world_pos[4*N_inst+0]=x;
                world_pos[4*N_inst+1]=y;
                world_pos[4*N_inst+2]=z;
                world_pos[4*N_inst+3]=1.0;
                
                screen_offset[2*N_inst+0]=offset_x+scale_x*(offset_sum+glyph_offset_x[ID]);
                screen_offset[2*N_inst+1]=offset_y+scale_y*glyph_offset_y[ID];
                
                screen_scale[2*N_inst+0]=scale_x*glyph_sx[ID];
                screen_scale[2*N_inst+1]=scale_y*glyph_sy[ID];
                
                uv_offset[2*N_inst+0]=texture_u[ID];
                uv_offset[2*N_inst+1]=texture_v[ID];
                
                uv_scale[2*N_inst+0]=texture_us[ID];
                uv_scale[2*N_inst+1]=texture_vs[ID];
                
                offset_sum+=glyph_advance[ID];
                
                N_inst++;
            }
        }
    }
    
    void Text::request_text(double x,double y,double z,
                            std::string const &text,
                            double scale_x ,double scale_y ,
                            double offset_x,double offset_y,
                            double align_x ,double align_y )
    {
        int ID;
        unsigned int n;
        unsigned int Nc=text.size();
        
//        double xmin=0,xmax=0;
//        double ymin=0,ymax=0;
        
        for(n=0;n<Nc;n++) ID_buffer[n]=get_ID(text[n]);
        
        double offset_sum=0;
        
        for(n=0;n<text.size();n++)
        {
            if(text[n]=='\n')
            {
                offset_sum=0;
                offset_y-=1.1*scale_y;
            }
            else
            {
                ID=get_ID(text[n]);
                
                world_pos[4*N_inst+0]=x;
                world_pos[4*N_inst+1]=y;
                world_pos[4*N_inst+2]=z;
                world_pos[4*N_inst+3]=1.0;
                
                screen_offset[2*N_inst+0]=offset_x+scale_x*(offset_sum+glyph_offset_x[ID]);
                screen_offset[2*N_inst+1]=offset_y+scale_y*glyph_offset_y[ID];
                
                screen_scale[2*N_inst+0]=scale_x*glyph_sx[ID];
                screen_scale[2*N_inst+1]=scale_y*glyph_sy[ID];
                
                uv_offset[2*N_inst+0]=texture_u[ID];
                uv_offset[2*N_inst+1]=texture_v[ID];
                
                uv_scale[2*N_inst+0]=texture_us[ID];
                uv_scale[2*N_inst+1]=texture_vs[ID];
                
                offset_sum+=glyph_advance[ID];
                
                N_inst++;
            }
        }
    }
    
    void Text::set_screen(int Sx_,int Sy_)
    {
        Sx=Sx_;
        Sy=Sy_;
        
        Glite::set_vector(screen_matrix,0,2.0/Sx,0,0,0);
        Glite::set_vector(screen_matrix,4,0,2.0/Sy,0,0);
        Glite::set_vector(screen_matrix,8,0,0,1,0);
        Glite::set_vector(screen_matrix,12,-1,-1,0,1);
    }
}
