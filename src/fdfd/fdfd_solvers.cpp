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


extern const Imdouble Im;
extern std::ofstream plog;

void FDFD::solve_prop_1D(double lambda_,AngleRad theta,AngleRad phi,AngleRad polar)
{
    int k;
    
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
    Imdouble udz=1.0/Dz;
    
    typedef Eigen::Triplet<Imdouble> T;
    std::vector<T> Trp,Trp_m;
    
    Trp.reserve(2*8*Nxyz);
    Trp_m.reserve(6*Nxyz);
    
    int Ex,Ey,Ez,Hx,Hy,Hz;
    
//    ProgDisp dsp(2*Nz);
    
    // curl H = e dE/dt
    
    for(k=0;k<Nz;k++)
    {
        // Ex
        
        udz=1.0/get_Dz(k,w);
        
        Imdouble er=1.0;
        
        er=mats[matsgrid(0,0,k)].get_eps(w);
        
        eq=index_Ex(k);
        Trp_m.push_back(T(eq,eq,-w*e0*er*Im));
        
                 Hz=index_Hz(k);   Trp.push_back(T(eq,Hz,ky*Im));
                 Hy=index_Hy(k);   Trp.push_back(T(eq,Hy,-udz));
        if(k>0){ Hy=index_Hy(k-1); Trp.push_back(T(eq,Hy,udz)); }
        
        // Ey
        
        udz=1.0/get_Dz(k,w);
        
        er=mats[matsgrid(0,0,k)].get_eps(w);
        
        eq=index_Ey(k);
        Trp_m.push_back(T(eq,eq,-w*e0*er*Im));
        
                 Hx=index_Hx(k);   Trp.push_back(T(eq,Hx,udz));
        if(k>0){ Hx=index_Hx(k-1); Trp.push_back(T(eq,Hx,-udz)); }
                 Hz=index_Hz(k);   Trp.push_back(T(eq,Hz,-kx*Im));
        
        // Ez
        
        udz=1.0/get_Dz(k+0.5,w);
        
        er=mats[matsgrid(0,0,k)].get_eps(w);
        
        eq=index_Ez(k);
        Trp_m.push_back(T(eq,eq,-w*e0*er*Im));
        
        Hy=index_Hy(k); Trp.push_back(T(eq,Hy,kx*Im));
        Hx=index_Hx(k); Trp.push_back(T(eq,Hx,-ky*Im));
        
//        ++dsp;
    }
    
    // curl E = - mu dH/dt
    
    for(k=0;k<Nz;k++)
    {
        // Hx
        
        udz=1.0/get_Dz(k+0.5,w);
        
        eq=index_Hx(k);
        Trp_m.push_back(T(eq,eq,w*mu0*Im));
        
                    Ez=index_Ez(k);   Trp.push_back(T(eq,Ez,ky*Im));
        if(k<Nz-1){ Ey=index_Ey(k+1); Trp.push_back(T(eq,Ey,-udz)); }
                    Ey=index_Ey(k);   Trp.push_back(T(eq,Ey,udz));
        
        // Hy
        
        udz=1.0/get_Dz(k+0.5,w);
        
        eq=index_Hy(k);
        Trp_m.push_back(T(eq,eq,w*mu0*Im));
        
        if(k<Nz-1){ Ex=index_Ex(k+1); Trp.push_back(T(eq,Ex,udz)); }
                    Ex=index_Ex(k);   Trp.push_back(T(eq,Ex,-udz));
                    Ez=index_Ez(k);   Trp.push_back(T(eq,Ez,-kx*Im));
        
        // Hz
        
        udz=1.0/get_Dz(k,w);
        
        eq=index_Hz(k);
        Trp_m.push_back(T(eq,eq,w*mu0*Im));
        
        Ey=index_Ey(k); Trp.push_back(T(eq,Ey,kx*Im));
        Ex=index_Ex(k); Trp.push_back(T(eq,Ex,-ky*Im));
        
//        ++dsp;
    }
    
    D_mat.setFromTriplets(Trp.begin(),Trp.end());
    M_mat.setFromTriplets(Trp_m.begin(),Trp_m.end());
    
    Trp.clear();
    
    Eigen::SparseMatrix<Imdouble> W_mat(6*Nxyz,6*Nxyz);
    
    W_mat=D_mat-M_mat;
    
    Eigen::SparseVector<Imdouble> F_src(6*Nxyz);
    Eigen::VectorXcd b(6*Nxyz);
    
    ImVector3 E_in,H_in,V_tmp;
    
    plane_wave(lambda,1.0,Pi-theta,phi,polar,0,0,0,0,E_in,V_tmp);
    plane_wave(lambda,1.0,Pi-theta,phi,polar,0,0,Dz/2.0,0,V_tmp,H_in);
    
    k=Nz-30;
    
    F_src.coeffRef(index_Ex(k))=-H_in.y/Dz;
    F_src.coeffRef(index_Ey(k))=+H_in.x/Dz;
    
    F_src.coeffRef(index_Hx(k))=-E_in.y/Dz;
    F_src.coeffRef(index_Hy(k))=+E_in.x/Dz;
    
    b=-F_src;
    
    Eigen::SparseLU<Eigen::SparseMatrix<Imdouble>> solver;
    solver.analyzePattern(W_mat);
    solver.factorize(W_mat);
    F=solver.solve(b);
    
//    double eta=std::sqrt(mu0/e0);
//    
//    for(k=0;k<Nz;k++)
//    {
////        plog<<k<<" "<<F(index_Ex(k)).real()
////               <<" "<<F(index_Ex(k)).imag()
////               <<" "<<F(index_Ey(k)).real()
////               <<" "<<F(index_Ey(k)).imag()
////               <<" "<<F(index_Hx(k)).real()*eta
////               <<" "<<F(index_Hx(k)).imag()*eta
////               <<" "<<F(index_Hy(k)).real()*eta
////               <<" "<<F(index_Hy(k)).imag()*eta
////               <<std::endl;
//               
//        plog<<k<<" "<<F(index_Ex(k)).real()
//               <<" "<<F(index_Ey(k)).real()
//               <<" "<<F(index_Ez(k)).real()
//               <<" "<<F(index_Hx(k)).real()*eta
//               <<" "<<F(index_Hy(k)).real()*eta
//               <<" "<<F(index_Hz(k)).real()*eta
//               <<std::endl;
//    }
}

