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
  std::list<std::string> post_restore_anydev;
  
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
  std::list<std::string> get_net_devices() const { return std::list<std::string>(net_devices);}
  std::list<std::pair<int,std::string>> get_actions() const { return  std::list<std::pair<int,std::string>>(actions);}
  std::list<std::pair<int,std::string>> get_retry_actions() const { return  std::list<std::pair<int,std::string>>(retry_actions);}
  std::list<std::string> get_restore_actions() const { return std::list<std::string>(onrestore_dev);}
  std::list<std::string> get_post_restore_actions() const { return std::list<std::string>(post_restore_anydev);}
  void set_interval(int i) {interval=i;}
  int get_interval() const {return interval;}
  void set_pidfile(const std::string &pf) {pidfile=pf;}
  const std::string get_pidfile() const { return pidfile;}
  void add_action(int time,const std::string &script) {actions.push_back(make_pair(time,script));}
  void add_retry_action(int time,const std::string &script) {retry_actions.push_back(make_pair(time,script));}
  void add_onrestore_dev(const std::string &dev) {onrestore_dev.push_back(dev);}
  void add_post_restore_anydev(const std::string &dev) {post_restore_anydev.push_back(dev);}
  void write_net_devices(std::ostream &os) const {
    if (net_devices.empty())
      return;
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
    if (pidfile.empty())
      return;
    os << "  <pidfile>" << pidfile << "</pidfile>" << std::endl;
  }
  void write_actions(std::ostream &os) const {
    if (actions.empty())
      return;
    os << "  <actions>" << std::endl;
    for (const auto &act:actions) {
      os << "    <action><time>" << act.first << "</time><script>"  << act.second << "</script></action>\n";
    }
    os << "  </actions>" << std::endl;
  }
  void write_retry_actions(std::ostream &os) const {
    if (retry_actions.empty())
      return;
    os << "  <retry_actions>" << std::endl;
    for (const auto &act:retry_actions) {
      os << "    <retry><interval>" << act.first << "</interval><script>"  << act.second << "</script></retry>\n";
    }
    os << "  </retry_actions>" << std::endl;
  }
  void write_restore_dev(std::ostream &os) const {
    if (onrestore_dev.empty())
       return;
    os << "  <restore_dev>" << std::endl;
    for (const auto &dev:onrestore_dev) {
	   os << "    <script>" << dev << "</script>\n";
    }
    os << "  </restore_dev>" << std::endl;
  }
   void write_post_restore_any(std::ostream &os) const {
    if (post_restore_anydev.empty())
       return;
    os << "  <post_restore_any>" << std::endl;
    for (const auto &script:post_restore_anydev) {
	   os << "    <script>" << script << "</script>\n";
    }
    os << "  </post_restore_any>" << std::endl;
  }
  void write(std::ostream &os) const {
    os << "<tun_mon>" << std::endl;
    write_net_devices(os);
    write_interval(os);
    write_pidfile(os);
    write_actions(os);
    write_retry_actions(os);
    write_restore_dev(os);
    write_post_restore_any(os);
    os << "</tun_mon>" << std::endl;
  }
  
  void write() {
    std::ofstream o(filename);
    write(o);
    write(std::cout);
      
  }
};


void fill_default(config_file &cfg_file) {
  cfg_file.set_interval(2);
}


BOOST_AUTO_TEST_CASE( test_config_interval_5 )
{
  config_file cfg_file{"./testfile.xml"};
  tunmon::cfg::config cfg;
  fill_default(cfg_file);
  cfg_file.set_interval(5);
  cfg_file.write();
  cfg.parse_xml(cfg_file.get_filename(), false);
  BOOST_CHECK_EQUAL( cfg_file.get_interval(), cfg.interval());
}


BOOST_AUTO_TEST_CASE( test_config_net_devices )
{
  config_file cfg_file{"./testfile.xml"};
  tunmon::cfg::config cfg;
  fill_default(cfg_file);
  cfg_file.add_net_device("eth0");
  cfg_file.add_net_device("eth1");
  cfg_file.write();
  cfg.parse_xml(cfg_file.get_filename(), false);
  const std::list<std::string> &expectation=cfg_file.get_net_devices();
  auto retrieved=cfg.get_net_devices();
  BOOST_CHECK_EQUAL_COLLECTIONS ( expectation.begin(),expectation.end(),retrieved.begin(), retrieved.end() );
}


