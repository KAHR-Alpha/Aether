description('Copper fit by Alexandre Vial between 400nm and 1 micron, from Palik data\nVial: Bubendorff, J. & Lei, F. (Eds.), Use of the critical points model as law of dispersion for the modeling of plasmonic structures using the FDTD method, 2, Transworld Research Network, 2008, 23 - 46')

name("Copper - 400nm to 1000nm - Vial")

epsilon_infty(1.8097)
add_drude(1.3209e16,1.4086e14)
add_crit_point(0.54253,3.2858e15,-1.5611,4.0476e14)
add_crit_point(5.4965,1.1316e16,0.79163,4.7327e15)

validity_range(400e-9,1000e-9)