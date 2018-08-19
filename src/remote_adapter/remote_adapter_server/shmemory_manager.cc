#include "shmemory_manager.h"
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <iostream>
#include <cstring>
#include "common/constants.h"
#include "rpc/detail/log.h"

#define INVALID_DESCRIPT -1

namespace shmemory_wrappers {

const char * SharedMemoryManager::shm_name_sdlqueue = "/SHNAME_SDLQUEUE";
const char * SharedMemoryManager::shm_name_sdlqueue2 = "/SHNAME_SDLQUEUE2";
const char * SharedMemoryManager::shm_name_sdlqueue3 = "/SHNAME_SDLQUEUE3";
const char SharedMemoryManager::shm_json_mem_ident[] = {
  //SHARED_MEMORY
  18,83,72,65,82,69,68,95,77,69,77,79,82,89,0
  };

RPCLIB_CREATE_LOG_CHANNEL(SharedMemoryManager)

SharedMemoryManager::SharedMemoryManager() {}

SharedMemoryManager::~SharedMemoryManager(){
  
  LOG_INFO("{0}",__func__);
  
  for(const auto & mem_obj : handles_){
       ftruncate(mem_obj.second.object_handle_, 0);
       munmap(mem_obj.second.object_data_, sizeof(shmem_t));
       close(mem_obj.second.object_handle_);   
  }
}

int SharedMemoryManager::ShmOpen(const std::string& shm_name,const int prot) {  
  
  LOG_INFO("{0}: {1}",__func__,shm_name); 
  
  if(handles_.find(shm_name) != handles_.end()){
    LOG_WARN("SHM Channel already exists: {}",shm_name);
    ShmClose(shm_name);
    ShmUnlink(shm_name);
  }

  object_descrp & mem_obj = handles_[shm_name];
  mem_obj.object_handle_ = shm_open(shm_name.c_str(), O_RDWR | O_CREAT, 0777);
  
  if(INVALID_DESCRIPT == mem_obj.object_handle_) {
      handles_.erase(shm_name);
      LOG_TRACE("Open failed: {}",strerror( errno ));
      return constants::error_codes::OPEN_FAILURE;
  }
  
  ftruncate(mem_obj.object_handle_, sizeof(shmem_t));
  // Get a pointer to the shared memory, map it into our address space
  mem_obj.object_data_ = (shmem_t*)mmap(0, sizeof(shmem_t), prot, MAP_SHARED,mem_obj.object_handle_, 0);

  if(MAP_FAILED == mem_obj.object_data_){
      LOG_TRACE("mmap failed: {}",strerror( errno ));
      ShmClose(shm_name);
      return constants::error_codes::OPEN_FAILURE;     
  }

  LOG_INFO("Map  mem_obj.object_data_ is: 0x%08x {}",(long)mem_obj.object_data_);
  
  return constants::error_codes::SUCCESS;
}

int SharedMemoryManager::ShmClose(const std::string& shm_name) {
  
  LOG_INFO("{0}: {1}",__func__,shm_name);
  
  if (handles_.find(shm_name) != handles_.end()) {
    object_descrp & mem_obj = handles_[shm_name];

    LOG_INFO("Handle found:{}",(long)mem_obj.object_handle_);

    ftruncate(mem_obj.object_handle_, 0);
    munmap(mem_obj.object_data_, sizeof(shmem_t));

    errno = 0;
    const int res = close(mem_obj.object_handle_);
    if (INVALID_DESCRIPT == res) {
      LOG_TRACE("Error occurred: {}",strerror(errno));
      return errno;
    }
    
    handles_.erase(shm_name);
    
    LOG_INFO("Returning successful result");
    
    return constants::error_codes::SUCCESS;
  }
  
  LOG_TRACE("Shared memory name: {} Not found",shm_name);
  
  return constants::error_codes::PATH_NOT_FOUND;
}

int SharedMemoryManager::ShmWrite(const std::string& shm_name, const std::string& data) {
  
  LOG_INFO("{0}: {1}",__func__,shm_name);
  
  if(handles_.find(shm_name) != handles_.end()){
      object_descrp & mem_obj = handles_[shm_name];
      LOG_INFO("Handle found:{}",mem_obj.object_handle_);
      LOG_INFO("Map  mem_obj.object_data_ is 0x%08x {}",(long)mem_obj.object_data_);
      
      mem_obj.object_data_->size_ = data.size() - 1;
      memset(mem_obj.object_data_->text_, 0, mem_obj.object_data_->size_);
      memcpy(mem_obj.object_data_->text_, &data[1], mem_obj.object_data_->size_);
      
      return constants::error_codes::SUCCESS;
  }
  
  LOG_TRACE("Shared memory name: {} Not found",shm_name);
  
  return constants::error_codes::PATH_NOT_FOUND;
}

SharedMemoryManager::ReceiveResult SharedMemoryManager::ShmRead(const std::string& shm_name) {

  LOG_INFO("{0}: {1}",__func__,shm_name);
  
  if(handles_.find(shm_name) != handles_.end()) {
      object_descrp & mem_obj = handles_[shm_name];
      LOG_INFO("Handle found: {}",mem_obj.object_handle_);
      LOG_INFO("Map  mem_obj.object_data_ is 0x%08x {}",(long)mem_obj.object_data_);      
      LOG_INFO("Returning successful result");
      return std::make_pair(std::string(mem_obj.object_data_->text_,mem_obj.object_data_->size_),constants::error_codes::SUCCESS);
  }  
  
  LOG_TRACE("Shared memory name : {} Not found",shm_name);  
  
  return std::make_pair(std::string(), constants::error_codes::PATH_NOT_FOUND);
}

int SharedMemoryManager::ShmUnlink(const std::string& shm_name) {
  LOG_INFO("{0}: {1}",__func__,shm_name);
  errno = 0;
  const int res = shm_unlink(shm_name.c_str());
  if (INVALID_DESCRIPT == res) {
    LOG_TRACE("Error occurred: {}",strerror(errno));
    return errno;
  }
  LOG_INFO("Returning successful result");
  return constants::error_codes::SUCCESS;
}

bool SharedMemoryManager::IsShmName(const std::string& path,std::string& sh_name){
  
  if(0 == path.compare(constants::shm_1_applink)){
    sh_name = shm_name_sdlqueue2;
    return true;
  }else if(0 == path.compare(constants::shm_2_applink)){
    sh_name = shm_name_sdlqueue3;
    return true;    
  }

  return false;
}

}  // namespace shmemory_wrappers
