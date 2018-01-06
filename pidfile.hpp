#ifndef PIDFILE_HEADER
#define PIDFILE_HEADER

/* project: tunmon
 * author: Jurgen Van Ham
 * license: GPL3
 *
 * description: tool to act on missing inbound traffic of a tunnel
 */



#include<string>
#include<memory>

namespace posix_util {

  
  class pidfile {

  private:
    class proc_entry;           // pimpl
    const std::string filename;
    std::unique_ptr<proc_entry> in_file;
    std::unique_ptr<proc_entry> new_file;
    bool exists() const;
    bool same_exe() const;

  public:
    pidfile(const std::string &path);
    bool same_pid() const;
    bool write();
    virtual ~pidfile();
  };


}

#endif
