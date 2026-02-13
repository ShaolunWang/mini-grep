conf:
	cmake -B build -D CMAKE_BUILD_TYPE=Debug -G Ninja
build:
	cmake --build build
test:
	cd build && GTEST_COLOR=1 ctest -V 
release: 
	cmake -B build_release -D CMAKE_BUILD_TYPE=Release -G Ninja
release_build:
	cmake --build build_release
clean:
	rm -rf build/ build_release