void solve_LU(Eigen::SparseMatrix<Imdouble> const &A,
              Eigen::VectorXcd &x,
              Eigen::VectorXcd const &b)
{
    Eigen::SparseLU<Eigen::SparseMatrix<Imdouble>> solver;
    
    solver.compute(A);
    
    x=solver.solve(b);
}


Imdouble inverse_power_iteration(Eigen::SparseMatrix<Imdouble> const &A,
                                 Imdouble guess,Eigen::VectorXcd &V,
                                 double conv,int max_it)
{
    int i,j,N=A.rows();
    
    Eigen::SparseMatrix<Imdouble> Id(N,N),A_shift(N,N);
    Eigen::SparseLU<Eigen::SparseMatrix<Imdouble>> solver;
    
    Id.setIdentity();
    
    A_shift=A-guess*Id;
    solver.compute(A_shift);
    
    Eigen::VectorXcd b(N);
    
    b=V;
    
    V=solver.solve(b);
    
    double m=std::abs(V(0));
    for(j=0;j<N;j++) m=std::max(m,std::abs(V(j)));
    for(j=0;j<N;j++) V(j)/=m;
    
    b=V;
    
    V=A*b;
    
    Imdouble eig_val=0;
    for(j=0;j<N;j++) eig_val+=V(j)/b(j);
    eig_val/=N;
    
    for(i=0;i<max_it;i++)
    {
        V=solver.solve(b);
        
        double m=std::abs(V(0));
        for(j=0;j<N;j++) m=std::max(m,std::abs(V(j)));
        for(j=0;j<N;j++) V(j)/=m;
        
        b=V;
                
        V=A*b;
        
        Imdouble next_eig_val=0;
        for(j=0;j<N;j++) next_eig_val+=V(j)/b(j);
        next_eig_val/=N;
        
        if(std::abs(eig_val-next_eig_val)/std::abs(eig_val)<conv)
        {
            eig_val=next_eig_val;
            break;
        }
        
        eig_val=next_eig_val; 
    }
    
    chk_var(eig_val);
    
    return eig_val;
}

