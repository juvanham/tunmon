/* project: tunmon 
 * author: Jurgen Van Ham 
 * license: GPL3 see http://www.fsf.org/licensing/
 * 
 * description: tool to act on missing inbound traffic of a tunnel
 */

#include "driver.hpp"
#include <iostream>
#include <sstream>

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/process.hpp>
#include <boost/process/shell.hpp>

namespace tunmon::output {
  using namespace std;
  namespace bp = boost::process;

  
  class driver::process {
    const list<string> exec_args;
    std::promise<int> exit_promise;
    std::string asString;
    bool mark;
    std::thread async_thread;
    std::future<int> exit_future;
    void async_runner(const std::string &args);
  public:
    process(const list<string> &exec_args_);
    ~process() = default;
    bool running();
    optional<int> exit_code();
    string to_string() const;
    void mark_to_remove() {mark=true;}
    bool is_marked() { return mark;}
  };

  
  driver::process::process(const  list<string> &exec_args_
			   ) : exec_args(exec_args_),
			       asString{boost::algorithm::join(exec_args_," ")},
                               mark{false},
			       async_thread([&]() {async_runner(asString);}),
			       exit_future{exit_promise.get_future()}
  {
    async_thread.detach();
  }


  void driver::process::async_runner(const std::string &args) {
    bp::child child("/bin/sh", "-c", args);
    child.wait();
    exit_promise.set_value(child.exit_code());
  }

  bool driver::process::running() {
    return !exit_future.valid();
  }

  optional<int> driver::process::exit_code() {
    if (exit_future.valid()) {
      if (async_thread.joinable())
	async_thread.join();
      return exit_future.get();
    }
    return {};
  } 

  
  string driver::process::to_string() const {
    return boost::algorithm::join(exec_args," ");
  }

  
  driver::driver() : debug_flag{false}
  {}

  
  driver::~driver()
  {}


  string driver::execute() {
    if (call_queue.empty())
      return "";
    set_env();
    stringstream debugstream;
    while (!call_queue.empty()) {
      const auto &call=call_queue.front();
      auto process_inst=make_unique<driver::process>(call);
      if (debug_flag)
	debugstream << "execute : " << process_inst->to_string() << endl;
      process_queue.push_back(move(process_inst));
      call_queue.pop();
    }
    return debugstream.str();
  }
  
  void driver::schedule(const string &name, const string &dev_name, int time) {
    call_queue.push(list<string>{name,dev_name,to_string(time)});
  }

  
  list<string> driver::list_failures() {
    list<string> result;
    for (auto &q_elem: process_queue) {
      if (auto r=q_elem->exit_code();r) {
	if (*r)
	  result.push_back(q_elem->to_string()+" -> "+std::to_string(*r));
	q_elem->mark_to_remove();
      }
    }

    process_queue.remove_if([](auto &a){return a->is_marked();});
    return result;
  }

  void driver::set_env() {
    auto current_env=boost::this_process::environment();
    for (const auto &e:env) {
      current_env[e.first]=to_string(e.second);
      if (debug_flag)
	std::cout << " [" << e.first << "]=" << e.second << std::endl;
    }
  }
  
  int driver::set_dev_downtime(const std::string &devname,
			       int age,
			       int factor) {
    auto retval=0;
    auto env_devname=boost::replace_all_copy(devname, "-", "_");
    if (env.find(env_devname)!=env.end())
      retval=1;
    
    env["TUN_DOWN_"+env_devname]=age*factor;
    return retval;
  }

  int  driver::set_dev_downtime(const std::map<string, int> &dev_ages,
				int factor) {
    auto retval=0;
    for (const auto &dev_age:dev_ages) {
      retval+=set_dev_downtime(dev_age.first, dev_age.second, factor);
    }
    return retval;
  }
  
  void driver::set_debug(bool debug) {
    debug_flag=debug;
  }

}
