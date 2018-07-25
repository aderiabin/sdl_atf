#include "utils_manager.h"
#include <sys/syspage.h>
#include <sys/debug.h> 
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <spawn.h>
#include <errno.h>
#include "common/constants.h"

namespace utils_wrappers {

using namespace constants;

int UtilsManager::StartApp(const std::string & app_path,const std::string & app_name){

    std::string app_full_path = app_path;
    app_full_path.append(app_name);

    char * const argv[] = {strdup(app_name.c_str()),NULL};  

    errno = 0;
    pid_t app_pid = spawnp(app_full_path.c_str(), 0, NULL,NULL, argv, NULL);

    free(argv[0]);

    if(error_codes::FAILED != app_pid){
        if(0 == kill(app_pid,0)){
            return constants::error_codes::SUCCESS;
        }
    }

    printf("\n%s:\n%s\n",strerror(errno));
    return error_codes::FAILED;
}

int UtilsManager::StopApp(const std::string & app_name,const int sig){ 

    ArrayPid arr_pid = GetPidApp(app_name);
    bool is_all_killed = true;

    for(const auto & app_pid : arr_pid){
        if(error_codes::FAILED == KillApp(app_pid,sig,app_name.c_str())){
            is_all_killed = false;
        }
    }

    if(is_all_killed){
        return error_codes::SUCCESS;
    }

    is_all_killed = true;
    for(const auto & app_pid : arr_pid){
        if(IsExistsApp(app_pid)){
            if(error_codes::FAILED == KillApp(app_pid,sig,app_name.c_str())){
                is_all_killed = false;
            }
        }
    }

    return is_all_killed ?
           error_codes::SUCCESS
           :
           error_codes::FAILED;
}

int UtilsManager::CheckStatusApp(const std::string & app_name){
    printf ("\nUtilsManager::CheckStatusApp");    
    ArrayPid arr_pid = GetPidApp(app_name);
    if(0 == arr_pid.size()){
        printf ("\%s is NOT_RUNNING",app_name.c_str());
        return stat_app_codes::NOT_RUNNING;
    }

    int num_threads = 0;
    procfs_info info;

    for(const auto & app_pid : arr_pid){
        GetNameApp(app_pid,&info);
        num_threads += info.num_threads;
    }
    printf ("\n%s has: %d thread ",app_name.c_str(),num_threads);
    if(num_threads > 1){
        printf ("\n%s is RUNNING",app_name.c_str());
        return stat_app_codes::RUNNING;
    }    
    printf ("\n%s is CRASHED",app_name.c_str());
    return stat_app_codes::CRASHED;
}

int UtilsManager::FileBackup(const std::string & file_path,const std::string & file_name){
    printf ("\nUtilsManager::FileBackup");
    return error_codes::SUCCESS;
}

int UtilsManager::FileRestore(const std::string & file_path,const std::string & file_name){
    printf ("\nUtilsManager::FileRestore");
    return error_codes::SUCCESS;
}

int UtilsManager::FileUpdate(const std::string & file_path,const std::string & file_name,const std::string & file_content){
    printf ("\nUtilsManager::FileUpdate");
    return error_codes::SUCCESS;
}

int UtilsManager::FileExists(const std::string & file_path,const std::string & file_name){
    printf ("\nUtilsManager::FileUpdate");
    return error_codes::SUCCESS;
}

int UtilsManager::FileDelete(const std::string & file_path,const std::string & file_name){
    printf ("\nUtilsManager::FileDelete");
    return error_codes::SUCCESS;
}

std::string UtilsManager::GetFileContent(const std::string & file_path,const std::string & file_name){
    printf ("\nUtilsManager::GetFileContent");
    return std::string();
}

int UtilsManager::FolderExists(const std::string & folder_path,const std::string & folder_name){
    printf ("\nUtilsManager::FolderExists");
    return error_codes::SUCCESS;
}

int UtilsManager::FolderDelete(const std::string & folder_path){
    printf ("\nUtilsManager::FolderDelete");
    return error_codes::SUCCESS;
}

UtilsManager::ArrayPid UtilsManager::GetPidApp(const std::string & app_name){

    struct dirent   *dirent;
    DIR             *dir;
    int             app_pid;

    if (!(dir = opendir ("/proc"))) {
    fprintf (stderr, "\ncouldn't open /proc, errno %d\n",errno);
    perror (NULL);
    return ArrayPid();
    }

    ArrayPid arr_pid;

    while(dirent = readdir (dir)){
        if(isdigit (*dirent -> d_name)){
            app_pid = atoi (dirent -> d_name);
            if(0 == app_name.compare(GetNameApp(app_pid))){
                arr_pid.push_back(app_pid);
            }
        }
    }

    closedir (dir);

    return arr_pid;
}

std::string UtilsManager::GetNameApp(int app_pid,procfs_info * proc_info){  

    char      paths [PATH_MAX];
    int       fd;
    static struct {
    procfs_debuginfo    info;
    char                buff [PATH_MAX];
    } name;

    sprintf (paths, "/proc/%d/as", app_pid);

    if ((fd = open (paths, O_RDONLY)) == -1) {
    return "";
    }

    if(devctl (fd, DCMD_PROC_MAPDEBUG_BASE, &name,sizeof (name), 0) != EOK){
        if(app_pid == 1){
            strcpy (name.info.path, "/(procnto)");
        }else{
            strcpy (name.info.path, "/(n/a)");
        }
    }

    if(proc_info){
        int sts = devctl (fd, DCMD_PROC_INFO,proc_info, sizeof (procfs_info), NULL);
        if (sts != EOK){
            fprintf(stderr, "\n%s: DCMD_PROC_INFO pid %d errno %d (%s)",
            strrchr(name.info.path, '/') + 1, app_pid, errno, strerror (errno));
        }
    }

    close (fd);

    return std::string(strrchr(name.info.path, '/') + 1);
}

int UtilsManager::KillApp(const pid_t app_pid,const int sig,const char * app_name){

    errno = 0;
    kill(app_pid,sig);

    if(false == IsExistsApp(app_pid)){
        printf("\nSucces kill pid: %d app: %s\n",app_pid,app_name ? app_name : "");
        return constants::error_codes::SUCCESS;
    }

    switch(errno){
        case EAGAIN:
        printf("\nFailed kill pid: %d app: %s "
                "Insufficient system resources are available to deliver the signal."
                ,app_pid,app_name ? app_name : ""); 
                break;
        case EINVAL:
        printf("\nFailed kill pid: %d app: %s "
                "The sig is invalid."
                ,app_pid,app_name ? app_name : "");
        break;
        case EPERM:
        printf("\nFailed kill pid: %d app: %s "
                "The process doesn't have permission to send this signal to any receiving process."
                ,app_pid,app_name ? app_name : ""); 
        break;
        case ESRCH:
        printf("\nFailed kill pid: %d app: %s "
                "The given pid doesn't exist."
                ,app_pid,app_name ? app_name : "");
        break;
        default:
            printf("\nFailed kill pid: %d app: %s Unknown error in errno"
                ,app_pid,app_name ? app_name : "");
    }

    return constants::error_codes::FAILED;
}

bool UtilsManager::IsExistsApp(const pid_t app_pid){

    struct stat stat_buff;
    char        proc_path[PATH_MAX];

    sprintf(proc_path,"/proc/%d",app_pid);

    return 0 == (stat(proc_path, &stat_buff));
}

}  // namespace utils_wrappers