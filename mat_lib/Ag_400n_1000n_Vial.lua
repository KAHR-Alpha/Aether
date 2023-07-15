description('Silver fit by Alexandre Vial between 400nm and 1 micron, from Palik data\n\nVial: Bubendorff, J. & Lei, F. (Eds.), Use of the critical points model as law of dispersion for the modeling of plasmonic structures using the FDTD method, 2, Transworld Research Network, 2008, 23 - 46')

name("Silver - 400nm to 1000nm - Vial")

epsilon_infty(3.7325)
add_drude(1.3354e16,9.6875e13)
add_crit_point(2.0297,4.5932e17,-0.70952,1.0524e18)
add_crit_point(-2.8925,4.7711e16,-1.4459,3.0719e15)

validity_range(400e-9,1000e-9)