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

#include <bitmap3.h>
#include <fieldblock_holder.h>
#include <lua_fdtd.h>
#include <string_tools.h>

void quick_print_fmap(Grid2<Imdouble>& fx, Grid2<Imdouble>& fy, Grid2<Imdouble>& fz, std::string fname)
{
    int i, j;
    int span1 = fx.L1();
    int span2 = fx.L2();

    Grid2<double> map(span1, span2, 0);
    Grid2<double> mapx(span1, span2, 0);
    Grid2<double> mapy(span1, span2, 0);
    Grid2<double> mapz(span1, span2, 0);

    for (i = 0; i < span1; i++) {
        for (j = 0; j < span2; j++)
        {
            using std::abs;
            using std::real;
            using std::imag;
            using std::sqrt;
            using std::norm;

            map(i, j) = sqrt(norm(fx(i, j)) + norm(fy(i, j)) + norm(fz(i, j)));
            mapx(i, j) = abs(fx(i, j));
            mapy(i, j) = abs(fy(i, j));
            mapz(i, j) = abs(fz(i, j));
        }
    }

    // Normalizations

    using std::abs;

    double c_norm = 0;
    int c_norm_N = 0;

    for (i = 0; i < span1; i++) {
        for (j = 0; j < span2; j++)
        {
            c_norm += mapx(i, j) * mapx(i, j); c_norm_N++;
            c_norm += mapy(i, j) * mapy(i, j); c_norm_N++;
            c_norm += mapz(i, j) * mapz(i, j); c_norm_N++;
        }
    }

    c_norm = std::sqrt(c_norm / c_norm_N);
    //    std::cout<<c_norm<<std::endl;
    c_norm = 550.0;

    double expo = 3.0;

    expo = 0.8;

    for (i = 0; i < span1; i++) {
        for (j = 0; j < span2; j++)
        {
            map(i, j) = 1.0 - std::exp(-map(i, j) / c_norm / expo);
            mapx(i, j) = 1.0 - std::exp(-mapx(i, j) / c_norm / expo);
            mapy(i, j) = 1.0 - std::exp(-mapy(i, j) / c_norm / expo);
            mapz(i, j) = 1.0 - std::exp(-mapz(i, j) / c_norm / expo);
        }
    }

    // Writing

    Bitmap fmap(2 * span1 + 4, 4 * span2 + 12);

    fmap.set_full(0, 0, 0);

    for (i = 0; i < span1; i++) {
        for (j = 0; j < span2; j++)
        {
            fmap.degra(i, j + 3 * span2 + 12, mapx(i, j), 0, 1);
            fmap.degra(i, j + 2 * span2 + 8, mapy(i, j), 0, 1);
            fmap.degra(i, j + span2 + 4, mapz(i, j), 0, 1);
            fmap.degra(i, j, map(i, j), 0, 1);

            fmap.degra_circ(i + span1 + 4, j + 3 * span2 + 12, std::arg(fx(i, j)), -Pi, Pi);
            fmap.degra_circ(i + span1 + 4, j + 2 * span2 + 8, std::arg(fy(i, j)), -Pi, Pi);
            fmap.degra_circ(i + span1 + 4, j + span2 + 4, std::arg(fz(i, j)), -Pi, Pi);
            fmap.degra_circ(i + span1 + 4, j, -Pi + 2.0 * Pi * i / (span1 - 1.0), -Pi, Pi);
        }
    }

    fmap.write(std::string(fname).append(".png"));
}


//###########################
//   FieldBlock processing
//###########################


void Fblock_treat_mode::add_map_extraction(std::string field_str, std::string dir_str, int index, std::string fname)
{
    int field = 0, dir = 0;

    if (field_str == "E") field = E_FIELD;
    else if (field_str == "H") field = H_FIELD;
    else if (field_str == "S") field = S_FIELD;
    else
    {
        std::cout << "Unrecognized field " << field_str << std::endl;
    }

    dir = dir_to_int(dir_str);
    if (dir == NORMAL_UNDEF)
    {
        std::cout << "Unrecognized orientation " << dir_str << std::endl;
    }

    add_map_extraction_sub(field, dir, index, fname);
}

void Fblock_treat_mode::add_map_extraction_sub(int dir, int field, int index, std::string const& fname)
{
    Extract extract;

    extract.dir = dir;
    extract.field = field;
    extract.index = index;

    extract.fname = fname;

    map.push_back(extract);
}

