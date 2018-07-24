#pragma once

#include <string>

namespace utils_wrappers {

class UtilsManager {
 public:
    int AppStart(const std::string & app_path,const std::string & app_name);
    int AppStop(const std::string & app_name);
    int AppCheckStatus(const std::string & app_name);
    int FileBackup(const std::string & file_path,const std::string & file_name);
    int FileRestore(const std::string & file_path,const std::string & file_name);
    int FileUpdate(const std::string & file_path,const std::string & file_name,const std::string & file_content);
 };

}  // namespace utils_wrappers
