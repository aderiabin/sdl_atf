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
    return 0;
  }
  std::cout << "Not connected" << std::endl;
  return 1;
} catch (rpc::rpc_error& e) {
  std::cout << "e " << e.what() << std::endl;
  using err_t = std::tuple<int, std::string>;
  auto err = e.get_error().as<err_t>();
  std::cout << "[error " << std::get<0>(err) << "]: " << std::get<1>(err)
            << std::endl;
  return 2;
}

int SDLRemoteTestAdapterClient::clear() try {
  if (connected()) {
    connection_.call(constants::clear);
    return 0;
  }
  std::cout << "Not connected" << std::endl;
  return 1;
} catch (rpc::rpc_error& e) {
  std::cout << "Error: " << e.what() << std::endl;
  return 2;
}

std::string SDLRemoteTestAdapterClient::receive(const std::string& name) try {
  if (connected()) {
    auto received =
        connection_.call(constants::receive, name).as<std::string>();
    return received;
  }
  return "";
} catch (rpc::rpc_error& e) {
  return "";
}

int SDLRemoteTestAdapterClient::send(const std::string& name,
                                     const std::string& data) try {
  if (connected()) {
    connection_.call(constants::send, name, data);
    return 0;
  }
  std::cout << "Not connected" << std::endl;
  return 1;
} catch (rpc::rpc_error& e) {
  std::cout << "Error: " << e.what() << std::endl;
  return 2;
}

}  // namespace lua_lib
