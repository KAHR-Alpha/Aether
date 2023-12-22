set(mat_src dielec_models.cpp
			lua_material.cpp
			material.cpp)
			 
set(mat_hdr aniso_retriever.h
            lua_material.h
            material.h)

add_library(materials STATIC ${mat_src} ${mat_hdr})
target_include_directories(materials PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(materials lua_core)