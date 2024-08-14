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

#include <fdfd.h>
#include <logger.h>

extern const Imdouble Im;
extern std::ofstream plog;

//####################
//       Slice
//####################

Slice::Slice()
    :absorbed(false),solved(false),
     ID(-1), btm_ID(-1), top_ID(-1),
     btm(nullptr), top(nullptr)
{
}

void Slice::absorb_backward(Slice *slc)
{
    btm_ID=slc->btm_ID;
    btm=slc->btm;
}

void Slice::absorb_forward(Slice *slc)
{
    top_ID=slc->top_ID;
    top=slc->top;
        
    Eigen::BiCGSTAB<Eigen::SparseMatrix<Imdouble>,Eigen::IncompleteLUT<Imdouble>> solver;
    solver.setMaxIterations(5);
    solver.setTolerance(1e-5);
    
    Eigen::SparseMatrix<Imdouble> x_mat(B_mat.rows(),B_mat.cols());
    Plog::print(slc->A_mat.nonZeros(), "\n");
    Plog::print(slc->B_mat.nonZeros(), "\n");
    Plog::print(slc->C_mat.nonZeros(), "\n");
    Plog::print("Absorb\n");
    solver.compute(slc->B_mat);
    Plog::print("Compute done\n");
    
    x_mat=solver.solve(slc->A_mat);
    Plog::print("Solve 1\n");
    B_mat=B_mat-C_mat*x_mat;
    Plog::print("Solve 1 done\n");
    
    x_mat=solver.solve(slc->C_mat);
    Plog::print("Solve 2\n");
    C_mat=-C_mat*x_mat;
    Plog::print("Solve 2 done\n");
}

void Slice::set_bottom(Slice *slc,int slc_ID) { btm=slc; btm_ID=slc_ID; }
void Slice::set_ID(int ID_) { ID=ID_; }
void Slice::set_top(Slice *slc,int slc_ID) { top=slc; top_ID=slc_ID; }
void Slice::show()
{
    Plog::print(ID, " ", btm_ID, " ", top_ID, " ",
                this, " ", btm, " ", top, "\n");
}

//####################
//       FDFD
//####################
        
