if((CMAKE_BUILD_TYPE MATCHES Debug)
    OR (CMAKE_BUILD_TYPE MATCHES Release))

    find_package(benchmark REQUIRED)

    set(GEN_INPUT ${CMAKE_CURRENT_BINARY_DIR}/input.dat)

    add_custom_target(gen_input ALL
        COMMAND python3 ${CMAKE_SOURCE_DIR}/utils/gen_data.py
                ${GEN_INPUT} --size 16 --pattern abcdef --stride 4096
        DEPENDS ${CMAKE_SOURCE_DIR}/utils/gen_data.py
        COMMENT "Generating benchmark input file ${GEN_INPUT}"
        VERBATIM
    )

    add_executable(end_to_end_benchmark
        end_to_end_benchmark.cpp
    )

    add_dependencies(end_to_end_benchmark gen_input)

    target_link_libraries(end_to_end_benchmark
        fmt::fmt
        search_lib
        re2
        benchmark::benchmark
    )

	target_compile_definitions(end_to_end_benchmark PRIVATE
		GEN_INPUT_FILE="${GEN_INPUT}"
	)
endif()

