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

#include <fdtd_core.h>

std::mutex cout_mutex;

void FDTD::update_E_simp()
{
    std::unique_lock<std::mutex> lock(alternator_E.get_main_mutex());
    
    allow_run_E=true;
    
    alternator_E.signal_threads();
    
    alternator_E.main_wait_threads(lock);
    
    allow_run_E=false;
}

void FDTD::threaded_process_E(int ID)
{
    std::unique_lock<std::mutex> lock(alternator_E.get_thread_mutex(ID));
    
    threads_ready_E[ID]=true;
    
    alternator_E.thread_wait_ok(ID,lock);
    
    while(allow_run_E)
    {
        // Materials Ante
        
        if(Nx>Nthreads)
        {
            advMats_ante((ID*Nx)/Nthreads,((ID+1)*Nx)/Nthreads);
        }
        else if(ID==0) advMats_ante(0,Nx);
        
        alternator_E.signal_main(ID);
        
        // E Field
        
        alternator_E.thread_wait_ok(ID,lock);
        
        int x1=(ID*Nx)/Nthreads; int x2=((ID+1)*Nx)/Nthreads;
        int y1=(ID*Ny)/Nthreads; int y2=((ID+1)*Ny)/Nthreads;
        int z1=(ID*Nz)/Nthreads; int z2=((ID+1)*Nz)/Nthreads;
        
        if(enable_Ex)
        {
                 if(Nz>Nthreads) advEx(0,Nx,0,Ny,z1,z2);
            else if(Ny>Nthreads) advEx(0,Nx,y1,y2,0,Nz);
            else if(Nx>Nthreads) advEx(x1,x2,0,Ny,0,Nz);
            else if(ID==0) advEx(0,Nx,0,Ny,0,Nz);
        }
        
        if(enable_Ey)
        {
                 if(Nz>Nthreads) advEy(0,Nx,0,Ny,z1,z2);
            else if(Ny>Nthreads) advEy(0,Nx,y1,y2,0,Nz);
            else if(Nx>Nthreads) advEy(x1,x2,0,Ny,0,Nz);
            else if(ID==0) advEy(0,Nx,0,Ny,0,Nz);
        }
        
        if(enable_Ez)
        {
                 if(Nz>Nthreads) advEz(0,Nx,0,Ny,z1,z2);
            else if(Ny>Nthreads) advEz(0,Nx,y1,y2,0,Nz);
            else if(Nx>Nthreads) advEz(x1,x2,0,Ny,0,Nz);
            else if(ID==0) advEz(0,Nx,0,Ny,0,Nz);
        }
        
        alternator_E.signal_main(ID);
        
        // Materials Simp
        
        alternator_E.thread_wait_ok(ID,lock);
        
        if(Nx>Nthreads)
        {
            advMats_simp((ID*Nx)/Nthreads,((ID+1)*Nx)/Nthreads);
        }
        else if(ID==0) advMats_simp(0,Nx);
        
        alternator_E.signal_main(ID);
        
        // Materials Post
        
        alternator_E.thread_wait_ok(ID,lock);
        
        if(Nx>Nthreads)
        {
            advMats_post((ID*Nx)/Nthreads,((ID+1)*Nx)/Nthreads);
        }
        else if(ID==0) advMats_post(0,Nx);
        
        alternator_E.signal_main(ID);
        
        // Materials Post
        
        alternator_E.thread_wait_ok(ID,lock);
        
        if(Nx>Nthreads)
        {
            advMats_self((ID*Nx)/Nthreads,((ID+1)*Nx)/Nthreads);
        }
        else if(ID==0) advMats_self(0,Nx);
        
        alternator_E.signal_main(ID);
        
        // PMLs E
        
        alternator_E.thread_wait_ok(ID,lock);
        
        if(enable_Ex)
        {
            if(Nx>Nthreads) app_pml_Ex((ID*Nx)/Nthreads,((ID+1)*Nx)/Nthreads);
            else if(ID==0) app_pml_Ex(0,Nx);
        }
        
        if(enable_Ey)
        {
            if(Ny>Nthreads) app_pml_Ey((ID*Ny)/Nthreads,((ID+1)*Ny)/Nthreads);
            else if(ID==0) app_pml_Ey(0,Ny);
        }
        
        if(enable_Ez)
        {
            if(Nz>Nthreads) app_pml_Ez((ID*Nz)/Nthreads,((ID+1)*Nz)/Nthreads);
            else if(ID==0) app_pml_Ez(0,Nz);
        }
        
        alternator_E.signal_main(ID);
        
        // Next Loop - Materials Ante
        
        alternator_E.thread_wait_ok(ID,lock);
    }
}

