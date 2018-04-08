/* project: tunmon
 * author: Jurgen Van Ham
 * license: GPL3 see http://www.fsf.org/licensing/
 *
 * description: tool to act on missing inbound traffic of a tunnel
 */

#include "actor.hpp"
#include "pidfile.hpp"
#include <iostream>
#include <unistd.h>




void conditional_output(std::string prefix,std::string line) {
  if (!line.empty())
    std::cout << prefix << " " << line << std::endl;
}

int main(int argc, char* argv[]) {
  tunmon::actor::Actor m(argc,argv);
  posix_util::pidfile pidfile(m.get_pidfile());
  
  if (!m.get_pidfile().empty() && !pidfile.write()) {
    std::cerr << "active pidfile found : " << m.get_pidfile() << std::endl;
    exit(1);
  }
 
  const auto interval=m.interval();
  while (pidfile.present()) {
    auto [bytes,packets]=m.parse_proc_file();
   
    if (m.tracing())
      std::cout << "after " << interval << "s, bytes " << bytes << ", packets " << packets << std::endl;
    auto actions_scheduled=m.schedule_actions();
    if (m.tracing() && actions_scheduled) {
      std::cout << actions_scheduled << " actions scheduled" << std::endl;
    }
    auto restore_count=m.schedule_restore_actions();
    if (m.tracing() && restore_count) {
      std::cout << restore_count << "restore_dev actions scheduled" << std::endl;
    }
    auto retry_count=m.schedule_retry_actions();
    if (m.tracing() && retry_count) {
      std::cout << retry_count << "retry actions scheduled" << std::endl;
    }
    if (restore_count) {
      
      auto on_restore_any_count=m.schedule_on_restore_any();
      if (m.tracing() && on_restore_any_count) {
	std::cout << on_restore_any_count << "post_restore_any actions scheduled" << std::endl;
      }
    }
    auto output=m.execute();
    if (!output.empty())
      std::cout << output << std::endl;
    for (auto &failure:m.list_failures()) {
      std::cout << failure << std::endl;
    }
    sleep(interval);
  }

}
