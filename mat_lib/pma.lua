-- Fit Al luxpop entre 1000nm et 2000nm

set_dielectric()

epsilon_infty(2.08)
add_lorentz(4.3e-2,3.2e14,1.1e13)

validity_range(1000e-9,10000e-9)