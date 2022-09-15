f_TE=MODE("fdtd_normal")
f_TE:prefix("Nrods_nTE_");
f_TE:polarization("TE");
f_TE:structure("samples/structures/nanorods_grid.lua");
f_TE:Dxyz(5e-9)
f_TE:N_tsteps(20000);
f_TE:spectrum(1000e-9,2000e-9,2001);
f_TE:pml_zp(25,25,1,0.2)
f_TE:pml_zm(25,25,0.5,0.2);
f_TE:material(0,const_material(1.0))
f_TE:material(1,const_material(3.42)) -- Silicon
f_TE:material(2,"mat_lib/Au_1m_5m_Vial.lua")

f_TM=MODE("fdtd_normal")
f_TM:prefix("Nrods_nTM_");
f_TM:polarization("TM");
f_TM:structure("samples/structures/nanorods_grid.lua");
f_TM:Dxyz(5e-9)
f_TM:N_tsteps(20000);
f_TM:spectrum(1000e-9,2000e-9,2001);
f_TM:pml_zp(25,25,1,0.2)
f_TM:pml_zm(25,25,0.5,0.2);
f_TM:material(0,const_material(1.0))
f_TM:material(1,const_material(3.42))
f_TM:material(2,"mat_lib/Au_1m_5m_Vial.lua")

-- Compl

f_TE=MODE("fdtd_normal")
f_TE:prefix("Nrods_nTE_C_");
f_TE:polarization("TE");
f_TE:structure("samples/structures/nanorods_grid_neg.lua");
f_TE:Dxyz(5e-9)
f_TE:N_tsteps(20000);
f_TE:spectrum(1000e-9,2000e-9,2001);
f_TE:pml_zp(25,25,1,0.2)
f_TE:pml_zm(25,25,0.5,0.2);
f_TE:material(0,const_material(1.0))
f_TE:material(1,const_material(3.42))
f_TE:material(2,"mat_lib/Au_1m_5m_Vial.lua")
