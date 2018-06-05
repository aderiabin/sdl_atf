#include "sdl_remote_adapter_client.h"

#include <iostream>

#include "rpc/rpc_error.h"

#include "common/constants.h"

namespace lua_lib {

SDLRemoteTestAdapterClient::SDLRemoteTestAdapterClient(const std::string& host,
                                                       uint32_t port)
    : connection_(host, port) {
  connection_.call(constants::client_connected);
}

bool SDLRemoteTestAdapterClient::connected() const {
  return rpc::client::connection_state::connected ==
         connection_.get_connection_state();
}

int SDLRemoteTestAdapterClient::open(const std::string& name) try {
  if (connected()) {
    connection_.call(constants::open, name);
    return constants::error_codes::SUCCESS;
  }
  std::cout << "Not connected" << std::endl;
  return constants::error_codes::NO_CONNECTION;
} catch (rpc::rpc_error& e) {
  std::cout << "EXCEPTION Occured" << std::endl;
  std::cout << "Error type:  " << e.what() << std::endl;
  using err_t = std::pair<int, std::string>;
  auto err = e.get_error().as<err_t>();
  std::cout << "[Error code]: " << err.first << std::endl
            << "[Error description]: " << err.second << std::endl;
  return err.first;
}

int SDLRemoteTestAdapterClient::open_with_params(const std::string& name,
                                                 const int max_messages_number,
                                                 const int max_message_size,
                                                 const int flags,
                                                 const int mode) try {
  if (connected()) {
    connection_.call(constants::open_with_params,
                     name,
                     max_messages_number,
                     max_message_size,
                     flags,
                     mode);
    return constants::error_codes::SUCCESS;
  }
  std::cout << "Not connected" << std::endl;
  return constants::error_codes::NO_CONNECTION;
} catch (rpc::rpc_error& e) {
  std::cout << "EXCEPTION Occured" << std::endl;
  std::cout << "Error type:  " << e.what() << std::endl;
  using err_t = std::pair<int, std::string>;
  auto err = e.get_error().as<err_t>();
  std::cout << "[Error code]: " << err.first << std::endl
            << "[Error description]: " << err.second << std::endl;
  return err.first;
}

int SDLRemoteTestAdapterClient::clear() try {
  if (connected()) {
    connection_.call(constants::clear);
    return constants::error_codes::SUCCESS;
  }
  std::cout << "Not connected" << std::endl;
  return constants::error_codes::NO_CONNECTION;
} catch (rpc::rpc_error& e) {
  std::cout << "EXCEPTION Occured" << std::endl;
  std::cout << "Error type:  " << e.what() << std::endl;
  using err_t = std::pair<int, std::string>;
  auto err = e.get_error().as<err_t>();
  std::cout << "[Error code]: " << err.first << std::endl
            << "[Error description]: " << err.second << std::endl;
  return err.first;
}

std::pair<std::string, int> SDLRemoteTestAdapterClient::receive(
    const std::string& name) try {
  if (connected()) {
    using result = std::pair<std::string, int>;
    const std::pair<std::string, int> received =
        connection_.call(constants::receive, name).as<result>();
    std::cout << "Returning received data" << std::endl;
    return std::make_pair(received.first, received.second);
  }
  return std::make_pair(std::string(), constants::error_codes::NO_CONNECTION);
} catch (rpc::rpc_error& e) {
  std::cout << "EXCEPTION Occured" << std::endl;
  std::cout << "Error type:  " << e.what() << std::endl;
  using err_t = std::pair<int, std::string>;
  auto err = e.get_error().as<err_t>();
  std::cout << "[Error code]: " << err.first << std::endl
            << "[Error description]: " << err.second << std::endl;
  return std::make_pair(err.second, err.first);
}

int SDLRemoteTestAdapterClient::send(const std::string& name,
                                     const std::string& data) try {
  if (connected()) {
    connection_.call(constants::send, name, data);
    return constants::error_codes::SUCCESS;
  }
  std::cout << "Not connected" << std::endl;
  return constants::error_codes::NO_CONNECTION;
} catch (rpc::rpc_error& e) {
  std::cout << "EXCEPTION Occured" << std::endl;
  std::cout << "Error type:  " << e.what() << std::endl;
  using err_t = std::pair<int, std::string>;
  auto err = e.get_error().as<err_t>();
  std::cout << "[Error code]: " << err.first << std::endl
            << "[Error description]: " << err.second << std::endl;
  return err.first;
}

}  // namespace lua_lib
