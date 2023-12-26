Material_0=Material()
Material_1=Material()
Material_2=Material()

Material_0:refractive_index(1)
Material_0:name("Air")

Material_1:refractive_index(3.42)
Material_1:name("Silicon")

Material_2:name("IR Gold")
Material_2:load_script("mat_lib/Au_1m_5m_Vial.lua")

structure=Structure("samples/fdtd/structures/nanorods_grid.lua")

f_TE=MODE("fdtd_normal")
f_TE:prefix("Nrods_nTE_");
f_TE:polarization("TE");
f_TE:structure(structure);
f_TE:Dxyz(5e-9)
f_TE:N_tsteps(20000);
f_TE:spectrum(1000e-9,2000e-9,2001);
f_TE:pml_zp(25,25,1,0.2)
f_TE:pml_zm(25,25,0.5,0.2);
f_TE:material(0,Material_0)
f_TE:material(1,Material_1)
f_TE:material(2,Material_2)

f_TE:compute()

f_TM=MODE("fdtd_normal")
f_TM:prefix("Nrods_nTM_");
f_TM:polarization("TM");
f_TM:structure(structure);
f_TM:Dxyz(5e-9)
f_TM:N_tsteps(20000);
f_TM:spectrum(1000e-9,2000e-9,2001);
f_TM:pml_zp(25,25,1,0.2)
f_TM:pml_zm(25,25,0.5,0.2);
f_TM:material(0,Material_0)
f_TM:material(1,Material_1)
f_TM:material(2,Material_2)

f_TM:compute()

-- Compl

f_TE=MODE("fdtd_normal")
f_TE:prefix("Nrods_nTE_C_");
f_TE:polarization("TE");
f_TE:structure(structure);
f_TE:Dxyz(5e-9)
f_TE:N_tsteps(20000);
f_TE:spectrum(1000e-9,2000e-9,2001);
f_TE:pml_zp(25,25,1,0.2)
f_TE:pml_zm(25,25,0.5,0.2);
f_TE:material(0,Material_0)
f_TE:material(1,Material_1)
f_TE:material(2,Material_2)

f_TE:compute()