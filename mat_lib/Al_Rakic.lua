description('Aluminium fit from Rakic et al using a Drude-Lorentz model between 100nm and 250 microns\nRakic, A. D.; Djurisic, A. B.; Elazar, J. M. & Majewski, M. L.\nOptical Properties of Metallic Films for Vertical-Cavity Optoelectronic Devices\nAppl. Opt., OSA, 1998, 37, 5271-5283')



ih=1.0/6.58211951440e-16

wp=14.98

C1=wp/(0.162)
C2=wp/(1.544)
C3=wp/(1.808)
C4=wp/(3.473)

epsilon_infty(1)
add_drude(math.sqrt(0.523)*wp*ih,0.047*ih)
add_lorentz(0.227*C1*C1,0.162*ih,0.333*ih)
add_lorentz(0.050*C2*C2,1.544*ih,0.312*ih)
add_lorentz(0.166*C3*C3,1.808*ih,1.351*ih)
add_lorentz(0.030*C4*C4,3.473*ih,3.382*ih)

validity_range(100e-9,250e-6)