/* project: tunmon 
 * author: Jurgen Van Ham 
 * license: GPL3 see http://www.fsf.org/licensing/
 * 
 * description: tool to act on missing inbound traffic of a tunnel
 */

#include "driver.hpp"
#include <sstream>
#include <boost/process.hpp>
#include <boost/process/shell.hpp>
#include <boost/algorithm/string/join.hpp>


namespace tunmon::output {
  using namespace std;
  namespace bp = boost::process;

  
  class driver::process {
    const list<string> exec_args;
    bp::child child;
    bool busy;
    optional<int> exit_code_;
    bool mark;
  public:
    process(const list<string> exec_args_);
    ~process() = default;
    bool running();
    optional<int> exit_code();
    string to_string() const;
    void mark_to_remove() {mark=true;}
    bool is_marked() { return mark;}
  };

  
  driver::process::process(const  list<string> exec_args_
			   ) : exec_args{exec_args_},
			       child("/bin/sh", "-c", boost::algorithm::join(exec_args_," ")),
			       busy{true},
			       exit_code_{},
			       mark{false}
  {
  }

  bool driver::process::running() {
    if (busy)
      busy=child.running();
    return busy;
  }

  optional<int> driver::process::exit_code() {
    if (busy)
      return {};
    if (exit_code_)
      return exit_code_;
    child.wait();
    exit_code_=child.exit_code();
    return exit_code_;
  } 

  
  string driver::process::to_string() const {
    return boost::algorithm::join(exec_args," ");
  }

  
  driver::driver() : debug_flag{false}
  {}

  
  driver::~driver()
  {}


  string driver::execute() {
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
    process_queue.remove_if([](auto &a)->bool{return a->is_marked();});
    return result;
  }

  void driver::set_env() {
    auto current_env=boost::this_process::environment();
    for (const auto &e:env) {
      current_env[e.first]=to_string(e.second);
    }
  }
  
  int driver::set_dev_downtime(const std::string &devname,
			       int age,
			       int factor) {
    auto retval=0;
    if (env.find(devname)!=env.end())
      retval=1;
    env[devname]=age*factor;
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
