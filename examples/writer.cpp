#include "nlohmann/json.hpp"
#include "richevent_writer.h"
#include <iostream>

using namespace std;
using namespace chkchk;

int main(int argc, char **argv) {
  int c = atoi(argv[1]);
  RichEventWriter writer;
  writer.register_push("test1", "ipc://kaka");
  writer.register_pub("test2", "tcp://127.0.0.1:5558");
  writer.register_req("test3", "tcp://127.0.0.1:5559");
  char data[1024];

  for (int i=0; i<sizeof(data)-1; i++) {
    data[i] = 'a';
  }
  data[sizeof(data)-1] = 0;

bool ok;
  for (int i=0; i<c; i++) {
    // writer.publish("test1", "push test1 aaaaaaaaaaaaaaaa");
    nlohmann::json j;
    j["id"] = i;
    j["message"] = data;
    ok = writer.send_json("test1", j);
    //printf("[%d] %d\n", i, ok);
/*
    j["id"] = cnt;
    j["message"] = "bbbbbbbbbbbbbbb";
    writer.send_json("test2", j);

    j["id"] = cnt;
    j["message"] = "ccccccccccccccccccc";
    writer.send_json("test3", j);
    cout << *writer.recv("test3") << endl;

    sleep(1);
*/
  }
  getchar();
}