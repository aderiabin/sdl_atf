#pragma once

#include <fcntl.h>    /* For O_* constants */
#include <sys/stat.h> /* For mode constants */
#include <mqueue.h>
#include <errno.h>

#include <map>
#include <string>
#include <tuple>

namespace mq_wrappers {

class MQueueManager {
 public:
  typedef std::tuple<std::string, int> ReceiveResult;
  MQueueManager();
  std::map<std::string, mqd_t> handles_;

  /**
   * @brief MqOpen creates mqueue
   * @param path path to create mqueue
   * @return error no (0 if success)
   */
  int MqOpen(const std::string& path);

  /**
   * @brief MqSend sends data to mqueue
   * @param path path to mqueue to write in
   * @param data data to be written to mqueue
   * @return error no (0 if success)
   */
  int MqSend(const std::__cxx11::string& path,
             const std::__cxx11::string& data);

  /**
   * @brief MqReceive reads data from mqueue
   * @param path path to mqueue to read from
   * @return tuple with read data and errno
   * in case of successful reading, otherwise tuple
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
  class Defaults {
   public:
    static const int MSGQ_MAX_MESSAGES = 10;
    static const int MAX_QUEUE_MSG_SIZE = 4096;
    static const int flags = O_RDWR | O_CREAT;
    static const int mode = S_IRUSR | S_IWUSR;
    static const int prio = 0;
  };
};

}  // namespace mq_wrappers

