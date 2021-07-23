#include "nlohmann/json.hpp"
#include "richevent_reader.h"
#include <iostream>

using namespace chkchk;

int main() {
  RichEventReader reader;
  reader.register_pull("test1", "tcp://127.0.0.1:5557",
                       [](zsock_t *zsock, RichEventReader *self) {
                         (void)self;
                         char *msg = zstr_recv(zsock);
                         if (!msg)
                           return false;

                         std::cout << "recv from port 5557: " << msg
                                   << std::endl;
                         zstr_free(&msg);
                         return true;
                       });

  reader.event_loop();
}