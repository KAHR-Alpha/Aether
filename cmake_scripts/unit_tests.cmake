include(CTest)

enable_testing()

##########################
#   Lua testing scripts
##########################

file(GLOB_RECURSE test_scripts tests/*.lua)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/tests PREFIX lua FILES ${test_scripts})

foreach(fname ${test_scripts})
	cmake_path(RELATIVE_PATH fname OUTPUT_VARIABLE relative_fname)
	# cmake_path(GET fname STEM fname_stem)
	add_test(NAME "${relative_fname}" COMMAND "Aether_CLI.exe" "-f" "${fname}" WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/Aether")
endforeach()


##########################
#   Compiled unit tests
##########################

file(GLOB_RECURSE cpp_tests tests/*.cpp)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/tests PREFIX cpp FILES ${cpp_tests})

foreach(src ${cpp_tests})
	cmake_path(GET src STEM fname)
	list(APPEND test_names ${fname})
endforeach()

create_test_sourcelist(Tests
					   UnitTests.cpp
					   ${test_names})
					   
add_executable(UnitTests UnitTests.cpp ${cpp_tests} ${test_scripts})
target_link_libraries(UnitTests lua_interface selene_core)
target_link_libraries(UnitTests ${LUA_LIBRARIES})

foreach(test ${test_names})
	add_test(NAME ${test} COMMAND UnitTests ${test})
endforeach()