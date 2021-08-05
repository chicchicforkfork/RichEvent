#include "kairos.h"
#include "nlohmann/json.hpp"
#include "richevent.h"
#include <getopt.h>
#include <iostream>

using namespace std;
using namespace chkchk;
using namespace nlohmann;

int main(int argc, char **argv) {
  int opt;
  int count = 1;
  int type = 4;
  int sock = 0;
  int datasize = 1024;
  while ((opt = getopt(argc, argv, "s:t:c:pu")) != -1) {
    switch (opt) {
    default:
      printf(" -c count\n");
      printf(" -p tcp mode\n");
      printf(" -u uds mode\n");
      printf(" -t type [1|2|3|4]\n");
      exit(1);
    case 's':
      datasize = atoi(optarg);
      break;
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
  const char *notify[] = {"ipc://kaka4", "tcp://127.0.0.1:5559"};

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
  {
    writer.register_push("notify", notify[sock]);
    printf("register notify\n");
    json jnotify;
    jnotify["count"] = count;
    writer.send_json("notify", jnotify);
  }

  char *data = (char *)malloc(datasize);
  for (size_t i = 0; i < datasize - 1; i++) {
    data[i] = 'a';
  }
  data[datasize - 1] = 0;

  printf("\nkey press Enter for writer\n");
  getchar();

  bool ok;
  int success = 0;
  int failure = 0;

  for (int i = 0; i < count; i++) {
    nlohmann::json j;
    j["id"] = i;
    j["message"] = data;

    if (type == 1 || type == 4) {
      ok = writer.send_json("test1", j);
      if (ok) {
        // printf("send #%d\n", success);
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

  printf("[data size]: %d\n", datasize);
  printf("[count]: %d\n", count);
  printf("[writer] success: %d, failure:%d\n", success, failure);
  getchar();
}