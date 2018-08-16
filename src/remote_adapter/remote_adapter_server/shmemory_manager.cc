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

namespace shmemory_wrappers {

const char * SharedMemoryManager::shm_name_sdlqueue = "/SHNAME_SDLQUEUE";
const char * SharedMemoryManager::shm_name_sdlqueue2 = "/SHNAME_SDLQUEUE2";
const char * SharedMemoryManager::shm_name_sdlqueue3 = "/SHNAME_SDLQUEUE3";
const char SharedMemoryManager::shm_json_mem_ident[] = {
  //SHARED_MEMORY
  18,83,72,65,82,69,68,95,77,69,77,79,82,89,0
  };

SharedMemoryManager::SharedMemoryManager() {}

SharedMemoryManager::~SharedMemoryManager(){
    
  for(const auto & mem_obj : handles_){
       ftruncate(mem_obj.second.object_handle_, 0);
       munmap(mem_obj.second.object_data_, sizeof(shmem_t));
       close(mem_obj.second.object_handle_);   
  }
}

int SharedMemoryManager::ShmOpen(const std::string& shm_name,const int prot) {
  std::cout << "ShmOpen : " << shm_name << std::endl;  

  if(handles_.find(shm_name) != handles_.end()){
    printf("\nSHM Channel already exists: %s\n",shm_name.c_str());
    return constants::error_codes::SUCCESS;
  }

  object_descrp & mem_obj = handles_[shm_name];
  mem_obj.object_handle_ = shm_open(shm_name.c_str(), O_RDWR | O_CREAT, 0777);
  
  if(-1 == mem_obj.object_handle_) {
      handles_.erase(shm_name);
      std::cout << "Open failed: " << strerror( errno ) << std::endl;
      return constants::error_codes::OPEN_FAILURE;
  }
  
  ftruncate(mem_obj.object_handle_, sizeof(shmem_t));
  // Get a pointer to the shared memory, map it into our address space
  mem_obj.object_data_ = (shmem_t*)mmap(0, sizeof(shmem_t), prot, MAP_SHARED,mem_obj.object_handle_, 0);

  if(MAP_FAILED == mem_obj.object_data_){
      std::cout <<"mmap failed: " << strerror( errno ) << std::endl;
      ShmClose(shm_name);
      return constants::error_codes::OPEN_FAILURE;     
  }

  printf( "Map  mem_obj.object_data_ is 0x%08x\n", mem_obj.object_data_);
  
  return constants::error_codes::SUCCESS;
}

int SharedMemoryManager::ShmClose(const std::string& shm_name) {
  std::cout << "ShmClose " << shm_name << std::endl;
  if (handles_.find(shm_name) != handles_.end()) {
    object_descrp & mem_obj = handles_[shm_name];

    std::cout << "Handle found : " << mem_obj.object_handle_ << " " << std::endl;

    ftruncate(mem_obj.object_handle_, 0);
    munmap(mem_obj.object_data_, sizeof(shmem_t));

    errno = 0;
    const int res = close(mem_obj.object_handle_);
    if (-1 == res) {
      std::cout << "Error occurred: " << strerror(errno) << std::endl;
      return errno;
    }
    handles_.erase(shm_name);
    std::cout << "Returning successful result" << std::endl;
    return constants::error_codes::SUCCESS;
  }
  std::cerr << "Shared memory name : '" << shm_name << "' NOT found";
  return constants::error_codes::PATH_NOT_FOUND;
}

int SharedMemoryManager::ShmWrite(const std::string& shm_name, const std::string& data) {
  std::cout << "ShmWrite to : " << shm_name << " : " << data << std::endl;
  
  if(handles_.find(shm_name) != handles_.end()){
      object_descrp & mem_obj = handles_[shm_name];
      std::cout << "Handle found : " << mem_obj.object_handle_ << " " << std::endl;
      printf( "Map  mem_obj.object_data_ is 0x%08x\n", mem_obj.object_data_);
      
      mem_obj.object_data_->size_ = data.length() - 1;
      memset(mem_obj.object_data_->text_, 0, mem_obj.object_data_->size_);
      memcpy(mem_obj.object_data_->text_, &data[1], mem_obj.object_data_->size_);
      
      return constants::error_codes::SUCCESS;
  }
  std::cerr << "Shared memory name : '" << shm_name << "' NOT found";
  return constants::error_codes::PATH_NOT_FOUND;
}

SharedMemoryManager::ReceiveResult SharedMemoryManager::ShmRead(const std::string& shm_name) {
  std::cout << "ShmRead from " << shm_name << std::endl;
  if(handles_.find(shm_name) != handles_.end()) {
      object_descrp & mem_obj = handles_[shm_name];
      std::cout << "Handle found : " << mem_obj.object_handle_ << " " << std::endl;
      printf( "Map  mem_obj.object_data_ is 0x%08x\n", mem_obj.object_data_);      
      std::cout << "Returning successful result" << std::endl;
      return std::make_pair(std::string(mem_obj.object_data_->text_,mem_obj.object_data_->size_),constants::error_codes::SUCCESS);
  }
  std::cerr << "Shared memory name : '" << shm_name << "' NOT found";
  return std::make_pair(std::string(), constants::error_codes::PATH_NOT_FOUND);
}

int SharedMemoryManager::ShmUnlink(const std::string& shm_name) {
  std::cout << "ShmUnlink " << shm_name << std::endl;
  errno = 0;
  const int res = shm_unlink(shm_name.c_str());
  if (-1 == res) {
    std::cout << "Error occurred: " << strerror(errno) << std::endl;
    return errno;
  }
  std::cout << "Returning successful result" << std::endl;
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
