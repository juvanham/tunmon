/* project: tunmon
 * author: Jurgen Van Ham
 * license: GPL3 see http://www.fsf.org/licensing/
 *
 * description: tool to act on missing inbound traffic of a tunnel
 */

#include "actor.hpp"

namespace tunmon::actor {
  
  Actor::Actor(int argc,const char *const *argv)  {
    cfg.parse_command_line(argc,argv);
    for (auto &dev_name:cfg.get_net_devices()) {
      dev.observe(dev_name);
      prev_down_times[dev_name]=0;
    }
    proc_driver.set_debug(cfg.tracing());
  }

  std::pair<long,long> Actor::parse_proc_file() {
    for (auto &dev_name:cfg.get_net_devices()) {
      auto age=dev.age(dev_name);
      if (age)
	prev_down_times[dev_name]=*age;
    }
    return dev.parse_proc_file();
  }


  
  int Actor::schedule_actions() {
    auto action_counter=0;
    auto actions=cfg.get_actions();
    for (const auto &dev_name:cfg.get_net_devices()) {
      const auto age_o=dev.age(dev_name);
      if (age_o) {
	const auto age=*age_o;
	if (auto action=actions.find(age); action!=actions.end()) {
	  proc_driver.schedule(action->second, dev_name, age* interval());
	}
	action_counter++;
      }
    }
    return action_counter;
  }

  int Actor::schedule_retry_actions() {
    auto action_counter=0;
    auto retry_actions=cfg.get_retry_actions();
    for (const auto &dev_name:cfg.get_net_devices()) {
      const auto age_o=dev.age(dev_name);
      if (age_o) {
	const auto age=(*age_o)-cfg.max_action_count();
	for (const auto &raction:retry_actions) {
	  if (age%raction.first==0) 
	    proc_driver.schedule(raction.second, dev_name, (*age_o) * interval());
	}
	action_counter++;
      }
    }
    return action_counter;
  }


  int Actor::schedule_restore_actions() {
    auto restore_counter=0;
    for (const auto &dev_name:cfg.get_net_devices()) {
      const auto age_o=dev.age(dev_name);
      if (age_o) {
	const auto age=*age_o;
	if (age==0 and prev_down_times[dev_name] ) {
	  for (auto &action:cfg.get_restore_dev_actions()) {
	    proc_driver.schedule(action, dev_name, prev_down_times[dev_name]*interval());
	  }
	  restore_counter++;
	}
      }
    }
    return restore_counter;
  }

  int Actor::schedule_on_restore_any() {
    auto cnt=0;
    for (auto &action:cfg.get_post_restore_anydev_actions()) {
      proc_driver.schedule(action, "", 0);
      ++cnt;
    }
    return cnt;
  }

  std::string Actor::execute() {
    proc_driver.set_dev_downtime(prev_down_times, interval());
    return proc_driver.execute();
  }
  
}
