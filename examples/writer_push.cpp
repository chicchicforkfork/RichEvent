#include "nlohmann/json.hpp"
#include "richevent_writer.h"
#include <iostream>

using namespace chkchk;

int main() {
  RichEventWriter writer;
  writer.register_push("test1", "tcp://127.0.0.1:5557");

  writer.publish("test1", "aaaaaaaaaaaaaaaaa");
}