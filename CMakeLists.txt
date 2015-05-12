cmake_minimum_required(VERSION 2.8.12)
project(GFAL2)

# Find Boost

set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED OFF)  
set(Boost_USE_STATIC_RUNTIME ON) 
find_package(Boost 1.58.0 COMPONENTS program_options)

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost libraries not found. Please make sure you have Boost version 1.44 or higher installed.")
endif()

# Set default include/library locations

set(GFAL2_INCLUDE_DIR /usr/include/gfal2)
set(GFAL2_LIBRARY_DIR /usr/lib64)
set(GFAL2_LIBRARIES gfal2)

set(GLIB_INCLUDE_DIR /usr/include/glib-2.0)
set(GLIB_CONFIG_INCLUDE_DIR /usr/lib64/glib-2.0/include)

# Compile library and gfal-find executable

include_directories(gfal2 ${CMAKE_CURRENT_SOURCE_DIR} ${Boost_INCLUDE_DIRS} ${GFAL2_INCLUDE_DIR} ${GLIB_INCLUDE_DIR} ${GLIB_CONFIG_INCLUDE_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

add_executable(gfal-find gfal2.cpp gfal-find.cpp ${GFAL2_INCLUDE_DIR}/gfal_api.h ${GLIB_INCLUDE_DIR}/glib.h ${GLIB_CONFIG_INCLUDE_DIR}/glibconfig.h)
target_link_libraries(gfal-find gfal2 ${Boost_LIBRARIES})
