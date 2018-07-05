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
    connection_.call(constants::open, name);
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
    connection_.call(constants::open_with_params,
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
    connection_.call(constants::close, name);
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
    connection_.call(constants::unlink, name);
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
    connection_.call(constants::clear);
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
        connection_.call(constants::receive, name).as<result>();
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
    connection_.call(constants::send, name, data);
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
