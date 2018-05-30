#pragma once

#include <string>

namespace constants {
static std::string open = "mq_open";
static std::string close = "mq_close";
static std::string unlink = "mq_unlink";
static std::string send = "mq_send";
static std::string receive = "mq_receive";
static std::string clear = "mq_clear";
static std::string client_connected = "client_connected";

namespace error_codes {
static const int SUCCESS = 0;
static const int READ_FAILURE = -1;
static const int WRITE_FAILURE = -2;
static const int PATH_NOT_FOUND = -3;
static const int CLOSE_FAILURE = -4;
}  // namespace error_codes
}  // namespace constants
