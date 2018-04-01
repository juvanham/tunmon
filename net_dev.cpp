/* project: tunmon
 * author: Jurgen Van Ham
 * license: GPL3 see http://www.fsf.org/licensing/
 *
 * description: tool to act on missing inbound traffic of a tunnel
 */

#include "net_dev.hpp"
#include <fstream>
#include <iostream>

namespace tunmon::input {
  using namespace std;

  net_dev::net_dev(const std::string &proc_file_) : proc_file{proc_file_}, current_time{0} 
  { }

  bool net_dev::observe(const std::string &dev_name) {
    if (devices.find(dev_name)==devices.cend()) {
      net_dev_stat st(this->current_time);
      devices.insert(std::pair(dev_name,st));
      return true;
    }
    return false;
  }

  bool net_dev::unobserve(const std::string &dev_name) {
    if (devices.find(dev_name)==devices.cend()) {
      devices.erase(dev_name);
      return true;
    }
    return false;
  }

  std::pair<long,long> net_dev::read_incoming_data(const std::string &line) {
    smatch matcher;
    if (std::regex_search (line,matcher,interface_re))
      if (auto statit=devices.find(matcher[1]);statit!=devices.end()) {
	auto &stat=statit->second;
	auto nbytes=stol(matcher[2]);
	auto npackets=stol(matcher[3]);
	auto bytes=nbytes-stat.bytes;
	auto packets=npackets-stat.packets;
	if (bytes)
	  stat.bytes=nbytes;
	if (packets)
	  stat.packets=npackets;
	if (bytes || packets)
	  stat.changed=this->current_time;
	return pair(bytes,packets);
      }
    return pair(0,0);
  }

  pair<long,long> net_dev::parse_proc_file() {
    std::ifstream is(proc_file,ios_base::in);
    long total_bytes=0;
    long total_packets=0;
    if (is.is_open()) {
      std::string line;
      ++this->current_time;
      while (std::getline(is, line)) {
        auto [bytes,packets]=read_incoming_data(line);
        total_bytes+=bytes;
        total_packets+=packets;
      }
      return pair(total_bytes,total_packets);
    }
    return pair(0,0);
  }

  optional<int> net_dev::age(const std::string &dev_name) const {
    if (auto dev=devices.find(dev_name);dev!=devices.cend()) {
      return (this->current_time)-(dev->second.changed);
    }
    return {};
  }


}
