#pragma once

#include <map>
#include <string>

namespace shmemory_wrappers {

typedef struct {
  int size_;
  char text_[30000];
} shmem_t;

typedef struct{
    int object_handle_;
    shmem_t * object_data_;
} object_descrp;

class SharedMemoryManager {
 public:  
  typedef std::pair<std::string, int> ReceiveResult;
  SharedMemoryManager();
  ~SharedMemoryManager();
  
  /**
   * @brief ShmOpen create descriptor that's associated with the shared “memory object”
   * specified by name and maps a region within the descriptor
   * @param shm_name name which associated with the shared “memory object”
   * @return error no (0 if success)
   */
  int ShmOpen(const std::string& shm_name,const int prot);
  
  /**
   * @brief ShmWrite write data to shared memory
   * @param shm_name name shared object to write in
   * @param data data to be written to shared memory
   * @return error no (0 if success)
   */
  int ShmWrite(const std::string& shm_name, const std::string& data);
  
  /**
   * @brief ShmRead reads data from shared memory
   * @param shm_name name shared object to read from
   * @return tuple with read data and errno
   * in case of successful reading, otherwise pair
   * with an empty string and errno
   */
  ReceiveResult ShmRead(const std::string& shm_name);

  /**
   * @brief MqClose closes shared memory specified by name
   * @param shm_name name shared object to close
   * @return error no (0 if success)
   */
  int ShmClose(const std::string& shm_name);
  
  /**
   * @brief ShmUnlink removing an object previously created by ShmOpen.
   * @param shm_name name shared object to remove
   */
  int ShmUnlink(const std::string& shm_name);  
  /**
   * @brief IsShmName checks is the path memory object name or queue name
   * @param path path to mqueue or memory object name
   * @param sh_name for write valid memory object name
   */
  bool IsShmName(const std::string& path,std::string& sh_name);

  static const char * shm_name_sdlqueue;
  static const char * shm_name_sdlqueue2;
  static const char * shm_name_sdlqueue3;
  static const char * shm_json_mem_ident;

private:
  std::map<std::string, object_descrp> handles_;
};

}  // namespace shmemory_wrappers
