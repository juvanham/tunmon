/* project: tunmon
 * author: Jurgen Van Ham
 * license: GPL3 see http://www.fsf.org/licensing/
 *
 * description: tool to act on missing inbound traffic of a tunnel
 */

#include "config.hpp"
#include "driver.hpp"
#include "net_dev.hpp"
#include "pidfile.hpp"
#include <iostream>
#include <unistd.h>

namespace tunmon::actor {
  
  class Actor {
    tunmon::input::net_dev dev;
    tunmon::cfg::config cfg;
    tunmon::output::driver proc_driver;
    std::map<std::string,int> prev_down_times;
    int get_prev_down_time(const std::string &dev) {return prev_down_times[dev];}
  public:
    Actor(int argc,const char *const *argv);
    std::string get_pidfile() {return cfg.pidfile();}
    bool tracing() const {return cfg.tracing();}
    int interval() const {return cfg.interval();}
    std::pair<long,long> parse_proc_file();
    int schedule_actions();
    int schedule_retry_actions();
    int schedule_restore_actions();
    int schedule_on_restore_any();
    std::string execute();
    std::list<std::string> list_failures() {return proc_driver.list_failures();}
    
  };

}
