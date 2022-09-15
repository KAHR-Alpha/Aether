
for r=20,100,10 do
	mie=MODE("mie")
	mie:radius(r*1e-9)
	mie:environment_index(1.0)
	mie:material("mat_lib/Au_400n_1000n_Vial.lua")
	mie:cext("mie_Au_Cext_r_" .. r,300e-9,1200e-9,901)
end