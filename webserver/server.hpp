#ifndef _SERVER_HPP_
#define _SERVER_HPP_

#define BOOST_BIND_GLOBAL_PLACEHOLDERS

#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <cassert>
#include <iostream>
#include <list>
#include <regex>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "exception.hpp"
#include "ranking.hpp"

struct Request {
  std::string method, path, http_version;
  std::string content;
  std::unordered_map<std::string, std::string> header;
  std::smatch path_match;
};

// path --- method --- function
typedef std::map<std::string,
                 std::unordered_map<
                     std::string, std::function<void(std::ostream&, Request&)>>>
    rc_t;

class Server {
 private:
  boost::asio::io_service io_service;
  boost::asio::ip::tcp::endpoint endpoint;
  boost::asio::ip::tcp::acceptor acceptor;
  boost::asio::signal_set signals;
  boost::asio::io_service::work work;
  rc_t rc;
  rc_t exception_rc;
  std::vector<rc_t::iterator> rc_vec;
  std::list<std::string> json_fields;
  uint32_t service_cnt;
  std::vector<std::thread> threads;
  const std::thread::id main_thread_id;

  Ranking rank;

  void handler(const boost::system::error_code& error, int signal_number) {
    if (!error) {
      std::cout << "Bye!" << std::endl;
      exit(1);
    }
  }

  void config_signal() {
    signals.add(SIGINT);
    signals.async_wait(boost::bind(&Server::handler, this, _1, _2));
  }

  void config_json() {
    const char* cstrs[] = {"uid", "name", "exp_pers", "active", "exp_gang"};
    json_fields.assign(cstrs, std::end(cstrs));
  }

  void write_response(std::ostream& response,
                      std::stringstream& content_stream) {
    content_stream.seekp(0, std::ios::end);
    response << "HTTP/1.1 200 OK\r\n"
             // length
             << "Content-Length: " << content_stream.tellp()
             << "\r\n"
             // cors
             << "Access-Control-Allow-Origin: *\r\n"
             << "Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n"
             << "Access-Control-Allow-Credentials: true\r\n"
             << "Access-Control-Allow-Headers: *\r\n"
             // end headers
             << "\r\n"
             // content
             << content_stream.rdbuf();
  }

  void config_rc() {
    // rc
    // get info by uid
    rc["(/info\\\?uid=)(\\d+)$"]["GET"] = [this](std::ostream& response,
                                                 Request& request) {
      std::stringstream content_stream;

      try {
        uint32_t uid = std::stoul(request.path_match[2], 0, 10);
        auto iter = rank.get_user(uid);
        content_stream << (*iter);
      } catch (const NoneOfUidException& e) {
        content_stream << "User " << e.what() << " doesn't exist.";
      }
      // will not caused by stoul bacause of regex match
      // catch (std::invalid_argument& e) {
      //   content_stream << "Bad Param";
      // }

      write_response(response, content_stream);
    };

    // put user
    rc["/put$"]["POST"] = [this](std::ostream& response, Request& request) {
      std::stringstream post_stream, content_stream;
      post_stream << request.content;
      boost::property_tree::ptree pt;

      try {
        boost::property_tree::read_json(post_stream, pt);
        auto iter = pt.begin();

        // parse
        for (auto field : json_fields)
          if (((iter++)->first).compare(field))
            throw IncorrectHttpRequestException(field);

        User user;
        user.assign(pt.begin());
        rank.put_user(user);
        std::cout << user;
        content_stream << "Put Successfully";
      } catch (const IncorrectHttpRequestException& e) {
        std::cout << "HttpRequest Incorrect: " << e.what() << std::endl;
        content_stream << "Bad Put";
      } catch (boost::property_tree::ptree_error& e) {
        std::cout << e.what() << std::endl;
        content_stream << "Bad Param";
      }

      write_response(response, content_stream);
    };

    // remove user
    rc["(/remove\\\?uid=)(\\d+)$"]["GET"] = [this](std::ostream& response,
                                                   Request& request) {
      std::stringstream content_stream;

      try {
        uint32_t uid = std::stoul(request.path_match[2], 0, 10);
        rank.remove_user(uid);
        content_stream << "Remove Successfully";
      } catch (const NoneOfUidException& e) {
        content_stream << "User " << e.what() << " doesn't exist.";
      }

      write_response(response, content_stream);
    };

    // get exp_pers rank
    rc["(/get_exp_pers\\\?uid=)(\\d+)$"]["GET"] = [this](std::ostream& response,
                                                         Request& request) {
      std::stringstream content_stream;

      try {
        uint32_t uid = std::stoul(request.path_match[2], 0, 10);
        content_stream << "Exp_Pers Rank: " << rank.get_exp_pers_rank(uid);
      } catch (const NoneOfUidException& e) {
        content_stream << "User " << e.what() << " doesn't exist.";
      }

      write_response(response, content_stream);
    };

    // get active rank
    rc["(/get_active\\\?uid=)(\\d+)$"]["GET"] = [this](std::ostream& response,
                                                       Request& request) {
      std::stringstream content_stream;

      try {
        uint32_t uid = std::stoul(request.path_match[2], 0, 10);
        content_stream << "Active Rank: " << rank.get_active_rank(uid);
      } catch (const NoneOfUidException& e) {
        content_stream << "User " << e.what() << " doesn't exist.";
      }

      write_response(response, content_stream);
    };

    // get exp_gang rank
    rc["(/get_exp_gang\\\?uid=)(\\d+)$"]["GET"] = [this](std::ostream& response,
                                                         Request& request) {
      std::stringstream content_stream;

      try {
        uint32_t uid = std::stoul(request.path_match[2], 0, 10);
        content_stream << "Exp_Gang Rank: " << rank.get_exp_gang_rank(uid);
      } catch (const NoneOfUidException& e) {
        content_stream << "User " << e.what() << " doesn't exist.";
      }

      write_response(response, content_stream);
    };

    // exception_rc
    // get
    exception_rc["(.*)"]["GET"] = [this](std::ostream& response,
                                         Request& request) {
      std::stringstream content_stream;

      content_stream << "<h1>Bad GET</h1>";

      write_response(response, content_stream);
    };

    // options
    exception_rc["(.*)"]["OPTIONS"] = [this](std::ostream& response,
                                             Request& request) {
      std::stringstream content_stream;

      content_stream << "<h1>OPTIONS</h1>";

      write_response(response, content_stream);
    };

    // put into vec
    for (auto it = rc.begin(); it != rc.end(); it++) rc_vec.push_back(it);
    for (auto it = exception_rc.begin(); it != exception_rc.end(); it++)
      rc_vec.push_back(it);
  }

