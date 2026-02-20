file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${Search_SOURCE_DIR}/include/search/*.h" "${Search_SOURCE_DIR}/include/queue/*.h")

add_library(search_lib lib.cpp ${HEADER_LIST})

target_include_directories(search_lib PUBLIC ../include)

target_link_libraries(search_lib PRIVATE fmt::fmt
re2)

target_compile_features(search_lib PUBLIC cxx_std_20)

source_group(
  TREE "${PROJECT_SOURCE_DIR}/include"
  PREFIX "Header Files"
  FILES ${HEADER_LIST})

