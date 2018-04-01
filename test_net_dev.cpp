#include "net_dev.hpp"

#define BOOST_TEST_MODULE MyTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>


#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>

namespace fs=boost::filesystem;

class proc_file {
  const std::string filename;
public:
  proc_file(const std::string &fname) : filename{fname} {}
  ~proc_file() {
    if (fs::exists(filename))
      fs::remove(filename);
  }
  const std::string get_filename() const {return filename;}
  void set_stat(int bytes,int packets) {
    std::ofstream o(filename);
    o << "Inter-|   Receive                                                |  Transmit\n"
      << "face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed\n"
      << "eth0: " <<  bytes << "  " << packets << "    0    0    0     0          0      2385 245559282 1501105    1    0    0     0       0          0\n";
  };
};


BOOST_AUTO_TEST_CASE( test_parse_proc_file_reads )
{
  proc_file proc_stat{"testfile.tmp"};
  tunmon::input::net_dev nd{proc_stat.get_filename()};
  nd.observe("eth0");
  proc_stat.set_stat(20,10);
  auto p1=nd.parse_proc_file();
  BOOST_CHECK_EQUAL( 20, p1.first );
  BOOST_CHECK_EQUAL( 10, p1.second );
}

BOOST_AUTO_TEST_CASE( test_parse_proc_file_increments )
{
  proc_file proc_stat{"testfile.tmp"};
  tunmon::input::net_dev nd{proc_stat.get_filename()};
  nd.observe("eth0");
  proc_stat.set_stat(20,10);
  nd.parse_proc_file();
  proc_stat.set_stat(50,20);

  auto p2=nd.parse_proc_file();
  BOOST_CHECK_EQUAL( 30, p2.first );
  BOOST_CHECK_EQUAL( 10, p2.second );
}

BOOST_AUTO_TEST_CASE( test_parse_proc_file_age )
{
  proc_file proc_stat{"testfile.tmp"};
  tunmon::input::net_dev nd{proc_stat.get_filename()};
  nd.observe("eth0");
  proc_stat.set_stat(20,10);
  nd.parse_proc_file();
  proc_stat.set_stat(50,20);

  nd.parse_proc_file();  
  auto age_before=nd.age("eth0");
  BOOST_CHECK_EQUAL( 0, *age_before );

  nd.parse_proc_file();
  auto age_no_change=nd.age("eth0");
  BOOST_CHECK_EQUAL( 1, *age_no_change );

  auto age_no_device=nd.age("eth0.no_real");
  BOOST_TEST( !age_no_device.has_value() );
}
