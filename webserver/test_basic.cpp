#include <benchmark/benchmark.h>

#include "ranking.hpp"
#include "test.h"

#define BM_CRUD
#define BM_RANK

static void Args_basic(benchmark::internal::Benchmark* b) {
  for (auto i : boost::irange(10, 20)) b->Args({1 << i, 100});
}

#ifdef BM_CRUD

static void BM_put_user(benchmark::State& state) {
  // pre-set part
  Ranking rank;
  std::set<User> test_data;
  uint32_t size = state.range(0), iter_cnt = state.range(1);
  init_rank(rank, size);
  for (auto i : boost::irange(size, iter_cnt + size))
    test_data.insert(generate_random_user(i, rank.get_ca()));

  // timing part
  for (auto _ : state) {
    for (auto data : test_data) rank.put_user(data);
  }
}
BENCHMARK(BM_put_user)->Apply(Args_basic);

static void BM_get_user(benchmark::State& state) {
  // pre-set part
  Ranking rank;
  std::set<uint32_t> test_data;
  init_env_uid(state.range(0), state.range(1), rank, test_data);

  // timing part
  for (auto _ : state) {
    for (auto data : test_data) rank.get_user(data);
  }
}
BENCHMARK(BM_get_user)->Apply(Args_basic);

static void BM_modify_user(benchmark::State& state) {
  // pre-set part
  Ranking rank;
  std::set<User> test_data;
  uint32_t size = state.range(0), iter_cnt = state.range(1);
  init_rank(rank, size);
  for (auto _ : boost::irange(iter_cnt)) {
    (void)_;
    test_data.insert(
        generate_random_user(generate_random_uid(size), rank.get_ca()));
  }

  // timing part
  for (auto _ : state) {
    for (auto data : test_data) rank.modify_user(data);
  }
}
BENCHMARK(BM_modify_user)->Apply(Args_basic);

static void BM_remove_user(benchmark::State& state) {
  // pre-set part
  Ranking rank;
  std::set<uint32_t> test_data;
  init_env_uid(state.range(0), state.range(1), rank, test_data);

  // timing part
  for (auto _ : state) {
    for (auto data : test_data) {
      try {
        rank.remove_user(data);
      } catch (NoneOfUidException& e) {
      }
    }
  }
}
BENCHMARK(BM_remove_user)->Apply(Args_basic);

#endif  // BM_CRUD

#ifdef BM_RANK

static void BM_get_exp_pers_rank(benchmark::State& state) {
  // pre-set part
  Ranking rank;
  std::set<uint32_t> test_data;
  init_env_uid(state.range(0), state.range(1), rank, test_data);

  // timing part
  for (auto _ : state) {
    for (auto data : test_data) rank.get_exp_pers_rank(data);
  }
}
BENCHMARK(BM_get_exp_pers_rank)->Apply(Args_basic);

static void BM_get_activity_rank(benchmark::State& state) {
  // pre-set part
  Ranking rank;
  std::set<uint32_t> test_data;
  init_env_uid(state.range(0), state.range(1), rank, test_data);

  // timing part
  for (auto _ : state) {
    for (auto data : test_data) rank.get_activity_rank(data);
  }
}
BENCHMARK(BM_get_activity_rank)->Apply(Args_basic);

static void BM_get_hybrid_rank(benchmark::State& state) {
  // pre-set part
  Ranking rank;
  std::set<uint32_t> test_data;
  init_env_uid(state.range(0), state.range(1), rank, test_data);

  // timing part
  for (auto _ : state) {
    for (auto data : test_data) rank.get_hybrid_rank(data);
  }
}
BENCHMARK(BM_get_hybrid_rank)->Apply(Args_basic);

#endif  // BM_RANK

BENCHMARK_MAIN();
