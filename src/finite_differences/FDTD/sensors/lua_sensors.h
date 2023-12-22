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

#ifndef LUA_SENSORS_H
#define LUA_SENSORS_H

#include <lua_base.h>

//####################
//      Sensors
//####################

int create_sensor(lua_State *L);

int sensor_set_location(lua_State *L);
int sensor_set_location_real(lua_State *L);
//int sensor_set_arbitrary_spectrum(lua_State *L);

void Sensor_generator_create_metatable(lua_State *L);

#endif