void FDFD::solve_prop_3D_SAM(double lambda_,AngleRad theta,AngleRad phi,AngleRad polar)
{
    int i,j,k;
    
    lambda=lambda_;
    inc_theta=theta;
    inc_phi=phi;
    
    double w=2.0*Pi*c_light/lambda;
    double kn=2.0*Pi/lambda;
    
    kx=kn*std::sin(theta)*std::cos(phi);
    ky=kn*std::sin(theta)*std::sin(phi);
    
    D_mat.setZero();
    M_mat.setZero();
    
    int eq=0;
    Imdouble udx=1.0/Dx;
    Imdouble udy=1.0/Dy;
    Imdouble udz=1.0/Dz;
    
    // Slices initialization
    
    std::vector<Slice> slc(Nz);
    
    for(k=0;k<Nz;k++)
    {
        slc[k].set_ID(k);
        if(k>0) slc[k].set_bottom(&slc[k-1],k-1);
        if(k<Nz-1) slc[k].set_top(&slc[k+1],k+1);
        
        slc[k].show();
        
        slc[k].A_mat.resize(6*Nxy,6*Nxy);
        slc[k].B_mat.resize(6*Nxy,6*Nxy);
        slc[k].C_mat.resize(6*Nxy,6*Nxy);
        
        slc[k].A_mat.setZero();
        slc[k].B_mat.setZero();
        slc[k].C_mat.setZero();
    }
    
    typedef Eigen::Triplet<Imdouble> T;
    std::vector<T> Trp,Trp_m;
    
    Trp.reserve(24*Nxyz);
    Trp_m.reserve(6*Nxyz);
    
    int Ex,Ey,Ez,Hx,Hy,Hz;
    
    // curl H = e dE/dt
    
    Imdouble shift_xp=std::exp( kx*Nx*Dx*Im);
    Imdouble shift_xm=std::exp(-kx*Nx*Dx*Im);
    
    Imdouble shift_yp=std::exp( ky*Ny*Dy*Im);
    Imdouble shift_ym=std::exp(-ky*Ny*Dy*Im);
    
    for(k=0;k<Nz;k++)
    {
        std::vector<T> Trp_A,Trp_B,Trp_C;
        
        for(i=0;i<Nx;i++) for(j=0;j<Ny;j++) 
        {
            // Ex
            
            udx=1.0/get_Dx(i+0.5,w);
            udy=1.0/get_Dy(j,w);
            udz=1.0/get_Dz(k,w);
            
            Imdouble er=1.0;
            
            er=mats[matsgrid(i,j,k)].get_eps(w);
            
            eq=slc_Ex(i,j);
            Trp_B.push_back(T(eq,eq,w*e0*er*Im));
            
                     Hz=slc_Hz(i,j);   Trp_B.push_back(T(eq,Hz,+udy));
            if(j>0){ Hz=slc_Hz(i,j-1); Trp_B.push_back(T(eq,Hz,-udy)); }
            else if(!pml_y)
            {
                Hz=slc_Hz(i,Ny-1); Trp_B.push_back(T(eq,Hz,-udy*shift_ym));
            }
                     Hy=slc_Hy(i,j);   Trp_B.push_back(T(eq,Hy,-udz));
            if(k>0){ Hy=slc_Hy(i,j);   Trp_A.push_back(T(eq,Hy,+udz)); }
            
            // Ey
            
            udx=1.0/get_Dx(i,w);
            udy=1.0/get_Dy(j+0.5,w);
            udz=1.0/get_Dz(k,w);
            
            er=mats[matsgrid(i,j,k)].get_eps(w);
            
            eq=slc_Ey(i,j);
            Trp_B.push_back(T(eq,eq,w*e0*er*Im));
            
                     Hx=slc_Hx(i,j);   Trp_B.push_back(T(eq,Hx,udz));
            if(k>0){ Hx=slc_Hx(i,j);   Trp_A.push_back(T(eq,Hx,-udz)); }
                     Hz=slc_Hz(i,j);   Trp_B.push_back(T(eq,Hz,-udx));
            if(i>0){ Hz=slc_Hz(i-1,j); Trp_B.push_back(T(eq,Hz,udx)); }
            else if(!pml_x)
            {
                Hz=slc_Hz(Nx-1,j); Trp_B.push_back(T(eq,Hz,udx*shift_xm));
            }
            
            // Ez
            
            udx=1.0/get_Dx(i,w);
            udy=1.0/get_Dy(j,w);
            udz=1.0/get_Dz(k+0.5,w);
            
            er=mats[matsgrid(i,j,k)].get_eps(w);
            
            eq=slc_Ez(i,j);
            Trp_B.push_back(T(eq,eq,w*e0*er*Im));
            
                     Hy=slc_Hy(i,j);   Trp_B.push_back(T(eq,Hy,+udx));
            if(i>0){ Hy=slc_Hy(i-1,j); Trp_B.push_back(T(eq,Hy,-udx)); }
            else if(!pml_x)
            {
                Hy=slc_Hy(Nx-1,j); Trp_B.push_back(T(eq,Hy,-udx*shift_xm));
            }
                     Hx=slc_Hx(i,j);   Trp_B.push_back(T(eq,Hx,-udy));
            if(j>0){ Hx=slc_Hx(i,j-1); Trp_B.push_back(T(eq,Hx,+udy)); }
            else if(!pml_y)
            {
                Hx=slc_Hx(i,Ny-1); Trp_B.push_back(T(eq,Hx,+udy*shift_ym));
            }
        }
        
        // curl E = - mu dH/dt
        
        for(i=0;i<Nx;i++) for(j=0;j<Ny;j++)
        {
            // Hx
            
            udx=1.0/get_Dx(i,w);
            udy=1.0/get_Dy(j+0.5,w);
            udz=1.0/get_Dz(k+0.5,w);
            
            eq=slc_Hx(i,j);
            Trp_B.push_back(T(eq,eq,-w*mu0*Im));
            
            if(j<Ny-1){ Ez=slc_Ez(i,j+1); Trp_B.push_back(T(eq,Ez,+udy)); }
            else if(!pml_y)
            {
                Ez=slc_Ez(i,0);   Trp_B.push_back(T(eq,Ez,+udy*shift_yp));
            }
                        Ez=slc_Ez(i,j);   Trp_B.push_back(T(eq,Ez,-udy));
            if(k<Nz-1){ Ey=slc_Ey(i,j);   Trp_C.push_back(T(eq,Ey,-udz)); }
                        Ey=slc_Ey(i,j);   Trp_B.push_back(T(eq,Ey,+udz));
            
            // Hy
            
            udx=1.0/get_Dx(i+0.5,w);
            udy=1.0/get_Dy(j,w);
            udz=1.0/get_Dz(k+0.5,w);
            
            eq=slc_Hy(i,j);
            Trp_B.push_back(T(eq,eq,-w*mu0*Im));
            
            if(k<Nz-1){ Ex=slc_Ex(i,j);   Trp_C.push_back(T(eq,Ex,+udz)); }
                        Ex=slc_Ex(i,j);   Trp_B.push_back(T(eq,Ex,-udz));
            if(i<Nx-1){ Ez=slc_Ez(i+1,j); Trp_B.push_back(T(eq,Ez,-udx)); }
            else if(!pml_x)
            {
                Ez=slc_Ez(0,j); Trp_B.push_back(T(eq,Ez,-udx*shift_xp));
            }
                        Ez=slc_Ez(i,j);   Trp_B.push_back(T(eq,Ez,+udx));
            
            // Hz
            
            udx=1.0/get_Dx(i+0.5,w);
            udy=1.0/get_Dy(j+0.5,w);
            udz=1.0/get_Dz(k,w);
            
            eq=slc_Hz(i,j);
            Trp_B.push_back(T(eq,eq,-w*mu0*Im));
            
            if(i<Nx-1){ Ey=slc_Ey(i+1,j); Trp_B.push_back(T(eq,Ey,+udx)); }
            else if(!pml_x)
            {
                Ey=slc_Ey(0,j); Trp_B.push_back(T(eq,Ey,udx*shift_xp));
            }
                        Ey=slc_Ey(i,j);   Trp_B.push_back(T(eq,Ey,-udx));
            if(j<Ny-1){ Ex=slc_Ex(i,j+1); Trp_B.push_back(T(eq,Ex,-udy)); }
            else if(!pml_y)
            {
                Ex=slc_Ex(i,0); Trp_B.push_back(T(eq,Ex,-udy*shift_yp));
            }
                        Ex=slc_Ex(i,j); Trp_B.push_back(T(eq,Ex,+udy));
        }
        
        slc[k].A_mat.setFromTriplets(Trp_A.begin(),Trp_A.end());
        slc[k].B_mat.setFromTriplets(Trp_B.begin(),Trp_B.end());
        slc[k].C_mat.setFromTriplets(Trp_C.begin(),Trp_C.end());
    }
    
//    Eigen::SparseVector<Imdouble> F_src(6*Nxyz);
//    Eigen::VectorXcd b(6*Nxyz);
//    
//    if(inj_type==SRC_PLANE_Z)
//    {
//        ImVector3 E_in,H_in;
//        
//        for(i=0;i<Nx;i++) for(j=0;j<Ny;j++)
//        {
//            k=inj_zp;
//            
//            plane_wave(lambda,1.0,Pi-theta,phi,polar,(i+0.5)*Dx,j*Dy,Dz/2.0,0,E_in,H_in);
//            F_src.coeffRef(index_Ex(i,j,k))=-H_in.y/Dz;
//            
//            plane_wave(lambda,1.0,Pi-theta,phi,polar,i*Dx,(j+0.5)*Dy,Dz/2.0,0,E_in,H_in);
//            F_src.coeffRef(index_Ey(i,j,k))=+H_in.x/Dz;
//            
//            plane_wave(lambda,1.0,Pi-theta,phi,polar,i*Dx,(j+0.5)*Dy,0,0,E_in,H_in);
//            F_src.coeffRef(index_Hx(i,j,k))=-E_in.y/Dz;
//            
//            plane_wave(lambda,1.0,Pi-theta,phi,polar,(i+0.5)*Dx,j*Dy,0,0,E_in,H_in);
//            F_src.coeffRef(index_Hy(i,j,k))=+E_in.x/Dz;
//        }
//    }
//    else if(inj_type==SRC_CBOX)
//    {
//        F_src=inj_F_src;
//    }
//    
//    b=-F_src;
    
    std::vector<Slice*> slc_p,slc_q;
    
    for(int k=0;k<Nz;k++) slc_p.push_back(&slc[k]);
    
    bool run=true;
    
    while(run)
    {
        if(slc_p.size()%2==0)
        {
            k=slc_p.size()/2;
            
            slc_p[k-1]->absorb_forward(slc_p[k]);
            slc_p[k+1]->absorb_backward(slc_p[k]);
            slc_p[k]->absorbed=true;
            
            for(unsigned int k=0;k<slc_p.size();k++)
            {
                if(!slc_p[k]->absorbed) slc_q.push_back(slc_p[k]);
            }
            
            slc_p=slc_q;
            slc_q.clear();
            Plog::print("\n");
            for(unsigned int k=0;k<slc_p.size();k++)
            {
                slc_p[k]->show();
            }
        }
        else
        {
            for(unsigned int k=0;k<slc_p.size();k++)
            {
                Plog::print(k, "\n");
                if(k%2==1)
                {
                    slc_p[k-1]->absorb_forward(slc_p[k]);
                    slc_p[k+1]->absorb_backward(slc_p[k]);
                    slc_p[k]->absorbed=true;
                }
                
                if(!slc_p[k]->absorbed) slc_q.push_back(slc_p[k]);
            }
            
            slc_p=slc_q;
            slc_q.clear();
            Plog::print("\n");
            for(unsigned int k=0;k<slc_p.size();k++)
            {
                slc_p[k]->show();
            }
        }
        
        run=false;
        if(slc_p.size()>2) run=true;
    }
}
