################## add test lib #########################
if(CMAKE_BUILD_TYPE MATCHES Debug OR 
	CMAKE_BUILD_TYPE MATCHES THREAD)
	FetchContent_Declare(
			googletest
			URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
	)
	# For Windows: Prevent overriding the parent project's compiler/linker settings
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
	FetchContent_MakeAvailable(googletest)

	enable_testing()
	add_executable(test_queue
			test_queue.cpp
	)
	target_link_libraries(test_queue
			GTest::gtest_main
			fmt::fmt
			search_lib
			re2
	)
	add_executable(test_policies
			test_policies.cpp
	)
	target_link_libraries(test_policies
			GTest::gtest_main
			fmt::fmt
			re2
			search_lib
	)
	add_executable(test_engine
			test_engine.cpp
	)
	target_link_libraries(test_engine
			GTest::gtest_main
			fmt::fmt
			re2
			search_lib
	)

	include(GoogleTest)
	gtest_discover_tests(test_queue)
	gtest_discover_tests(test_policies)
	gtest_discover_tests(test_engine)
endif()
##########################################################
