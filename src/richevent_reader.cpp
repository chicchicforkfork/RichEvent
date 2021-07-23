#include "richevent_reader.h"
#include <iostream>
#include <string>

using namespace chkchk;

RichEventReader::RichEventReader() {
  // ignore zmq signal
  zsys_handler_set(NULL);
  _poller = zpoller_new(NULL);
}

RichEventReader::~RichEventReader() {
  for (auto it : _callback_map) {
    zsock_t *zsock = it.first;
    zsock_destroy(&zsock);
    zpoller_remove(_poller, zsock);
  }
  zpoller_destroy(&_poller);
}

void RichEventReader::event_loop() {
  while (true) {
    dispatch(-1);
  }
}

void RichEventReader::event_once(int ms) { //
  dispatch(ms);
}

void RichEventReader::dispatch(int ms) {
  zsock_t *reader = (zsock_t *)zpoller_wait(_poller, ms);
  auto cb_iter = _callback_map.find(reader);
  if (cb_iter == _callback_map.end()) {
    zpoller_remove(_poller, reader);
    zsock_destroy(&reader);
  }

  rbase_cb_fn callback = cb_iter->second;
  if (!callback(reader, this)) {
    zpoller_remove(_poller, reader);
    zsock_destroy(&reader);
    _callback_map.erase(cb_iter);
  }
}

bool RichEventReader::register_sub(const char *service, const char *endpoint,
                                   rbase_cb_fn callback) {
  return subscribe(RICH_EVENT_SUB, service, endpoint, callback);
}

bool RichEventReader::register_pull(const char *service, const char *endpoint,
                                    rbase_cb_fn callback) {
  return subscribe(RICH_EVENT_SUB, service, endpoint, callback);
}

bool RichEventReader::subscribe(int type, const char *service,
                                const char *endpoint, rbase_cb_fn callback) {
  SOCKET sockfd;
  zsock_t *reader = nullptr;

  if (type == RICH_EVENT_SUB) {
    reader = zsock_new_sub(endpoint, service);
  } else if (type == RICH_EVENT_SUB) {
    reader = zsock_new_pull(endpoint);
  }

  if (!reader) {
    return false;
  }
  sockfd = zsock_fd(reader);
  if (sockfd <= 0) {
    zsock_destroy(&reader);
    return false;
  }
  zpoller_add(_poller, reader);

  _callback_map[reader] = callback;
  _service_map[reader] = service;
  return true;
}
