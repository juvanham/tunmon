#ifndef DRIVER_HEADER
#define DRIVER_HEADER

/* project: tunmon
 * author: Jurgen Van Ham
 * license: GPL3
 *
 * description: tool to act on missing inbound traffic of a tunnel
 */

#include <memory>
#include <list>

namespace tunmon::output {
  class driver {
    class process;  // pimpl
    std::list<std::unique_ptr<driver::process>> process_queue;
    bool debug_flag;
  public:
    driver();
    virtual ~driver();
    std::string execute(const std::string &name, const std::string &dev_name, int time);
    std::list<std::string> list_failures();
    void set_debug(bool debug);
  };
}


#endif
