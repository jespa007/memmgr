cmake_minimum_required(VERSION 2.8)
 
project(memmgr)

#--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#
# COMPILE PARAMS
#
 
message("dest dir:" ${CMAKE_INSTALL_PREFIX})


if(MSVC)
	MESSAGE ("-- Compiler: MSVC" )
	MESSAGE ("-- Platform: Windows" )
	
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)

	#disable C4103 warning because is perfectly valid. Only MSVC complains about it
	add_definitions(/wd4103)
	add_definitions(/wd4800)
	add_definitions(/wd4244)

elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")  # g++ / gcc
	SET( COMMON_C_PARAMS	 " -pthread ")
	SET( COMMON_CXX_PARAMS 	 " -pthread -std=gnu++0x -g -O0 ")
	
endif()
 


# Set C flags.
SET(CMAKE_CXX_FLAGS   	" ${TARGET_FLAGS} ${COMMON_CXX_PARAMS}  " )
SET(CMAKE_C_FLAGS   	" ${TARGET_FLAGS} ${COMMON_C_PARAMS}  " )


MESSAGE ("Target : " ${TARGET})

#Redefine output dir ...
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})


#-----------------------------------------
#
# EXECUTABLES
#

add_executable(test_c
	${PROJECT_SOURCE_DIR}/test_c.c
	${PROJECT_SOURCE_DIR}/memmgr.c
)
target_link_libraries(test_c "-Wl,--no-as-needed" )

add_executable(test_cpp
	${PROJECT_SOURCE_DIR}/test_cpp.cpp
	${PROJECT_SOURCE_DIR}/memmgr.cpp
)
target_link_libraries(test_cpp "-Wl,--no-as-needed" )








