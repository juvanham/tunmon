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
    class process;
    std::list<std::unique_ptr<driver::process>> process_queue;
  public:
    driver();
    virtual ~driver();
    void execute(const std::string &name, const std::string &dev_name, int time);
    std::list<std::string> list_failures();
  };
}


#endif
