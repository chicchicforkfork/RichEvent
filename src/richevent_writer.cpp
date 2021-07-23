#include "richevent_writer.h"
#include <iostream>
#include <string>

using namespace chkchk;

RichEventWriter::RichEventWriter(bool auto_connection) {
  _auto_connection = auto_connection;
  // ignore zmq signal
  zsys_handler_set(NULL);
}

RichEventWriter::~RichEventWriter() {
  for (auto it : _service_map) {
    zsock_t *zsock = it.second;
    zsock_destroy(&zsock);
  }
}

bool RichEventWriter::publish(const char *service, const char *msg) {
  int rc;
  zsock_t *writer = _service_map[service];

  switch (zsock_type(writer)) {
  case ZMQ_PUB:
    rc = zstr_send(writer, msg);
    zsock_flush(writer);
    break;
  case ZMQ_PUSH:
    rc = zstr_send(writer, msg);
    zsock_flush(writer);
    break;
  default:
    return false;
  }

  if (rc < 0) {
    zsock_destroy(&writer);
    _service_map.erase(service);
  }

  return true;
}

bool RichEventWriter::register_pub(const char *service, const char *endpoint) {
  return register_writer(RICH_EVENT_PUB, service, endpoint);
}

bool RichEventWriter::register_push(const char *service, const char *endpoint) {
  return register_writer(RICH_EVENT_PUSH, service, endpoint);
}

bool RichEventWriter::register_writer(int type, const char *service,
                                      const char *endpoint) {
  zsock_t *writer = nullptr;

  if (type == RICH_EVENT_PUB) {
    writer = zsock_new_pub(endpoint);
  } else if (type == RICH_EVENT_PUSH) {
    writer = zsock_new_push(endpoint);
  }
  if (!writer) {
    return false;
  }
  _service_map[service] = writer;
  return true;
}
