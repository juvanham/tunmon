#include "config.hpp"

#define BOOST_TEST_MODULE ConfigTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>


#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>

namespace fs=boost::filesystem;

class config_file {
  const std::string filename;
  std::list<std::string> net_devices;
  std::list<std::pair<int,std::string>> actions;
  std::list<std::pair<int,std::string>> retry_actions;
  std::list<std::string> onrestore_dev;
  std::list<std::string> onrestore_anydev;
  
  int interval;
  std::string pidfile;
public:
  config_file(const std::string &fname) : filename{fname} {}
  ~config_file() {
    if (fs::exists(filename))
      fs::remove(filename);
  }
  const std::string get_filename() const {return filename;}
  void add_net_device(const std::string &dev) {net_devices.push_back(dev);}
  void set_interval(int i) {interval=i;}
  void set_pidfile(const std::string &pf) {pidfile=pf;}
  void add_action(int time,const std::string &script) {actions.push_back(make_pair(time,script));}
  void add_retry_action(int time,const std::string &script) {retry_actions.push_back(make_pair(time,script));}
  void add_onrestore_dev(const std::string &dev) {onrestore_dev.push_back(dev);}
  void add_onrestore_anydev(const std::string &dev) {onrestore_anydev.push_back(dev);}
  void write_net_devices(std::ostream &os) const {
    os << "  <net_devices>" << std::endl;
    for (const auto &dev:net_devices) {
	   os << "    <net_device>" << dev << "</net_device>\n";
    }
    os << "  </net_devices>" << std::endl;
  }

  void write_interval(std::ostream &os) const {
      os << "  <interval>" << interval << "</interval>" << std::endl;
  }
  void write_pidfile(std::ostream &os) const {
      os << "  <pidfile>" << pidfile << "</pidfile>" << std::endl;
  }
  void write_actions(std::ostream &os) const {
    os << "  <actions>" << std::endl;
    for (const auto &act:actions) {
      os << "    <action><time>" << act.first << "</time><script>"  << act.second << "</script></action>\n";
    }
    os << "  </actions>" << std::endl;
  }
  void write_retry_actions(std::ostream &os) const {
    os << "  <retry_actions>" << std::endl;
    for (const auto &act:retry_actions) {
      os << "    <retry><time>" << act.first << "</time><script>"  << act.second << "</script></retry>\n";
    }
    os << "  </retry_actions>" << std::endl;
  }
   void write_restore_dev(std::ostream &os) const {
    os << "  <restore_dev>" << std::endl;
    for (const auto &dev:onrestore_dev) {
	   os << "<script>" << dev << "</script>\n";
    }
    os << "  </restore_dev>" << std::endl;
  }
   void write_restore_any(std::ostream &os) const {
    os << "  <restore_any>" << std::endl;
    for (const auto &dev:onrestore_anydev) {
	   os << "<script>" << dev << "</script>\n";
    }
    os << "  </restore_any>" << std::endl;
  }
  void write(std::ostream &os) const {
    os << "<tun_mon>" << std::endl;
    write_net_devices(os);
    write_interval(os);
    write_pidfile(os);
    write_actions(os);
    write_retry_actions(os);
    write_restore_dev(os);
    write_restore_any(os);
    os << "</tun_mon>" << std::endl;
  }
  
  void write() {
    std::ofstream o(filename);
    write(o);
    write(std::cout);
      
  }
};


void fill_default(config_file &cfg_file) {
  cfg_file.add_net_device("eth0");
  cfg_file.add_net_device("eth1");
  cfg_file.add_action(5,"./ping.sh");
  cfg_file.add_retry_action(15,"./retry.sh");
  cfg_file.set_interval(2);
}


BOOST_AUTO_TEST_CASE( test_config_interval_2 )
{
  config_file cfg_file{"./testfile.xml"};
  tunmon::cfg::config cfg;
  fill_default(cfg_file);
  cfg_file.write();
  cfg.parse_xml(cfg_file.get_filename());

  BOOST_CHECK_EQUAL( 2, cfg.interval());
}

BOOST_AUTO_TEST_CASE( test_config_interval_5 )
{
  config_file cfg_file{"./testfile.xml"};
  tunmon::cfg::config cfg;
  fill_default(cfg_file);
  cfg_file.set_interval(5);
  cfg_file.write();
  cfg.parse_xml(cfg_file.get_filename());
  BOOST_CHECK_EQUAL( 5, cfg.interval());
}

BOOST_AUTO_TEST_CASE( test_config_interval_net_devices )
{
  config_file cfg_file{"./testfile.xml"};
  tunmon::cfg::config cfg;
  fill_default(cfg_file);
  cfg_file.write();
  cfg.parse_xml(cfg_file.get_filename());
  const std::string eth0{"eth0"};
  const std::string eth1{"eth1"};
  const std::list<std::string> expectation{"eth0","eth1"};
  auto retrieved=cfg.get_net_devices();
  BOOST_CHECK_EQUAL_COLLECTIONS ( expectation.begin(),expectation.end(),retrieved.begin(), retrieved.end() );
}
