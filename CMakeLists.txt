#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

# add_definitions ( -DUSE_INFLUXDB=ON )

project(analyzer)

set(CMAKE_CXX_FLAGS "-std=c++17 -Wall ${CMAKE_CXX_FLAGS}")
#----------------------------------------------------------------------------
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})

# <------------ add hiredis dependency --------------->
find_path(HIREDIS_HEADER hiredis)
include_directories(PUBLIC ${HIREDIS_HEADER})

find_library(HIREDIS_LIB hiredis)
link_libraries(${HIREDIS_LIB})

# <------------ add redis-plus-plus dependency -------------->
# NOTE: this should be *sw* NOT *redis++*
find_path(REDIS_PLUS_PLUS_HEADER sw)
include_directories(PUBLIC ${REDIS_PLUS_PLUS_HEADER})

find_library(REDIS_PLUS_PLUS_LIB redis++)
link_libraries(${REDIS_PLUS_PLUS_LIB})
#----------------


# find_package(ROOT CONFIG REQUIRED)
# find_package(InfluxDB REQUIRED)
find_package(ZLIB REQUIRED)

# include(${ROOT_USE_FILE})

include_directories(${PROJECT_SOURCE_DIR}/include $ENV{MIDASSYS}/include)

set(CMAKE_CXX_STANDARD 17)

#----------------------------------------------------------------------------
# Locate sources and headers for this project
# NB: headers are included so they will show up in IDEs
#
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.h)

#----------------------------------------------------------------------------

add_executable(analyzer ${sources} ${headers})

target_link_libraries(analyzer ${ROOT_LIBRARIES} $ENV{MIDASSYS}/lib/mana.o $ENV{MIDASSYS}/lib/libmidas.a m ${ZLIB_LIBRARIES} pthread util rt redis++ hiredis)
#target_link_libraries(analyzer InfluxDB ${ROOT_LIBRARIES} $ENV{MIDASSYS}/lib/mana.o $ENV{MIDASSYS}/lib/libmidas.a m ${ZLIB_LIBRARIES} pthread util rt)

install(TARGETS analyzer DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
