# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

# Optionally glob, but only for CMake 3.12 or later:
file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${Search_SOURCE_DIR}/include/mini-grep/*.h" "${Search_SOURCE_DIR}/include/mpmc/*.h")
#set(HEADER_LIST "${Search_SOURCE_DIR}/include/mini-grep/lib.hpp")

# Make an automatic library - will be static or dynamic based on user setting
add_library(mini-grep_lib lib.cpp ${HEADER_LIST})

# We need this directory, and users of our library will need it too
target_include_directories(mini-grep_lib PUBLIC ../include)

# This depends on (header only) boost
target_link_libraries(mini-grep_lib PRIVATE fmt::fmt)

# All users of this library will need at least C++11
target_compile_features(mini-grep_lib PUBLIC cxx_std_20)

# IDEs should put the headers in a nice place
source_group(
  TREE "${PROJECT_SOURCE_DIR}/include"
  PREFIX "Header Files"
  FILES ${HEADER_LIST})

