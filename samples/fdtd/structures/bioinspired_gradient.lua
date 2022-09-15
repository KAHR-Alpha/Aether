declare_parameter("p",300e-9)
declare_parameter("h",300e-9)

lx=p
ly=p*math.sqrt(3)
lz=200e-9+h

default_material(0)
add_layer("Z",0,100e-9,1)
add_cone(0,0,100e-9,0,0,h,p/2,1)
add_cone(lx/2,ly/2,100e-9,0,0,h,p/2,1)

loop(1,1,0)