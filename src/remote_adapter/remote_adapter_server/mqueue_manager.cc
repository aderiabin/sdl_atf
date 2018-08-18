#include "mqueue_manager.h"

#include <errno.h>
#include <cstring>

#include <iostream>
#include <memory>

#include "common/constants.h"
#include "rpc/detail/log.h"

#define INVALID_DESCRIPT -1

namespace mq_wrappers {

RPCLIB_CREATE_LOG_CHANNEL(MQueueManager)

MQueueManager::MQueueManager() {}

int MQueueManager::MqOpen(const std::string& path) {
  LOG_INFO("{0}: {1}",__func__,path);
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
  LOG_INFO("{0}: {1}",__func__,path);
  struct mq_attr attributes;
  attributes.mq_maxmsg = max_messages_number;
  attributes.mq_msgsize = max_message_size;
  // mq_flags is ignored for mq_open
  attributes.mq_flags = 0;
  errno = 0;
  
  if(handles_.find(path) != handles_.end()){
    LOG_INFO("Mq channel already exists: {}",path);
    MqClose(path);
    MqUnlink(path);
  }
  
  const auto mq_descriptor = mq_open(path.c_str(), flags, mode, &attributes);
  if (0 == errno) {
    if(!(O_WRONLY & flags)){
      MqClearMsg(mq_descriptor);
    }
    handles_[path] = mq_descriptor;
    LOG_INFO("Returning successful result");
    return constants::error_codes::SUCCESS;
  }

  switch(errno){
    case EACCES:
      LOG_ERROR("The message queue exists, and you don't have permission to"
                "open the queue under the given oflag, or the message queue"
                "doesn't exist, and you don't have permission to create one");
      break;
    case EEXIST:
      LOG_ERROR("You specified the O_CREAT" 
                "and O_EXCL flags in oflag, and the queue name exists");
      break;
    case EINTR:
      LOG_ERROR("The operation was interrupted by a signal");
      break;
    case EINVAL:
      LOG_ERROR("You specified the O_CREAT flag in oflag,"
                "and mq_attr wasn't NULL, but some values"
                "in the mq_attr structure were invalid.");
      break;
    case ELOOP:
      LOG_ERROR("Too many levels of symbolic links or prefixes.");
      case EMFILE:
      LOG_ERROR("Too many message queue descriptors or file"
                "descriptors are in use by the calling process.");
      break;
    case  ENAMETOOLONG:
      LOG_ERROR("The length of name exceeds PATH_MAX.");
      break;
    case ENFILE:
      LOG_ERROR("Too many message queues are open in the system.");
      break;
    case ENOENT:
      LOG_ERROR("You didn't set the O_CREAT flag,"
                "and the queue name doesn't exist.");
      break;
    case ENOSPC:
      LOG_ERROR("The message queue server has run out of memory.");
      break;
    case EPERM:
      LOG_ERROR("The process doesn't have the necessary MAC"
                "permissions to connect.");
      break;
    case ENOSYS:
      LOG_ERROR("The mq_open() function isn't implemented"
                "for the filesystem specified in name, or"
                "the message queue manager (mq or mqueue) isn't running.");
      break;
    default:
      LOG_ERROR("Unknown error in errno");
  }

  return errno;
}

int MQueueManager::MqSend(const std::string& path,std::string data) {
  LOG_INFO("{0}: {1} Size: {2} Data: {3}",__func__, path,data.length(),data);

  if(data.length() >= Defaults::MAX_QUEUE_MSG_SIZE){   
      shm_manager.ShmWrite(shm_manager.shm_name_sdlqueue,data);
      data = shm_manager.shm_json_mem_ident;    
  }

  char queue_msg[Defaults::MAX_QUEUE_MSG_SIZE - 1] = {0};
  memcpy(queue_msg,data.c_str(),data.size());
  
  if(handles_.find(path) != handles_.end()) {
    LOG_INFO("Handle found: {}",handles_[path]);
    errno = 0;
    const int res =
        mq_send(handles_[path],queue_msg, Defaults::MAX_QUEUE_MSG_SIZE - 1, Defaults::prio);
    if (-1 == res) {
      LOG_ERROR("Occurred: {}",strerror(errno));
      return errno;
    }
    LOG_INFO("Returning successful result");
    return constants::error_codes::SUCCESS;
  }
  LOG_ERROR("Mqueue path: {} Not Found",path);
  return constants::error_codes::PATH_NOT_FOUND;
}

MQueueManager::ReceiveResult MQueueManager::MqReceive(const std::string& path) {
  LOG_INFO("{0}: from: {1}",__func__,path);
  if (handles_.find(path) != handles_.end()) {
    LOG_INFO("Handle found: {}",handles_[path]);
    char buffer[Defaults::MAX_QUEUE_MSG_SIZE];
    errno = 0;
    const ssize_t length =
        mq_receive(handles_[path], buffer, sizeof(buffer), 0);
    LOG_INFO("Length of read data: {}",length);
    if(0 < length){
      LOG_INFO("Read data: {}",buffer);
    }
    if (-1 == length) {
      switch(errno){
        case EAGAIN:
          LOG_ERROR("The O_NONBLOCK flag was set and there are"
                    "no messages currently on the specified queue.");
          break; 
        case EBADF:
          LOG_ERROR("The mqdes argument doesn't represent"
                    "a valid queue open for reading.");
          break;
        case EINTR:
          LOG_ERROR("The operation was interrupted by a signal.");
          break;
        case EINVAL:
          LOG_ERROR("The msg_ptr argument isn't a valid pointer,"
                 "\nor msg_len is less than 0, or msg_len is less"
                 "\nthan the message size specified in mq_open()."
                 "\nThe default message size is 4096 bytes for the traditional (mqueue) implementation,"
                 "\nand 256 bytes for the alternate (mq) implementation."); 
          break;
        case EMSGSIZE:
          LOG_ERROR("The given msg_len is shorter than"
                    "\nthe mq_msgsize for the given queue or the given msg_len"
                    "\nis too short for the message that would have been received.");
          break;
        default:
          LOG_INFO("Unknown error in errno");

      }
      return std::make_pair(std::string(), errno);
    }

    std::string shm_name;
    if(shm_manager.IsShmName(buffer,shm_name)){
      return shm_manager.ShmRead(shm_name);
    }

    LOG_INFO("Returning successful result");
    return std::make_pair(std::string(buffer, length),
                          constants::error_codes::SUCCESS);
  }
  LOG_ERROR("Mqueue path: {} NOT found",path);
  return std::make_pair(std::string(), constants::error_codes::PATH_NOT_FOUND);
}

int MQueueManager::MqClose(const std::string& path) {
  LOG_INFO("{0}: {1}",__func__,path);
  if (handles_.find(path) != handles_.end()) {
    LOG_INFO("Handle found: {}",handles_[path]);
    errno = 0;
    const int res = mq_close(handles_[path]);
    if (-1 == res) {
      LOG_ERROR("Error occurred: {}",strerror(errno));
      return errno;
    }
    handles_.erase(path);
    LOG_INFO("Returning successful result");
    return constants::error_codes::SUCCESS;
  }
  LOG_ERROR("Mqueue path: {} NOT found",path);
  return constants::error_codes::PATH_NOT_FOUND;
}

int MQueueManager::MqUnlink(const std::string& path) {
  LOG_INFO("{0}: {}",__func__,path);
  errno = 0;
  const int res = mq_unlink(path.c_str());
  if (-1 == res) {
    LOG_ERROR("Error occurred: {}",strerror(errno));
    return errno;
  }
  LOG_INFO("Returning successful result");
  return constants::error_codes::SUCCESS;
}

int MQueueManager::MqClear() {
  LOG_INFO("{}",__func__);
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

void MQueueManager::MqClearMsg(const mqd_t mq_descriptor){ 
  LOG_INFO("{}",__func__);
  if(INVALID_DESCRIPT == mq_descriptor){
    return;
  }

  struct mq_attr attr,old_attr;
  unsigned int priority;
  if(0 != mq_getattr (mq_descriptor, &attr)){
    switch(errno){
      case EBADF:
        LOG_ERROR("The message queue descriptor specified in mqdes is invalid");
        break;
      case EINVAL:
        LOG_ERROR("nnewattr->mq_flags contained set bits other than O_NONBLOCK");
      default:
        LOG_ERROR("Unknown error in errno");
    }
    return;
  }
  LOG_INFO("{} messages are currently on the queue.",(int)attr.mq_curmsgs);
  LOG_INFO("{} size messages currently",(int)attr.mq_msgsize);    
  if (0 != attr.mq_curmsgs) {
    //There are some messages on this queue....eat them
    // First set the queue to not block any calls
    attr.mq_flags = O_NONBLOCK;
    mq_setattr(mq_descriptor,&attr,&old_attr);

    // Now eat all of the messages
    std::unique_ptr<char[]> buf(new char[attr.mq_msgsize]);
    while (mq_receive (mq_descriptor, buf.get(),attr.mq_msgsize, &priority) != -1){
      LOG_INFO("Received a message {0} with priority {1}.\n",buf.get(), priority);
    }
    // The call failed. Make sure errno is EAGAIN
    if(EAGAIN != errno){
      LOG_ERROR("mq_receive()");
    }
    mq_setattr (mq_descriptor, &old_attr, 0);
  }
}

int MQueueManager::ShmOpen(const std::string& shm_name,const int prot){

    return shm_manager.ShmOpen(shm_name,prot);  
}

int MQueueManager::ShmClose(const std::string& shm_name){

    return shm_manager.ShmClose(shm_name);  
}

}  // namespace mq_wrappers
