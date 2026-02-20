#include <utility>
template <typename Derived> struct ExecutorPolicyBased {
  template <typename Job> void process_chunks(const Job &&job) {
    static_cast<Derived>(*this)->process_chunk_impl(std::forward<Job>(job));
  }
};

struct SequentialPolicy : public ExecutorPolicyBased<SequentialPolicy> {
  template <typename Job> void process_chunk_impl(const Job &job) {
    std::size_t local_count = 0;
    const char *data = job.file->data();

    std::size_t begin = job.offset;
    std::size_t end = job.offset + job.length;
    // pattern matching
  }
};
