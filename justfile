conf:
	cmake -B build_debug -D CMAKE_BUILD_TYPE=Debug -G Ninja
debug:
	cmake --build build_debug
thread:
	cmake --build build_thread
test_debug:
	cd build_debug && GTEST_COLOR=1 ctest --output-on-failure --repeat until-fail:10 -j 10
test_thread:
	cd build_thread && GTEST_COLOR=1 ctest --output-on-failure --repeat until-fail:10 -j 1
conf_release: 
	cmake -B build_release -D CMAKE_BUILD_TYPE=Release -G Ninja
release:
	cmake --build build_release
conf_thread:
	cmake -B build_thread -D CMAKE_BUILD_TYPE=THREAD -G Ninja
clean:
	rm -rf build_debug/ build_thread build_release

base:
	cmake -B build -D CMAKE_BUILD_TYPE=Debug -G Ninja && cmake --build build/
