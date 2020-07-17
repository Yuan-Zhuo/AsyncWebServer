#include <chrono>

#include "ranking.hpp"
#include "test.h"

static void BM_get_limit_by_activity(uint32_t shift, uint32_t iter_cnt,
                                     uint32_t iter_times) {
  Ranking rank;
  std::set<uint32_t> test_data;
  uint32_t size = 1 << shift;
  init_env_uid(size, iter_cnt, rank, test_data);

  auto start = std::chrono::high_resolution_clock::now();

  // timing part
  // iteration times: specified by arguments
  for (auto _ : boost::irange(iter_times)) {
    (void)_;
    for (auto data : test_data) rank.get_hybrid_rank(data);
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto elapsed =
      std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
  std::cout << "BM_get_limit_by_activity/" << shift << "/" << size << "/"
            << iter_cnt << "\t"
            << (static_cast<uint64_t>(elapsed.count()) / iter_times)
            << std::endl;
}

int main() {
  uint32_t iter_cnt = 100;
  uint32_t iter_times = 100;
  // size
  for (auto shift : boost::irange(10, 32)) {
    BM_get_limit_by_activity(shift, iter_cnt, iter_times);
  }
}