void Fblock_treat_mode::process()
{
    int i, j, k;

    FieldBlockHolder holder;
    holder.load(fname);
    holder.set_baseline(baseline);

    int Nx = holder.Nx;
    int Ny = holder.Ny;
    int Nz = holder.Nz;

    Grid3<Imdouble>& Ex = holder.Ex;
    Grid3<Imdouble>& Ey = holder.Ey;
    Grid3<Imdouble>& Ez = holder.Ez;

    Grid3<Imdouble>& Hx = holder.Hx;
    Grid3<Imdouble>& Hy = holder.Hy;
    Grid3<Imdouble>& Hz = holder.Hz;

    Grid3<double> Px(Nx, Ny, Nz), Py(Nx, Ny, Nz), Pz(Nx, Ny, Nz);

    for (i = 0; i < Nx; i++) for (j = 0; j < Ny; j++) for (k = 0; k < Nz; k++)
    {
        using std::real;
        using std::conj;

        Px(i, j, k) = 0.5 * real(Ey(i, j, k) * conj(Hz(i, j, k)) - Ez(i, j, k) * conj(Hy(i, j, k)));
        Py(i, j, k) = 0.5 * real(Ez(i, j, k) * conj(Hx(i, j, k)) - Ex(i, j, k) * conj(Hz(i, j, k)));
        Pz(i, j, k) = 0.5 * real(Ex(i, j, k) * conj(Hy(i, j, k)) - Ey(i, j, k) * conj(Hx(i, j, k)));
    }

    // Fieldmap extraction

    for (unsigned int l = 0; l < map.size(); l++)
    {
        Extract& current_map = map[l];

        int span1 = 0, span2 = 0;

        if (current_map.dir == NORMAL_X) { span1 = Ny; span2 = Nz; }
        else if (current_map.dir == NORMAL_Y) { span1 = Nx; span2 = Nz; }
        else if (current_map.dir == NORMAL_Z) { span1 = Nx; span2 = Ny; }

        Grid2<Imdouble> Gx(span1, span2), Gy(span1, span2),
            Gz(span1, span2), tHx(span1, span2),
            tHy(span1, span2), tHz(span1, span2);
        Grid2<unsigned int> mats(span1, span2);
        int index = current_map.index;

        if (current_map.dir == NORMAL_X)
        {
            for (j = 0; j < Ny; j++) for (k = 0; k < Nz; k++)
                mats(j, k) = holder.mats(index, j, k);

            if (current_map.field == E_FIELD)
                for (j = 0; j < Ny; j++) for (k = 0; k < Nz; k++)
                {
                    Gx(j, k) = Ex(index, j, k); Gy(j, k) = Ey(index, j, k); Gz(j, k) = Ez(index, j, k);
                }
            else if (current_map.field == H_FIELD)
                for (j = 0; j < Ny; j++) for (k = 0; k < Nz; k++)
                {
                    Gx(j, k) = Hx(index, j, k); Gy(j, k) = Hy(index, j, k); Gz(j, k) = Hz(index, j, k);
                }
            else if (current_map.field == S_FIELD)
                for (j = 0; j < Ny; j++) for (k = 0; k < Nz; k++)
                {
                    Gx(j, k) = Px(index, j, k); Gy(j, k) = Py(index, j, k); Gz(j, k) = Pz(index, j, k);
                }
        }
        else if (current_map.dir == NORMAL_Y)
        {
            for (i = 0; i < Nx; i++) for (k = 0; k < Nz; k++)
                mats(i, k) = holder.mats(i, index, k);

            if (current_map.field == E_FIELD)
                for (i = 0; i < Nx; i++) for (k = 0; k < Nz; k++)
                {
                    Gx(i, k) = Ex(i, index, k); Gy(i, k) = Ey(i, index, k); Gz(i, k) = Ez(i, index, k);
                }
            else if (current_map.field == H_FIELD)
                for (i = 0; i < Nx; i++) for (k = 0; k < Nz; k++)
                {
                    Gx(i, k) = Hx(i, index, k); Gy(i, k) = Hy(i, index, k); Gz(i, k) = Hz(i, index, k);
                }
            else if (current_map.field == S_FIELD)
                for (i = 0; i < Nx; i++) for (k = 0; k < Nz; k++)
                {
                    Gx(i, k) = Px(i, index, k); Gy(i, k) = Py(i, index, k); Gz(i, k) = Pz(i, index, k);
                }
        }
        else if (current_map.dir == NORMAL_Z)
        {
            for (i = 0; i < Nx; i++) for (j = 0; j < Ny; j++)
                mats(i, j) = holder.mats(i, j, index);

            if (current_map.field == E_FIELD)
                for (i = 0; i < Nx; i++) for (j = 0; j < Ny; j++)
                {
                    Gx(i, j) = Ex(i, j, index); Gy(i, j) = Ey(i, j, index); Gz(i, j) = Ez(i, j, index);
                }
            else if (current_map.field == H_FIELD)
                for (i = 0; i < Nx; i++) for (j = 0; j < Ny; j++)
                {
                    Gx(i, j) = Hx(i, j, index); Gy(i, j) = Hy(i, j, index); Gz(i, j) = Hz(i, j, index);
                }
            else if (current_map.field == S_FIELD)
                for (i = 0; i < Nx; i++) for (j = 0; j < Ny; j++)
                {
                    Gx(i, j) = Px(i, j, index); Gy(i, j) = Py(i, j, index); Gz(i, j) = Pz(i, j, index);
                }
        }

        if (current_map.field != S_FIELD)
        {
            fmap_script(current_map.fname, current_map.field);
            fmap_raw(current_map.fname, current_map.field, Gx, Gy, Gz);
            fmap_mats_raw(current_map.fname, mats);
        }
        else
        {
            fmap_script(current_map.fname, current_map.field, true);
            fmap_raw(current_map.fname, current_map.field, Gx, Gy, Gz, true);
            fmap_mats_raw(current_map.fname, mats);
        }
    }

    if (blender_output)
    {
        std::string fname_blender = fname;
        fname_blender.append("_blender");

        std::ofstream file_b(fname_blender, std::ios::out | std::ios::trunc | std::ios::binary);

        int Nf = 1;

        file_b.write(reinterpret_cast<char*>(&Nx), sizeof(int));
        file_b.write(reinterpret_cast<char*>(&Ny), sizeof(int));
        file_b.write(reinterpret_cast<char*>(&Nz), sizeof(int));
        file_b.write(reinterpret_cast<char*>(&Nf), sizeof(int));

        double Emax = 0;
        float Ef;

        for (k = 0; k < Nz; k++) {
            for (j = 0; j < Ny; j++) {
                for (i = 0; i < Nx; i++)
                {
                    double E = std::real(Ex(i, j, k) * conj(Ex(i, j, k)) +
                        Ey(i, j, k) * conj(Ey(i, j, k)) +
                        Ez(i, j, k) * conj(Ez(i, j, k)));

                    E = std::sqrt(E);

                    Emax = std::max(E, Emax);
                }
            }
        }

        for (k = 0; k < Nz; k++) {
            for (j = 0; j < Ny; j++) {
                for (i = 0; i < Nx; i++)
                {
                    double E = std::real(Ex(i, j, k) * conj(Ex(i, j, k)) +
                        Ey(i, j, k) * conj(Ey(i, j, k)) +
                        Ez(i, j, k) * conj(Ez(i, j, k)));

                    E = std::sqrt(E) / Emax;
                    E = 1.0 - std::exp(-1 * E);

                    Ef = static_cast<float>(E);

                    file_b.write(reinterpret_cast<char*>(&Ef), sizeof(float));
                }
            }
        }
    }

    if (surface_poynting_compute)
    {
        double Spx = 0, Spy = 0, Spz = 0;
        Imdouble px, py, pz;

        std::cout << "Surface poynting" << std::endl;

        i = 0;
        Spx = Spy = Spz = 0;
        for (j = 0; j < Ny; j++) for (k = 0; k < Nz; k++)
        {
            px = Ey(i, j, k) * conj(Hz(i, j, k)) - Ez(i, j, k) * conj(Hy(i, j, k));
            py = Ez(i, j, k) * conj(Hx(i, j, k)) - Ex(i, j, k) * conj(Hz(i, j, k));
            pz = Ex(i, j, k) * conj(Hy(i, j, k)) - Ey(i, j, k) * conj(Hx(i, j, k));

            //std::cout<<Ey(i,j,k)<<" "<<Hz(i,j,k)<<std::endl;

            Spx += 0.5 * std::real(px);
            Spy += 0.5 * std::real(py);
            Spz += 0.5 * std::real(pz);
        }
        std::cout << "X-: " << Spx << " " << Spy << " " << Spz << std::endl;

        i = Nx - 1;
        i = 1;
        Spx = Spy = Spz = 0;
        for (j = 0; j < Ny; j++) for (k = 0; k < Nz; k++)
        {
            px = Ey(i, j, k) * conj(Hz(i, j, k)) - Ez(i, j, k) * conj(Hy(i, j, k));
            py = Ez(i, j, k) * conj(Hx(i, j, k)) - Ex(i, j, k) * conj(Hz(i, j, k));
            pz = Ex(i, j, k) * conj(Hy(i, j, k)) - Ey(i, j, k) * conj(Hx(i, j, k));

            //std::cout<<Ey(i,j,k)<<" "<<Hz(i,j,k)<<std::endl;

            Spx += 0.5 * std::real(px);
            Spy += 0.5 * std::real(py);
            Spz += 0.5 * std::real(pz);
        }
        std::cout << "X+: " << Spx << " " << Spy << " " << Spz << std::endl;

        j = 0;
        Spx = Spy = Spz = 0;
        for (i = 0; i < Nx; i++) for (k = 0; k < Nz; k++)
        {
            px = Ey(i, j, k) * conj(Hz(i, j, k)) - Ez(i, j, k) * conj(Hy(i, j, k));
            py = Ez(i, j, k) * conj(Hx(i, j, k)) - Ex(i, j, k) * conj(Hz(i, j, k));
            pz = Ex(i, j, k) * conj(Hy(i, j, k)) - Ey(i, j, k) * conj(Hx(i, j, k));

            Spx += 0.5 * std::real(px);
            Spy += 0.5 * std::real(py);
            Spz += 0.5 * std::real(pz);
        }
        std::cout << "Y-: " << Spx << " " << Spy << " " << Spz << std::endl;

        j = Ny - 1;
        Spx = Spy = Spz = 0;
        for (i = 0; i < Nx; i++) for (k = 0; k < Nz; k++)
        {
            px = Ey(i, j, k) * conj(Hz(i, j, k)) - Ez(i, j, k) * conj(Hy(i, j, k));
            py = Ez(i, j, k) * conj(Hx(i, j, k)) - Ex(i, j, k) * conj(Hz(i, j, k));
            pz = Ex(i, j, k) * conj(Hy(i, j, k)) - Ey(i, j, k) * conj(Hx(i, j, k));

            Spx += 0.5 * std::real(px);
            Spy += 0.5 * std::real(py);
            Spz += 0.5 * std::real(pz);
        }
        std::cout << "Y+: " << Spx << " " << Spy << " " << Spz << std::endl;

        k = 0;
        Spx = Spy = Spz = 0;
        for (i = 0; i < Nx; i++) for (j = 0; j < Ny; j++)
        {
            px = Ey(i, j, k) * conj(Hz(i, j, k)) - Ez(i, j, k) * conj(Hy(i, j, k));
            py = Ez(i, j, k) * conj(Hx(i, j, k)) - Ex(i, j, k) * conj(Hz(i, j, k));
            pz = Ex(i, j, k) * conj(Hy(i, j, k)) - Ey(i, j, k) * conj(Hx(i, j, k));

            Spx += 0.5 * std::real(px);
            Spy += 0.5 * std::real(py);
            Spz += 0.5 * std::real(pz);
        }
        std::cout << "Z-: " << Spx << " " << Spy << " " << Spz << std::endl;

        k = Nz - 1;
        Spx = Spy = Spz = 0;
        for (i = 0; i < Nx; i++) for (j = 0; j < Ny; j++)
        {
            px = Ey(i, j, k) * conj(Hz(i, j, k)) - Ez(i, j, k) * conj(Hy(i, j, k));
            py = Ez(i, j, k) * conj(Hx(i, j, k)) - Ex(i, j, k) * conj(Hz(i, j, k));
            pz = Ex(i, j, k) * conj(Hy(i, j, k)) - Ey(i, j, k) * conj(Hx(i, j, k));

            Spx += 0.5 * std::real(px);
            Spy += 0.5 * std::real(py);
            Spz += 0.5 * std::real(pz);
        }
        std::cout << "Z+: " << Spx << " " << Spy << " " << Spz << std::endl;
    }
}

