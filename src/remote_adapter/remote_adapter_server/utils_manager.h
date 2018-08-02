#pragma once

#include <string>
#include <vector>
#include <sys/procfs.h>
#include <signal.h>

namespace utils_wrappers {

#ifndef __QNX__
typedef void procfs_info;
#endif

class UtilsManager {
 public:
 	typedef std::vector<int> ArrayPid;

    static int StartApp(const std::string & app_path,const std::string & app_name);
    static int StopApp(const std::string & app_name,const int sig = SIGTERM);
    static int CheckStatusApp(const std::string & app_name);
    static int FileBackup(const std::string & file_path,const std::string & file_name);
    static int FileRestore(const std::string & file_path,const std::string & file_name);
    static int FileUpdate(const std::string & file_path,const std::string & file_name,const std::string & file_content);
    static int FileExists(const std::string & file_path,const std::string & file_name);
    static int FileDelete(const std::string & file_path,const std::string & file_name);
    static std::string GetFileContent(const std::string & file_path,const std::string & file_name);
    static int FolderExists(const std::string & folder_path,const std::string & folder_name);
    static int FolderDelete(const std::string & folder_path,const std::string & folder_name);
    static int FolderCreate(const std::string & folder_path,const std::string & folder_name);

 private:
    static ArrayPid GetPidApp(const std::string & app_name);
    static std::string GetNameApp(int pid,procfs_info * proc_info = 0);
    static int KillApp(const pid_t app_pid,const int sig,const char * app_name = 0);
    static bool IsExistsApp(const pid_t app_pid);
 };

}  // namespace utils_wrappers
