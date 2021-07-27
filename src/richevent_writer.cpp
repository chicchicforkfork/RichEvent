#include "richevent_writer.h"
#include <iostream>
#include <string>

using namespace chkchk;
using namespace nlohmann;
using namespace std;

RichEventWriter::RichEventWriter(bool auto_connection) {
  _auto_connection = auto_connection;
  // ignore zmq signal
  zsys_handler_set(NULL);
}

RichEventWriter::~RichEventWriter() {
  for (auto &it : _writers) {
    writer_context_t &ctx = it.second;
    zsock_destroy(&ctx.zsock);
  }
}

bool RichEventWriter::send_json(const char *service, json &j) {
  return send(service, j.dump().c_str());
}

bool RichEventWriter::send(const char *service, const char *msg) {
  bool ok = false;
  auto it = _writers.find(service);
  if (it == _writers.end()) {
    return false;
  }

  int rc1;
  int rc2;
  writer_context_t &ctx = it->second;
  switch (ctx.event_type) {
  case RICH_EVENT_PUB:
    if (zsock_send(ctx.zsock, "ss", service, msg) != -1) {
      ok = true;
    }
    break;
  case RICH_EVENT_PUSH:
    rc1 = zstr_send(ctx.zsock, msg);
    //rc2 = zsock_wait(ctx.zsock);
    //printf("...........%d, %d\n", rc1, rc2);
    if (rc1 != 0) {
      printf("eeeeeeeeeeeeeeeeeeeeeeee\n");
      exit(1);
    }
    if (rc1 != -1) {
      ok = true;
    }
    break;
  case RICH_EVENT_REQ:
    if (zstr_send(ctx.zsock, msg) != -1) {
      ok = true;
    }
    break;
  }

  if (!ok) {
    zsock_destroy(&ctx.zsock);
    _writers.erase(it);
    return false;
  }

  return true;
}

optional<string> RichEventWriter::recv(const char *service) {
  char *msg = nullptr;
  auto it = _writers.find(service);
  if (it == _writers.end()) {
    return nullopt;
  }

  writer_context_t &ctx = it->second;
  msg = zstr_recv(ctx.zsock);
  if (!msg) {
    _writers.erase(it);
    zsock_destroy(&ctx.zsock);
    return nullopt;
  }
  string data = string(msg);
  free(msg);
  return data;
}

optional<json> RichEventWriter::recv_json(const char *service) {
  auto data = recv(service);
  if (data.has_value()) {
    return json::parse(data->c_str());
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
  zsock_t *writer = nullptr;
    int sndhwm = 1;

  switch (type) {
  case RICH_EVENT_PUB:
    writer = zsock_new_pub(endpoint);
    break;
  case RICH_EVENT_PUSH:
    writer = zsock_new_push(endpoint);    
    break;
  case RICH_EVENT_REQ:
    writer = zsock_new_req(endpoint);
    break;
  }
  if (!writer) {
    return false;
  }
  zmq_setsockopt(writer, ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm));

  writer_context_t ctx;
  ctx.self = this;
  ctx.endpoint = endpoint;
  ctx.service = service;
  ctx.zsock = writer;
  ctx.event_type = type;

  _writers[service] = ctx;
  return true;
}
