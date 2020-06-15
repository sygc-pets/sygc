cmake_minimum_required (VERSION 3.1)
project (sygc)
set(NAME "sygc")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_path(CMAKE_FOLDER NAMES cmake/emp-tool-config.cmake)
include(${CMAKE_FOLDER}/cmake/common.cmake)
include(${CMAKE_FOLDER}/cmake/source_of_randomness.cmake)
include(${CMAKE_FOLDER}/cmake/threading.cmake)

FIND_PACKAGE(emp-ot REQUIRED)
INCLUDE_DIRECTORIES(${EMP-OT_INCLUDE_DIRS})

find_package(Boost REQUIRED COMPONENTS program_options)
set(SYGC_LIBRARIES ${SYGC_LIBRARIES} ${Boost_LIBRARIES})
set(SYGC_DIRS ${SYGC_DIR} ${Boost_INCLUDE_DIRS})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fconcepts")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconcepts")


# Remove warnings specifically for this project
STRING( REPLACE " -Wall" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} )
STRING( REPLACE " -Wall" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG} )
STRING( REPLACE " -Wall" "" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE} )

# Installation
install(FILES cmake/sygc-config.cmake DESTINATION cmake/)
install(DIRECTORY sygc DESTINATION include/)

macro (add_exec _name)
	add_executable(${_name} "exec/${_name}.cpp")
	target_link_libraries(${_name}  ${RELIC_LIBRARIES} ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES} ${GMP_LIBRARIES} ${SYGC_LIBRARIES})
endmacro()

add_exec(SYGC)
add_exec(SYGCPI)
add_exec(eval_SYGC)
add_exec(formatInOut)
add_exec(readCircuitFile)
add_exec(LeNet)
add_exec(millionaire)

configure_file("sygc/SYGC_config.h.in" "sygc/SYGC_config.h" @ONLY)

include(test/testfiles.cmake)