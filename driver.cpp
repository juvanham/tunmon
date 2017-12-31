/* project: tunmon 
 * author: Jurgen Van Ham 
 * license: GPL3 see http://www.fsf.org/licensing/
 * 
 * description: tool to act on missing inbound traffic of a tunnel
 */

#include "driver.hpp"
#include <boost/process.hpp>
#include <boost/process/shell.hpp>

namespace tunmon::output {
  using namespace std;
  namespace bp = boost::process;

  class driver::process {
    const string name;
    const string dev_name;
    const int time;
    bp::child child;
    bool busy;
    optional<int> exit_code_;
    bool mark;
  public:
    process(const string &name_, const string &dev_name_, int time_);
    bool running();
    optional<int> exit_code();
    string to_string() const;
    void mark_to_remove() {mark=true;}
    bool is_marked() { return mark;}
    
  };

  driver::process::process(const string &name_,
			   const string &dev_name_,
			   int time_
			   ) : name{name_},
			       dev_name{dev_name_},
			       time{time_},					
			       child("/bin/sh", "-c", name_ + " "+ dev_name_+ " "+std::to_string(time_)),
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
    return name+" "+dev_name+" "+std::to_string(time);
  }

  driver::driver() = default;
  driver::~driver()=default;
  
  void driver::execute(const string &name, const string &dev_name, int time) {
    process_queue.push_back(make_unique<driver::process>(name,dev_name,time));
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
}
