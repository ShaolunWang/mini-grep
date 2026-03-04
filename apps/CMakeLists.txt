add_executable(mini-grep main.cpp)
target_compile_features(app PRIVATE cxx_std_20)
target_link_libraries(app PRIVATE  fmt::fmt search_lib)
