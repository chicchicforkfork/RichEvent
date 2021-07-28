#include "nlohmann/json.hpp"
#include "richevent.h"
#include <getopt.h>
#include <iostream>
#include "kairos.h"

using namespace std;
using namespace chkchk;

int main(int argc, char **argv) {
  int opt;
  int count = 1;
  int type = 4;
  int sock = 0;
  while ((opt = getopt(argc, argv, "t:c:pu")) != -1) {
    switch (opt) {
    default:
      printf(" -c count\n");
      printf(" -p tcp mode\n");
      printf(" -u uds mode\n");
      printf(" -t type [1|2|3|4]\n");
      exit(1);
    case 't':
      type = atoi(optarg);
      break;
    case 'p':
      sock = 1;
      break;
    case 'u':
      sock = 0;
      break;
    case 'c':
      count = atoi(optarg);
      break;
    }
  }

  const char *test1[] = {"ipc://kaka1", "tcp://127.0.0.1:5556"};
  const char *test2[] = {"ipc://kaka2", "tcp://127.0.0.1:5557"};
  const char *test3[] = {"ipc://kaka3", "tcp://127.0.0.1:5558"};

  RichEventWriter writer;
  if (type == 1 || type == 4) {
    writer.register_push("test1", test1[sock]);
    printf("register push\n");
  }
  if (type == 2 || type == 4) {
    writer.register_pub("test2", test2[sock]);
    printf("register pub\n");
  }
  if (type == 3 || type == 4) {
    writer.register_req("test3", test3[sock]);
    printf("register req\n");
  }
  getchar();

  char data[1024];
  for (size_t i = 0; i < sizeof(data) - 1; i++) {
    data[i] = 'a';
  }
  data[sizeof(data) - 1] = 0;

  bool ok;
  size_t success = 0;
  size_t failure = 0;

  KairosStack kstack("benchmark", 10);

  Kairos kairos = Kairos("writer");
  kairos.begin();
  
  for (int i = 0; i < count; i++) {
    nlohmann::json j;
    j["id"] = i;
    j["message"] = data;

    if (type == 1 || type == 4) {
      ok = writer.send_json("test1", j);
      if (ok) {
        success++;
      } else {
        failure++;
        printf("error send: test1\n");
        break;
      }
    }

    if (type == 2 || type == 4) {
      ok = writer.send_json("test2", j);
      if (ok) {
        success++;
      } else {
        failure++;
        printf("error send: test2\n");
        break;
      }
    }

    if (type == 3 || type == 4) {
      ok = writer.send_json("test3", j);
      if (ok) {
        success++;
      } else {
        failure++;
        printf("error send: test3\n");
        break;
      }
    }
  }
  kairos.end();
  kstack.addKairos(kairos);

  printf("success: %ld, failure:%ld\n", success, failure);
  getchar();
  cout << kstack.toString();
}