BOOST_AUTO_TEST_CASE( test_config_pidfile )
{
  config_file cfg_file{"./testfile.xml"};
  tunmon::cfg::config cfg;
  fill_default(cfg_file);
  cfg_file.set_pidfile("alternative_pidfile.pid");
  cfg_file.write();
  cfg.parse_xml(cfg_file.get_filename(), false);
  const auto &expectation=cfg_file.get_pidfile();
  auto retrieved=cfg.pidfile();
  BOOST_TEST ( expectation == retrieved );
}


BOOST_AUTO_TEST_CASE( test_config_actions )
{
  config_file cfg_file{"./testfile.xml"};
  tunmon::cfg::config cfg;
  fill_default(cfg_file);
  cfg_file.add_action(5,"./ping.sh");
  cfg_file.write();
  cfg.parse_xml(cfg_file.get_filename(), false);
  const auto expectation=cfg_file.get_actions();
  auto retrieved=cfg.get_actions();
  BOOST_CHECK_EQUAL ( expectation.size(),retrieved.size());
  for (const auto &expect:expectation) {
    auto found= retrieved.find(expect.first);
    if (found == retrieved.end()) {
      BOOST_TEST_MESSAGE("action not found");
    } else {
      BOOST_TEST(found->second == expect.second);
    }
  }
}


BOOST_AUTO_TEST_CASE( test_config_retry_actions )
{
  config_file cfg_file{"./testfile.xml"};
  tunmon::cfg::config cfg;
  fill_default(cfg_file);
  cfg_file.add_retry_action(15,"./retry.sh");
  cfg_file.write();
  cfg.parse_xml(cfg_file.get_filename(), false);
  const auto expectation=cfg_file.get_retry_actions();
  auto retrieved=cfg.get_retry_actions();
  BOOST_CHECK_EQUAL ( expectation.size(),retrieved.size());
  for (const auto &expect:expectation) {
    auto found= retrieved.find(expect.first);
    if (found == retrieved.end()) {
      BOOST_TEST_MESSAGE("retry action not found");
    } else {
      BOOST_TEST(found->second == expect.second);
    }
  }
}


BOOST_AUTO_TEST_CASE( test_config_on_restore_2 )
{
  config_file cfg_file{"./testfile.xml"};
  tunmon::cfg::config cfg;
  fill_default(cfg_file);
  cfg_file.add_onrestore_dev("./dev_script1.sh");
  cfg_file.add_onrestore_dev("./dev_script2.sh");
  cfg_file.write();
  cfg.parse_xml(cfg_file.get_filename(), false);
  const auto expectation=cfg_file.get_restore_actions();
  auto retrieved=cfg.get_restore_dev_actions();
  BOOST_CHECK_EQUAL ( expectation.size(),retrieved.size());
  for (const auto &expect:expectation) {
    auto found= std::find(std::cbegin(retrieved),std::cend(retrieved),expect);
    if (found == std::cend(retrieved)) {
      BOOST_TEST_MESSAGE("restore_dev_action not found");
    } 
  }
}


BOOST_AUTO_TEST_CASE( test_config_post_restore_2 )
{
  config_file cfg_file{"./testfile.xml"};
  tunmon::cfg::config cfg;
  fill_default(cfg_file);
  cfg_file.add_post_restore_anydev("./post_script1.sh");
  cfg_file.add_post_restore_anydev("./post_script2.sh");
  cfg_file.write();
  cfg.parse_xml(cfg_file.get_filename(), false);
  const auto expectation=cfg_file.get_post_restore_actions();
  auto retrieved=cfg.get_post_restore_anydev_actions();
  BOOST_CHECK_EQUAL ( expectation.size(),retrieved.size());
  for (const auto &expect:expectation) {
    auto found= std::find(std::cbegin(retrieved),std::cend(retrieved),expect);
    if (found == std::cend(retrieved)) {
      BOOST_TEST_MESSAGE("restore_dev_action not found");
    } 
  }
}
