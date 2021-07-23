#include <czmq.h>
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace chkchk {

class RichEventReader;
using rbase_cb_fn = std::function<bool(zsock_t *, RichEventReader *)>;

class RichEventReader {
private:
  enum {
    RICH_EVENT_SUB,
    RICH_EVENT_PULL,
  };
  zpoller_t *_poller;
  std::map<zsock_t *, rbase_cb_fn> _callback_map;
  std::map<zsock_t *, std::string> _service_map;

public:
  RichEventReader();
  virtual ~RichEventReader();
  bool register_sub(const char *service, const char *endpoint,
                    rbase_cb_fn callback);
  bool register_pull(const char *service, const char *endpoint,
                     rbase_cb_fn callback);
  void event_loop();
  void event_once(int ms);

private:
  bool subscribe(int type, const char *service, const char *endpoint,
                 rbase_cb_fn callback);
  void dispatch(int ms);
};

}; // namespace chkchk