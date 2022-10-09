-- Fit d'Alexandre Or entre 400nm et 1 micron



epsilon_infty(1.03)
add_drude(1.3064e16,1.1274e14)
add_crit_point(0.86822,4.0812e15,-0.60756,7.3277e14)
add_crit_point(1.3700,6.4269e15,-0.087341,6.7371e14)

enable_pcrc2()

validity_range(400e-9,1000e-9)