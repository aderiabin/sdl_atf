#include <iostream>
#include <string>

#include "rpc/server.h"
#include "rpc/this_handler.h"

#include "mqueue_manager.h"
#include "common/constants.h"

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
  const auto err_obj = std::make_tuple(res, stringified_error);
  rpc::this_handler().respond_error(err_obj);
}

int main(int argc, char* argv[]) {
  int port = 5555;
  if (argc > 1) {
    const int arg_port = std::atoi(argv[1]);
    port = 0 != arg_port ? arg_port : port;
  }
  std::cout << "Listen on " << port << std::endl;
  rpc::server srv(port);

  mq_wrappers::MQueueManager mq_manager;

  srv.bind(constants::client_connected,
           []() { std::cout << "Hello" << std::endl; });

  srv.bind(constants::open,
           [&mq_manager](std::string path) {
             const int res = mq_manager.MqOpen(path);
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
           [&mq_manager](std::string path) -> std::string {
             const auto receive_result = mq_manager.MqReceive(path);
             auto data = std::get<0>(receive_result);
             auto res = std::get<1>(receive_result);
             CheckError(res);
             return data;
           });

  srv.bind(constants::clear,
           [&mq_manager]() {
             const auto res = mq_manager.MqClear();
             CheckError(res);
           });
  // Run the server loop.
  srv.run();

  return 0;
}
