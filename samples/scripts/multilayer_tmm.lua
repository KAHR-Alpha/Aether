ml=MODE("multilayer_tmm")
ml:compute_angle(45)
ml:spectrum(400e-9,1000e-9,3001)
ml:substrate(const_material(1.5))
ml:superstrate(const_material(1.0))
ml:output("ml_test")
ml:add_layer(50e-9,"mat_lib/Au_400n_1000n_Vial.lua")

ml=MODE("multilayer_tmm")
ml:spectrum(400e-9,800e-9,401)
ml:substrate(const_material(1.5))
ml:superstrate(const_material(1.0))
ml:add_layer(10e-9,"mat_lib/Ag_200n_1000n_Vial.lua")
ml:add_layer(150e-9,const_material(3.42))
ml:output("ml_test")
ml:compute_guided("TM",500e-9,3,0)

for i=1,10 do
	h=i*5
	hn=h*1e-9
	
	output="Al_R_layer_" .. h .. "_nm"
	
	ml=MODE("multilayer_tmm")
	ml:compute_angle(0)
	ml:spectrum(300e-9,900e-9,601)
	ml:substrate(const_material(1.6))
	ml:superstrate(const_material(1.0))
	ml:output(output)
	ml:add_layer(hn,"mat_lib/Al_Rakic.lua")
end