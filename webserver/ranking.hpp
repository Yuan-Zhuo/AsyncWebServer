#include <stdint.h>

#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/ranked_index.hpp>
#include <boost/multi_index_container.hpp>
#include <iostream>
#include <string>

struct User {
  User() {}
  User(uint32_t uid_, const std::string &name_, uint32_t exp_pers_,
       uint32_t active_, uint32_t exp_gang_)
      : uid(uid_),
        name(name_),
        exp_pers(exp_pers_),
        active(active_),
        exp_gang(exp_gang_) {}

  uint32_t uid;
  std::string name;
  uint32_t exp_pers;
  uint32_t active;
  uint32_t exp_gang;

  uint32_t by_exp_pers() const { return exp_pers; }
  uint32_t by_exp_gang() const { return exp_gang; }

  friend std::ostream &operator<<(std::ostream &out, const User &user) {
    out << user.uid << "\t" << user.name << '\t' << user.exp_pers << '\t'
        << user.active << '\t' << user.exp_gang << std::endl;
    return out;
  }
};

struct tag_uid {};
struct tag_exp_pers {};
struct tag_active {};
struct tag_exp_gang {};
struct tag_random_access {};

typedef boost::multi_index_container<
    User, boost::multi_index::indexed_by<
              boost::multi_index::ranked_non_unique<
                  boost::multi_index::tag<tag_exp_pers>,
                  boost::multi_index::const_mem_fun<User, uint32_t,
                                                    &User::by_exp_pers>,
                  std::greater<uint32_t>>,
              boost::multi_index::ranked_non_unique<
                  boost::multi_index::tag<tag_active>,
                  boost::multi_index::member<User, uint32_t, &User::active>,
                  std::greater<uint32_t>>,
              boost::multi_index::ranked_non_unique<
                  boost::multi_index::tag<tag_exp_gang>,
                  boost::multi_index::const_mem_fun<User, uint32_t,
                                                    &User::by_exp_gang>,
                  std::greater<uint32_t>>,
              boost::multi_index::ordered_unique<
                  boost::multi_index::tag<tag_uid>,
                  boost::multi_index::member<User, uint32_t, &User::uid>>,
              boost::multi_index::random_access<
                  boost::multi_index::tag<tag_random_access>>>>

    container_t;

// // templated typedef
// template <typename tag>
// struct Index_Type {
//   typedef container_t::index<tag>::type index_t;
// };

typedef container_t::index<tag_uid>::type uid_index_t;
typedef container_t::index<tag_exp_pers>::type exp_pers_index_t;
typedef container_t::index<tag_exp_gang>::type exp_gang_index_t;
typedef container_t::index<tag_active>::type active_index_t;
typedef container_t::index<tag_random_access>::type random_access_index_t;

class Ranking {
 private:
  container_t users;
  uid_index_t &uid_index = boost::get<tag_uid>(users);
  exp_pers_index_t &exp_pers_index = boost::get<tag_exp_pers>(users);
  exp_gang_index_t &exp_gang_index = boost::get<tag_exp_gang>(users);
  active_index_t &active_index = boost::get<tag_active>(users);
  random_access_index_t &random_access_index =
      boost::get<tag_random_access>(users);

  void modify(User &dest, User src) { dest.name = src.name; }

 public:
  Ranking() {}

  void put_user(User const &user) { users.insert(user); }

  void modify_user(User const &user) {
    auto iter = uid_index.find(user.uid);
    if (iter != uid_index.end()) {
      uid_index.modify(iter, [&user](User &user_) { user_ = user; });
    }
  }

  void remove_user(uint32_t uid) {
    auto iter = uid_index.find(uid);
    if (iter != uid_index.end()) uid_index.erase(iter);
  }

  User *get_user(uint32_t uid) {
    auto iter = uid_index.find(uid);
    if (iter != uid_index.end())
      return (User *)&(*iter);
    else
      return nullptr;
  }

  uint32_t get_exp_pers_rank(uint32_t uid) {
    auto iter = uid_index.find(uid);
    return exp_pers_index.find_rank(iter->exp_pers);
  }

  uint32_t get_exp_gang_rank(uint32_t uid) {
    auto iter = uid_index.find(uid);
    return exp_gang_index.find_rank(iter->exp_gang);
  }

  uint32_t get_active_rank(uint32_t uid) {
    auto iter = uid_index.find(uid);
    return active_index.find_rank(iter->active);
  }
};