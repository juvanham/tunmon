/* project: tunmon
 * author: Jurgen Van Ham
 * license: GPL3  see http://www.fsf.org/licensing/
 *
 * description: tool to act on missing inbound traffic of a tunnel
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/program_options.hpp>
//#include <optional>
#include <functional>
#include <iostream>
#include "config.hpp"


namespace tunmon::cfg {
  using namespace std;
  namespace po = boost::program_options;
  namespace pt = boost::property_tree;

  struct config::config_impl {
  public:
    po::options_description desc;
    po::variables_map vm;
    config_impl() : desc("config options") {}
  };

  struct config::ptree_impl {
    pt::ptree tree;
  public:
    ptree_impl() {}
    bool store_entries_to_list(const string &pth, list<const string> &storage);
    template<typename K,typename V>
    bool store_entries_to_map(const string &pth,
			      const string &wrap_elem,
			      const string &key_elem,
			      const string &val_elem,
			      map<K,const V> &storage,
			      std::function<pair<K,V>(K,V)> fn);
  };

  config::config() : option_desc{std::make_unique<config_impl>()} {
    option_desc->desc.add_options()
      ("help", "produce help message")
      ("config-file", po::value<std::string>()->default_value("tunmon.conf"))
      ("trace" , po::value<bool>()->default_value(false));
      ;
  }

  config::~config()=default;


  void config::parse_command_line(int& argc, const char *const *argv) {
    po::store(po::command_line_parser(argc, argv).options(option_desc->desc).run(),
      option_desc->vm);
    po::notify(option_desc->vm);
    if ( option_desc->vm.count("help")  ) {
        std::cout << "tunmon" << std::endl
                  << "Executes script when no data is received for a number of seconds via specified network interfaces" << endl 
                  << option_desc->desc << std::endl;
        exit(0);
      }
    if (option_desc->vm.count("config-file")) {
      parse_xml(option_desc->vm["config-file"].as<string>());
    }
    trace_flag=option_desc->vm["trace"].as<bool>();
  }

  
  bool config::ptree_impl::store_entries_to_list(
						 const string &pth,
						 list<const string> &storage) {
    bool found{false};
    for (auto &chld:tree.get_child(pth)) {
      storage.push_back(chld.second.data());
      found=true;
    }
    return found;
  }

  template<typename K, typename V>
  bool config::ptree_impl::store_entries_to_map(const string &pth,
						const string &wrap_elem,
						const string &key_elem,
						const string &val_elem,
						map<K,const V> &storage,
						std::function<pair<K,V>(K,V)> fn
						) {
  bool found{false};
  for (auto &chld:tree.get_child(pth)) {
    if (wrap_elem.empty() || wrap_elem==chld.first) {
      const auto &key=chld.second.get<K>(key_elem);
      const auto &value=chld.second.get<V>(val_elem);
      const pair<K,V> result=fn(key,value);
      if (result.first)
	storage.emplace(result.first,result.second);
      found=true;
    }
  }
  return found;
  }
  
  
  void config::parse_ptree(unique_ptr<config::ptree_impl> &tree_impl) {
    net_devices.clear();
    if (tree_impl->store_entries_to_list("tun_mon.net_devices", net_devices)) {
      for (auto &dev:net_devices) {
	cout << "net_device : " << dev << endl;
      }
    }
    if (tree_impl->store_entries_to_list("tun_mon.restore_dev", restore_dev_actions)) {
      for (auto &dev:restore_dev_actions) {
	cout << "each_restore_dev_action : " << dev << endl;
      }
    }
    if (tree_impl->store_entries_to_list("tun_mon.restore_any", restore_anydev_actions)) {
      for (auto &dev:restore_anydev_actions) {
	cout << "each_restore_anydev_action : " << dev << endl;
      }
    }
    
    interval_sec=tree_impl->tree.get("tun_mon.interval",1);
    cout << "interval : " << interval_sec << "sec" << endl;
    auto half_interval=interval_sec/2;
    actions.clear();
    last_action_iter=std::numeric_limits<int>::min();
    tree_impl->store_entries_to_map<int,string>(
						      "tun_mon.actions",
						      "action",
						      "time",
						      "script",
						      actions,
						      [half_interval,this](auto key, auto val) {
     							auto iteration_count=static_cast<decltype(key)>(
													(key+half_interval)/this->interval_sec
													);
							return pair(iteration_count,val);
						      } );

      
    for (auto &chld:tree_impl->tree.get_child("tun_mon.actions"))
      if (chld.first=="action") {
        auto time=chld.second.get<int>("time",0);
        auto iteration_count=static_cast<decltype(time)>((time+half_interval)/interval_sec);
        auto script=chld.second.get<string>("script");
        if (actions.find(iteration_count)!=actions.end()) {
          cerr << "fatal: action " << script
               << "collides iteration " << iteration_count << "."
               << endl << "Consider a smaller interval" << endl;
          exit(1);
        }
	last_action_iter=std::max(iteration_count,last_action_iter);	  
        actions.insert(pair(iteration_count,
                            script)
                       );
      }
    for (auto &action:actions) {
      cout << "at " << action.first << " iterations without incoming traffic, call " << action.second << endl;
    }
    cout << "max_action_age " << last_action_iter << endl;
    retry_actions.clear(); 
    for (auto &chld:tree_impl->tree.get_child("tun_mon.retry_actions"))
      if (chld.first=="retry") {
        auto time=chld.second.get<int>("interval",0);
        auto iteration_count=static_cast<decltype(time)>((time+half_interval)/interval_sec);
        auto script=chld.second.get<string>("script");
        if (retry_actions.find(iteration_count)!=retry_actions.end()) {
          cerr << "fatal: retry_action " << script
               << "collides iteration " << iteration_count << "."
               << endl << "Consider a smaller interval" << endl;
          exit(1);
        }
        retry_actions.insert(pair(iteration_count,
                            script)
                       );
      }
    for (auto &action:retry_actions) {
      cout << "retry each " << action.first << " iterations without incoming traffic, call " << action.second << endl;
    }
    on_restore_script=tree_impl->tree.get("tun_mon.on_restore.script","");
    cout << "on_restore/script :" << on_restore_script << endl;
    pid_file_setting=tree_impl->tree.get("tun_mon.pidfile","");
    cout << "pid_file :" << pid_file_setting << endl;
  }

  void config::parse_xml(const std::string &filename) {
    auto tree_impl=make_unique<config::ptree_impl>();
    cout << "parse_xml " << filename << endl;
    pt::read_xml(filename, tree_impl->tree);
    parse_ptree(tree_impl);
  }


  list<const string> config::get_net_devices() const {
    list<const string> result;
    for (const auto& nd: net_devices)
      result.emplace_back(nd);
    return result;
  }

  list<const string> config::get_restore_dev_actions() const {
    list<const string> result;
    for (const auto &action: restore_dev_actions)
      result.emplace_back(action);
    return result;
  }
  
  list<const string> config::get_restore_anydev_actions() const {
     list<const string> result;
     for (const auto &action: restore_anydev_actions)
       result.emplace_back(action);
     return result;
  }

  map<int,std::string> config::get_actions() const {
    map<int,std::string> result;
    for (auto &elm:actions) {
      result.insert(pair(elm.first,string(elm.second)));
    }
    return result;
  }

  map<int,std::string> config::get_retry_actions() const {
    map<int,std::string> result;
    for (auto &elm:retry_actions) {
      result.insert(pair(elm.first,string(elm.second)));
    }
    return result;
  }

  bool config::tracing() const {
    return trace_flag;
  }

  int config::interval() const {
    return interval_sec;
  }

  const string& config::pidfile() const {
    return pid_file_setting;
  }

  int config::max_action_count() const {
    return last_action_iter;
  }

  const string& config::restored() const {
    return on_restore_script;
  }
  
}
