#include <iostream>
#include <string>
#include <exception>

#include "rpc/server.h"
#include "rpc/this_handler.h"

#include "mqueue_manager.h"
#include "utils_manager.h"
#include "common/constants.h"

using utils_wrappers::UtilsManager;

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
    case constants::error_codes::ALREADY_EXISTS:
      stringified_error = "Channel already exists";
      break;
    default:
      stringified_error = strerror(res);
      break;
  }
  const auto err_obj = std::make_pair(res, stringified_error);
  rpc::this_handler().respond_error(err_obj);
}

int main(int argc, char* argv[]) {

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

    srv.bind(constants::mq_open,
             [&mq_manager](std::string path) {
               const int res = mq_manager.MqOpen(path);
               CheckError(res);
             });

    srv.bind(constants::mq_open_with_params,
             [&mq_manager](std::string path,
                           const int max_messages_number,
                           const int max_message_size,
                           const int flags,
                           const int mode) {
               const int res = mq_manager.MqOpenWithParams(
                   path, max_messages_number, max_message_size, flags, mode);
               CheckError(res);
             });

    srv.bind(constants::mq_close,
             [&mq_manager](std::string path) {
               const int res = mq_manager.MqClose(path);
               CheckError(res);
             });

    srv.bind(constants::mq_unlink,
             [&mq_manager](std::string path) {
               const int res = mq_manager.MqUnlink(path);
               CheckError(res);
             });

    srv.bind(constants::mq_send,
             [&mq_manager](std::string path, std::string data) {
               const int res = mq_manager.MqSend(path, data);
               CheckError(res);
             });

    srv.bind(constants::mq_receive,
             [&mq_manager](std::string path) -> std::pair<std::string, int> {
               const auto receive_result = mq_manager.MqReceive(path);
               return receive_result;
             });

    srv.bind(constants::mq_clear,
             [&mq_manager]() {
               const auto res = mq_manager.MqClear();
               CheckError(res);
             });

    srv.bind(constants::shm_open,
             [&mq_manager](std::string shm_name,
                           const int prot){
               const int res = mq_manager.ShmOpen(shm_name,prot);
               CheckError(res);
             });

    srv.bind(constants::shm_close,
             [&mq_manager](std::string shm_name){
               const int res = mq_manager.ShmClose(shm_name);
               CheckError(res);
             });

    srv.bind(constants::app_start,
             [](std::string app_path,std::string app_name){
               const int res = UtilsManager::StartApp(
                                                  app_path,
                                                  app_name);
               return res;
             });

    srv.bind(constants::app_stop,
             [](std::string app_name){
               const int res = UtilsManager::StopApp(app_name);
               return res;
             });

    srv.bind(constants::app_check_status,
             [](std::string app_name){
               const int res = UtilsManager::CheckStatusApp(app_name);
               return res;
             });

    srv.bind(constants::file_backup,
             [](std::string file_path,std::string file_name){
               const int res = UtilsManager::FileBackup(
                                                  file_path,
                                                  file_name);
               return res;
             });

     srv.bind(constants::file_restore,
             [](std::string file_path,std::string file_name){
               const int res = UtilsManager::FileRestore(
                                                file_path,
                                                file_name);
               return res;
             });

    srv.bind(constants::file_update,
             [](std::string file_path,
                std::string file_name,
                std::string file_content){

                  const int res = UtilsManager::FileUpdate(
                                                    file_path,
                                                    file_name,
                                                    file_content);
                  return res;
             });

    srv.bind(constants::file_exists,
             [](std::string file_path,std::string file_name){
               const int res = UtilsManager::FileExists(
                                                    file_path,
                                                    file_name);
               return res;
             });

    srv.bind(constants::file_delete,
             [](std::string file_path,std::string file_name){
               const int res = UtilsManager::FileDelete(
                                                    file_path,
                                                    file_name);
               return res;
             });

    srv.bind(constants::file_content,
             [](std::string file_path,
                std::string file_name,
                size_t offset,
                size_t max_size_content){

                  std::string file_content =
                           UtilsManager::GetFileContent(
                                          file_path,
                                          file_name,
                                          offset,
                                          max_size_content
                                          );

                  return std::make_pair(
                                  file_content,
                                  offset
                                  );
             });

    srv.bind(constants::folder_exists,
             [](std::string folder_path){
               const int res = UtilsManager::FolderExists(
                                                    folder_path);
               return res;
             });





    srv.bind(constants::folder_delete,
             [](std::string folder_path){
               const int res = UtilsManager::FolderDelete(
                                                    folder_path);
               return res ?
                constants::error_codes::FAILED
                :
                constants::error_codes::SUCCESS;
             });

    srv.bind(constants::folder_create,
             [](std::string folder_path){
               const int res =
                  UtilsManager::FolderCreate(folder_path);
               return res;
             });

    srv.bind(constants::command_execute,
             [](std::string bash_command){

                auto receive_result =
                            UtilsManager::ExecuteCommand(bash_command);

                return receive_result;

             });



    // Run the server loop with 2 worker threads.
    srv.async_run(2);
    std::cin.ignore();
  } catch (std::exception& e) {
    std::cout << "Error: " << e.what() << std::endl;
    std::cout << "Exception occured" << std::endl;
    PrintUsage();
  }

  return 0;
}
