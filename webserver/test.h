#include <boost/range/irange.hpp>
#include <random>
#include <set>

#include "ranking.hpp"

std::random_device rd;

static User generate_random_user(uint32_t uid_, char_allocator& ca) {
  User user(ca);
  user.uid = uid_;
  user.name = shm_string(std::to_string(rd()).c_str(), ca);
  user.exp_pers = rd();
  user.activity = rd();
  return user;
}

static inline uint32_t generate_random_uid(uint32_t bound) {
  return rd() % bound;
}

static inline void init_rank(Ranking& rank, uint32_t size) {
  for (auto i : boost::irange(size))
    rank.put_user(generate_random_user(i, rank.get_ca()));
}

static void init_env_uid(uint32_t size, uint32_t iter_cnt, Ranking& rank,
                         std::set<uint32_t>& test_data) {
  init_rank(rank, size);
  for (auto _ : boost::irange(iter_cnt)) {
    (void)_;
    test_data.insert(generate_random_uid(size));
  }
}
