add_executable(search main.cpp)
target_compile_features(search PRIVATE cxx_std_23)
target_link_libraries(search PRIVATE  fmt::fmt search_lib re2)
