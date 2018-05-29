#include "mqueue_manager.h"

#include <errno.h>
#include <cstring>

#include <iostream>

namespace mq_wrappers {

MQueueManager::MQueueManager() {}

int MQueueManager::MqOpen(std::string path) {
  std::cout << "MqOpen : " << path << std::endl;
  struct mq_attr attributes;
  attributes.mq_maxmsg = Defaults::MSGQ_MAX_MESSAGES;
  attributes.mq_msgsize = Defaults::MSGQ_MAX_MESSAGES;
  attributes.mq_flags = 0;
  errno = 0;
  auto mq_desc =
      mq_open(path.c_str(), Defaults::flags, Defaults::mode, &attributes);
  if (0 == errno) {
    handles_[path] = mq_desc;
  }
  return errno;
}

int MQueueManager::MqSend(std::string path, std::string data) {
  std::cout << "MqSend : " << path << " , " << data << std::endl;
  if (handles_.find(path) != handles_.end()) {
    errno = 0;
    const int res =
        mq_send(handles_[path], data.c_str(), data.size(), Defaults::prio);
    std::cout << "res   = " << res << std::endl;
    std::cout << "err = " << strerror(errno) << std::endl;
    return 0 == res ? 0 : errno;
  }
  return -1;
}

MQueueManager::ReceiveResult MQueueManager::MqReceive(std::string path) {
  std::cout << "MqReceive " << path << std::endl;
  if (handles_.find(path) != handles_.end()) {
    std::cout << "Handle found : " << handles_[path] << " " << std::endl;

    char buffer[Defaults::MAX_QUEUE_MSG_SIZE];
    ssize_t length = mq_receive(handles_[path], buffer, sizeof(buffer), 0);
    std::cout << "length  = " << length << std::endl;
    if (length == -1) {
      return std::make_tuple(std::string(), errno);
    }
    return std::make_tuple(std::string(buffer, length), 0);
  }
  return std::make_tuple(std::string(), -1);
}

int MQueueManager::MqClose(std::string path) {
  std::cout << "MqClose " << path << std::endl;
  if (handles_.find(path) != handles_.end()) {
    MqCloseForce(path);
    return 0;
  }
  return -1;
}

int MQueueManager::MqCloseForce(std::string path) {
  std::cout << "MqCloseForce " << path << std::endl;
  if (-1 == mq_close(handles_[path])) {
    return errno;
  }
  if (-1 == mq_unlink(path.c_str())) {
    return errno;
  }
  return 0;
}

int MQueueManager::MqClear() {
    std::cout << "MqClear" << std::endl;
    bool success = true;
    for (const auto& pair : handles_ ) {
        success &= MqCloseForce(pair.first);
    }
    return success = true ? 0 : 1;
}

}  // namespace mq_wrappers
