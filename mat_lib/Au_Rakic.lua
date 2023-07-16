description('Gold fit from Rakic et al using a Drude-Lorentz model between 250nm and 6.2 microns\nRakic, A. D.; Djurisic, A. B.; Elazar, J. M. & Majewski, M. L.\nOptical Properties of Metallic Films for Vertical-Cavity Optoelectronic Devices\nAppl. Opt., OSA, 1998, 37, 5271-5283')

name("Gold - 250nm to 6200nm - Rakic")

ih=1.0/6.58211951440e-16

wp=9.03

C1=wp/(0.415)
C2=wp/(0.830)
C3=wp/(2.969)
C4=wp/(4.304)
C5=wp/(13.32)

epsilon_infty(1)
add_drude(math.sqrt(0.760)*wp*ih,0.053*ih)
add_lorentz(0.024*C1*C1,0.415*ih,0.241*ih)
add_lorentz(0.010*C2*C2,0.830*ih,0.345*ih)
add_lorentz(0.071*C3*C3,2.969*ih,0.870*ih)
add_lorentz(0.601*C4*C4,4.304*ih,2.494*ih)
add_lorentz(4.384*C5*C5,13.32*ih,2.214*ih)

validity_range(250e-9,6200e-9)