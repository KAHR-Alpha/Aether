-- Fit d'Alexandre Ag entre 400nm et 1 micron

name("Silver - 400nm to 1000nm - Lorentz model - Vial")

epsilon_infty(1.7984)
add_drude(1.3359e16,8.7167e13)
add_lorentz(3.0079,8.1635e15,4.3785e17)
add_lorentz(2.3410,3.8316e16,6.0574e16)

-- enable_pcrc2()