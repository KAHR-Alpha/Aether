declare_parameter("p",200e-9)
declare_parameter("r",50e-9)
declare_parameter("h",50e-9)

lx=p
ly=p
lz=200e-9+h

default_material(0)
add_cylinder(p/2.0,p/2.0,100e-9,0,0,h,r,2)
add_layer("Z",0,100e-9,1)
add_coating(60,20,20e-9,1e-9,0,3)

loop(1,1,0)