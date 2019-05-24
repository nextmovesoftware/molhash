cmake_minimum_required(VERSION 2.8.11)
project(molhash)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

find_package(RDKit)

#################################### Set up the build for different targets
set(TARGET "RDKIT")

set(RDKIT_DIR "${RDKIT_DIR}" CACHE PATH "Where is RDKit installed?")

message(STATUS "Targeting RDKit toolkit")
if(NOT RDKIT_FOUND)
  message(FATAL_ERROR "Please specify the RDKit directory -DRDKIT_DIR")
endif()
find_package(ZLIB)

# set(Boost_DEBUG ON)
# Attempt 1: Search for static components
set(Boost_USE_STATIC_LIBS ON)
# find_package(Boost COMPONENTS date_time system thread)
find_package(Boost QUIET COMPONENTS system thread)
if(Boost_FOUND)
  # Again without QUIET to report version number
  find_package(Boost COMPONENTS system thread)
else()
  # Attempt 2: Search for dynamic components
  set(Boost_USE_STATIC_LIBS OFF)
  find_package(Boost QUIET COMPONENTS system thread)
  if(Boost_FOUND)
    # Again without QUIET to report version number
    find_package(Boost COMPONENTS system thread)
  else()
    # Attempt 3: Search for static
    set(Boost_USE_STATIC_LIBS ON)
    find_package(Boost)
    if(NOT Boost_FOUND)
      message(FATAL_ERROR "Please specify the Boost directories")
    endif()
  endif()
endif()

add_definitions(-DUSE_RDKIT)
set(TOOLKIT_CPP rdktoolkit.cpp rdktoolkit.h)
set(TOOLKIT_INCLUDE_DIR ${RDKIT_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS}) # Needed for some MSVC compilers
set(TOOLKIT_LIBS ${RDKIT_LIBRARIES} ${Boost_SYSTEM_LIBRARY}
         ${Boost_THREAD_LIBRARY} ${Boost_DATE_TIME_LIBRARY}
         ${ZLIB_LIBRARIES})
# Boost_THREAD_LIBRARY can depend upon pthread and rt on Linux
if(NOT MSVC)
  if (NOT APPLE AND NOT MINGW)
    set(TOOLKIT_LIBS ${TOOLKIT_LIBS} pthread rt)
  else()
    set(TOOLKIT_LIBS ${TOOLKIT_LIBS} pthread)
  endif()
endif()

# Find what version of RDKit - for MSVC test multiple build types
if (MSVC)
  foreach(TEST_BUILD_TYPE ${CMAKE_BUILD_TYPE} RELEASE DEBUG)
    set(CMAKE_TRY_COMPILE_CONFIGURATION ${TEST_BUILD_TYPE})
    try_run(RUNOK COMPILEOK "${CMAKE_BINARY_DIR}/getrdkitversion" "${CMAKE_SOURCE_DIR}/cmake/ReportRDKitVersion.cpp"
                    LINK_LIBRARIES ${TOOLKIT_LIBS}
                    RUN_OUTPUT_VARIABLE RDKITVERSION
                    CMAKE_FLAGS
                      "-DINCLUDE_DIRECTORIES=${TOOLKIT_INCLUDE_DIR}")
    if(COMPILEOK)
      break()
    endif()
  endforeach()
else()
  try_run(RUNOK COMPILEOK "${CMAKE_BINARY_DIR}/getrdkitversion" "${CMAKE_SOURCE_DIR}/cmake/ReportRDKitVersion.cpp"
                LINK_LIBRARIES ${TOOLKIT_LIBS}
                RUN_OUTPUT_VARIABLE RDKITVERSION
                CMAKE_FLAGS
                  "-DINCLUDE_DIRECTORIES=${TOOLKIT_INCLUDE_DIR}")
endif()
if(COMPILEOK)
  message(STATUS "The RDKit version is ${RDKITVERSION}")
else()
  message(FATAL_ERROR "Compiling a test program using RDKit failed. Are the library paths correct? (${TOOLKIT_LIBS})")
endif()

add_definitions(-DNMS_RDKIT_VERSION=${RDKITVERSION})

###########################################################################


######################################################### Build the Project
if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -W -Wall -ffast-math")
  if(MINGW)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
    set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS
        "${CMAKE_SHARED_LIBRARY_LINK_C_FLAGS} -static-libgcc -s")
    set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS
        "${CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS} -static-libgcc -static-libstdc++ -s")
  elseif(NOT CYGWIN) # GCC on Linux (needed for Java bindings to compile)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  endif()
  if(NOT "${RDKITVERSION}" LESS 20180101)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
  endif()
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -W -Wall -ffast-math")
  if(NOT "${RDKITVERSION}" LESS 20180101)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
  endif()
endif()

include_directories(${TOOLKIT_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR})

set(molhashlib_srcs
  mf.h
  molhash.h
  hashfunctions.cpp
  normalize.cpp
  ${TOOLKIT_CPP}
  )

# Static library
add_library(molhashlib ${molhashlib_srcs})
target_link_libraries(molhashlib ${TOOLKIT_LIBS})

add_executable(molhash molhash.cpp)
target_link_libraries(molhash molhashlib)
