description('Nickel fit from Rakic et al using a Drude-Lorentz model between 300nm and 6.2 microns\nRakic, A. D.; Djurisic, A. B.; Elazar, J. M. & Majewski, M. L.\nOptical Properties of Metallic Films for Vertical-Cavity Optoelectronic Devices\nAppl. Opt., OSA, 1998, 37, 5271-5283')



ih=1.0/6.58211951440e-16

wp=15.92

C1=wp/(0.174)
C2=wp/(0.582)
C3=wp/(1.597)
C4=wp/(6.089)

epsilon_infty(1)
add_drude(math.sqrt(0.096)*wp*ih,0.048*ih)
add_lorentz(0.100*C1*C1,0.174*ih,4.511*ih)
add_lorentz(0.135*C2*C2,0.582*ih,1.334*ih)
add_lorentz(0.106*C3*C3,1.597*ih,2.178*ih)
add_lorentz(0.729*C4*C4,6.089*ih,6.292*ih)

validity_range(300e-9,6200e-9)