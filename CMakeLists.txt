# CMakeList.txt : CMake project for TFG, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project (TFG)

find_package (OpenCL)
include_directories( ${OPENCL_INCLUDE_DIR} )
LINK_DIRECTORIES(${OpenCL_LIBRARY})
if( NOT OpenCL_FOUND )
	MESSAGE( SEND_ERROR "OpenCL no encontrado. Instala el SDK de NVidia (CUDA), AMD o Intel" )
endif( NOT OpenCL_FOUND )

# Add source to this project's executable.
add_executable (TFG "TFG.cpp" "TFG.h" "caja.h" "dinamica.h" "analisis.h")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/adyacencia.cl
    ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
 configure_file(${CMAKE_CURRENT_SOURCE_DIR}/fuerzas.cl
    ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
 configure_file(${CMAKE_CURRENT_SOURCE_DIR}/suma.cl
    ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/distancias.cl
    ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/posiciones)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/analisis)

add_custom_target(posiciones ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory posiciones)
add_custom_target(analisis ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory analisis)


# TODO: Add tests and install targets if needed.
target_link_libraries(TFG OpenCL::OpenCL)