#include "nlohmann/json.hpp"
#include "richevent_reader.h"
#include <iostream>

using namespace chkchk;
using namespace std;
using namespace nlohmann;

int main() {
  bool print = true;
  RichEventReader reader;
  reader.register_pull("test1", "ipc://kaka",
                       [&](zsock_t *zsock, reader_context_t &ctx) {
                         auto j = RichEventReader::recv_json(zsock, ctx);
                         if (!j.has_value()) {
                           return false;
                         }
                        if (print) {
                         std::cout
                             << "[pull] recv from port 5557: " << (*j)["id"]
                             << std::endl;
                        }
                         return true;
                       });

  reader.register_sub("test2", "tcp://127.0.0.1:5558",
                      [&](zsock_t *zsock, reader_context_t &ctx) {
                        auto j = RichEventReader::recv_json(zsock, ctx);
                        if (!j.has_value()) {
                          return false;
                        }
if (print) {
                        std::cout << "[pull] recv from port 5558: " << j->dump()
                                  << std::endl;
}
                        return true;
                      });

  reader.register_rep("test2", "tcp://127.0.0.1:5559",
                      [&](zsock_t *zsock, reader_context_t &ctx) {
                        auto j = RichEventReader::recv_json(zsock, ctx);
                        if (!j.has_value()) {
                          return false;
                        }

                        json jp;
                        jp["result"] = 1;
                        RichEventReader::send_json(zsock, ctx, jp);
                        if (print) {
                        std::cout << "[pull] recv from port 5559: " << j->dump()
                                  << std::endl;
                        }
                        return true;
                      });

  reader.event_loop();
}