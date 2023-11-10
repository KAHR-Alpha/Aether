include(CTest)

enable_testing()

file(GLOB_RECURSE test_scripts tests/*.lua)

add_test(NAME Test_0 COMMAND "Aether_CLI.exe" "-f" "tests/test0.lua" WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/Aether/")
add_test(NAME Test_1 COMMAND "Aether_CLI.exe" "-f" "tests/test1.lua" WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/Aether/")


# file(GLOB_RECURSE test_scripts tests/*.lua)

# foreach(fname ${test_scripts})
	# cmake_path(RELATIVE_PATH fname OUTPUT_VARIABLE relative_fname)
	# cmake_path(GET relative_fname STEM fname_stem)
	# add_test(NAME "${fname_stem}" COMMAND "Aether_CLI.exe" "-f" "${relative_fname}$" WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/Aether/tests")
# endforeach()