Imdouble inverse_power_iteration_BICGSTAB(Eigen::SparseMatrix<Imdouble> const &A,
                                          Imdouble guess,Eigen::VectorXcd &V,
                                          double conv,int max_it)
{
    int i,j,N=A.rows();
    
    Eigen::SparseMatrix<Imdouble> Id(N,N),A_shift(N,N);
    Eigen::BiCGSTAB<Eigen::SparseMatrix<Imdouble>,Eigen::IncompleteLUT<Imdouble>> solver;
    
    Id.setIdentity();
    
    A_shift=A-guess*Id;
    
    solver.setMaxIterations(100);
    solver.setTolerance(1e-3);
    solver.compute(A_shift);
    
    Eigen::VectorXcd b(N);
    
    b=V;
    
    chk_var("A");
    V=solver.solve(b);
//    V=solver.solveWithGuess(b,b);
    chk_var("B");
    
    double m=std::abs(V(0));
    for(j=0;j<N;j++) m=std::max(m,std::abs(V(j)));
    for(j=0;j<N;j++) V(j)/=m;
    
    b=V;
    
    V=A*b;
    
    Imdouble eig_val=0;
    for(j=0;j<N;j++) eig_val+=V(j)/b(j);
    eig_val/=N;
    
    for(i=0;i<max_it;i++)
    {
//        V=solver.solveWithGuess(b,b);
        V=solver.solve(b);
        
        double m=std::abs(V(0));
        for(j=0;j<N;j++) m=std::max(m,std::abs(V(j)));
        for(j=0;j<N;j++) V(j)/=m;
        
        b=V;
                
        V=A*b;
        
        Imdouble next_eig_val=0;
        for(j=0;j<N;j++) next_eig_val+=V(j)/b(j);
        next_eig_val/=N;
        
        if(std::abs(eig_val-next_eig_val)/std::abs(eig_val)<conv)
        {
            eig_val=next_eig_val;
            break;
        }
        
        eig_val=next_eig_val; 
        chk_var(eig_val);
    }
    
    chk_var(eig_val);
    
    return eig_val;
}

void solve_BiCGSTAB(Eigen::SparseMatrix<Imdouble> const &A,
                    Eigen::VectorXcd &x,
                    Eigen::VectorXcd const &b,
                    Eigen::VectorXcd const &guess)
{
    Eigen::BiCGSTAB<Eigen::SparseMatrix<Imdouble>,Eigen::IncompleteLUT<Imdouble>> solver;
    
    solver.setMaxIterations(10000);
    solver.setTolerance(1e-5);
    solver.compute(A);
    
    x=solver.solveWithGuess(b,guess);
    
    chk_msg_sc(solver.iterations());
    chk_msg_sc(solver.error());
}

