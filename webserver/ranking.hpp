#ifndef _RANKING_HPP_
#define _RANKING_HPP_

#define BOOST_BIND_GLOBAL_PLACEHOLDERS

#include <stdint.h>

#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/ranked_index.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/range/irange.hpp>
#include <cassert>
#include <iostream>
#include <random>

#include "exception.hpp"

typedef boost::interprocess::managed_shared_memory::allocator<char>::type
    char_allocator;
typedef boost::interprocess::basic_string<char, std::char_traits<char>,
                                          char_allocator>
    shm_string;

struct User {
  uint32_t uid;
  shm_string name;
  uint32_t exp_pers;
  uint32_t activity;

  User(const char_allocator &a) : name("", a) {}
  User(uint32_t uid_, uint32_t exp_pers_, uint32_t activity_, const char *name_,
       const char_allocator &a)
      : uid(uid_), name(name_, a), exp_pers(exp_pers_), activity(activity_) {}

  uint32_t by_exp_pers() const { return exp_pers; }
  uint32_t by_hybrid() const { return exp_pers * 0.7 + activity * 0.3; }

  void assign(boost::property_tree::ptree::iterator iter, char_allocator &ca) {
    uid = (iter++)->second.get_value<uint32_t>();
    name = shm_string((iter++)->second.get_value<std::string>().c_str(), ca);
    exp_pers = (iter++)->second.get_value<uint32_t>();
    activity = (iter++)->second.get_value<uint32_t>();
  }

  bool operator<(const User &user) const { return uid < user.uid; }

  friend std::ostream &operator<<(std::ostream &os, shm_string &str) {
    for (auto ch : str) os << static_cast<char>(ch);
    return os;
  }

  friend std::ostream &operator<<(std::ostream &out, const User &user) {
    out << "User: " << user.uid << "\tname: " << user.name
        << "\texp_pers: " << user.exp_pers << "\tactivity: " << user.activity
        << std::endl;
    return out;
  }
};

struct tag_uid {};
struct tag_exp_pers {};
struct tag_activity {};
struct tag_hybrid {};

typedef boost::multi_index_container<
    User,
    boost::multi_index::indexed_by<
        boost::multi_index::ranked_non_unique<
            boost::multi_index::tag<tag_exp_pers>,
            boost::multi_index::const_mem_fun<User, uint32_t,
                                              &User::by_exp_pers>,
            std::greater<uint32_t>>,
        boost::multi_index::ranked_non_unique<
            boost::multi_index::tag<tag_activity>,
            boost::multi_index::member<User, uint32_t, &User::activity>,
            std::greater<uint32_t>>,
        boost::multi_index::ranked_non_unique<
            boost::multi_index::tag<tag_hybrid>,
            boost::multi_index::const_mem_fun<User, uint32_t, &User::by_hybrid>,
            std::greater<uint32_t>>,
        boost::multi_index::ordered_unique<
            boost::multi_index::tag<tag_uid>,
            boost::multi_index::member<User, uint32_t, &User::uid>>>,
    boost::interprocess::managed_shared_memory::allocator<User>::type>

    container_t;

typedef container_t::index<tag_uid>::type uid_index_t;
typedef container_t::index<tag_exp_pers>::type exp_pers_index_t;
typedef container_t::index<tag_activity>::type activity_index_t;
typedef container_t::index<tag_hybrid>::type hybrid_index_t;

class Ranking {
 private:
  container_t *users;
  uid_index_t *uid_index;
  exp_pers_index_t *exp_pers_index;
  activity_index_t *activity_index;
  hybrid_index_t *hybrid_index;

  // interprocess
  boost::interprocess::managed_shared_memory *segment;
  char_allocator *ca_ptr;
  std::string mem_obj = "MySharedMemory";
  struct shm_remove {
    shm_remove() {
      boost::interprocess::shared_memory_object::remove("MySharedMemory");
    }
    ~shm_remove() {
      boost::interprocess::shared_memory_object::remove("MySharedMemory");
    }
  } remover;

  void init_index() {
    uid_index = &boost::get<tag_uid>(*users);
    exp_pers_index = &boost::get<tag_exp_pers>(*users);
    activity_index = &boost::get<tag_activity>(*users);
    hybrid_index = &boost::get<tag_hybrid>(*users);
  }

 public:
  Ranking(uint64_t mem_size = 1 << 20) {
    segment = new boost::interprocess::managed_shared_memory(
        boost::interprocess::create_only, "MySharedMemory", mem_size);

    users = segment->construct<container_t>("My MultiIndex Container")(
        container_t::ctor_args_list(), segment->get_allocator<User>());

    ca_ptr = new char_allocator(segment->get_allocator<char>());

    init_index();
  }

  void clear() { users->clear(); }

  inline auto &get_ca() { return *ca_ptr; }

  inline auto get_user(uint32_t uid) {
    auto iter = uid_index->find(uid);
    if (iter == uid_index->end()) throw NoneOfUidException(uid);
    return iter;
  }

  void put_user(User const &user) { users->insert(user); }

  void modify_user(User const &user) {
    auto iter = get_user(user.uid);
    uid_index->modify(iter, [&user](User &user_) { user_ = user; });
  }

  void remove_user(uint32_t uid) {
    auto iter = get_user(uid);
    uid_index->erase(iter);
  }

  uint32_t get_size() { return users->size(); }

  uint32_t get_exp_pers_rank(uint32_t uid) {
    return exp_pers_index->find_rank(get_user(uid)->exp_pers);
  }

  uint32_t get_activity_rank(uint32_t uid) {
    return activity_index->find_rank(get_user(uid)->activity);
  }

  uint32_t get_hybrid_rank(u_int32_t uid) {
    return hybrid_index->find_rank(get_user(uid)->by_hybrid());
  }
};

#endif  // !_RANKING_HPP_
