structure=Structure("../../samples/fdtd/structures/basic_parametric.lua")

fdtd=MODE("fdtd_normal")
fdtd:structure(structure)

lx=fdtd:Lx()
ly=fdtd:Ly()
lz=fdtd:Lz()

if lx~=200e-9 or ly~=300e-9 or lz~=400e-9 then
	print("Failed initial value")
	print("lx " .. lx)
	print("ly " .. ly)
	print("lz " .. lz)
	fail_test()
end

structure:parameter("px",220e-9)
structure:parameter("py",330e-9)
structure:parameter("pz",440e-9)

fdtd=MODE("fdtd_normal")
fdtd:structure(structure)

lx=fdtd:Lx()
ly=fdtd:Ly()
lz=fdtd:Lz()

if lx~=220e-9 or ly~=330e-9 or lz~=440e-9 then
	print("Failed modification")
	print("lx " .. lx)
	print("ly " .. ly)
	print("lz " .. lz)
	fail_test()
end