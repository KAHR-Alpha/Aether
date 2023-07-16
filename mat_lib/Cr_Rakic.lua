-- Fit Rakic Chromium

name("Chromium - Rakic")

ih=1.0/6.58211951440e-16

wp=10.75

C1=wp/(0.121)
C2=wp/(0.543)
C3=wp/(1.970)
C4=wp/(8.775)

epsilon_infty(1)
add_drude(math.sqrt(0.168)*wp*ih,0.047*ih)
add_lorentz(0.151*C1*C1,0.121*ih,3.175*ih)
add_lorentz(0.150*C2*C2,0.543*ih,1.305*ih)
add_lorentz(1.149*C3*C3,1.970*ih,2.676*ih)
add_lorentz(0.825*C4*C4,8.775*ih,1.335*ih)