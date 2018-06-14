#include <iostream>

#include "qt_impl/sdl_remote_adapter_qt_client.h"
#include "qt_impl/sdl_remote_adapter_receive_thread.h"

namespace lua_lib {

SDLRemoteTestAdapterQtClient::SDLRemoteTestAdapterQtClient(QObject* parent)
        : QObject(parent) { }

SDLRemoteTestAdapterQtClient::~SDLRemoteTestAdapterQtClient() {
    if (nullptr != listener_ptr_) {
        delete(listener_ptr_);
    }

    if (nullptr != remote_adapter_client_ptr_) {
        remote_adapter_client_ptr_->clear();
        delete(remote_adapter_client_ptr_);
    }
}

// void SDLRemoteTestAdapterQtClient::connect(const std::string& host, uint32_t port, const std::string& name) {
//     name_ = name;
//     try {
//         remote_adapter_client_ptr_ = new SDLRemoteTestAdapterClient(host, port);
//         if (0 == open(name)) {
//             emit connected();
//             // ToDo (aderiabin): Add incoming message listening
//         } else {
//             emit disconnected();
//         }
//     } catch (std::exception& e) {
//         remote_adapter_client_ptr_ = nullptr;
//         std::cout << e.what() << std::endl;
//         emit disconnected();
//     }
// }

void SDLRemoteTestAdapterQtClient::connectMq(const std::string& host,
                                             uint32_t port,
                                             MqParams& in_params,
                                             MqParams& out_params) {
    try {
        remote_adapter_client_ptr_ = new SDLRemoteTestAdapterClient(host, port);
        int in_open_result = openWithParams(in_params);
        int out_open_result = openWithParams(out_params);
        if ((0 == in_open_result) && (0 == out_open_result)) {
            in_mq_name_ = in_params.name;
            out_mq_name_ = out_params.name;

            listener_ptr_ = new SDLRemoteTestAdapterReceiveThread(this);
            connect(listener_ptr_, &SDLRemoteTestAdapterReceiveThread::dataAvailable,
                    this, &SDLRemoteTestAdapterQtClient::textMessageReceived);
            connect(listener_ptr_, &SDLRemoteTestAdapterReceiveThread::sourceUnreachable,
                    this, &SDLRemoteTestAdapterQtClient::connectionLost);

            isconnected_ = true;
            emit connected();
            listener_ptr_->start();
        // } else {
        //     // emit disconnected();
        }
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        // emit disconnected();
    }

}

int SDLRemoteTestAdapterQtClient::send(const std::string& data) {
    int result = remote_adapter_client_ptr_->send(out_mq_name_, data);
    if (0 == result) {
        emit bytesWritten(data.length());
    }
    else if (1 == result) {
        connectionLost();
    }
    return result;
}

std::pair<std::string, int> SDLRemoteTestAdapterQtClient::receive() {
    return remote_adapter_client_ptr_->receive(in_mq_name_);
}

// std::string SDLRemoteTestAdapterQtClient::receive(const std::string& name) {
//     return remote_adapter_client_ptr_->receive(name);
// }

// int SDLRemoteTestAdapterQtClient::clear() {
//     int result = remote_adapter_client_ptr_->clear();
//     // if (1 == result) {
//     //     connectionLost();
//     // }
//     listener_ptr_->terminate();
//     listener_ptr_->wait();
//     delete(listener_ptr_);
//     listener_ptr_ = nullptr;
//     connectionLost();
//     return result;
// }

// bool SDLRemoteTestAdapterQtClient::isconnected() const {
//     return remote_adapter_client_ptr_ && remote_adapter_client_ptr_->connected();
// }

// int SDLRemoteTestAdapterQtClient::open(const std::string& name) {
//     return remote_adapter_client_ptr_->open(name);
// }

int SDLRemoteTestAdapterQtClient::openWithParams(MqParams& params) {
    return remote_adapter_client_ptr_->open_with_params(params.name,
                                                        params.max_messages_number,
                                                        params.max_message_size,
                                                        params.flags,
                                                        params.mode);
}

void SDLRemoteTestAdapterQtClient::connectionLost() {
    if (isconnected_) {
        isconnected_ = false;
        emit disconnected();
    }
}

} // namespace lua_lib
