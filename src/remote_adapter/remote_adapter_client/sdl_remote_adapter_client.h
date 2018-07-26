#pragma once

#include <string>
#include <utility>

#include "rpc/client.h"
#include "rpc/rpc_error.h"

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

   /**
    * @brief Close mqueue opened by server
    * @param name mqueue name to be closed
    * @return 0 in successful case, 1 - if client is not connected,
    * 2 - in case of exception
    */
  int close(const std::string& name);

    /**
    * @brief Unlink mqueue opened by server
    * @param name mqueue name to be unlinked
    * @return 0 in successful case, 1 - if client is not connected,
    * 2 - in case of exception
    */
  int unlink(const std::string& name);

  /**
   * @brief Clears the system from mqueues opened by server
   * @return 0 in successful case,  - if client is not connected,
   * 2 - in case of exception
   */
  int clear();

  /**
   * @brief Sends open shared memory request to server
   * @param name shared memory name which should be opened by server
   * @param prot shared memory prot which should be opened by server
   * @return 0 in successful case, 1 - if client is not connected,
   * 2 - in case of exception
   */
  int shm_open(const std::string& name, const int prot);

  /**
   * @brief Sends close shared memory request to server
   * @param name shared memory name which should be closed by server
     * @return 0 in successful case, 1 - if client is not connected,
   * 2 - in case of exception
   */
  int shm_close(const std::string& name);

  /**
   * @brief Sends start application request to server
   * @param path Path to application
   * @param name Name of application
   * @return 0 in successful case, 1 - if client is not connected,
   * 2 - in case of exception
   */
  int app_start(const std::string& path, const std::string& name);

  /**
   * @brief Sends stop application request to server
   * @param name Name of application
   * @return 0 in successful case, 1 - if client is not connected,
   * 2 - in case of exception
   */
  int app_stop(const std::string& name);

  /**
   * @brief Sends check application status request to server
   * @param name Name of application
   * @return 0 in successful case, 1 - if client is not connected,
   * 2 - in case of exception
   */
  int app_check_status(const std::string& name);

  /**
   * @brief Sends check file existing request to server
   * @param path Path to file
   * @param name Name of file
   * @return 0 in successful case, 1 - if client is not connected,
   * 2 - in case of exception
   */
  int file_exists(const std::string& path, const std::string& name);

  /**
   * @brief Sends update file content request to server
   * @param path Path to file
   * @param name Name of file
   * @param content Content of file
   * @return 0 in successful case, 1 - if client is not connected,
   * 2 - in case of exception
   */
  int file_update(const std::string& path,
                  const std::string& name,
                  const std::string& content);

  /**
   * @brief Sends get file content request to server
   * @param path Path to file
   * @param name Name of file
   * @return 0 in successful case, 1 - if client is not connected,
   * 2 - in case of exception
   */
  std::pair<std::string, int> file_content(const std::string& path,
                                           const std::string& name);

  /**
   * @brief Sends delete file request to server
   * @param path Path to file
   * @param name Name of file
   * @return 0 in successful case, 1 - if client is not connected,
   * 2 - in case of exception
   */
  int file_delete(const std::string& path, const std::string& name);

  /**
   * @brief Sends backup file request to server
   * @param path Path to file
   * @param name Name of file
   * @return 0 in successful case, 1 - if client is not connected,
   * 2 - in case of exception
   */
  int file_backup(const std::string& path, const std::string& name);

  /**
   * @brief Sends restore backuped file request to server
   * @param path Path to file
   * @param name Name of file
   * @return 0 in successful case, 1 - if client is not connected,
   * 2 - in case of exception
   */
  int file_restore(const std::string& path, const std::string& name);

  /**
   * @brief Check existance of folder request to server
   * @param path Path to file
   * @param name Name of file
   * @return 0 in successful case, 1 - if client is not connected,
   * 2 - in case of exception
   */
  int folder_exists(const std::string& path, const std::string& name);

  /**
   * @brief Sends create folder request to server
   * @param path Path to file
   * @param name Name of file
   * @return 0 in successful case, 1 - if client is not connected,
   * 2 - in case of exception
   */
  int folder_create(const std::string& path, const std::string& name);

  /**
   * @brief Sends delete folder request to server
   * @param path Path to file
   * @param name Name of file
   * @return 0 in successful case, 1 - if client is not connected,
   * 2 - in case of exception
   */
  int folder_delete(const std::string& path, const std::string& name);

 private:
  int handleRpcError(rpc::rpc_error& e);
  int handleRpcTimeout(rpc::timeout& t);

  rpc::client connection_;
  friend struct SDLRemoteTestAdapterLuaWrapper;
};

}  // namespace lua_lib
