lx=300e-9
ly=300e-9
lz=150e-9

default_material(0)
add_layer("Z",0,50e-9,1)
add_layer("Z",50e-9,70e-9,2)
add_block(105e-9,235e-9,0,35e-9,50e-9,70e-9,0)
add_block(0,35e-9,105e-9,235e-9,50e-9,70e-9,0)
