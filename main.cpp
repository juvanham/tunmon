/* project: tunmon
 * author: Jurgen Van Ham
 * license: GPL3 see http://www.fsf.org/licensing/
 *
 * description: tool to act on missing inbound traffic of a tunnel
 */

#include "net_dev.hpp"
#include "config.hpp"
#include "driver.hpp"
#include "pidfile.hpp"
#include <iostream>
#include <unistd.h>

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
  auto devices=cfg.get_net_devices();
  for (auto &dev_name:devices)
    dev.observe(dev_name);
  auto actions=cfg.get_actions();
  auto retry_actions=cfg.get_retry_actions();
  while (pidfile.present()) {
    auto [bytes,packets]=dev.parse_proc_file();
    if (cfg.tracing())
      std::cout << "bytes " << bytes << "  packets " << packets << std::endl;
    for (const auto &dev_name:devices) {
      const auto age=*(dev.age(dev_name));
      const auto restored_age=age*cfg.interval();
      if (auto action=actions.find(age); action!=actions.end()) {
        if (cfg.tracing())
          std::cout << "execute: " << action->second << "  " << dev_name << " "  << restored_age << std::endl;
        proc_driver.execute(action->second, dev_name, restored_age);
      } else if (auto offset_tm=age-cfg.max_action_count(); offset_tm>0) {
	for (const auto &ra:retry_actions) {
	  if ((offset_tm%ra.first)==0) {
	    if (cfg.tracing())
	      std::cout << "execute (retry): " << ra.second << "  " << dev_name << " "  << restored_age << std::endl;
	    proc_driver.execute(ra.second, dev_name, restored_age);
	  }
	}
      }
    }
    for (auto &failure:proc_driver.list_failures()) {
      std::cout << failure << std::endl;
    }
    sleep(cfg.interval());
    if (cfg.tracing())
      std::cout << "loop after sleeping " << cfg.interval() << "s" << std::endl;
  }

}
