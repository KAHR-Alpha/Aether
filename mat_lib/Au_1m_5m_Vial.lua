-- Fit d'Alexandre entre 1 et 5 microns

set_dielectric()

epsilon_infty(6.0031)
add_drude(1.2772e16,3.4578e13)

evaluate(1000,1000e-9,5000e-9)