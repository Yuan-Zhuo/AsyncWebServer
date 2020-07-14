#include "server.hpp"

int main() {
  Server server(10000, 10);
  server.start();
  return 0;
}
