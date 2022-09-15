material_0=Material()
material_0:refractive_index(1)
material_0:name("Material 0")

material_1=Material()
material_1:refractive_index(1.5)
material_1:name("Material 1")

object_0=Selene_object("box",1,0.1,0.1)
object_1=Selene_object("box",1.2,0.2,0.2)

light_0=Selene_light("point")

object_0:name("Box_0")
object_0:location(0,0,0)
object_0:rotation(0,0,0)

light_0:name("Point_Source_1")
light_0:location(-0.55,0,0)
light_0:rotation(0,0,0)

object_1:name("Box_2")
object_1:location(0,0,0)
object_1:rotation(0,0,0)

object_0:default_out_mat(material_0)
object_0:default_in_mat(material_1)
object_0:default_IRF(SEL_IRF_FRESNEL)

object_1:default_out_mat(material_0)
object_1:default_in_mat(material_0)
object_1:default_IRF(SEL_IRF_FRESNEL)
object_1:sensor("abs","wavelength","source","generation","path","world_intersection","world_direction")

light_0:material(material_0)
light_0:wavelength(5.5e-07)


selene=MODE("selene")

selene:N_rays_total(1000000)
selene:N_rays_disp(1000)

selene:output_directory("")
selene:add_object(object_0)
selene:add_object(object_1)

selene:add_light(light_0)

selene:render()