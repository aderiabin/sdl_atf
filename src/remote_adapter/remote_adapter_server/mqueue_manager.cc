#include "mqueue_manager.h"

#include <errno.h>
#include <cstring>

#include <iostream>
#include "common/constants.h"

namespace mq_wrappers {

MQueueManager::MQueueManager() {}

int MQueueManager::MqOpen(const std::string& path) {
  std::cout << "MqOpen : " << path << std::endl;
  struct mq_attr attributes;
  attributes.mq_maxmsg = Defaults::MSGQ_MAX_MESSAGES;
  attributes.mq_msgsize = Defaults::MAX_QUEUE_MSG_SIZE;
  // mq_flags is ignored for mq_open
  attributes.mq_flags = 0;
  errno = 0;
  const auto mq_descriptor =
      mq_open(path.c_str(), Defaults::flags, Defaults::mode, &attributes);
  if (0 == errno) {
    handles_[path] = mq_descriptor;
  }
  return errno;
}

int MQueueManager::MqOpenWithParams(const std::string& path,
                                    const int max_messages_number,
                                    const int max_message_size,
                                    const int flags,
                                    const int mode) {
  std::cout << "MqOpenWithParams : " << path << std::endl;
  struct mq_attr attributes;
  attributes.mq_maxmsg = max_messages_number;
  attributes.mq_msgsize = max_message_size;
  // mq_flags is ignored for mq_open
  attributes.mq_flags = 0;
  errno = 0;
  const auto mq_descriptor = mq_open(path.c_str(), flags, mode, &attributes);
  if (errno == 0) {
    handles_[path] = mq_descriptor;
  }
  return errno;
}

int MQueueManager::MqSend(const std::string& path, const std::string& data) {
  std::cout << "MqSend to : " << path << " : " << data << std::endl;
  if (handles_.find(path) != handles_.end()) {
    errno = 0;
    const int res =
        mq_send(handles_[path], data.c_str(), data.size(), Defaults::prio);
    std::cout << "res   = " << res << std::endl;
    std::cout << "Errno = " << strerror(errno) << std::endl;
    return 0 == res ? constants::error_codes::SUCCESS : errno;
  }
  std::cerr << "Mqueue path : '" << path << "' NOT found";
  return constants::error_codes::PATH_NOT_FOUND;
}

MQueueManager::ReceiveResult MQueueManager::MqReceive(const std::string& path) {
  std::cout << "MqReceive from " << path << std::endl;
  if (handles_.find(path) != handles_.end()) {
    std::cout << "Handle found : " << handles_[path] << " " << std::endl;

    char buffer[Defaults::MAX_QUEUE_MSG_SIZE];
    const ssize_t length =
        mq_receive(handles_[path], buffer, sizeof(buffer), 0);
    std::cout << "Length of read data = " << length << std::endl;
    if (-1 == length) {
      return std::make_pair(std::string(), errno);
    }
    std::cout << "Returning successful result" << std::endl;
    return std::make_pair(std::string(buffer, length),
                          constants::error_codes::SUCCESS);
  }
  return std::make_pair(std::string(), constants::error_codes::PATH_NOT_FOUND);
}

int MQueueManager::MqClose(const std::string& path) {
  std::cout << "MqClose " << path << std::endl;
  if (handles_.find(path) != handles_.end()) {
    if (MqUnlink(path) != constants::error_codes::SUCCESS) {
      return errno;
    }
    return constants::error_codes::SUCCESS;
  }
  return constants::error_codes::PATH_NOT_FOUND;
}

int MQueueManager::MqUnlink(const std::string& path) {
  std::cout << "MqUnlink " << path << std::endl;
  if (-1 == mq_unlink(path.c_str())) {
    return errno;
  }
  return constants::error_codes::SUCCESS;
}

int MQueueManager::MqClear() {
  std::cout << "MqClear" << std::endl;
  bool success = true;
  bool result = true;
  for (const auto& pair : handles_) {
    if (MqUnlink(pair.first) != constants::error_codes::SUCCESS) {
      result = false;
    }
    result &= success;
  }
  return result ? constants::error_codes::SUCCESS
                : constants::error_codes::CLOSE_FAILURE;
}

}  // namespace mq_wrappers
