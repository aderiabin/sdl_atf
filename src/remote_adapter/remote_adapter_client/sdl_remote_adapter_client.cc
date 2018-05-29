#include "sdl_remote_adapter_client.h"

#include <iostream>

#include "rpc/rpc_error.h"

#include "common/constants.h"

namespace lua_lib {



const char *SDLRemoteTestAdapterClient::get_data() {
    return data_.c_str();
}


SDLRemoteTestAdapterClient::SDLRemoteTestAdapterClient(const std::string &host,
                                                       uint32_t port):
    connection_(host, port) {
    connection_.call(constants::client_connected);
}

bool SDLRemoteTestAdapterClient::connected() const {
    return rpc::client::connection_state::connected == connection_.get_connection_state();
}

int SDLRemoteTestAdapterClient::open(const std::string &name) try {
    if (connected()) {
        connection_.call(constants::open, name);
        return 0;
    }
    return 1;
} catch (rpc::rpc_error& e) {
    return 1;
}

int SDLRemoteTestAdapterClient::clear()try {
    if (connected()) {
        connection_.call(constants::clear);
        return 0;
    }
    return 1;
} catch (rpc::rpc_error& e) {
    return 1;
}

std::string SDLRemoteTestAdapterClient::receive(const std::string &name) try {
    if (connected()) {
        auto received = connection_.call(constants::receive, name).as<std::string>();
        return received ;
    }
    return "";
} catch (rpc::rpc_error& e) {
    return "";
}

int SDLRemoteTestAdapterClient::send(const std::string &name, const std::string &data)  try {
    if (connected()) {
        connection_.call(constants::send, name, data);
        return 0;
    }
    return 1;
} catch (rpc::rpc_error& e) {
    return 1;
}




void SDLRemoteTestAdapterClient::set_data(const char *data) {
    data_ = std::string(data);
}


}  // namespace lua_lib
