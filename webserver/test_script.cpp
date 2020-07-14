#include <benchmark/benchmark.h>

#include <boost/range/irange.hpp>
#include <chrono>
#include <random>
#include <set>

#include "ranking.hpp"

// #define BM_CRUD
// #define BM_RANK
#define BM_LIMIT

std::random_device rd;

static User generate_random_user(uint32_t uid_) {
  User user;
  user.uid = uid_;
  user.name = std::to_string(rd());
  user.exp_pers = rd();
  user.active = rd();
  user.exp_gang = rd();
  return user;
}

static inline uint32_t generate_random_uid(uint32_t bound) {
  return rd() % bound;
}

static inline void init_rank(Ranking& rank, uint32_t size) {
  for (auto i : boost::irange(size)) rank.put_user(generate_random_user(i));
}

static void Args_basic(benchmark::internal::Benchmark* b) {
  for (auto i : boost::irange(10, 20)) b->Args({1 << i, 100});
}

static void Args_limit(benchmark::internal::Benchmark* b) {
  for (auto i : boost::irange(10, 26)) b->Args({1 << i, 100});
}

static void init_env_uid(uint32_t size, uint32_t iter_cnt, Ranking& rank,
                         std::set<uint32_t>& test_data) {
  init_rank(rank, size);
  for (auto _ : boost::irange(iter_cnt)) {
    (void)_;
    test_data.insert(generate_random_uid(size));
  }
}

#ifdef BM_CRUD

static void BM_put_user(benchmark::State& state) {
  // pre-set part
  Ranking rank;
  std::set<User> test_data;
  uint32_t size = state.range(0), iter_cnt = state.range(1);
  init_rank(rank, size);
  for (auto i : boost::irange(size, iter_cnt + size))
    test_data.insert(generate_random_user(i));

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
    test_data.insert(generate_random_user(generate_random_uid(size)));
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

static void BM_get_exp_gang_rank(benchmark::State& state) {
  // pre-set part
  Ranking rank;
  std::set<uint32_t> test_data;
  init_env_uid(state.range(0), state.range(1), rank, test_data);

  // timing part
  for (auto _ : state) {
    for (auto data : test_data) rank.get_exp_gang_rank(data);
  }
}
BENCHMARK(BM_get_exp_gang_rank)->Apply(Args_basic);

static void BM_get_active_rank(benchmark::State& state) {
  // pre-set part
  Ranking rank;
  std::set<uint32_t> test_data;
  init_env_uid(state.range(0), state.range(1), rank, test_data);

  // timing part
  for (auto _ : state) {
    for (auto data : test_data) rank.get_active_rank(data);
  }
}
BENCHMARK(BM_get_active_rank)->Apply(Args_basic);

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

#ifdef BM_LIMIT

static void BM_limit(benchmark::State& state) {
  // pre-set part
  Ranking rank;
  std::set<uint32_t> test_data;
  init_env_uid(state.range(0), state.range(1), rank, test_data);

  // timing part
  for (auto _ : state) {
    for (auto data : test_data) rank.get_active_rank(data);
  }
}
BENCHMARK(BM_limit)->Apply(Args_limit);

#endif  // BM_LIMIT

BENCHMARK_MAIN();
