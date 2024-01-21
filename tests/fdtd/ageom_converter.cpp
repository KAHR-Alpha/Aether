/*Copyright 2008-2024 - Loïc Le Cunff

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

 #include <lua_structure.h>

#include <fstream>
#include <iostream>
#include <sstream>

int ageom_converter(int argc,char *argv[])
{
	std::stringstream geostr, output;
	
	geostr<<"declare_parameter(\"str\",v)\n";
	output<<"declare_parameter(\"str\",v)\n";

	geostr<<"lx(\"lx_str\")\n";
	output<<"lx=lx_str\n";

	geostr<<"ly(\"ly_str\")\n";
	output<<"ly=ly_str";

	geostr<<"lz(\"lz_str\")\n";
	output<<"lz=lz_str\n";

	geostr<<"set(\"var\",\"var_str\")";
	output<<"var=var_str\n";

	geostr<<"default_material(mat)\n";
	output<<"default_material(mat)\n";

	geostr<<"add_block(\"arg1\",\"arg2\",\"arg3\",\"arg4\",\"arg5\",\"arg6\",mat)\n";
	output<<"add_block(arg1,arg2,arg3,arg4,arg5,arg6,mat)\n";

	//geostr<<"add_coating",LuaUI::structure_add_coating);
	
	geostr<<"add_cone(\"arg1\",\"arg2\",\"arg3\",\"arg4\",\"arg5\",\"arg6\",\"rad\",mat)\n";
	output<<"add_cone(arg1,arg2,arg3,arg4,arg5,arg6,rad,mat)\n";
	
	geostr<<"add_cylinder(\"arg1\",\"arg2\",\"arg3\",\"arg4\",\"arg5\",\"arg6\",\"rad\",mat)\n";
	output<<"add_cylinder(arg1,arg2,arg3,arg4,arg5,arg6,rad,mat)\n";

	//geostr<<"add_ellipsoid",LuaUI::structure_add_ellipsoid);
	
	geostr<<"add_layer(\"dir\",\"arg1\",\"arg2\",mat)\n";
	geostr<<"add_layer(\"dir\",arg1,arg2,mat)\n";

	//geostr<<"add_lua_def",LuaUI::structure_add_lua_def);
	//geostr<<"add_mesh",LuaUI::structure_add_mesh);
	//geostr<<"add_sin_layer",LuaUI::structure_add_sin_layer);

	geostr<<"add_sphere(\"arg1\",\"arg2\",\"arg3\",\"rad\",mat)\n";
	output<<"add_sphere(arg1,arg2,arg3,rad,mat)\n";

	std::filesystem::path fname="ageom_sample.ageom";
	std::ofstream script(fname,std::ios::out|std::ios::trunc);

	script<<geostr.str();
	script.close();

	std::string cmp_str=ageom_to_lua(fname);

	if(cmp_str==output.str())
	{
		return 0;
	}
	else return 1;
}