void FDTD::update_H_simp()
{
    std::unique_lock<std::mutex> lock(alternator_H.get_main_mutex());
    
    allow_run_H=true;
    
    alternator_H.signal_threads();
    
    alternator_H.main_wait_threads(lock);
    
    allow_run_H=false;
}

void FDTD::threaded_process_H(int ID)
{
    std::unique_lock<std::mutex> lock(alternator_H.get_thread_mutex(ID));
    
    threads_ready_H[ID]=true;
    
    alternator_H.thread_wait_ok(ID,lock);
    
    while(allow_run_H)
    {
        // H Field
        
        int x1=(ID*Nx)/Nthreads; int x2=((ID+1)*Nx)/Nthreads;
        int y1=(ID*Ny)/Nthreads; int y2=((ID+1)*Ny)/Nthreads;
        int z1=(ID*Nz)/Nthreads; int z2=((ID+1)*Nz)/Nthreads;
        
        if(enable_Hx)
        {
                 if(Nz>Nthreads) advHx(0,Nx,0,Ny,z1,z2);
            else if(Ny>Nthreads) advHx(0,Nx,y1,y2,0,Nz);
            else if(Nx>Nthreads) advHx(x1,x2,0,Ny,0,Nz);
            else if(ID==0) advHx(0,Nx,0,Ny,0,Nz);
        }
        
        if(enable_Hy)
        {
                 if(Nz>Nthreads) advHy(0,Nx,0,Ny,z1,z2);
            else if(Ny>Nthreads) advHy(0,Nx,y1,y2,0,Nz);
            else if(Nx>Nthreads) advHy(x1,x2,0,Ny,0,Nz);
            else if(ID==0) advHy(0,Nx,0,Ny,0,Nz);
        }
        
        if(enable_Hz)
        {
                 if(Nz>Nthreads) advHz(0,Nx,0,Ny,z1,z2);
            else if(Ny>Nthreads) advHz(0,Nx,y1,y2,0,Nz);
            else if(Nx>Nthreads) advHz(x1,x2,0,Ny,0,Nz);
            else if(ID==0) advHz(0,Nx,0,Ny,0,Nz);
        }
        
        alternator_H.signal_main(ID);
        
        // PMLs H
        
        alternator_H.thread_wait_ok(ID,lock);
        
        if(enable_Hx)
        {
            if(Nx>Nthreads) app_pml_Hx((ID*Nx)/Nthreads,((ID+1)*Nx)/Nthreads);
            else if(ID==0) app_pml_Hx(0,Nx);
        }
        
        if(enable_Hy)
        {
            if(Ny>Nthreads) app_pml_Hy((ID*Ny)/Nthreads,((ID+1)*Ny)/Nthreads);
            else if(ID==0) app_pml_Hy(0,Ny);
        }
        
        if(enable_Hz)
        {
            if(Nz>Nthreads) app_pml_Hz((ID*Nz)/Nthreads,((ID+1)*Nz)/Nthreads);
            else if(ID==0) app_pml_Hz(0,Nz);
        }
        
        alternator_H.signal_main(ID);
        
        // Next Loop - H Field
                
        alternator_H.thread_wait_ok(ID,lock);
    }
}
