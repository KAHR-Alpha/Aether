-- Fit Or luxpop-palik entre 1000nm et 10000nm

name("Gold - 1 micron to 10 microns - LLC")

epsilon_infty(1.04156)
add_drude(1.16733e+016,8.3213e+013)
add_lorentz(50.1004,4.5026e+014,8.90655e+014)
add_lorentz(0.410548,1.15332e+015,6.34925e+014)

validity_range(1000e-9,10000e-9)