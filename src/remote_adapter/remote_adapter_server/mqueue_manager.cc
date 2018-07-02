#include "mqueue_manager.h"

#include <errno.h>
#include <cstring>

#include <iostream>
#include "common/constants.h"

namespace mq_wrappers {

MQueueManager::MQueueManager() {}

int MQueueManager::MqOpen(const std::string& path) {
  std::cout << "MqOpen : " << path << std::endl;
  return MqOpenWithParams(path,
                   Defaults::MSGQ_MAX_MESSAGES,
                   Defaults::MAX_QUEUE_MSG_SIZE,
                   Defaults::flags,
                   Defaults::mode);
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
  if (0 == errno) {
    handles_[path] = mq_descriptor;
    std::cout << "Returning successful result" << std::endl;
    return constants::error_codes::SUCCESS;
  }
  std::cout << "Error occurred: " << strerror(errno) << std::endl;
  return errno;
}

int MQueueManager::MqSend(const std::string& path, const std::string& data) {
  std::cout << "MqSend to : " << path << " : " << data << std::endl;
  if (handles_.find(path) != handles_.end()) {
    std::cout << "Handle found : " << handles_[path] << " " << std::endl;
    errno = 0;
    const int res =
        mq_send(handles_[path], data.c_str(), data.size(), Defaults::prio);
    if (-1 == res) {
      std::cout << "Error occurred: " << strerror(errno) << std::endl;
      return errno;
    }
    std::cout << "Returning successful result" << std::endl;
    return constants::error_codes::SUCCESS;
  }
  std::cerr << "Mqueue path : '" << path << "' NOT found";
  return constants::error_codes::PATH_NOT_FOUND;
}

MQueueManager::ReceiveResult MQueueManager::MqReceive(const std::string& path) {
  std::cout << "MqReceive from " << path << std::endl;
  if (handles_.find(path) != handles_.end()) {
    std::cout << "Handle found : " << handles_[path] << " " << std::endl;

    char buffer[Defaults::MAX_QUEUE_MSG_SIZE];
    errno = 0;
    const ssize_t length =
        mq_receive(handles_[path], buffer, sizeof(buffer), 0);
    std::cout << "Length of read data = " << length << std::endl;
    if (-1 == length) {
      std::cout << "Error occurred: " << strerror(errno) << std::endl;
      return std::make_pair(std::string(), errno);
    }
    std::cout << "Returning successful result" << std::endl;
    return std::make_pair(std::string(buffer, length),
                          constants::error_codes::SUCCESS);
  }
  std::cerr << "Mqueue path : '" << path << "' NOT found";
  return std::make_pair(std::string(), constants::error_codes::PATH_NOT_FOUND);
}

int MQueueManager::MqClose(const std::string& path) {
  std::cout << "MqClose " << path << std::endl;
  if (handles_.find(path) != handles_.end()) {
    std::cout << "Handle found : " << handles_[path] << " " << std::endl;
    errno = 0;
    const int res = mq_close(handles_[path]);
    if (-1 == res) {
      std::cout << "Error occurred: " << strerror(errno) << std::endl;
      return errno;
    }
    handles_.erase(path);
    std::cout << "Returning successful result" << std::endl;
    return constants::error_codes::SUCCESS;
  }
  std::cerr << "Mqueue path : '" << path << "' NOT found";
  return constants::error_codes::PATH_NOT_FOUND;
}

int MQueueManager::MqUnlink(const std::string& path) {
  std::cout << "MqUnlink " << path << std::endl;
  errno = 0;
  const int res = mq_unlink(path.c_str());
  if (-1 == res) {
    std::cout << "Error occurred: " << strerror(errno) << std::endl;
    return errno;
  }
  std::cout << "Returning successful result" << std::endl;
  return constants::error_codes::SUCCESS;
}

int MQueueManager::MqClear() {
  std::cout << "MqClear" << std::endl;
  bool success = true;
  bool result = true;
  for (const auto& pair : handles_) {
    if (MqClose(pair.first) != constants::error_codes::SUCCESS
        || MqUnlink(pair.first) != constants::error_codes::SUCCESS) {
      result = false;
    }
    result &= success;
  }
  return result ? constants::error_codes::SUCCESS
                : constants::error_codes::CLOSE_FAILURE;
}

}  // namespace mq_wrappers