//#########################
//   Fieldmap processing
//#########################

void Fmap_treat_mode::process()
{
    int i, j;

    double lambda;
    int type;
    int span1, span2, x1, z1;
    double Dx, Dy, Dz;

    std::ifstream file(fname, std::ios::in | std::ios::binary);
    chk_var(file.is_open());

    file.read(reinterpret_cast<char*>(&lambda), sizeof(double));
    file.read(reinterpret_cast<char*>(&type), sizeof(int));

    file.read(reinterpret_cast<char*>(&x1), sizeof(int));
    file.read(reinterpret_cast<char*>(&span1), sizeof(int));

    file.read(reinterpret_cast<char*>(&z1), sizeof(int));
    file.read(reinterpret_cast<char*>(&span2), sizeof(int));

    file.read(reinterpret_cast<char*>(&Dx), sizeof(double));
    file.read(reinterpret_cast<char*>(&Dy), sizeof(double));
    file.read(reinterpret_cast<char*>(&Dz), sizeof(double));

    chk_var(lambda);
    chk_var(span1);
    chk_var(span2);

    Grid2<Imdouble> fx(span1, span2), fy(span1, span2), fz(span1, span2);

    double p_r, p_i;

    for (i = 0; i < span1; i++) for (j = 0; j < span2; j++)
    {
        file.read(reinterpret_cast<char*>(&p_r), sizeof(double));
        file.read(reinterpret_cast<char*>(&p_i), sizeof(double));
        fx(i, j).real(p_r / baseline); fx(i, j).imag(p_i / baseline);

        file.read(reinterpret_cast<char*>(&p_r), sizeof(double));
        file.read(reinterpret_cast<char*>(&p_i), sizeof(double));
        fy(i, j).real(p_r / baseline); fy(i, j).imag(p_i / baseline);

        file.read(reinterpret_cast<char*>(&p_r), sizeof(double));
        file.read(reinterpret_cast<char*>(&p_i), sizeof(double));
        fz(i, j).real(p_r / baseline); fz(i, j).imag(p_i / baseline);
    }

    fmap_script(fname, E_FIELD);
    fmap_raw(fname, E_FIELD, fx, fy, fz);

    //    double kn=2.0*Pi/lambda;
    //    double w=2.0*Pi*c_light/lambda;
    //    double kx,ky;
    //    
    //    int Nk=401;
    //    
    //    Grid2<Imdouble> Sp_x(Nk,Nk,0),
    //                    Sp_y(Nk,Nk,0),
    //                    Sp_z(Nk,Nk,0);
    //    
    //    ProgDisp dsp(Nk*Nk);
    //    
    //    for(k=0;k<Nk;k++) for(l=0;l<Nk;l++)
    //    {
    ////        kx=-kn+2.0*kn*k/(Nk-1.0);
    ////        ky=-kn+2.0*kn*l/(Nk-1.0);
    //        
    //        kx=kn*k/(Nk-1.0);
    //        ky=kn*l/(Nk-1.0);
    //        
    //        for(i=0;i<span1;i++)
    //        {
    //            Imdouble coeff_x=std::exp(-i*Dx*w*kx*Im);
    //            
    //            for(j=0;j<span2;j++)
    //            {
    //                Imdouble coeff_y=std::exp(-j*Dy*w*ky*Im);
    //                
    //                Sp_x(k,l)+=fx(i,j)*coeff_x*coeff_y;
    //                Sp_y(k,l)+=fy(i,j)*coeff_x*coeff_y;
    //                Sp_z(k,l)+=fz(i,j)*coeff_x*coeff_y;
    //            }
    //        }
    //        
    //        double Lx=span1*Dx;
    //        double Ly=span2*Dx;
    //        
    //        int N=20;
    //        Imdouble kmod_x=0,kmod_y=0;
    //        
    //        for(i=-N;i<=N;i++)
    //        {
    //            kmod_x+=exp(-i*Lx*kx*Im);
    //            kmod_y+=exp(-i*Ly*ky*Im);
    //        }
    //        
    //        Sp_x(k,l)*=kmod_x*kmod_y;
    //        Sp_y(k,l)*=kmod_x*kmod_y;
    //        Sp_z(k,l)*=kmod_x*kmod_y;
    //        
    //        ++dsp;
    //    }
    //    
    //    Grid2<double> img(Nk,Nk);
    //    
    //    for(k=0;k<Nk;k++) for(l=0;l<Nk;l++)
    //    {
    //        img(k,l)=std::log10(std::abs(Sp_x(k,l)))
    //                +std::log10(std::abs(Sp_y(k,l)))
    //                +std::log10(std::abs(Sp_z(k,l)));
    //    }
    //    
    //    for(k=0;k<Nk;k++) for(l=0;l<Nk;l++)
    //    {
    //        img(k,l)=std::log10(std::abs(Sp_x(k,l))+std::abs(Sp_x(l,k)))
    //                +std::log10(std::abs(Sp_y(k,l))+std::abs(Sp_y(l,k)))
    //                +std::log10(std::abs(Sp_z(k,l))+std::abs(Sp_z(l,k)));
    //    }
    //    
    //    G2_to_degra(img,"test.bmp");
}
