cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

########################### Executables

if(NOT CUDA_ON_BACKEND STREQUAL "None")
	set(CUDA_SOURCES HDF5_wr/HDF5_writer_cuda.cu)
endif()

if ( CUDA_ON_BACKEND STREQUAL "HIP" AND HIP_FOUND )
	list(APPEND HIP_HIPCC_FLAGS ${CMAKE_CXX_FLAGS_DEBUG})

	if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		list(APPEND HIP_HIPCC_FLAGS -O0)
	endif()

	#set(CMAKE_CXX_COMPILER ${HIP_HIPCC_EXECUTABLE})

	list(APPEND HIP_HIPCC_FLAGS -D__NVCC__ -D__HIP__  -DCUDART_VERSION=11000 -D__CUDACC__ -D__CUDACC_VER_MAJOR__=11 -D__CUDACC_VER_MINOR__=0 -D__CUDACC_VER_BUILD__=0)

	set_source_files_properties(${CUDA_SOURCES} PROPERTIES LANGUAGE CXX)

	hip_add_executable(io main.cpp
		MetaParser/MetaParser_unit_test.cpp
		${CUDA_SOURCES}
		ObjReader/ObjReader_unit_test.cpp
		CSVReader/tests/CSVReader_unit_test.cpp)
else()
	add_executable(io main.cpp
		MetaParser/MetaParser_unit_test.cpp
		${CUDA_SOURCES}
		ObjReader/ObjReader_unit_test.cpp
		CSVReader/tests/CSVReader_unit_test.cpp)

	set_property(TARGET io PROPERTY CUDA_ARCHITECTURES OFF)
endif()

add_dependencies(io ofpmmemory)
add_dependencies(io vcluster)

if ( CMAKE_COMPILER_IS_GNUCC )
	target_compile_options(io PRIVATE "-Wno-deprecated-declarations")
	if (TEST_COVERAGE)
		target_compile_options(io PRIVATE $<$<COMPILE_LANGUAGE:CXX>: -fprofile-arcs -ftest-coverage>)
	endif()
endif()

if (CMAKE_CUDA_COMPILER_ID STREQUAL "Clang")
	add_definitions(-D__STRICT_ANSI__)
endif()

###########################

if (CUDA_FOUND)
	target_include_directories (io PUBLIC ${MPI_C_INCLUDE_DIRS})

	if( CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0 )
		add_compile_options( "-fabi-version=6" )
	endif()

	if (TEST_COVERAGE)
		target_compile_options(io PRIVATE $<$<COMPILE_LANGUAGE:CUDA>: -Xcompiler "-fprofile-arcs -ftest-coverage" >)
	endif()
endif()

target_include_directories (io PUBLIC ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})
target_include_directories (io PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories (io PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../openfpm_devices/src/)
target_include_directories (io PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../openfpm_vcluster/src/)
target_include_directories (io PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../openfpm_data/src/)
target_include_directories (io PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../src/)
target_include_directories (io PUBLIC ${CMAKE_BINARY_DIR}/config)
target_include_directories (io PUBLIC ${HDF5_ROOT}/include)
target_include_directories (io PUBLIC ${TINYOBJLOADER_INCLUDE_DIRS} )
target_include_directories (io PUBLIC ${Boost_INCLUDE_DIRS})
target_include_directories (io PUBLIC ${ALPAKA_ROOT}/include)

if (PETSC_FOUND)
	target_include_directories(io PUBLIC ${PETSC_INCLUDES})
endif()

target_link_libraries(io ${Boost_LIBRARIES})
target_link_libraries(io ${HDF5_LIBRARIES})
target_link_libraries(io ${TINYOBJLOADER_LIBRARIES} )
target_link_libraries(io vcluster)
target_link_libraries(io ofpmmemory)

if (OPENMP_FOUND)
	target_link_libraries(io OpenMP::OpenMP_CXX)
endif()

if (PETSC_FOUND)
        target_link_libraries(io ${PETSC_LIBRARIES})
endif()

# Request that particles be built with -std=c++17
# As this is a public compile feature anything that links to particles
# will also build with -std=c++17
target_compile_features(io PUBLIC cxx_std_17)
target_link_libraries(io ${MPI_C_LIBRARIES})
target_link_libraries(io ${MPI_CXX_LIBRARIES})

if (TEST_COVERAGE)
	target_link_libraries(io -lgcov)
endif()

install(FILES RawReader/RawReader.hpp
	RawReader/RawReader_unit_tests.hpp
	DESTINATION openfpm_io/include/RawReader
	COMPONENT OpenFPM)

install(FILES CSVWriter/csv_multiarray.hpp 
	CSVWriter/CSVWriter.hpp
	CSVWriter/is_csv_writable.hpp
	DESTINATION openfpm_io/include/CSVWriter/
	COMPONENT OpenFPM)

install(FILES CSVReader/CSVReader.hpp
	DESTINATION openfpm_io/include/CSVReader/
	COMPONENT OpenFPM)

install(FILES GraphMLWriter/GraphMLWriter.hpp 
	DESTINATION openfpm_io/include/GraphMLWriter
	COMPONENT OpenFPM)

install(FILES util/util.hpp util/GBoxes.hpp
	DESTINATION openfpm_io/include/util
	COMPONENT OpenFPM)

install(FILES VTKWriter/VTKWriter.hpp 
	VTKWriter/byteswap_portable.hpp
	VTKWriter/VTKWriter_dist_graph.hpp
	VTKWriter/VTKWriter_graph.hpp
	VTKWriter/VTKWriter_point_set.hpp
	VTKWriter/VTKWriter_grids.hpp
	VTKWriter/VTKWriter_grids_st.hpp
	VTKWriter/VTKWriter_grids_util.hpp
	VTKWriter/VTKWriter_vector_box.hpp
	VTKWriter/is_vtk_writable.hpp
	DESTINATION openfpm_io/include/VTKWriter/
	COMPONENT OpenFPM)

install(FILES MetaParser/MetaParser.hpp
	DESTINATION openfpm_io/include/MetaParser/
	COMPONENT OpenFPM)

install(FILES HDF5_wr/HDF5_wr.hpp
	HDF5_wr/HDF5_writer.hpp
	HDF5_wr/HDF5_writer_vd.hpp
	HDF5_wr/HDF5_writer_gd.hpp
	HDF5_wr/HDF5_reader_gd.hpp
	HDF5_wr/HDF5_reader.hpp
	HDF5_wr/HDF5_reader_vd.hpp
	DESTINATION openfpm_io/include/HDF5_wr
	COMPONENT OpenFPM)

install(FILES RawReader/InitGridWithPixel.hpp
	DESTINATION openfpm_io/include/RawReader/
	COMPONENT OpenFPM)

install(FILES util/PathsAndFiles.hpp
	DESTINATION openfpm_io/include/util
	COMPONENT OpenFPM)

install(FILES Plot/GoogleChart.hpp Plot/util.hpp
	DESTINATION openfpm_io/include/Plot
	COMPONENT OpenFPM)

