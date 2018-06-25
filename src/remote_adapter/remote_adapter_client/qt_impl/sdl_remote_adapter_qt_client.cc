#include <iostream>

#include "qt_impl/sdl_remote_adapter_qt_client.h"
#include "qt_impl/sdl_remote_adapter_receive_thread.h"
#include "common/constants.h"

namespace lua_lib {

SDLRemoteTestAdapterQtClient::SDLRemoteTestAdapterQtClient(
            const std::string& host,
            uint32_t port,
            MqParams& in_params,
            MqParams& out_params,
            MqParams& control_params,
            QObject* parent)
        : QObject(parent),
          host_(host),
          port_(port),
          in_mq_params_(in_params),
          out_mq_params_(out_params),
          control_mq_params_(control_params) { }

SDLRemoteTestAdapterQtClient::~SDLRemoteTestAdapterQtClient() {
    if (remote_adapter_client_ptr_) {
        remote_adapter_client_ptr_->clear();
    }
}

void SDLRemoteTestAdapterQtClient::connectMq() {
    if (isconnected_) {
        return;
    }

    try {
        remote_adapter_client_ptr_.reset(new SDLRemoteTestAdapterClient(host_, port_));
        int in_open_result = openWithParams(in_mq_params_);
        int out_open_result = openWithParams(out_mq_params_);
        int control_open_result = openWithParams(control_mq_params_);
        if ((constants::error_codes::SUCCESS == in_open_result)
                && (constants::error_codes::SUCCESS == out_open_result)
                && (constants::error_codes::SUCCESS == control_open_result)) {
            listener_ptr_.reset(new SDLRemoteTestAdapterReceiveThread(this));
            connect(listener_ptr_.get(), &SDLRemoteTestAdapterReceiveThread::dataAvailable,
                    this, &SDLRemoteTestAdapterQtClient::textMessageReceived);
            connect(listener_ptr_.get(), &SDLRemoteTestAdapterReceiveThread::sourceUnreachable,
                    this, &SDLRemoteTestAdapterQtClient::connectionLost);
            isconnected_ = true;
            emit connected();
            listener_ptr_->start();
        } else {
            remote_adapter_client_ptr_->clear();
            remote_adapter_client_ptr_ = nullptr;
        }
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}

int SDLRemoteTestAdapterQtClient::send(const std::string& data) {
    sendToMq(data, false);
}

int SDLRemoteTestAdapterQtClient::sendControl(const std::string& data) {
    sendToMq(data, true);
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

int SDLRemoteTestAdapterQtClient::sendToMq(const std::string& data, bool isControl) {
    std::string mq_name(out_mq_params_.name);
    if (isControl) {
        mq_name = control_mq_params_.name;
    }
    int result = remote_adapter_client_ptr_->send(out_mq_params_.name, data);
    if (constants::error_codes::SUCCESS == result) {
        if (!isControl) {
            emit bytesWritten(data.length());
        }
    }
    else if (constants::error_codes::NO_CONNECTION == result) {
        connectionLost();
    }
    return result;
}

void SDLRemoteTestAdapterQtClient::connectionLost() {
    if (isconnected_) {
        isconnected_ = false;
        listener_ptr_ = nullptr;
        remote_adapter_client_ptr_ = nullptr;
        emit disconnected();
    }
}

} // namespace lua_lib
