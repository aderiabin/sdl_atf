#pragma once

#include <fcntl.h>    /* For O_* constants */
#include <sys/stat.h> /* For mode constants */
#include <mqueue.h>
#include <errno.h>

#include <map>
#include <string>

#include "shmemory_manager.h"

namespace mq_wrappers {

class MQueueManager {
 public:
  typedef std::pair<std::string, int> ReceiveResult;
  MQueueManager();
  std::map<std::string, mqd_t> handles_;

  /**
   * @brief MqOpen creates mqueue
   * @param path path to create mqueue
   * @return error no (0 if success)
   */
  int MqOpen(const std::string& path);

  /**
   * @brief MqOpenWithParams creates mqueue with specifies parameters
   * @param path - path to create mqueue
   * @param max_messages_number -max messages number in mqueue
   * @param max_message_size - max message size in mqueue
   * @param flags - specifies flags that control the operation of the call
   * @param mode - specifies the permissions to be placed on the new queue
   * @return error no (0 if success)
   */
  int MqOpenWithParams(const std::string& path,
                       const int max_messages_number,
                       const int max_message_size,
                       const int flags,
                       const int mode);

  /**
   * @brief MqSend sends data to mqueue
   * @param path path to mqueue to write in
   * @param data data to be written to mqueue
   * @return error no (0 if success)
   */
  int MqSend(const std::string& path,std::string data);

  /**
   * @brief MqReceive reads data from mqueue
   * @param path path to mqueue to read from
   * @return tuple with read data and errno
   * in case of successful reading, otherwise pair
   * with an empty string and errno
   */
  ReceiveResult MqReceive(const std::string& path);

  /**
   * @brief MqClose closes mqueue specified by path
   * @param path path to mqueue to close
   * @return error no (0 if success)
   */
  int MqClose(const std::string& path);

  int MqUnlink(const std::string& path);

  int MqClear();

 private:
  
  shmemory_wrappers::SharedMemoryManager shm_manager;
  
  class Defaults {
   public:
    static const int MSGQ_MAX_MESSAGES = 10;
    static const int MAX_QUEUE_MSG_SIZE = 4096;
    static const int flags = O_RDWR | O_CREAT ;
    static const int mode = S_IRUSR | S_IWUSR;
    static const int prio = 0;
  };
};

}  // namespace mq_wrappers
