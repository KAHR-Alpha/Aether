-- Fit Al luxpop entre 1000nm et 2000nm

name("Aluminium - 1000nm to 2000nm - LLC")

epsilon_infty(4.5651)
add_drude(2.0294e16,1.8159e14)
add_lorentz(14.445,2.6105e15,1.4245e15)

validity_range(1000e-9,2000e-9)