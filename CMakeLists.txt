cmake_minimum_required(VERSION 2.6)

PROJECT(FileUpdater)

# setup glib2
#if (NOT GLIB2_FOUND)
#				# here, under win32, update the possible list of paths
#				find_path(GLIB2_INCLUDE_DIR glib.h 
#								PATHS 
#									/usr/include 
#								PATH_SUFFIXES glib-2.0 
#								NO_DEFAULT_PATH
#				)
#				find_path(GLIB2_CONFIG_INCLUDE_DIR glibconfig.h
#								PATHS /usr/lib /usr/lib64 
#								PATH_SUFFIXES glib-2.0/include
#								NO_DEFAULT_PATH
#				)
#
# setup SoQt	# setup core include dirs
find_library(PTHREAD_LIB 
	NAMES pthread
	PATHS
		/usr/lib
		/usr/MacLibs/
	
)
find_library(STD_LIB 
	NAMES c
	PATHS
		/usr/lib
		/usr/MacLibs/
	
)
include_directories(/usr/include)
add_library(Network STATIC Network.cpp)
target_link_libraries(Network ${PTHREAD_LIB})
add_executable(FileUpdater FileUpdater.cpp )
target_link_libraries(FileUpdater ${STD_LIB})
target_link_libraries(FileUpdater Network)
	
	
