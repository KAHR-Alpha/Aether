description('Copper fit from Rakic et al using a Drude-Lorentz model between 200nm and 12.4 microns\nRakic, A. D.; Djurisic, A. B.; Elazar, J. M. & Majewski, M. L.\nOptical Properties of Metallic Films for Vertical-Cavity Optoelectronic Devices\nAppl. Opt., OSA, 1998, 37, 5271-5283')

set_dielectric()

ih=1.0/6.58211951440e-16

wp=10.83

C1=wp/(0.291)
C2=wp/(2.957)
C3=wp/(5.300)
C4=wp/(11.18)

epsilon_infty(1)
add_drude(math.sqrt(0.575)*wp*ih,0.030*ih)
add_lorentz(0.061*C1*C1,0.291*ih,0.378*ih)
add_lorentz(0.104*C2*C2,2.957*ih,1.056*ih)
add_lorentz(0.723*C3*C3,5.300*ih,3.213*ih)
add_lorentz(0.638*C4*C4,11.18*ih,4.305*ih)

validity_range(200e-9,12400e-9)