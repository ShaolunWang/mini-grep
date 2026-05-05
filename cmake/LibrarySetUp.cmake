################## fetch content #########################
set(ABSL_ENABLE_INSTALL ON)
include(FetchContent)
FetchContent_Declare(
  fmtlib
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 12.1.0
  GIT_SHALLOW TRUE
)
FetchContent_Declare(
    absl
    GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
    GIT_TAG "20260107.1"  # You can specify a specific tag or commit here
)
FetchContent_Declare(
    ringbuffer
	GIT_REPOSITORY https://github.com/ShaolunWang/ringbuffer.git
    GIT_TAG "v0.1.0"
)
FetchContent_Declare(
    re2
    GIT_REPOSITORY https://github.com/google/re2.git
    GIT_TAG "2025-11-05"
)

FetchContent_Declare(
	spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog
	GIT_TAG "v1.17.0"
)

FetchContent_MakeAvailable(fmtlib absl ringbuffer re2 spdlog)

# thanks re2 and abseil :)
target_link_libraries(re2
    PUBLIC
    absl::base
    absl::strings
    absl::synchronization
    absl::flat_hash_map
    absl::flat_hash_set
    absl::hash
    absl::inlined_vector
    absl::optional
    absl::span
    absl::str_format
)
