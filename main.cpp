/* project: tunmon
 * author: Jurgen Van Ham
 * license: GPL3 see http://www.fsf.org/licensing/
 *
 * description: tool to act on missing inbound traffic of a tunnel
 */

#include "net_dev.hpp"
#include "config.hpp"
#include "driver.hpp"
#include <iostream>
#include <unistd.h>

int main(int argc, char* argv[]) {
  tunmon::input::net_dev dev;
  tunmon::cfg::config cfg;
  tunmon::output::driver proc_driver;
  cfg.parse_command_line(argc, argv);
  auto devices=cfg.get_net_devices();
  for (auto &dev_name:devices)
    dev.observe(dev_name);
  auto actions=cfg.get_actions();
  for (;;) {
    auto [bytes,packets]=dev.parse_proc_file();
    if (cfg.tracing())
      std::cout << "bytes " << bytes << "  packets " << packets << std::endl;
    for (auto &dev_name:devices) {
      auto age=*(dev.age(dev_name));
      if (auto action=actions.find(age); action!=actions.end()) {
        if (cfg.tracing())
          std::cout << "execute: " << action->second << "  " << dev_name << " "  << age << std::endl;
        proc_driver.execute(action->second,dev_name,age);
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
