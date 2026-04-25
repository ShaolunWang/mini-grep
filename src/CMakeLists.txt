file(GLOB HEADER_LIST CONFIGURE_DEPENDS 
	"${Search_SOURCE_DIR}/include/search/*.h"
	"${Search_SOURCE_DIR}/include/queue/*.h"
)

add_library(search_lib matcher.cpp lib.cpp ${HEADER_LIST})
target_link_libraries(search_lib PUBLIC fmt::fmt ringbuffer::ringbuffer re2 spdlog)
target_include_directories(search_lib PUBLIC ../include)

target_compile_features(search_lib PUBLIC cxx_std_23)

get_target_property(inc ringbuffer INTERFACE_INCLUDE_DIRECTORIES)
message(STATUS "ringbuffer includes: ${inc}")

source_group(
  TREE "${PROJECT_SOURCE_DIR}/include"
  PREFIX "Header Files"
  FILES ${HEADER_LIST})

