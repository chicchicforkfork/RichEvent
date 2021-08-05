#include "richevent_reader.h"
#include <iostream>
#include <string>

using namespace chkchk;
using namespace std;
using namespace nlohmann;

RichEventReader::RichEventReader() {
  zsys_handler_set(NULL);

  _poller = zpoller_new(NULL);
  assert(_poller != nullptr);
}

RichEventReader::~RichEventReader() {
  for (auto &it : _zsock_readers) {
    zsock_t *zsock = it.first;
    zsock_destroy(&zsock);
    zpoller_remove(_poller, zsock);
  }
  zpoller_destroy(&_poller);
}

void RichEventReader::event_loop() {
  _running = true;
  while (_running) {
    dispatch(-1);
  }
}

void RichEventReader::event_once(int ms) { //
  dispatch(ms);
}

optional<char *> RichEventReader::recv(zsock_t *zsock, reader_ctx_t &ctx) {
  char *msg = nullptr;
  bool ok = false;

  switch (ctx.event_type) {
  case RICH_EVENT_SUB:
  case RICH_EVENT_PULL:
  case RICH_EVENT_REP:
    msg = zstr_recv(zsock);
    if (msg) {
      ok = true;
    }
    break;
  }

  if (!ok) {
    return nullopt;
  }
  return make_optional<char *>(msg);
}

std::optional<char *> RichEventReader::recv(const char *service) {
  auto it = _service_readers.find(service);
  if (it == _service_readers.end()) {
    return nullopt;
  }
  reader_ctx_t ctx = it->second;
  return recv(ctx.zsock, ctx);
}

optional<json> RichEventReader::recv_json(zsock_t *zsock, reader_ctx_t &ctx) {
  optional<char *> data = recv(zsock, ctx);
  char *msg = nullptr;
  if (data.has_value()) {
    msg = *data;
    json j = json::parse(msg);
    zstr_free(&msg);
    return make_optional<json>(j);
  }
  return nullopt;
}

std::optional<nlohmann::json> RichEventReader::recv_json(const char *service) {
  auto it = _service_readers.find(service);
  if (it == _service_readers.end()) {
    return nullopt;
  }
  reader_ctx_t ctx = it->second;
  return recv_json(ctx.zsock, ctx);
}

bool RichEventReader::send(zsock_t *zsock, reader_ctx_t &ctx,
                           const string &data) {
  if (ctx.event_type != RICH_EVENT_REP) {
    return false;
  }
  if (zstr_send(zsock, data.c_str()) != 0) {
    recovery(zsock);
    return false;
  }
  return true;
}

bool RichEventReader::send(const char *service, const std::string &data) { //
  auto it = _service_readers.find(service);
  if (it == _service_readers.end()) {
    return false;
  }
  reader_ctx_t ctx = it->second;
  return send(ctx.zsock, ctx, data);
}

bool RichEventReader::send_json(zsock_t *zsock, reader_ctx_t &ctx, json &data) {
  return send(zsock, ctx, data.dump());
}

bool RichEventReader::send_json(const char *service, nlohmann::json &data) {
  auto it = _service_readers.find(service);
  if (it == _service_readers.end()) {
    return false;
  }
  reader_ctx_t ctx = it->second;
  return send_json(ctx.zsock, ctx, data);
}

void RichEventReader::dispatch(int ms) {
  zsock_t *zsock = (zsock_t *)zpoller_wait(_poller, ms);
  auto it = _zsock_readers.find(zsock);
  if (it == _zsock_readers.end()) {
    return;
  }

  reader_ctx_t ctx = it->second;
  if (!ctx.cb(zsock, ctx)) {
    fprintf(stderr, "[dispatch] error callback: %s\n", ctx.service.c_str());
    recovery(zsock);
  }
}

bool RichEventReader::recovery(zsock_t *zsock) {
  auto it = _zsock_readers.find(zsock);
  if (it == _zsock_readers.end()) {
    return true;
  }
  reader_ctx_t ctx = it->second;
  if (ctx.is_listen) {
    return true;
  }
  zpoller_remove(_poller, zsock);
  zsock_destroy(&zsock);
  _service_readers.erase(ctx.service.c_str());
  _zsock_readers.erase(it);

  fprintf(stderr, "[recovery] re-register %s: %s\n", ctx.service.c_str(),
          ctx.endpoint.c_str());

  return register_reader(ctx.event_type, ctx.service.c_str(),
                         ctx.endpoint.c_str(), ctx.cb);
}

bool RichEventReader::register_sub(const char *service, const char *endpoint,
                                   rbase_cb_fn callback) {
  return register_reader(RICH_EVENT_SUB, service, endpoint, callback);
}

bool RichEventReader::register_pull(const char *service, const char *endpoint,
                                    rbase_cb_fn callback) {
  return register_reader(RICH_EVENT_PULL, service, endpoint, callback);
}

bool RichEventReader::register_rep(const char *service, const char *endpoint,
                                   rbase_cb_fn callback) {
  return register_reader(RICH_EVENT_REP, service, endpoint, callback);
}

bool RichEventReader::register_reader(int type, const char *service,
                                      const char *endpoint,
                                      rbase_cb_fn callback) {
  zsock_t *zsock = nullptr;
  bool is_listen = true;

  if (_service_readers.find(service) != _service_readers.end()) {
    fprintf(stderr, "already registered service: %s\n", service);
    return false;
  }

  switch (type) {
  case RICH_EVENT_SUB:
    zsock = zsock_new_sub(endpoint, "");
    is_listen = false;
    break;
  case RICH_EVENT_PULL:
    zsock = zsock_new_pull(endpoint);
    break;
  case RICH_EVENT_REP:
    zsock = zsock_new_rep(endpoint);
    break;
  }
  if (!zsock) {
    return false;
  }

  zpoller_add(_poller, zsock);

  reader_ctx_t ctx;
  ctx.zsock = zsock;
  ctx.self = this;
  ctx.event_type = type;
  ctx.cb = callback;
  ctx.endpoint = endpoint;
  ctx.service = service;
  ctx.is_listen = is_listen;

  _zsock_readers[zsock] = ctx;
  _service_readers[service] = ctx;
  return true;
}
