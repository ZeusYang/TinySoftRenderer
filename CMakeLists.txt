
cmake_minimum_required(VERSION 3.5)

# You'd better build it in release mode for performance
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)

project(TinySoftRenderer VERSION "1.0")

# C++ 11 is required
set(CMAKE_CXX_STANDARD 11)

include_directories(${PROJECT_SOURCE_DIR}/include)

############################################################
# Windows or Linux options
############################################################
IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
	link_directories(${PROJECT_SOURCE_DIR}/libs)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Linux")
	find_package(SDL2 REQUIRED)

	# check if boost was found
	if(SDL2_FOUND)
	    message ("SDL2 found")
	else()
	    message (FATAL_ERROR "Cannot find SDL2")
	endif()
ENDIF()

############################################################

############################################################
# If SDL2 was not found, please install SDL2 according to
# the following terminal commands:
# sudo apt-get update
# sudo apt-get install libsdl2-2.0
# sudo apt-get install libsdl2-dev
# sdl2-config --version
###########################################################


############################################################
# Create an executable
############################################################

file(GLOB_RECURSE HEADERS ./src/*.h)
source_group("Header Files" FILES ${HEADERS})
aux_source_directory(./src/ DIR_SRCS)

# Add an executable with the above sources
add_executable(${PROJECT_NAME} ${DIR_SRCS} ${HEADERS})

# link the target with the SDL2
target_link_libraries( ${PROJECT_NAME} 
    PRIVATE 
        SDL2
	SDL2main
)
