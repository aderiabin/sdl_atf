#pragma once

#include <string>
#include "rpc/client.h"

namespace lua_lib {

class SDLRemoteTestAdapterClient {
public:
    SDLRemoteTestAdapterClient(const std::string& host, uint32_t port);

    void set_data(const char* data);
    const char* get_data();

    bool connected() const;
    int open(const std::string& name);
    int send(const std::string& name, const std::string& data);
    std::string receive(const std::string& name);
    int clear();

private:
  std::string data_;
  rpc::client connection_;
  friend struct SDLRemoteTestAdapterLuaWrapper;
};



}
