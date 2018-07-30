#include "sdl_remote_adapter_client.h"

#include <iostream>

#include "common/constants.h"

namespace lua_lib {

SDLRemoteTestAdapterClient::SDLRemoteTestAdapterClient(const std::string& host,
                                                       uint32_t port)
    : connection_(host, port) {
  const int timeout_ = 100000;
  connection_.set_timeout(timeout_);
  try {
    std::cout << "Check connection: ";
    connection_.call(constants::client_connected);
    std::cout << "OK" << std::endl;
  } catch (rpc::timeout &t) {
    handleRpcTimeout(t);
  }
}

bool SDLRemoteTestAdapterClient::connected() const {
  std::cout << "Check connection: ";
  if (rpc::client::connection_state::connected ==
         connection_.get_connection_state()) {
    std::cout << "OK" << std::endl;
    return true;
  }
  std::cout << "Not connected" << std::endl;
  return false;
}

int SDLRemoteTestAdapterClient::open(const std::string& name) try {
  std::cout << "Open Mq " << name << " on remote host:" << std::endl;
  if (connected()) {
    connection_.call(constants::mq_open, name);
    std::cout << "SUCCESS\n" << std::endl;
    return constants::error_codes::SUCCESS;
  }
  return constants::error_codes::NO_CONNECTION;
} catch (rpc::rpc_error& e) {
  return handleRpcError(e);
} catch (rpc::timeout &t) {
  return handleRpcTimeout(t);
}

int SDLRemoteTestAdapterClient::open_with_params(const std::string& name,
                                                 const int max_messages_number,
                                                 const int max_message_size,
                                                 const int flags,
                                                 const int mode) try {
  std::cout << "Open Mq " << name << " on remote host with next parameters:"
      << "\nmax_messages_number: " << max_messages_number
      << "\nmax_message_size: " << max_message_size
      << "\nflags: " << flags
      << "\nmode: " << mode << std::endl;
  if (connected()) {
    connection_.call(constants::mq_open_with_params,
                     name,
                     max_messages_number,
                     max_message_size,
                     flags,
                     mode);
    std::cout << "SUCCESS\n" << std::endl;
    return constants::error_codes::SUCCESS;
  }
  return constants::error_codes::NO_CONNECTION;
} catch (rpc::rpc_error& e) {
  return handleRpcError(e);
} catch (rpc::timeout &t) {
  return handleRpcTimeout(t);
}

int SDLRemoteTestAdapterClient::close(const std::string& name) try {
  std::cout << "Close Mq " << name << " on remote host:" << std::endl;
  if (connected()) {
    connection_.call(constants::mq_close, name);
    std::cout << "SUCCESS\n" << std::endl;
    return constants::error_codes::SUCCESS;
  }
  return constants::error_codes::NO_CONNECTION;
} catch (rpc::rpc_error& e) {
  return handleRpcError(e);
} catch (rpc::timeout &t) {
  return handleRpcTimeout(t);
}

int SDLRemoteTestAdapterClient::unlink(const std::string& name) try {
  std::cout << "Unlink Mq " << name << " on remote host:" << std::endl;
  if (connected()) {
    connection_.call(constants::mq_unlink, name);
    std::cout << "SUCCESS\n" << std::endl;
    return constants::error_codes::SUCCESS;
  }
  return constants::error_codes::NO_CONNECTION;
} catch (rpc::rpc_error& e) {
  return handleRpcError(e);
} catch (rpc::timeout &t) {
  return handleRpcTimeout(t);
}

int SDLRemoteTestAdapterClient::clear() try {
  std::cout << "Close all Mq handled by server on remote host:" << std::endl;
  if (connected()) {
    connection_.call(constants::mq_clear);
    std::cout << "SUCCESS\n" << std::endl;
    return constants::error_codes::SUCCESS;
  }
  return constants::error_codes::NO_CONNECTION;
} catch (rpc::rpc_error& e) {
  return handleRpcError(e);
} catch (rpc::timeout &t) {
  return handleRpcTimeout(t);
}

std::pair<std::string, int> SDLRemoteTestAdapterClient::receive(
    const std::string& name) try {
  std::cout << "Receive data from Mq " << name << " on remote host:" << std::endl;
  if (connected()) {
    using result = std::pair<std::string, int>;
    const std::pair<std::string, int> received =
        connection_.call(constants::mq_receive, name).as<result>();
    std::cout << "SUCCESS\nReceived data: " << received.first << "\n" << std::endl;
    return std::make_pair(received.first, received.second);
  }
  return std::make_pair(std::string(), constants::error_codes::NO_CONNECTION);
} catch (rpc::rpc_error& e) {
  return std::make_pair(std::string(), handleRpcError(e));
} catch (rpc::timeout &t) {
  return std::make_pair(std::string(), handleRpcTimeout(t));
}

int SDLRemoteTestAdapterClient::send(const std::string& name,
                                     const std::string& data) try {
  std::cout << "Send data to Mq " << name << " on remote host"
      << "\nData to send: "<< data << std::endl;
  if (connected()) {
    connection_.call(constants::mq_send, name, data);
    std::cout << "SUCCESS\n" << std::endl;
    return constants::error_codes::SUCCESS;
  }
  return constants::error_codes::NO_CONNECTION;
} catch (rpc::rpc_error& e) {
  return handleRpcError(e);
} catch (rpc::timeout &t) {
  return handleRpcTimeout(t);
}

int SDLRemoteTestAdapterClient::shm_open(const std::string& name, const int prot) try {
  std::cout << "Open shared memory " << name << " on remote host:" << std::endl;
  if (connected()) {
    connection_.call(constants::shm_open, name, prot);
    std::cout << "SUCCESS\n" << std::endl;
    return constants::error_codes::SUCCESS;
  }
  return constants::error_codes::NO_CONNECTION;
} catch (rpc::rpc_error& e) {
  return handleRpcError(e);
} catch (rpc::timeout &t) {
  return handleRpcTimeout(t);
}

int SDLRemoteTestAdapterClient::shm_close(const std::string& name) try {
  std::cout << "Close shared memory " << name << " on remote host:" << std::endl;
  if (connected()) {
    connection_.call(constants::shm_close, name);
    std::cout << "SUCCESS\n" << std::endl;
    return constants::error_codes::SUCCESS;
  }
  return constants::error_codes::NO_CONNECTION;
} catch (rpc::rpc_error& e) {
  return handleRpcError(e);
} catch (rpc::timeout &t) {
  return handleRpcTimeout(t);
}

int SDLRemoteTestAdapterClient::app_start(const std::string& path,
                                          const std::string& name) try {
  std::cout << "Start application " << name << " on remote host"
      << "\nPath to application: "<< path << std::endl;
  if (connected()) {
    connection_.call(constants::app_start, path, name);
    std::cout << "SUCCESS\n" << std::endl;
    return constants::error_codes::SUCCESS;
  }
  return constants::error_codes::NO_CONNECTION;
} catch (rpc::rpc_error& e) {
  return handleRpcError(e);
} catch (rpc::timeout &t) {
  return handleRpcTimeout(t);
}

int SDLRemoteTestAdapterClient::app_stop(const std::string& name) try {
  std::cout << "Stop application " << name
            << " on remote host" << std::endl;
  if (connected()) {
    connection_.call(constants::app_stop, name);
    std::cout << "SUCCESS\n" << std::endl;
    return constants::error_codes::SUCCESS;
  }
  return constants::error_codes::NO_CONNECTION;
} catch (rpc::rpc_error& e) {
  return handleRpcError(e);
} catch (rpc::timeout &t) {
  return handleRpcTimeout(t);
}

std::pair<int, int> SDLRemoteTestAdapterClient::app_check_status(const std::string& name) try {
  std::cout << "Check status of application " << name
            << " on remote host" << std::endl;
  if (connected()) {
    using result = std::pair<int, int>;
    const std::pair<int, int> received =
        connection_.call(constants::app_check_status, name).as<result>();
    std::cout << "SUCCESS\nReceived data: " << received.first << "\n" << std::endl;
    return std::make_pair(received.first, received.second);
  }
  return std::make_pair(0, constants::error_codes::NO_CONNECTION);
} catch (rpc::rpc_error& e) {
  return std::make_pair(0, handleRpcError(e));
} catch (rpc::timeout &t) {
  return std::make_pair(0, handleRpcTimeout(t));
}

std::pair<bool, int> SDLRemoteTestAdapterClient::file_exists(const std::string& path,
                                          const std::string& name) try {
  std::cout << "Check existance of file " << name << " on remote host"
      << "\nPath to file: "<< path << std::endl;
  if (connected()) {
    using result = std::pair<bool, int>;
    const std::pair<bool, int> received =
        connection_.call(constants::file_exists, path, name).as<result>();
    std::cout << "SUCCESS\nReceived data: " << received.first << "\n" << std::endl;
    return std::make_pair(received.first, received.second);
  }
  return std::make_pair(false, constants::error_codes::NO_CONNECTION);
} catch (rpc::rpc_error& e) {
  return std::make_pair(false, handleRpcError(e));
} catch (rpc::timeout &t) {
  return std::make_pair(false, handleRpcTimeout(t));
}

int SDLRemoteTestAdapterClient::file_update(const std::string& path,
                                            const std::string& name,
                                            const std::string& content) try {
  std::cout << "Update content of file " << name << " on remote host"
      << "\nPath to file: "<< path << std::endl;
  if (connected()) {
    connection_.call(constants::file_update, path, name, content);
    std::cout << "SUCCESS\n" << std::endl;
    return constants::error_codes::SUCCESS;
  }
  return constants::error_codes::NO_CONNECTION;
} catch (rpc::rpc_error& e) {
  return handleRpcError(e);
} catch (rpc::timeout &t) {
  return handleRpcTimeout(t);
}

std::pair<std::string, int> SDLRemoteTestAdapterClient::file_content(
                                              const std::string& path,
                                              const std::string& name) try {
  std::cout << "Get content of file " << name << " on remote host"
      << "\nPath to file: "<< path << std::endl;
  if (connected()) {
    using result = std::pair<std::string, int>;
    const std::pair<std::string, int> received =
        connection_.call(constants::file_content, path, name).as<result>();
    std::cout << "SUCCESS\nReceived data: " << received.first << "\n" << std::endl;
    return std::make_pair(received.first, received.second);
  }
  return std::make_pair(std::string(), constants::error_codes::NO_CONNECTION);
} catch (rpc::rpc_error& e) {
  return std::make_pair(std::string(), handleRpcError(e));
} catch (rpc::timeout &t) {
  return std::make_pair(std::string(), handleRpcTimeout(t));
}

int SDLRemoteTestAdapterClient::file_delete(const std::string& path,
                                          const std::string& name) try {
  std::cout << "Delete file " << name << " on remote host"
      << "\nPath to file: "<< path << std::endl;
  if (connected()) {
    connection_.call(constants::file_delete, path, name);
    std::cout << "SUCCESS\n" << std::endl;
    return constants::error_codes::SUCCESS;
  }
  return constants::error_codes::NO_CONNECTION;
} catch (rpc::rpc_error& e) {
  return handleRpcError(e);
} catch (rpc::timeout &t) {
  return handleRpcTimeout(t);
}

int SDLRemoteTestAdapterClient::file_backup(const std::string& path,
                                          const std::string& name) try {
  std::cout << "Backup file " << name << " on remote host"
      << "\nPath to file: "<< path << std::endl;
  if (connected()) {
    connection_.call(constants::file_backup, path, name);
    std::cout << "SUCCESS\n" << std::endl;
    return constants::error_codes::SUCCESS;
  }
  return constants::error_codes::NO_CONNECTION;
} catch (rpc::rpc_error& e) {
  return handleRpcError(e);
} catch (rpc::timeout &t) {
  return handleRpcTimeout(t);
}

int SDLRemoteTestAdapterClient::file_restore(const std::string& path,
                                          const std::string& name) try {
  std::cout << "Restore backuped file " << name << " on remote host"
      << "\nPath to file: "<< path << std::endl;
  if (connected()) {
    connection_.call(constants::file_restore, path, name);
    std::cout << "SUCCESS\n" << std::endl;
    return constants::error_codes::SUCCESS;
  }
  return constants::error_codes::NO_CONNECTION;
} catch (rpc::rpc_error& e) {
  return handleRpcError(e);
} catch (rpc::timeout &t) {
  return handleRpcTimeout(t);
}

std::pair<bool, int> SDLRemoteTestAdapterClient::folder_exists(const std::string& path,
                                          const std::string& name) try {
  std::cout << "Check existance of folder  " << name << " on remote host"
      << "\nPath to file: "<< path << std::endl;
  if (connected()) {
    using result = std::pair<bool, int>;
    const std::pair<bool, int> received =
        connection_.call(constants::folder_exists, path, name).as<result>();
    std::cout << "SUCCESS\nReceived data: " << received.first << "\n" << std::endl;
    return std::make_pair(received.first, received.second);
  }
  return std::make_pair(false, constants::error_codes::NO_CONNECTION);
} catch (rpc::rpc_error& e) {
  return std::make_pair(false, handleRpcError(e));
} catch (rpc::timeout &t) {
  return std::make_pair(false, handleRpcTimeout(t));
}

int SDLRemoteTestAdapterClient::folder_create(const std::string& path,
                                          const std::string& name) try {
  std::cout << "Create folder " << name << " on remote host"
      << "\nPath to file: "<< path << std::endl;
  if (connected()) {
    connection_.call(constants::folder_create, path, name);
    std::cout << "SUCCESS\n" << std::endl;
    return constants::error_codes::SUCCESS;
  }
  return constants::error_codes::NO_CONNECTION;
} catch (rpc::rpc_error& e) {
  return handleRpcError(e);
} catch (rpc::timeout &t) {
  return handleRpcTimeout(t);
}

int SDLRemoteTestAdapterClient::folder_delete(const std::string& path,
                                          const std::string& name) try {
  std::cout << "Delete folder " << name << " on remote host"
      << "\nPath to file: "<< path << std::endl;
  if (connected()) {
    connection_.call(constants::folder_delete, path, name);
    std::cout << "SUCCESS\n" << std::endl;
    return constants::error_codes::SUCCESS;
  }
  return constants::error_codes::NO_CONNECTION;
} catch (rpc::rpc_error& e) {
  return handleRpcError(e);
} catch (rpc::timeout &t) {
  return handleRpcTimeout(t);
}

int SDLRemoteTestAdapterClient::handleRpcError(rpc::rpc_error& e) {
  std::cout << "EXCEPTION Occured in function: "
            << e.get_function_name() <<  std::endl;
  std::cout << "[Error type]: " << e.what() << std::endl;
  auto err = e.get_error().as<std::pair<int, std::string> >();
  std::cout << "[Error code]: " << err.first << std::endl
            << "[Error description]: " << err.second << "\n" << std::endl;
  return err.first;
}

int SDLRemoteTestAdapterClient::handleRpcTimeout(rpc::timeout& t) {
  std::cout << "TIMEOUT expired: " << t.what() << "\n" << std::endl;
  return constants::error_codes::TIMEOUT_EXPIRED;
}

}  // namespace lua_lib
