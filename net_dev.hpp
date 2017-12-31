#ifndef NETDEV_HEADER
#define NETDEV_HEADER


/* project: tunmon
 * author: Jurgen Van Ham
 * license: GPL3 see http://www.fsf.org/licensing/
 *
 * description: tool to act on missing inbound traffic of a tunnel
 */

#include<string>
#include<utility>
#include<optional>
#include<map>
#include<regex>

namespace tunmon::input {

  class net_dev {
  private:
    const std::string proc_file{"/proc/net/dev"};
    const std::string re_str{"([a-z0-9]+):\\s+([0-9]+)\\s+([0-9]+)\\s+"};
    const std::regex interface_re=std::regex(re_str,std::regex_constants::icase);

    struct net_dev_stat {
    public:
      long bytes;
      long packets;
      int changed;
      net_dev_stat(int chg) : bytes{0}, packets{0}, changed{chg} {}
      net_dev_stat() : net_dev_stat(0) {}
    };

    std::map<const std::string,net_dev_stat> devices;
    int current_time;
    std::pair<long,long> read_incoming_data(const std::string &line);
  public:
    net_dev();
    bool observe(const std::string &dev_name);
    bool unobserve(const std::string &dev_name);
    std::pair<long,long> parse_proc_file();
    std::optional<int> age(const std::string &dev_name) const;
  };

}

#endif
