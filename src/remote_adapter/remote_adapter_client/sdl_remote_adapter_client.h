#pragma once

#include <string>
#include <utility>
#include "rpc/client.h"

namespace lua_lib {

class SDLRemoteTestAdapterClient {
 public:
  SDLRemoteTestAdapterClient(const std::string& host, uint32_t port);

  /**
   * @brief connected checks if client is connected to server
   * @return true if connected otherwise false
   */
  bool connected() const;

  /**
   * @brief Sends open mqueue request to server
   * @param name mqueue name which should be opened by server
   * @return 0 in successful case, 1 - if client is not connected,
   * 2 - in case of exception
   */
  int open(const std::string& name);

  /**
   * @brief Sends open mqueue parametrized request to server
   * @param name mqueue name which should be opened by server
   * @param max_messages_number -max messages number in mqueue
   * @param max_message_size - max message size in mqueue
   * @param flags - specifies flags that control the operation of the call
   * @param mode - specifies the permissions to be placed on the new queue
   * @return 0 in successful case, 1 - if client is not connected,
   * 2 - in case of exception
   */
  int open_with_params(const std::string& name,
                       const int max_messages_number,
                       const int max_message_size,
                       const int flags,
                       const int mode);

  /**
   * @brief Sends data to mqueue opened by server
   * @param name mqueue name for data reading
   * @param data - data to be send to mqueue
   * @return 0 in successful case, 1 - if client is not connected,
   * 2 - in case of exception
   */
  int send(const std::string& name, const std::string& data);

  /**
   * @brief Reads data from mqueue opened by server
   * @param name mqueue name from which data should be received
   * @return received data in successful case,
   * otherwise empty string
   */
  std::pair<std::string, int> receive(const std::string& name);

  //  /**
  //   * @brief Reads data from mqueue opened by server
  //   * @param name mqueue name from which data should be received
  //   * @return received data in successful case,
  //   * otherwise empty string
  //   */
  //  std::string receive(const std::string& name);

  /**
   * @brief Clears the system from mqueues opened by server
   * @return 0 in successful case,  - if client is not connected,
   * 2 - in case of exception
   */
  int clear();

 private:
  rpc::client connection_;
  friend struct SDLRemoteTestAdapterLuaWrapper;
};

}  // namespace lua_lib