void FDFD::solve_prop_2D(double lambda_,AngleRad theta,AngleRad phi,AngleRad polar)
{
    int i,k;
    
    lambda=lambda_;
    inc_theta=theta;
    inc_phi=phi;
    
    double w=2.0*Pi*c_light/lambda;
    
    double up_index=mats[matsgrid(0,0,zs_e)].get_eps(w).real();
    up_index=std::sqrt(up_index);
    
    double kn=2.0*Pi/lambda*up_index;
    
    kx=kn*std::sin(theta)*std::cos(phi);
    ky=kn*std::sin(theta)*std::sin(phi);
    
    D_mat.setZero();
    M_mat.setZero();
    
    int eq=0;
    Imdouble udx=1.0/Dx;
    Imdouble udz=1.0/Dz;
    
    typedef Eigen::Triplet<Imdouble> T;
    std::vector<T> Trp,Trp_m;
    
    Trp.reserve(2*10*Nxyz);
    Trp_m.reserve(6*Nxyz);
    
    int Ex,Ey,Ez,Hx,Hy,Hz;
    
    // curl H = e dE/dt
    
    Imdouble shift_xp=std::exp( kx*Nx*Dx*Im);
    Imdouble shift_xm=std::exp(-kx*Nx*Dx*Im);
    
    for(i=0;i<Nx;i++) for(k=0;k<Nz;k++)
    {
        // Ex
        
        udz=1.0/get_Dz(k,w);
        udx=1.0/get_Dx(i+0.5,w);
        
        Imdouble er=1.0;
        
        er=mats[matsgrid(i,0,k)].get_eps(w);
        
        eq=index_Ex(i,0,k);
        Trp_m.push_back(T(eq,eq,-w*e0*er*Im));
        
                 Hz=index_Hz(i,0,k);   Trp.push_back(T(eq,Hz,ky*Im));
                 Hy=index_Hy(i,0,k);   Trp.push_back(T(eq,Hy,-udz));
        if(k>0){ Hy=index_Hy(i,0,k-1); Trp.push_back(T(eq,Hy,udz)); }
        
        // Ey
        
        udz=1.0/get_Dz(k,w);
        udx=1.0/get_Dx(i,w);
        
        er=mats[matsgrid(i,0,k)].get_eps(w);
        
        eq=index_Ey(i,0,k);
        Trp_m.push_back(T(eq,eq,-w*e0*er*Im));
        
                 Hx=index_Hx(i,0,k);   Trp.push_back(T(eq,Hx,udz));
        if(k>0){ Hx=index_Hx(i,0,k-1); Trp.push_back(T(eq,Hx,-udz)); }
                 Hz=index_Hz(i,0,k);   Trp.push_back(T(eq,Hz,-udx));
        if(i>0){ Hz=index_Hz(i-1,0,k); Trp.push_back(T(eq,Hz,udx)); }
        else if(!pml_x)
        {
            Hz=index_Hz(Nx-1,0,k); Trp.push_back(T(eq,Hz,udx*shift_xm));
        }
        
        // Ez
        
        udz=1.0/get_Dz(k+0.5,w);
        udx=1.0/get_Dx(i,w);
        
        er=mats[matsgrid(i,0,k)].get_eps(w);
        
        eq=index_Ez(i,0,k);
        Trp_m.push_back(T(eq,eq,-w*e0*er*Im));
        
                 Hy=index_Hy(i,0,k);   Trp.push_back(T(eq,Hy,udx));
        if(i>0){ Hy=index_Hy(i-1,0,k); Trp.push_back(T(eq,Hy,-udx)); }
        else if(!pml_x)
        {
            Hy=index_Hy(Nx-1,0,k); Trp.push_back(T(eq,Hy,-udx*shift_xm));
        }
                 Hx=index_Hx(i,0,k);   Trp.push_back(T(eq,Hx,-ky*Im));
    }
    
    // curl E = - mu dH/dt
    
    for(i=0;i<Nx;i++) for(k=0;k<Nz;k++)
    {
        // Hx
        
        udz=1.0/get_Dz(k+0.5,w);
        udx=1.0/get_Dx(i,w);
        
        eq=index_Hx(i,0,k);
        Trp_m.push_back(T(eq,eq,w*mu0*Im));
        
                    Ez=index_Ez(i,0,k);   Trp.push_back(T(eq,Ez,ky*Im));
        if(k<Nz-1){ Ey=index_Ey(i,0,k+1); Trp.push_back(T(eq,Ey,-udz)); }
                    Ey=index_Ey(i,0,k);   Trp.push_back(T(eq,Ey,udz));
        
        // Hy
        
        udz=1.0/get_Dz(k+0.5,w);
        udx=1.0/get_Dx(i+0.5,w);
        
        eq=index_Hy(i,0,k);
        Trp_m.push_back(T(eq,eq,w*mu0*Im));
        
        if(k<Nz-1){ Ex=index_Ex(i,0,k+1); Trp.push_back(T(eq,Ex,udz)); }
                    Ex=index_Ex(i,0,k);   Trp.push_back(T(eq,Ex,-udz));
        if(i<Nx-1){ Ez=index_Ez(i+1,0,k); Trp.push_back(T(eq,Ez,-udx)); }
        else if(!pml_x)
        {
            Ez=index_Ez(0,0,k); Trp.push_back(T(eq,Ez,-udx*shift_xp));
        }
                    Ez=index_Ez(i,0,k);   Trp.push_back(T(eq,Ez,udx));
        
        // Hz
        
        udz=1.0/get_Dz(k,w);
        udx=1.0/get_Dx(i+0.5,w);
        
        eq=index_Hz(i,0,k);
        Trp_m.push_back(T(eq,eq,w*mu0*Im));
        
        if(i<Nx-1){ Ey=index_Ey(i+1,0,k); Trp.push_back(T(eq,Ey,udx)); }
        else if(!pml_x)
        {
            Ey=index_Ey(0,0,k); Trp.push_back(T(eq,Ey,udx*shift_xp));
        }
                    Ey=index_Ey(i,0,k); Trp.push_back(T(eq,Ey,-udx));
                    Ex=index_Ex(i,0,k); Trp.push_back(T(eq,Ex,-ky*Im));
    }
    
    D_mat.setFromTriplets(Trp.begin(),Trp.end());
    M_mat.setFromTriplets(Trp_m.begin(),Trp_m.end());
    
    Trp.clear();
    
    Eigen::SparseMatrix<Imdouble> W_mat(6*Nxyz,6*Nxyz);
    
    W_mat=D_mat-M_mat;
    
    Eigen::SparseVector<Imdouble> F_src(6*Nxyz);
    Eigen::VectorXcd b(6*Nxyz);
    
    if(inj_type==SRC_PLANE_Z)
    {
        ImVector3 E_in,H_in;
        
        k=inj_zp;
        double inj_index=mats[matsgrid(0,0,k)].get_eps(w).real();
        inj_index=std::sqrt(inj_index);
        
        for(i=0;i<Nx;i++)
        {
            
            plane_wave(lambda,inj_index,Pi-theta,phi,polar,i*Dx,0,Dz/2.0,0,E_in,H_in);
            F_src.coeffRef(index_Ex(i,0,k))=-H_in.y/Dz;
            
            plane_wave(lambda,inj_index,Pi-theta,phi,polar,(i+0.5)*Dx,0,Dz/2.0,0,E_in,H_in);
            F_src.coeffRef(index_Ey(i,0,k))=+H_in.x/Dz;
            
            plane_wave(lambda,inj_index,Pi-theta,phi,polar,(i+0.5)*Dx,0,0,0,E_in,H_in);
            F_src.coeffRef(index_Hx(i,0,k))=-E_in.y/Dz;
            
            plane_wave(lambda,inj_index,Pi-theta,phi,polar,i*Dx,0,0,0,E_in,H_in);
            F_src.coeffRef(index_Hy(i,0,k))=+E_in.x/Dz;
        }
    }
    else if(inj_type==SRC_CBOX)
    {
        F_src=inj_F_src;
    }
    
    b=-F_src;
    
    if(solver_type==SOLVE_LU) solve_LU(W_mat,F,b);
    else if(solver_type==SOLVE_BiCGSTAB)
    {
        Eigen::VectorXcd F_guess(6*Nxyz);
        
        for(i=0;i<Nx;i++) for(k=0;k<Nz;k++)
        {
            ImVector3 E_in,H_in;
            
            plane_wave(lambda,1.0,Pi-theta,phi,polar,(i+0.5)*Dx,0,(k+0.0)*Dz,0,E_in,H_in);
            F_guess(index_Ex(i,0,k))=E_in.x;
            
            plane_wave(lambda,1.0,Pi-theta,phi,polar,(i+0.0)*Dx,0,(k+0.0)*Dz,0,E_in,H_in);
            F_guess(index_Ey(i,0,k))=E_in.y;
            
            plane_wave(lambda,1.0,Pi-theta,phi,polar,(i+0.0)*Dx,0,(k+0.5)*Dz,0,E_in,H_in);
            F_guess(index_Ez(i,0,k))=E_in.z;
            
            plane_wave(lambda,1.0,Pi-theta,phi,polar,(i+0.0)*Dx,0,(k+0.5)*Dz,0,E_in,H_in);
            F_guess(index_Hx(i,0,k))=H_in.x;
            
            plane_wave(lambda,1.0,Pi-theta,phi,polar,(i+0.5)*Dx,0,(k+0.5)*Dz,0,E_in,H_in);
            F_guess(index_Hy(i,0,k))=H_in.y;
            
            plane_wave(lambda,1.0,Pi-theta,phi,polar,(i+0.5)*Dx,0,(k+0.0)*Dz,0,E_in,H_in);
            F_guess(index_Hz(i,0,k))=H_in.z;
        }
        
        solve_BiCGSTAB(W_mat,F,b,F_guess);
    }
}

