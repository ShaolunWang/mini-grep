cmake_minimum_required(VERSION 3.15...4.0)

project(
  Search
  VERSION 0.1
  DESCRIPTION "Search"
  LANGUAGES CXX)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)


if(CMAKE_BUILD_TYPE MATCHES Debug)
	add_compile_options(-fsanitize=address -g)
	add_link_options(-fsanitize=address)
endif()
if(CMAKE_BUILD_TYPE MATCHES THREAD)
	add_compile_options(-fsanitize=thread -g)
	add_link_options(-fsanitize=thread)
endif()


# Only do these if this is the main project, and not if it is included through add_subdirectory
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

  # Optionally set things like CMAKE_CXX_STANDARD, CMAKE_POSITION_INDEPENDENT_CODE here

  # Let's ensure -std=c++xx instead of -std=g++xx
  set(CMAKE_CXX_EXTENSIONS OFF)

  # Let's nicely support folders in IDEs
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  # Testing only available if this is the main app
  # Note this needs to be done in the main CMakeLists
  # since it calls enable_testing, which must be in the
  # main CMakeLists.
  include(CTest)

  # Docs only available if this is the main app
  find_package(Doxygen)
  if(Doxygen_FOUND)
    add_subdirectory(docs)
  else()
    message(STATUS "Doxygen not found, not building docs")
  endif()
endif()
################## fetch content #########################
set(ABSL_ENABLE_INSTALL ON)
include(FetchContent)
FetchContent_Declare(
  fmtlib
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 12.1.0
  GIT_SHALLOW TRUE
)
FetchContent_Declare(
    absl
    GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
    GIT_TAG master  # You can specify a specific tag or commit here
)

FetchContent_Declare(
    re2
    GIT_REPOSITORY https://github.com/google/re2.git
    GIT_TAG "2025-11-05" 
)

FetchContent_MakeAvailable(fmtlib absl re2)

# target_include_directories (re2 INTERFACE $<BUILD_INTERFACE:${re2lib_SOURCE_DIR}>)
# target_link_libraries(re2 PUBLIC absl::base absl::strings absl::synchronization absl::container)
target_link_libraries(re2
    PUBLIC
    absl::base
    absl::strings
    absl::synchronization
    absl::flat_hash_map
    absl::flat_hash_set
    absl::hash
    absl::inlined_vector
    absl::optional
    absl::span
    absl::str_format
)
add_subdirectory(src)
add_subdirectory(test)
