function(Dll_Copy library_name)
	cmake_path(GET library_name EXTENSION ext)
	cmake_path(GET library_name FILENAME fname)
	if("${ext}" STREQUAL ".dll")
		add_custom_command(TARGET Aether_CLI POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${library_name} "${CMAKE_BINARY_DIR}/Aether/${fname}")
		add_custom_command(TARGET Aether_GUI POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${library_name} "${CMAKE_BINARY_DIR}/Aether/${fname}")
	endif()
endfunction()

if(IDE_SETUP)

add_custom_command(TARGET Aether_CLI POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/mat_lib ${CMAKE_BINARY_DIR}/Aether/mat_lib)
add_custom_command(TARGET Aether_CLI POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources ${CMAKE_BINARY_DIR}/Aether/resources)

set_target_properties(Aether_CLI PROPERTIES RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_BINARY_DIR}/Aether>") 
set_target_properties(Aether_CLI PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/Aether") 

Dll_Copy(${LUA_LIBRARIES})
Dll_Copy(${FFTW_LIB})
Dll_Copy(${PNG_LIBRARY_RELEASE})
Dll_Copy(${ZLIB_LIBRARY_RELEASE})

if(GUI)
	add_custom_command(TARGET Aether_GUI POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/mat_lib ${CMAKE_BINARY_DIR}/Aether/mat_lib)
	add_custom_command(TARGET Aether_GUI POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources ${CMAKE_BINARY_DIR}/Aether/resources)
	
	set_target_properties(Aether_GUI PROPERTIES RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_BINARY_DIR}/Aether>") 
	set_target_properties(Aether_GUI PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/Aether") 
	
	Dll_Copy(${WXWIDGETS_BASE})
	Dll_Copy(${WXWIDGETS_CORE})
	Dll_Copy(${WXWIDGETS_GL})
	Dll_Copy(${WXWIDGETS_ADV})
endif()

endif()