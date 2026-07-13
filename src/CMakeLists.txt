cmake_minimum_required(VERSION 3.5)
 
project(memmgr)

option(BUILD_SHARED_LIBS "Build libraries as SHARED" ON)


IF( NOT CMAKE_BUILD_TYPE )
   SET( CMAKE_BUILD_TYPE Release )
ENDIF()

set(POST_NAME "")
set(IS_DEBUG FALSE)
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	set(IS_DEBUG TRUE)
	SET( CMAKE_C_FLAGS " ${CMAKE_C_FLAGS} -D__DEBUG__ ")
	set( POST_NAME "_d")
endif()

MESSAGE (STATUS "Target: " ${CMAKE_BUILD_TYPE})

file(READ "memmgr.h" memmgr_h)

string(REGEX MATCH "MEMMGR_VERSION_MAJOR([\t-\ ][0-9])*" _ ${memmgr_h})
string(STRIP "${CMAKE_MATCH_1}" MEMMGR_VERSION_MAJOR)
string(REGEX MATCH "MEMMGR_VERSION_MINOR([\t-\ ][0-9])*" _ ${memmgr_h})
string(STRIP "${CMAKE_MATCH_1}" MEMMGR_VERSION_MINOR)
string(REGEX MATCH "MEMMGR_VERSION_PATCH([\t-\ ][0-9])*" _ ${memmgr_h})
string(STRIP "${CMAKE_MATCH_1}" MEMMGR_VERSION_PATCH)



MESSAGE(STATUS "Memmgr ${MEMMGR_VERSION_MAJOR}.${MEMMGR_VERSION_MINOR}.${MEMMGR_VERSION_PATCH}" )

SET(MEMMGR_LIB_NAME "memmgr-${MEMMGR_VERSION_MAJOR}-${MEMMGR_VERSION_MINOR}-${MEMMGR_VERSION_PATCH}")
SET(MEMMGR_CPP_LIB_NAME "memmgr-cpp-${MEMMGR_VERSION_MAJOR}-${MEMMGR_VERSION_MINOR}-${MEMMGR_VERSION_PATCH}")

#--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#
# COMPILE PARAMS
#
 
include_directories (
	${PROJECT_SOURCE_DIR}
)
 
MESSAGE("-- Install prefix: " ${CMAKE_INSTALL_PREFIX})


if(MSVC)
	MESSAGE ("-- Compiler: MSVC" )
	MESSAGE ("-- Platform: Windows" )
	
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)

	#disable C4103 warning because is perfectly valid. Only MSVC complains about it
	add_definitions(/wd4103)
	add_definitions(/wd4800)
	add_definitions(/wd4244)

elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")  # g++ / gcc

	SET( CMAKE_COMPILE_COMMON " -pthread -D__STDC_CONSTANT_MACROS -fomit-frame-pointer  -pipe -Wall -Wextra -pedantic -Wno-sign-conversion -Wtype-limits ")

	if (MINGW)
		MESSAGE(STATUS "Plataform: MINGW" )
		message(STATUS "Mingw path: $ENV{MINGWDIR} ")
		set(CMAKE_LEGACY_MINGW_WIN32 0)
	elseif(UNIX)
    	MESSAGE (STATUS "Plataform: UNIX" )
    else()
    	MESSAGE (STATUS "Plataform: Unknown" )
	endif()
	
	IF ( IS_DEBUG)
		set(CMAKE_COMPILE_COMMON " ${CMAKE_COMPILE_COMMON} -O0 -g ")
	ELSE()
		set(CMAKE_COMPILE_COMMON " ${CMAKE_COMPILE_COMMON} -O2 ")
	ENDIF()

 	 
	SET( CMAKE_C_FLAGS	 " ${CMAKE_COMPILE_COMMON} -std=c99 -Wstrict-prototypes  ")
	SET( CMAKE_CXX_FLAGS 	 " ${CMAKE_COMPILE_COMMON} -std=gnu++0x ")
	
	#Redefine output dir ...
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../bin/gcc/${CMAKE_BUILD_TYPE})
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../bin/gcc/${CMAKE_BUILD_TYPE})
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../bin/gcc/${CMAKE_BUILD_TYPE})
	LINK_DIRECTORIES(${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})		
endif()

#
# COMPILE PARAMS
#
#--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#
# SOURCE PARAMS
#

# Library


set( MEMMGR_LIB_SRCS
	memmgr.c
)

set( MEMMGR_LIB_CPP_SRCS
	memmgr.cpp
)

#-------------------------------------------------------------------------------------------------------------------------------------
#
# LINKING PARAMS
#

#set library generation as static by default
if(BUILD_SHARED_LIBS)
	add_library( ${MEMMGR_LIB_NAME}${POST_NAME} SHARED ${MEMMGR_LIB_SRCS} )
	target_compile_definitions(${MEMMGR_LIB_NAME}${POST_NAME} PRIVATE BUILD_SHARED_LIBS)
	target_link_libraries(${MEMMGR_LIB_NAME}${POST_NAME} ${LIBRARY_DEPS})
	
	add_library( ${MEMMGR_CPP_LIB_NAME}${POST_NAME} SHARED ${MEMMGR_LIB_CPP_SRCS} )
	target_compile_definitions(${MEMMGR_CPP_LIB_NAME}${POST_NAME} PRIVATE BUILD_SHARED_LIBS)
	target_link_libraries(${MEMMGR_CPP_LIB_NAME}${POST_NAME} ${LIBRARY_DEPS})

	
else()
	# static builds
	
	set( MEMMGR_LIB_NAME ${MEMMGR_LIB_NAME}-static)
	add_library( ${MEMMGR_LIB_NAME}${POST_NAME} STATIC ${MEMMGR_LIB_C_SRCS} )
	target_compile_definitions(${MEMMGR_LIB_NAME}${POST_NAME} PUBLIC BUILD_STATIC_LIBS)
	
	set( MEMMGR_CPP_LIB_NAME ${MEMMGR_CPP_LIB_NAME}-static)
	add_library( ${MEMMGR_CPP_LIB_NAME}${POST_NAME} STATIC ${MEMMGR_LIB_CPP_SRCS} )
	target_compile_definitions(${MEMMGR_CPP_LIB_NAME}${POST_NAME} PUBLIC BUILD_STATIC_LIBS)
	
endif()


#-----------------------------------------
#
# EXECUTABLES
#

add_executable(test_c
	${PROJECT_SOURCE_DIR}/../test/test_c.c
)

target_link_libraries(test_c ${MEMMGR_LIB_NAME}${POST_NAME})

add_executable(test_cpp
	${PROJECT_SOURCE_DIR}/../test/test_cpp.cpp
)

target_link_libraries(test_cpp ${MEMMGR_CPP_LIB_NAME}${POST_NAME} )

#-----------------------------------------
#
# INSTALL
#

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/memmgr"
    FILES_MATCHING
        PATTERN "*.h"
)

INSTALL(TARGETS ${MEMMGR_LIB_NAME}${POST_NAME}
	ARCHIVE DESTINATION  		${CMAKE_INSTALL_PREFIX}/lib
	RUNTIME DESTINATION  		${CMAKE_INSTALL_PREFIX}/bin
	LIBRARY DESTINATION 		${CMAKE_INSTALL_PREFIX}/lib
	PUBLIC_HEADER DESTINATION	${CMAKE_INSTALL_PREFIX}/includes
)








