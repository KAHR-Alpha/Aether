description('Titanium fit from Rakic et al using a Drude-Lorentz model between 470nm and 20 microns\nRakic, A. D.; Djurisic, A. B.; Elazar, J. M. & Majewski, M. L.\nOptical Properties of Metallic Films for Vertical-Cavity Optoelectronic Devices\nAppl. Opt., OSA, 1998, 37, 5271-5283')

set_dielectric()

ih=1.0/6.58211951440e-16

wp=7.29

C1=wp/(0.777)
C2=wp/(1.545)
C3=wp/(2.509)
C4=wp/(19.43)

epsilon_infty(1)
add_drude(math.sqrt(0.148)*wp*ih,0.082*ih)
add_lorentz(0.899*C1*C1,0.777*ih,2.276*ih)
add_lorentz(0.393*C2*C2,1.545*ih,2.518*ih)
add_lorentz(0.187*C3*C3,2.509*ih,1.663*ih)
add_lorentz(0.001*C4*C4,19.43*ih,1.762*ih)

validity_range(470e-9,20e-6)