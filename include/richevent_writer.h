#include <czmq.h>
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace chkchk {

class RichEventWriter {
private:
  enum {
    RICH_EVENT_PUB,
    RICH_EVENT_PUSH,
  };
  std::map<std::string, zsock_t *> _service_map;
  bool _auto_connection;

public:
  RichEventWriter(bool auto_connection = false);
  virtual ~RichEventWriter();
  bool register_pub(const char *service, const char *endpoint);
  bool register_push(const char *service, const char *endpoint);
  bool publish(const char *service, const char *msg);

private:
  bool register_writer(int type, const char *service, const char *endpoint);
};

}; // namespace chkchk