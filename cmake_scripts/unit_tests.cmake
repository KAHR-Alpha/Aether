include(CTest)

enable_testing()

file(GLOB_RECURSE test_scripts tests/*.lua)

file(GLOB_RECURSE test_scripts tests/*.lua)

foreach(fname ${test_scripts})
	cmake_path(RELATIVE_PATH fname OUTPUT_VARIABLE relative_fname)
	# cmake_path(GET fname STEM fname_stem)
	add_test(NAME "${relative_fname}" COMMAND "Aether_CLI.exe" "-f" "${fname}" WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/Aether")
endforeach()
