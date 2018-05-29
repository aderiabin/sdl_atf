#ifndef MQUEUEMANAGER_H
#define MQUEUEMANAGER_H

#include <fcntl.h>    /* For O_* constants */
#include <sys/stat.h> /* For mode constants */
#include <mqueue.h>
#include <errno.h>

#include <map>
#include <string>

namespace mq_wrappers {

class MQueueManager {
 public:
  MQueueManager();
  static const unsigned SUCCESS = 0;
  std::map<std::string, mqd_t> handles_;

  /**
   * @brief MqOpen creates mqueue
   * @param path path to create mqueue
   * @return error no (0 if success)
   */
  int MqOpen(std::string path);

  int MqSend(std::string path, std::string data);

  typedef std::tuple<std::string, int> ReceiveResult;
  ReceiveResult MqReceive(std::string path);

  int MqClose(std::string path);

  int MqCloseForce(std::string path);


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

#endif  // MQUEUEMANAGER_H
