#include "kairos.h"
#include "nlohmann/json.hpp"
#include "richevent.h"
#include <getopt.h>
#include <iostream>

using namespace chkchk;
using namespace std;
using namespace nlohmann;

int main(int argc, char **argv) {
  int opt;
  int sock = 0;
  while ((opt = getopt(argc, argv, "pu")) != -1) {
    switch (opt) {
    default:
      printf(" -p tcp mode\n");
      printf(" -u uds mode\n");
      exit(1);
    case 'p':
      sock = 1;
      break;
    case 'u':
      sock = 0;
      break;
    }
  }

  bool print = false;

  const char *test1[] = {"ipc://kaka1", "tcp://127.0.0.1:5556"};
  const char *test2[] = {"ipc://kaka2", "tcp://127.0.0.1:5557"};
  const char *test3[] = {"ipc://kaka3", "tcp://127.0.0.1:5558"};
  const char *notify[] = {"ipc://kaka4", "tcp://127.0.0.1:5559"};

  int req_count;
  int recv_count = 0;
  KairosStack kstack("benchmark", 1000);
  Kairos kairos = Kairos("reader");
  RichEventReader reader;
  reader.register_pull("test1", test1[sock], //
                       [&](zsock_t *zsock, reader_ctx_t &ctx) {
                         auto j = reader.recv_json(zsock, ctx);
                         if (!j.has_value()) {
                           return false;
                         }
                         if (print) {
                           cout << test1[sock] << ": " << (*j)["id"] << endl;
                         }
                         recv_count++;

                         if (recv_count == 1) {
                           kairos.begin();
                         } else if (recv_count == req_count) {
                           kairos.end();
                           kstack.addKairos(kairos);
                           cout << kstack.toString();
                           recv_count = 0;
                           req_count = 0;
                         }
                         return true;
                       });

  reader.register_sub("test2", test2[sock], //
                      [&](zsock_t *zsock, reader_ctx_t &ctx) {
                        auto j = reader.recv_json(zsock, ctx);
                        if (!j.has_value()) {
                          return false;
                        }
                        if (print) {
                          cout << test2[sock] << ": " << (*j)["id"] << endl;
                        }
                        return true;
                      });

  reader.register_rep("test3", test3[sock], //
                      [&](zsock_t *zsock, reader_ctx_t &ctx) {
                        auto j = reader.recv_json(zsock, ctx);
                        if (!j.has_value()) {
                          return false;
                        }
                        if (print) {
                          cout << test3[sock] << ": " << (*j)["id"] << endl;
                        }

                        json jp;
                        jp["result"] = 1;
                        reader.send_json(zsock, ctx, jp);

                        return true;
                      });

  reader.register_pull("notify", notify[sock], //
                       [&](zsock_t *zsock, reader_ctx_t &ctx) {
                         auto j = reader.recv_json(zsock, ctx);
                         if (!j.has_value()) {
                           return false;
                         }
                         req_count = (*j)["count"].get<int>();
                         cout << "req_count: " << req_count << endl;
                         return true;
                       });

  reader.event_loop();
}