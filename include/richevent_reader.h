#ifndef __RICHEVENT_READER_H__
#define __RICHEVENT_READER_H__

#include "nlohmann/json.hpp"
#include <czmq.h>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>

namespace chkchk {

class RichEventReader;
struct reader_ctx;

using rbase_cb_fn = std::function<bool(zsock_t *, struct reader_ctx &)>;

typedef struct reader_ctx {
  RichEventReader *self;
  int event_type;
  rbase_cb_fn cb;
  std::string endpoint;
  std::string service;
  bool is_listen;
} reader_ctx_t;

class RichEventReader {
private:
  enum {
    RICH_EVENT_SUB,
    RICH_EVENT_PULL,
    RICH_EVENT_REP,
  };

private:
  zpoller_t *_poller;
  std::map<zsock_t *, reader_ctx_t> _readers;

public:
  RichEventReader();
  virtual ~RichEventReader();
  bool register_sub(const char *service, const char *endpoint,
                    rbase_cb_fn callback);
  bool register_pull(const char *service, const char *endpoint,
                     rbase_cb_fn callback);
  bool register_rep(const char *service, const char *endpoint,
                    rbase_cb_fn callback);

public:
  bool send_json(zsock_t *zsock, reader_ctx_t &ctx, nlohmann::json &data);
  bool send(zsock_t *zsock, reader_ctx_t &ctx, const std::string &data);
  std::optional<char *> recv(zsock_t *zsock, reader_ctx_t &ctx);
  std::optional<nlohmann::json> recv_json(zsock_t *zsock, reader_ctx_t &ctx);
  void event_loop();
  void event_once(int ms);

private:
  bool recovery(zsock_t *zsock);
  bool register_reader(int type, const char *service, const char *endpoint,
                       rbase_cb_fn callback);
  void dispatch(int ms);
};

}; // namespace chkchk

#endif