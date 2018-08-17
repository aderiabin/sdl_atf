#include "mqueue_manager.h"

#include <errno.h>
#include <cstring>

#include <iostream>
#include "common/constants.h"
#include <memory>

#define INVALID_DESCRIPT -1

namespace mq_wrappers {

MQueueManager::MQueueManager() {}

int MQueueManager::MqOpen(const std::string& path) {
  std::cout << "\nMqOpen : " << path << std::endl;
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
  std::cout << "\nMqOpenWithParams : " << path << std::endl;
  struct mq_attr attributes;
  attributes.mq_maxmsg = max_messages_number;
  attributes.mq_msgsize = max_message_size;
  // mq_flags is ignored for mq_open
  attributes.mq_flags = 0;
  errno = 0;
  
  if(handles_.find(path) != handles_.end()){
    printf("\nMq channel already exists: %s\n",path.c_str());
    return constants::error_codes::SUCCESS;
  }
  
  const auto mq_descriptor = mq_open(path.c_str(), flags, mode, &attributes);
  if (0 == errno) {
    if(!(O_WRONLY & flags)){
      MqClearMsg(mq_descriptor);
    }
    handles_[path] = mq_descriptor;
    std::cout << "\nReturning successful result" << std::endl;
    return constants::error_codes::SUCCESS;
  }

  switch(errno){
    case EACCES:
      printf("\nThe message queue exists, and you don't have permission to"
             "open the queue under the given oflag, or the message queue"
             "doesn't exist, and you don't have permission to create one\n");
      break;
    case EEXIST:
      printf("\nYou specified the O_CREAT and O_EXCL flags in oflag, and the queue name exists.\n");
      break;
    case EINTR:
      printf("\nThe operation was interrupted by a signal.\n");
      break;
    case EINVAL:
      printf("\nYou specified the O_CREAT flag in oflag,"
             "and mq_attr wasn't NULL, but some values in the mq_attr structure were invalid.\n"
             );
      break;
    case ELOOP:
      printf("\nToo many levels of symbolic links or prefixes.\n");
      case EMFILE:
      printf("\nToo many message queue descriptors or file descriptors are in use by the calling process.\n");
      break;
    case  ENAMETOOLONG:
      printf("\nThe length of name exceeds PATH_MAX.\n");
      break;
    case ENFILE:
      printf("\nToo many message queues are open in the system.\n");
      break;
    case ENOENT:
      printf("\nYou didn't set the O_CREAT flag, and the queue name doesn't exist.\n");
      break;
    case ENOSPC:
      printf("\nThe message queue server has run out of memory.\n");
      break;
    case EPERM:
      printf("\nThe process doesn't have the necessary MAC permissions to connect.\n");
      break;
    case ENOSYS:
      printf("\nThe mq_open() function isn't implemented for the filesystem specified in name, or the message queue manager (mq or mqueue) isn't running.\n");
      break;
    default:
      printf("\nUnknown error in errno\n");
  }

  return errno;
}

int MQueueManager::MqSend(const std::string& path,std::string data) {
  std::cout << "\nMqSend to : " << path <<" Size: "<<data.length()<<" : " << data << std::endl;

  if(data.length() >= Defaults::MAX_QUEUE_MSG_SIZE){   
      shm_manager.ShmWrite(shm_manager.shm_name_sdlqueue,data);
      data = shm_manager.shm_json_mem_ident;    
  }

  char queue_msg[Defaults::MAX_QUEUE_MSG_SIZE - 1] = {0};
  memcpy(queue_msg,data.c_str(),data.size());
  
  if (handles_.find(path) != handles_.end()) {
    std::cout << "\nHandle found : " << handles_[path] << " " << std::endl;
    errno = 0;
    const int res =
        mq_send(handles_[path],queue_msg, Defaults::MAX_QUEUE_MSG_SIZE - 1, Defaults::prio);
    if (-1 == res) {
      std::cout << "\nError occurred: " << strerror(errno) << std::endl;
      return errno;
    }
    std::cout << "\nReturning successful result" << std::endl;
    return constants::error_codes::SUCCESS;
  }
  std::cerr << "\nMqueue path : '" << path << "' NOT found\n";
  return constants::error_codes::PATH_NOT_FOUND;
}

MQueueManager::ReceiveResult MQueueManager::MqReceive(const std::string& path) {
  std::cout << "\nMqReceive from " << path << std::endl;
  if (handles_.find(path) != handles_.end()) {
    std::cout << "\Handle found : " << handles_[path] << " " << std::endl;
    char buffer[Defaults::MAX_QUEUE_MSG_SIZE];
    errno = 0;
    const ssize_t length =
        mq_receive(handles_[path], buffer, sizeof(buffer), 0);
    std::cout << "\nLength of read data = " << length << std::endl;
    if(0 < length){
      std::cout << "\nRead data: " << buffer << std::endl;
    }
    if (-1 == length) {
      switch(errno){
        case EAGAIN:
          printf("\nThe O_NONBLOCK flag was set and there are no messages currently on the specified queue.\n");
          break; 
        case EBADF:
          printf("\nThe mqdes argument doesn't represent a valid queue open for reading.\n");
          break;
        case EINTR:
          printf("\nThe operation was interrupted by a signal.\n");
          break;
        case EINVAL:
          printf("\nThe msg_ptr argument isn't a valid pointer,"
                 "\nor msg_len is less than 0, or msg_len is less"
                 "\nthan the message size specified in mq_open()."
                 "\nThe default message size is 4096 bytes for the traditional (mqueue) implementation,"
                 "\nand 256 bytes for the alternate (mq) implementation.\n"); 
          break;
        case EMSGSIZE:
          printf("\nThe given msg_len is shorter than the mq_msgsize for the given queue or the given msg_len\n"
                 "is too short for the message that would have been received.\n");
          break;
        default:
          printf("\nUnknown error in errno\n");

      }
      return std::make_pair(std::string(), errno);
    }

    std::string shm_name;
    if(shm_manager.IsShmName(buffer,shm_name)){
      return shm_manager.ShmRead(shm_name);
    }

    std::cout << "\nReturning successful result" << std::endl;
    return std::make_pair(std::string(buffer, length),
                          constants::error_codes::SUCCESS);
  }
  std::cerr << "\nMqueue path : '" << path << "' NOT found\n";
  return std::make_pair(std::string(), constants::error_codes::PATH_NOT_FOUND);
}

int MQueueManager::MqClose(const std::string& path) {
  std::cout << "\nMqClose " << path << std::endl;
  if (handles_.find(path) != handles_.end()) {
    std::cout << "\nHandle found : " << handles_[path] << " " << std::endl;
    errno = 0;
    const int res = mq_close(handles_[path]);
    if (-1 == res) {
      std::cout << "\nError occurred: " << strerror(errno) << std::endl;
      return errno;
    }
    handles_.erase(path);
    std::cout << "\nReturning successful result" << std::endl;
    return constants::error_codes::SUCCESS;
  }
  std::cerr << "\nMqueue path : '" << path << "' NOT found\n";
  return constants::error_codes::PATH_NOT_FOUND;
}

int MQueueManager::MqUnlink(const std::string& path) {
  std::cout << "\nMqUnlink " << path << std::endl;
  errno = 0;
  const int res = mq_unlink(path.c_str());
  if (-1 == res) {
    std::cout << "\nError occurred: " << strerror(errno) << std::endl;
    return errno;
  }
  std::cout << "\nReturning successful result" << std::endl;
  return constants::error_codes::SUCCESS;
}

int MQueueManager::MqClear() {
  std::cout << "\nMqClear" << std::endl;
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
  printf("\nMQueueManager::MqClearMsg\n");
  if(INVALID_DESCRIPT == mq_descriptor){
    return;
  }

  struct mq_attr attr,old_attr;
  unsigned int priority;
  if(0 != mq_getattr (mq_descriptor, &attr)){
    switch(errno){
      case EBADF:
        printf("\nThe message queue descriptor specified in mqdes is invalid\n");
        break;
      case EINVAL:
        printf("\nnewattr->mq_flags contained set bits other than O_NONBLOCK\n");
        default:
          printf("\nUnknown error in errno\n");
    }
    return;
  }
  printf("\n%d messages are currently on the queue.\n",(int)attr.mq_curmsgs);
  printf("\n%d size messages currently\n",(int)attr.mq_msgsize);    
  if (0 != attr.mq_curmsgs) {
    //There are some messages on this queue....eat them
    // First set the queue to not block any calls
    attr.mq_flags = O_NONBLOCK;
    mq_setattr(mq_descriptor,&attr,&old_attr);

    // Now eat all of the messages
    std::unique_ptr<char[]> buf(new char[attr.mq_msgsize]);
    while (mq_receive (mq_descriptor, buf.get(),attr.mq_msgsize, &priority) != -1){
      printf ("\nReceived a message with priority %d.\n", priority);
    }
    // The call failed. Make sure errno is EAGAIN
    if(EAGAIN != errno){
      perror ("\nmq_receive()\n");
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
