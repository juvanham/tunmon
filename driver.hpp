#ifndef DRIVER_HEADER
#define DRIVER_HEADER

/* project: tunmon
 * author: Jurgen Van Ham
 * license: GPL3
 *
 * description: tool to act on missing inbound traffic of a tunnel
 */

#include <queue>
#include <memory>
#include <list>
#include <map>

namespace tunmon::output {
  class driver {
    class process;  // pimpl
    std::list<std::unique_ptr<driver::process>> process_queue;
    std::queue<std::list<std::string>> call_queue;
    std::map<std::string,int> env;
    bool debug_flag;
    void set_env();
  public:
    driver();
    virtual ~driver();
    std::string execute();
    void schedule(const std::string &name, const std::string &dev_name, int time);
    std::list<std::string> list_failures();
    int set_dev_downtime(const std::string &devname, int age, int factor);
    int set_dev_downtime(const std::map<std::string, int> &dev_ages, int factor);    
    void set_debug(bool debug);
  };
}


#endif
