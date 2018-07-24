#pragma once

#include <string>

namespace constants {
static std::string client_connected = "client_connected";
static std::string mq_open = "mq_open";
static std::string mq_open_with_params = "mq_open_with_params";
static std::string mq_close = "mq_close";
static std::string mq_unlink = "mq_unlink";
static std::string mq_send = "mq_send";
static std::string mq_receive = "mq_receive";
static std::string mq_clear = "mq_clear";
static std::string shm_open = "shm_open";
static std::string shm_close = "shm_close";
static const char * const shm_1_applink = "{SDL_TO_APPLINK_SHM_1}";
static const char * const shm_2_applink = "{SDL_TO_APPLINK_SHM_2}";
static std::string app_start = "app_start";
static std::string app_stop = "app_stop";
static std::string app_check_status = "app_check_status";
static std::string file_backup = "file_backup";
static std::string file_restore = "file_restore";
static std::string file_update = "file_update";


namespace error_codes {
static const int SUCCESS = 0;
static const int READ_FAILURE = -1;
static const int WRITE_FAILURE = -2;
static const int PATH_NOT_FOUND = -3;
static const int CLOSE_FAILURE = -4;
static const int OPEN_FAILURE = -5;
static const int NO_CONNECTION = -6;
static const int EXCEPTION_THROWN = -7;
static const int TIMEOUT_EXPIRED = -8;
static const int ALREADY_EXISTS = -9;
}  // namespace error_codes
}  // namespace constants
