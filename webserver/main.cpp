#include "server.hpp"

int main() {
  Server server(10000);
  server.start();
  return 0;
}
