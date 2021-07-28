#ifndef __RICHEVENT_WRITER_H__
#define __RICHEVENT_WRITER_H__

#include "nlohmann/json.hpp"
#include <czmq.h>
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace chkchk {

class RichEventWriter;

typedef struct writer_ctx {
  RichEventWriter *self;
  int event_type;
  std::string endpoint;
  std::string service;
  zsock_t *zsock;
} writer_ctx_t;

class RichEventWriter {
private:
  enum {
    RICH_EVENT_PUB,
    RICH_EVENT_PUSH,
    RICH_EVENT_REQ,
  };
  std::map<std::string, writer_ctx_t> _writers;

public:
  RichEventWriter();
  virtual ~RichEventWriter();
  bool register_pub(const char *service, const char *endpoint);
  bool register_push(const char *service, const char *endpoint);
  bool register_req(const char *service, const char *endpoint);
  bool send(const char *service, const char *msg);
  bool send_json(const char *service, nlohmann::json &j);
  std::optional<char *> recv(const char *service);
  std::optional<nlohmann::json> recv_json(const char *service);

private:
  bool register_writer(int type, const char *service, const char *endpoint);
};

}; // namespace chkchk

#endif