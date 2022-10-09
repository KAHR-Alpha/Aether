description('Chromium fit by Alexandre Vial between 400nm and 1 micron, from Palik data\nVial: Bubendorff, J. & Lei, F. (Eds.), Use of the critical points model as law of dispersion for the modeling of plasmonic structures using the FDTD method, 2, Transworld Research Network, 2008, 23 - 46')



epsilon_infty(1.1297)
add_drude(8.8128e15,3.8828e14)
add_crit_point(33.086,1.7398e15,-0.25722,1.6329e15)
add_crit_point(1.6592,3.7925e15,0.83533,7.3567e14)

validity_range(400e-9,1000e-9)