#include <iostream>
#include <string>
#include <exception>

#include "rpc/server.h"
#include "rpc/this_handler.h"

#include "mqueue_manager.h"
#include "common/constants.h"

#define KILL_START_SDLSCRIPT "\
#/bin/bash \n\
sleep 0.2\n\
PID=\"$(ps -ef | grep -e \"^$(whoami).*SmartDeviceLink\" | grep -v grep | awk '{print $2}')\"\n\
if [ -n \"$PID\" ]; then\n\
kill -9 $PID\n\
fi\n\
sleep 0.2\n\
./SmartDeviceLink &\
"
#define KILLSDLSCRIPT "\
#/bin/bash \n\
sleep 0.2\n\
PID=\"$(ps -ef | grep -e \"^$(whoami).*SmartDeviceLink\" | grep -v grep | awk '{print $2}')\"\n\
if [ -n \"$PID\" ]; then\n\
kill -9 $PID\n\
fi\n\
sleep 0.2\
"

void PrintUsage() {
  std::cout << "\nUsage:" << std::endl;
  std::cout << "------------------------------------------------" << std::endl;
  std::cout << "For default port usage(port 5555): " << std::endl;
  std::cout << "./RemoteTestingAdapterServer" << std::endl;
  std::cout << "------------------------------------------------" << std::endl;
  std::cout << "For custom port usage: " << std::endl;
  std::cout << "./RemoteTestingAdapterServer <port>" << std::endl;
  std::cout << "------------------------------------------------\n"
            << std::endl;
  std::cout << "NOTE: Port must be unsigned integer within 1024 - 65535\n";
}

bool IsUnsignedNumber(const std::string& number) {
  // Checking for negative numbers
  if ('-' == number[0] && isdigit(number[1])) {
    std::cout << "\nNumber is negative!" << std::endl;
    return false;
  }
  // Check that every symbol is a number
  for (const char& c : number) {
    if (!isdigit(c))
      return false;
  }
  return true;
}

void CheckError(const int res) {
  if (constants::error_codes::SUCCESS == res) {
    return;
  }
  std::string stringified_error;
  switch (res) {
    case constants::error_codes::PATH_NOT_FOUND:
      stringified_error = "Mqueue not found";
      break;
    case constants::error_codes::READ_FAILURE:
      stringified_error = "Mqueue reading failure";
      break;
    case constants::error_codes::WRITE_FAILURE:
      stringified_error = "Mqueue writing failure";
      break;
    case constants::error_codes::CLOSE_FAILURE:
      stringified_error = "Mqueue closing failure";
      break;
    default:
      stringified_error = strerror(res);
      break;
  }
  const auto err_obj = std::make_pair(res, stringified_error);
  rpc::this_handler().respond_error(err_obj);
}

int main(int argc, char* argv[]) {

  system(KILL_START_SDLSCRIPT);

  uint16_t port = 5555;
  if (2 == argc) {
    const std::string number = argv[1];
    if (IsUnsignedNumber(number)) {
      const uint16_t arg_port = std::atoi(number.c_str());
      port = 0 != arg_port ? arg_port : port;
    } else {
      PrintUsage();
      return 1;
    }
  }

  if (argc > 2) {
    PrintUsage();
    return 1;
  }
  std::cout << "Listen on " << port << std::endl;
  try {
    rpc::server srv(port);

    mq_wrappers::MQueueManager mq_manager;

    srv.bind(constants::client_connected,
             []() {
               std::cout << "Hello" << std::endl;
               std::cout << "Client connected" << std::endl;
             });

    srv.bind(constants::open,
             [&mq_manager](std::string path) {
               const int res = mq_manager.MqOpen(path);
               CheckError(res);
             });

    srv.bind(constants::open_with_params,
             [&mq_manager](std::string path,
                           const int max_messages_number,
                           const int max_message_size,
                           const int flags,
                           const int mode) {
               const int res = mq_manager.MqOpenWithParams(
                   path, max_messages_number, max_message_size, flags, mode);
               CheckError(res);
             });

    srv.bind(constants::close,
             [&mq_manager](std::string path) {
               const int res = mq_manager.MqClose(path);
               CheckError(res);
             });

    srv.bind(constants::unlink,
             [&mq_manager](std::string path) {
               const int res = mq_manager.MqUnlink(path);
               CheckError(res);
             });

    srv.bind(constants::send,
             [&mq_manager](std::string path, std::string data) {
               const int res = mq_manager.MqSend(path, data);
               CheckError(res);
             });

    srv.bind(constants::receive,
             [&mq_manager](std::string path) -> std::pair<std::string, int> {
               const auto receive_result = mq_manager.MqReceive(path);
               auto res = receive_result.second;
               CheckError(res);
               return receive_result;
             });

    srv.bind(constants::clear,
             [&mq_manager]() {
               const auto res = mq_manager.MqClear();
               CheckError(res);
             });

    // Run the server loop with 2 worker threads.
    srv.async_run(2);
    std::cin.ignore();
  } catch (std::exception& e) {
    std::cout << "Error: " << e.what() << std::endl;
    std::cout << "Exception occured" << std::endl;
    PrintUsage();
  }

  system(KILLSDLSCRIPT);

  return 0;
}
