cmake_minimum_required(VERSION 3.16)
project(sfm_from_scratch VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# compile_commands.json for clangd, symlinked to the source root where editors look for it.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
                  ${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_SOURCE_DIR}/compile_commands.json)
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

option(SFM_BUILD_TESTS "Build unit tests" ON)

###########
## Deps  ##
###########

find_package(Eigen3 REQUIRED NO_MODULE)
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs features2d flann calib3d)
find_package(Ceres REQUIRED)
find_package(glog QUIET) # Ceres already pulls glog in; this is only for the imported target if available

message(STATUS "sfm: Eigen ${Eigen3_VERSION}, OpenCV ${OpenCV_VERSION}, Ceres ${Ceres_VERSION}")
if(Ceres_VERSION VERSION_LESS "2.1.0")
  message(WARNING "Ceres ${Ceres_VERSION} has no Manifold API (SetManifold, SubsetManifold, EigenQuaternionManifold), "
                  "which M1 relies on. Install Ceres >= 2.1 and point CMAKE_PREFIX_PATH at it.")
endif()

###########
## Build ##
###########

file(GLOB_RECURSE SFM_SRCS CONFIGURE_DEPENDS src/*.cpp)

if(SFM_SRCS)
  add_library(sfm ${SFM_SRCS})
  target_include_directories(sfm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${OpenCV_INCLUDE_DIRS})
  target_link_libraries(sfm PUBLIC Eigen3::Eigen ${OpenCV_LIBS} ceres)
  target_compile_options(sfm PRIVATE -Wall -Wextra)
else()
  # Nothing in src/ yet: header-only until the first .cpp lands (re-run cmake then).
  add_library(sfm INTERFACE)
  target_include_directories(sfm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include ${OpenCV_INCLUDE_DIRS})
  target_link_libraries(sfm INTERFACE Eigen3::Eigen ${OpenCV_LIBS} ceres)
endif()

add_executable(sfm_main apps/sfm_main.cpp)
target_link_libraries(sfm_main PRIVATE sfm)

###########
## Tests ##
###########

if(SFM_BUILD_TESTS)
  find_package(GTest REQUIRED)
  enable_testing()
  include(GoogleTest)
  file(GLOB SFM_TEST_SRCS CONFIGURE_DEPENDS tests/*.cpp)
  add_executable(sfm_tests ${SFM_TEST_SRCS})
  target_link_libraries(sfm_tests PRIVATE sfm GTest::GTest GTest::Main pthread)
  gtest_discover_tests(sfm_tests)
endif()
