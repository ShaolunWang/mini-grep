conf:
	cmake -B build -D CMAKE_BUILD_TYPE=Debug -G Ninja
build:
	cmake --build build
test:
	./build/test/test_mini-grep
release: 
	cmake -B build_release -D CMAKE_BUILD_TYPE=Release -G Ninja
release_build:
	cmake --build build_release
clean:
	rm -rf build/ build_release