void FDFD::solve_prop_3D(double lambda_,AngleRad theta,AngleRad phi,AngleRad polar)
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
    
    shift_yp=shift_ym=1.0;
    
    for(i=0;i<Nx;i++) for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
    {
        // Ex
        
        udx=1.0/get_Dx(i+0.5,w);
//        udy=1.0/get_Dy(j,w);
        udz=1.0/get_Dz(k,w);
        
        Imdouble er=1.0;
        
        er=mats[matsgrid(i,j,k)].get_eps(w);
        
        eq=index_Ex(i,j,k);
        Trp_m.push_back(T(eq,eq,-w*e0*er*Im));
        
                 Hz=index_Hz(i,j,k);   Trp.push_back(T(eq,Hz,+udy));
        if(j>0){ Hz=index_Hz(i,j-1,k); Trp.push_back(T(eq,Hz,-udy)); }
        else if(!pml_y)
        {
            Hz=index_Hz(i,Ny-1,k); Trp.push_back(T(eq,Hz,-udy*shift_ym));
        }
                 Hy=index_Hy(i,j,k);   Trp.push_back(T(eq,Hy,-udz));
        if(k>0){ Hy=index_Hy(i,j,k-1); Trp.push_back(T(eq,Hy,+udz)); }
        
        // Ey
        
        udx=1.0/get_Dx(i,w);
//        udy=1.0/get_Dy(j+0.5,w);
        udz=1.0/get_Dz(k,w);
        
        er=mats[matsgrid(i,j,k)].get_eps(w);
        
        eq=index_Ey(i,j,k);
        Trp_m.push_back(T(eq,eq,-w*e0*er*Im));
        
                 Hx=index_Hx(i,j,k);   Trp.push_back(T(eq,Hx,udz));
        if(k>0){ Hx=index_Hx(i,j,k-1); Trp.push_back(T(eq,Hx,-udz)); }
                 Hz=index_Hz(i,j,k);   Trp.push_back(T(eq,Hz,-udx));
        if(i>0){ Hz=index_Hz(i-1,j,k); Trp.push_back(T(eq,Hz,udx)); }
        else if(!pml_x)
        {
            Hz=index_Hz(Nx-1,j,k); Trp.push_back(T(eq,Hz,udx*shift_xm));
        }
        
        // Ez
        
        udx=1.0/get_Dx(i,w);
//        udy=1.0/get_Dy(j,w);
        udz=1.0/get_Dz(k+0.5,w);
        
        er=mats[matsgrid(i,j,k)].get_eps(w);
        
        eq=index_Ez(i,j,k);
        Trp_m.push_back(T(eq,eq,-w*e0*er*Im));
        
                 Hy=index_Hy(i,j,k);   Trp.push_back(T(eq,Hy,+udx));
        if(i>0){ Hy=index_Hy(i-1,j,k); Trp.push_back(T(eq,Hy,-udx)); }
        else if(!pml_x)
        {
            Hy=index_Hy(Nx-1,j,k); Trp.push_back(T(eq,Hy,-udx*shift_xm));
        }
                 Hx=index_Hx(i,j,k);   Trp.push_back(T(eq,Hx,-udy));
        if(j>0){ Hx=index_Hx(i,j-1,k); Trp.push_back(T(eq,Hx,+udy)); }
        else if(!pml_y)
        {
            Hx=index_Hx(i,Ny-1,k); Trp.push_back(T(eq,Hx,+udy*shift_ym));
        }
    }
    
    // curl E = - mu dH/dt
    
    for(i=0;i<Nx;i++) for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
    {
        // Hx
        
        udx=1.0/get_Dx(i,w);
//        udy=1.0/get_Dy(j+0.5,w);
        udz=1.0/get_Dz(k+0.5,w);
        
        eq=index_Hx(i,j,k);
        Trp_m.push_back(T(eq,eq,w*mu0*Im));
        
        if(j<Ny-1){ Ez=index_Ez(i,j+1,k);   Trp.push_back(T(eq,Ez,+udy)); }
        else if(!pml_y)
        {
            Ez=index_Ez(i,0,k);   Trp.push_back(T(eq,Ez,+udy*shift_yp));
        }
                    Ez=index_Ez(i,j,k);   Trp.push_back(T(eq,Ez,-udy));
        if(k<Nz-1){ Ey=index_Ey(i,j,k+1); Trp.push_back(T(eq,Ey,-udz)); }
                    Ey=index_Ey(i,j,k);   Trp.push_back(T(eq,Ey,+udz));
        
        // Hy
        
        udx=1.0/get_Dx(i+0.5,w);
//        udy=1.0/get_Dy(j,w);
        udz=1.0/get_Dz(k+0.5,w);
        
        eq=index_Hy(i,j,k);
        Trp_m.push_back(T(eq,eq,w*mu0*Im));
        
        if(k<Nz-1){ Ex=index_Ex(i,j,k+1); Trp.push_back(T(eq,Ex,+udz)); }
                    Ex=index_Ex(i,j,k);   Trp.push_back(T(eq,Ex,-udz));
        if(i<Nx-1){ Ez=index_Ez(i+1,j,k); Trp.push_back(T(eq,Ez,-udx)); }
        else if(!pml_x)
        {
            Ez=index_Ez(0,j,k); Trp.push_back(T(eq,Ez,-udx*shift_xp));
        }
                    Ez=index_Ez(i,j,k);   Trp.push_back(T(eq,Ez,+udx));
        
        // Hz
        
        udx=1.0/get_Dx(i+0.5,w);
//        udy=1.0/get_Dy(j+0.5,w);
        udz=1.0/get_Dz(k,w);
        
        eq=index_Hz(i,j,k);
        Trp_m.push_back(T(eq,eq,w*mu0*Im));
        
        if(i<Nx-1){ Ey=index_Ey(i+1,j,k); Trp.push_back(T(eq,Ey,+udx)); }
        else if(!pml_x)
        {
            Ey=index_Ey(0,j,k); Trp.push_back(T(eq,Ey,udx*shift_xp));
        }
                    Ey=index_Ey(i,j,k); Trp.push_back(T(eq,Ey,-udx));
        if(j<Ny-1){ Ex=index_Ex(i,j+1,k); Trp.push_back(T(eq,Ex,-udy)); }
        else if(!pml_y)
        {
            Ex=index_Ex(i,0,k); Trp.push_back(T(eq,Ex,-udy*shift_yp));
        }
                    Ex=index_Ex(i,j,k); Trp.push_back(T(eq,Ex,+udy));
    }
    
    D_mat.setFromTriplets(Trp.begin(),Trp.end());
    M_mat.setFromTriplets(Trp_m.begin(),Trp_m.end());
    
    Trp.clear();
    
    Eigen::SparseMatrix<Imdouble> W_mat(6*Nxyz,6*Nxyz);
    
    W_mat=D_mat-M_mat;
    