  void accept() {
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_service);

    acceptor.async_accept(*socket,
                          [this, socket](const boost::system::error_code& ec) {
                            accept();
                            if (!ec) process(socket);
                          });
  }

  void process(std::shared_ptr<boost::asio::ip::tcp::socket> socket) const {
    auto read_buffer = std::make_shared<boost::asio::streambuf>();
    boost::asio::async_read_until(
        *socket, *read_buffer, "\r\n\r\n",
        [this, socket, read_buffer](const boost::system::error_code& ec,
                                    size_t bytes_transferred) {
          if (ec) return;

          // parse request
          size_t total = read_buffer->size();
          std::istream stream(read_buffer.get());
          auto request = std::make_shared<Request>();
          *request = parse_request(stream);

          size_t num_additional_bytes = total - bytes_transferred;

          // handle request
          if (request->header.count("Content-Length") > 0) {
            boost::asio::async_read(
                *socket, *read_buffer,
                boost::asio::transfer_exactly(
                    stoull(request->header["Content-Length"]) -
                    num_additional_bytes),
                [this, socket, read_buffer, request](
                    const boost::system::error_code& ec,
                    size_t bytes_transferred) {
                  if (!ec) {
                    respond(socket, request);
                  }
                });
          } else {
            respond(socket, request);
          }
        });
  }

  void respond(std::shared_ptr<boost::asio::ip::tcp::socket> socket,
               std::shared_ptr<Request> request) const {
    for (auto res_it : rc_vec) {
      std::regex e(res_it->first);
      std::smatch sm_res;
      // path match
      if (std::regex_match(request->path, sm_res, e)) {
        // method match
        if (res_it->second.count(request->method)) {
          request->path_match = move(sm_res);

          auto write_buffer = std::make_shared<boost::asio::streambuf>();
          std::ostream response(write_buffer.get());
          res_it->second[request->method](response, *request);

          boost::asio::async_write(
              *socket, *write_buffer,
              [this, socket, request, write_buffer](
                  const boost::system::error_code& ec,
                  size_t bytes_transferred) {
                if (!ec && stof(request->http_version) > 1.05) process(socket);
              });
          return;
        }
      }
      // continue
    }
  }

  Request parse_request(std::istream& stream) const {
    Request request;
    std::regex e("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");

    std::smatch sub_match;

    std::string line;
    getline(stream, line);
    std::cout << line << std::endl;

    line.pop_back();
    if (std::regex_match(line, sub_match, e)) {
      request.method = sub_match[1];
      request.path = sub_match[2];
      request.http_version = sub_match[3];
      bool matched;
      e = "^([^:]*): ?(.*)$";
      do {
        getline(stream, line);
        line.pop_back();
        matched = std::regex_match(line, sub_match, e);
        if (matched) {
          request.header[sub_match[1]] = sub_match[2];
        }
      } while (matched == true);
    }
    // body
    if (!stream.eof()) getline(stream, request.content);

    return request;
  }

  inline void join_all_thread() {
    for (auto& t : threads) t.join();
    std::cout << "Bye!" << std::endl;
  }

  void config() {
    config_json();
    config_rc();
    config_signal();
  }

 public:
  Server(uint32_t port, u_int32_t service_cnt_ = 1)
      : endpoint(boost::asio::ip::tcp::v4(), port),
        acceptor(io_service, endpoint),
        signals(io_service),
        work(io_service),
        service_cnt(service_cnt_),
        main_thread_id(std::this_thread::get_id()) {}

  void start() {
    config();

    accept();

    for (uint32_t i = 0; i < service_cnt; i++) {
      threads.emplace_back([this]() { io_service.run(); });
    }

    // io_service.run();

    join_all_thread();
  }
};

#endif  // !_SERVER_HPP_
