#ifndef CONFIG_HEADER
#define CONFIG_HEADER

/* project: tunmon
 * author: Jurgen Van Ham
 * license: GPL3 see http://www.fsf.org/licensing/
 *
 * description: tool to act on missing inbound traffic of a tunnel
 */

#include <memory>
#include <string>
#include <list>
#include <map>

namespace tunmon::cfg {

  class config {
    struct config_impl;
    struct ptree_impl;
    std::unique_ptr<config_impl> option_desc;
    std::list<const std::string> net_devices;
    std::map<int,const std::string> actions;
    std::list<const std::string> restore_dev_actions;
    std::list<const std::string> post_restore_anydev_actions;
    void parse_ptree(std::unique_ptr<config::ptree_impl> &pree_impl,bool verbose);
    bool trace_flag;
    int interval_sec;
    std::string pid_file_setting;
    int last_action_iter;
    std::map<int,const std::string> retry_actions;
    std::string on_restore_script;
  public:
    config();
    ~config();
    void parse_command_line(int& argc, const char *const *argv);
    void parse_xml(const std::string &filename, bool verbose=true);
    std::list<const std::string> get_net_devices() const;
    std::list<const std::string> get_restore_dev_actions() const;
    std::list<const std::string> get_post_restore_anydev_actions() const;
    std::map<int,std::string> get_actions() const;
    std::map<int,std::string> get_retry_actions() const;
    bool tracing() const;
    int interval() const;
    std::string pidfile() const;
    int max_action_count() const;
  };

}


#endif
