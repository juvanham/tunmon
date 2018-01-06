/* project: tunmon
 * author: Jurgen Van Ham
 * license: GPL3 see http://www.fsf.org/licensing/
 *
 * description: tool to act on missing inbound traffic of a tunnel
 */

#include "pidfile.hpp"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <string>
#include <fstream>
#include <optional>
#include <unistd.h>

using namespace std;

namespace posix_util {
  namespace fs = boost::filesystem;  // for now std::filesystem is called experimental


  class pidfile::proc_entry {
    optional<pid_t> pid;
    mutable optional<fs::path> p;
  public:
    proc_entry(const fs::path &pidfile);
    proc_entry(optional<pid_t> id);
    proc_entry();
    const optional<fs::path> get_procdir_path() const;
    const optional<fs::path> get_exe_link() const;
    bool exists() const;
    const optional<const pid_t> get_pid() const;
  };


  pidfile::proc_entry::proc_entry() : proc_entry(::getpid())
  {}


  pidfile::proc_entry::proc_entry(optional<pid_t> pid_) :pid{*pid_}
  {}


  pidfile::proc_entry::proc_entry(const fs::path &pidfile) : proc_entry{
    [&pidfile]()->optional<pid_t>{
      if (!fs::exists(pidfile)) {
        return {};
      }
      fs::ifstream f(pidfile);
      pid_t p;
      f >> p;
      return p;
    }()}
  {}


  const optional<fs::path> pidfile::proc_entry::proc_entry::get_procdir_path() const {
    if (!pid)
      return {};
    if (!p) {
      auto pth=fs::path("/proc/") /  to_string(*pid);
      if (fs::exists(pth))
	p=pth;
    }
    return p;
  }


  const optional<fs::path> pidfile::proc_entry::proc_entry::get_exe_link() const {
    auto pth=get_procdir_path();
    if (!pth)
      return {};
    return fs::read_symlink(*get_procdir_path() / "exe");
  }


  bool pidfile::proc_entry::exists() const {
    if (!pid)
      return false;
    if (auto pth=get_procdir_path();pth) {
      return fs::exists(*pth);
    }
    return false;
  }


  const optional<const pid_t> pidfile::proc_entry::get_pid() const {
    return pid;
  }


  pidfile::pidfile(const string &pth) :
    filename{pth},
    in_file{make_unique<proc_entry>(pth)},
    new_file{make_unique<proc_entry>()}
  { }


  pidfile::~pidfile() {
    if (! exists())
      return;
    if (same_exe())
      fs::remove(filename);
  }


  bool pidfile::exists() const {
    return in_file->exists();
  }


  bool pidfile::same_exe() const {
    if (!exists())
      return false;
    return in_file->get_exe_link()==new_file->get_exe_link();
  }


  bool pidfile::same_pid() const {
    if (!exists())
      return false;
    if (!same_exe())
      return false;
    return (in_file->get_pid() == new_file->get_pid() );
  }


  bool pidfile::write() {
    if (exists() && same_exe()) {
      if (same_pid())
        return true;
      else
        return false;
    }

    ofstream of(filename);
    of << *(new_file->get_pid()) << endl;
    return true;
  }

}
