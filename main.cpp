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


void conditional_output(std::string prefix,std::string line) {
  if (!line.empty())
    std::cout << prefix << " " << line << std::endl;
}

int main(int argc, char* argv[]) {
  tunmon::input::net_dev dev;
  tunmon::cfg::config cfg;
  tunmon::output::driver proc_driver;
  cfg.parse_command_line(argc, argv);
  posix_util::pidfile pidfile(cfg.pidfile());
  if (!cfg.pidfile().empty() &&
      !pidfile.write()) {
    std::cerr << "active pidfile found : " << cfg.pidfile() << std::endl;
    exit(1);
  }
  std::map<std::string,int> prev_down_times;
  auto devices=cfg.get_net_devices();
  for (auto &dev_name:devices) {
    dev.observe(dev_name);
    prev_down_times[dev_name]=0;
  }
  auto actions=cfg.get_actions();
  auto retry_actions=cfg.get_retry_actions();
  const auto interval=cfg.interval();
  proc_driver.set_debug(cfg.tracing());
  while (pidfile.present()) {
    auto [bytes,packets]=dev.parse_proc_file();
    if (cfg.tracing())
      std::cout << "after " << interval << "s, bytes " << bytes << ", packets " << packets << std::endl;
    for (const auto &dev_name:devices) {
      const auto age=*(dev.age(dev_name));
      const auto restored_age=age*interval;
      if (age==0 and prev_down_times[dev_name] && !(cfg.restored().empty())) {
	auto output= proc_driver.execute(cfg.restored(), dev_name, prev_down_times[dev_name]*interval);
	conditional_output("(restored)", output);
      }
      if (auto action=actions.find(age); action!=actions.end()) {
	auto output= proc_driver.execute(action->second, dev_name, restored_age);
	conditional_output("(timeout)", output);
      } else if (auto offset_tm=age-cfg.max_action_count(); offset_tm>0) {
	for (const auto &ract:retry_actions) {
	  if ((offset_tm%ract.first)==0) {
	    auto output=proc_driver.execute(ract.second, dev_name, restored_age);
	    conditional_output("(retry)", output);	      
	  }
	}
      }
      prev_down_times[dev_name]=age;
    }
    for (auto &failure:proc_driver.list_failures()) {
      std::cout << failure << std::endl;
    }
    sleep(interval);
  }

}
