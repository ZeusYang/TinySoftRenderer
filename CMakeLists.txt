cmake_minimum_required (VERSION 3.5)

project(TinySoftRenderer)

# C++ 11 is required
set(CMAKE_CXX_STANDARD 11)

include_directories(include)
include_directories(${PROJECT_SOURCE_DIR}/external/include)

IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
	link_directories(${PROJECT_SOURCE_DIR}/external/libs)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Linux")
	find_package(SDL2 REQUIRED)

	# check if boost was found
	if(SDL2_FOUND)
	    message ("SDL2 found")
	else()
	    message (FATAL_ERROR "Cannot find SDL2")
	endif()
ENDIF()

file(GLOB_RECURSE SRCS ./src/*.cpp)
file(GLOB_RECURSE HEADERS include/*.h)
source_group("Header Files" FILES ${HEADERS})

add_library(${PROJECT_NAME} ${SRCS} ${HEADERS})
add_library(TinySoftRenderer::renderer ALIAS ${PROJECT_NAME})

# link the target with the SDL2
target_link_libraries( ${PROJECT_NAME} 
    PUBLIC 
        SDL2
	SDL2main
		tbb
		tbb12
		assimp
)

# Add sub directories
add_subdirectory(examples/example1_point_lighting)
add_subdirectory(examples/example2_spot_lighting)
add_subdirectory(examples/example3_directional_lighting)
add_subdirectory(examples/example4_alpha_blending)
add_subdirectory(examples/example5_alpha_to_coverage)
add_subdirectory(examples/example6_diablo3_pose)
add_subdirectory(examples/example7_normal_mapping)
add_subdirectory(examples/example8_complicated_scene)