//    plog<<W_mat<<std::endl;
    
    chk_msg_sc(W_mat.nonZeros());
    
    Eigen::SparseVector<Imdouble> F_src(6*Nxyz);
    Eigen::VectorXcd b(6*Nxyz);
    
    if(inj_type==SRC_PLANE_Z)
    {
        ImVector3 E_in,H_in;
        
        for(i=0;i<Nx;i++) for(j=0;j<Ny;j++)
        {
            k=inj_zp;
            
            plane_wave(lambda,1.0,Pi-theta,phi,polar,(i+0.5)*Dx,j*Dy,Dz/2.0,0,E_in,H_in);
            F_src.coeffRef(index_Ex(i,j,k))=-H_in.y/Dz;
            
            plane_wave(lambda,1.0,Pi-theta,phi,polar,i*Dx,(j+0.5)*Dy,Dz/2.0,0,E_in,H_in);
            F_src.coeffRef(index_Ey(i,j,k))=+H_in.x/Dz;
            
            plane_wave(lambda,1.0,Pi-theta,phi,polar,i*Dx,(j+0.5)*Dy,0,0,E_in,H_in);
            F_src.coeffRef(index_Hx(i,j,k))=-E_in.y/Dz;
            
            plane_wave(lambda,1.0,Pi-theta,phi,polar,(i+0.5)*Dx,j*Dy,0,0,E_in,H_in);
            F_src.coeffRef(index_Hy(i,j,k))=+E_in.x/Dz;
        }
    }
    else if(inj_type==SRC_CBOX)
    {
        F_src=inj_F_src;
    }
    
    b=-F_src;
    
    if(solver_type==SOLVE_LU) solve_LU(W_mat,F,b);
    else if(solver_type==SOLVE_BiCGSTAB)
    {
        Eigen::VectorXcd F_guess(6*Nxyz);
        
        for(i=0;i<Nx;i++) for(j=0;j<Ny;j++) for(k=0;k<Nz;k++)
        {
            ImVector3 E_in,H_in;
            
            plane_wave(lambda,1.0,Pi-theta,phi,polar,(i+0.5)*Dx,(j+0.0)*Dy,(k+0.0)*Dz,0,E_in,H_in);
            F_guess(index_Ex(i,j,k))=E_in.x;
            
            plane_wave(lambda,1.0,Pi-theta,phi,polar,(i+0.0)*Dx,(j+0.5)*Dy,(k+0.0)*Dz,0,E_in,H_in);
            F_guess(index_Ey(i,j,k))=E_in.y;
            
            plane_wave(lambda,1.0,Pi-theta,phi,polar,(i+0.0)*Dx,(j+0.0)*Dy,(k+0.5)*Dz,0,E_in,H_in);
            F_guess(index_Ez(i,j,k))=E_in.z;
            
            plane_wave(lambda,1.0,Pi-theta,phi,polar,(i+0.0)*Dx,(j+0.5)*Dy,(k+0.5)*Dz,0,E_in,H_in);
            F_guess(index_Hx(i,j,k))=H_in.x;
            
            plane_wave(lambda,1.0,Pi-theta,phi,polar,(i+0.5)*Dx,(j+0.0)*Dy,(k+0.5)*Dz,0,E_in,H_in);
            F_guess(index_Hy(i,j,k))=H_in.y;
            
            plane_wave(lambda,1.0,Pi-theta,phi,polar,(i+0.5)*Dx,(j+0.5)*Dy,(k+0.0)*Dz,0,E_in,H_in);
            F_guess(index_Hz(i,j,k))=H_in.z;
        }
        
        solve_BiCGSTAB(W_mat,F,b,F_guess);
    }
}
