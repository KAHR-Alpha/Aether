-- 181018

set_dielectric()

epsilon_infty(1.0)
add_drude(8.07619e15,5.83082e14)
add_crit_point(7.65938,6.5286e15,-0.304772,5.57763e15)
add_crit_point(34.3229,1.22502e15,0.302888,1.34603e15)

-- enable_pcrc2()

validity_range(300e-9,2500e-9)