#include <iostream>

#include "qt_impl/sdl_remote_adapter_qt_client.h"
#include "qt_impl/sdl_remote_adapter_receive_thread.h"

namespace lua_lib {

SDLRemoteTestAdapterQtClient::SDLRemoteTestAdapterQtClient(
            const std::string& host,
            uint32_t port,
            MqParams& in_params,
            MqParams& out_params,
            QObject* parent)
        : QObject(parent),
          host_(host),
          port_(port),
          in_mq_params_(in_params),
          out_mq_params_(out_params)  { }

SDLRemoteTestAdapterQtClient::~SDLRemoteTestAdapterQtClient() {
    if (nullptr != listener_ptr_) {
        delete(listener_ptr_);
    }

    if (nullptr != remote_adapter_client_ptr_) {
        remote_adapter_client_ptr_->clear();
        delete(remote_adapter_client_ptr_);
    }
}

void SDLRemoteTestAdapterQtClient::connectMq() {
    if (!isconnected_) {
        try {
            remote_adapter_client_ptr_ = new SDLRemoteTestAdapterClient(host_, port_);
            int in_open_result = openWithParams(in_mq_params_);
            int out_open_result = openWithParams(out_mq_params_);
            if ((0 == in_open_result) && (0 == out_open_result)) {
                listener_ptr_ = new SDLRemoteTestAdapterReceiveThread(this);
                connect(listener_ptr_, &SDLRemoteTestAdapterReceiveThread::dataAvailable,
                        this, &SDLRemoteTestAdapterQtClient::textMessageReceived);
                connect(listener_ptr_, &SDLRemoteTestAdapterReceiveThread::sourceUnreachable,
                        this, &SDLRemoteTestAdapterQtClient::connectionLost);

                isconnected_ = true;
                emit connected();
                listener_ptr_->start();
            } else {
                remote_adapter_client_ptr_->clear();
                delete(remote_adapter_client_ptr_);
                remote_adapter_client_ptr_ = nullptr;
            }
        } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }
    }
}

int SDLRemoteTestAdapterQtClient::send(const std::string& data) {
    int result = remote_adapter_client_ptr_->send(out_mq_params_.name, data);
    if (0 == result) {
        emit bytesWritten(data.length());
    }
    else if (1 == result) {
        connectionLost();
    }
    return result;
}

std::pair<std::string, int> SDLRemoteTestAdapterQtClient::receive() {
    return remote_adapter_client_ptr_->receive(in_mq_params_.name);
}

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
        delete(listener_ptr_);
        delete(remote_adapter_client_ptr_);
        emit disconnected();
    }
}

} // namespace lua_lib
