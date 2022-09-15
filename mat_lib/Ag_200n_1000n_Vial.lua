description('Silver fit by Alexandre Vial between 200nm and 1 micron, from Palik data\nVial: Bubendorff, J. & Lei, F. (Eds.), Use of the critical points model as law of dispersion for the modeling of plasmonic structures using the FDTD method, 2, Transworld Research Network, 2008, 23 - 46')

set_dielectric()

epsilon_infty(1.1211)
add_drude(1.3280e16,1.0212e14)
add_crit_point(-1.4234,6.6840e15,2.5575,3.5669e15)
add_crit_point(0.19508,6.1250e15,-1.7398,4.6124e14)

-- enable_pcrc2()

validity_range(200e-9,1000e-9)