#include "richevent_writer.h"
#include <iostream>
#include <string>

using namespace chkchk;
using namespace nlohmann;
using namespace std;

RichEventWriter::RichEventWriter() { //
  zsys_handler_set(NULL);
}

RichEventWriter::~RichEventWriter() {
  for (auto &it : _writers) {
    writer_ctx_t &ctx = it.second;
    zsock_destroy(&ctx.zsock);
  }
}

bool RichEventWriter::send(const char *service, const char *msg) {
  bool ok = false;
  auto it = _writers.find(service);
  if (it == _writers.end()) {
    return false;
  }

  writer_ctx_t &ctx = it->second;
  switch (ctx.event_type) {
  case RICH_EVENT_PUB:
  case RICH_EVENT_PUSH:
  case RICH_EVENT_REQ:
    if (zstr_send(ctx.zsock, msg) == 0) {
      // zsock_flush(ctx.zsock);
      ok = true;
    }
    break;
  }
  return ok;
}

bool RichEventWriter::send_json(const char *service, json &j) {
  return send(service, j.dump().c_str());
}

optional<char *> RichEventWriter::recv(const char *service) {
  char *msg = nullptr;
  auto it = _writers.find(service);
  if (it == _writers.end()) {
    return nullopt;
  }

  writer_ctx_t &ctx = it->second;
  msg = zstr_recv(ctx.zsock);
  if (!msg) {
    return nullopt;
  }
  return make_optional<char *>(msg);
}

optional<json> RichEventWriter::recv_json(const char *service) {
  optional<char *> data = recv(service);
  char *msg = nullptr;
  if (data.has_value()) {
    msg = *data;
    json j = json::parse(msg);
    zstr_free(&msg);
    return make_optional<json>(j);
  }
  return nullopt;
}

bool RichEventWriter::register_pub(const char *service, const char *endpoint) {
  return register_writer(RICH_EVENT_PUB, service, endpoint);
}

bool RichEventWriter::register_push(const char *service, const char *endpoint) {
  return register_writer(RICH_EVENT_PUSH, service, endpoint);
}

bool RichEventWriter::register_req(const char *service, const char *endpoint) {
  return register_writer(RICH_EVENT_REQ, service, endpoint);
}

bool RichEventWriter::register_writer(int type, const char *service,
                                      const char *endpoint) {
  zsock_t *zsock = nullptr;

  switch (type) {
  case RICH_EVENT_PUB:
    zsock = zsock_new_pub(endpoint);
    break;
  case RICH_EVENT_PUSH:
    zsock = zsock_new_push(endpoint);
    break;
  case RICH_EVENT_REQ:
    zsock = zsock_new_req(endpoint);
    break;
  }
  if (!zsock) {
    return false;
  }

  writer_ctx_t ctx;
  ctx.self = this;
  ctx.endpoint = endpoint;
  ctx.service = service;
  ctx.zsock = zsock;
  ctx.event_type = type;

  _writers[service] = ctx;
  return true;
}
