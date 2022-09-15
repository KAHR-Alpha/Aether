description('Aluminium fit by Loïc Le Cunff between 300nm et 1 micron, from Luxpop data (Palik)\nhttp://www.luxpop.com/')

set_dielectric()

epsilon_infty(1)
add_drude(2.03216e16,1.08818e14)
add_lorentz(10.9907,2.3734e15,7.36794e14)
add_crit_point(3.83999,2.76322e15,-0.258865,1.05348e15)

validity_range(300e-9,1000e-9)