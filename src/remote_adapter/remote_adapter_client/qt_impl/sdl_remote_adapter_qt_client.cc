#include <iostream>

#include "qt_impl/sdl_remote_adapter_qt_client.h"
#include "qt_impl/sdl_remote_adapter_receive_thread.h"
#include "common/constants.h"

namespace lua_lib {

SDLRemoteTestAdapterQtClient::SDLRemoteTestAdapterQtClient(
            SDLRemoteTestAdapterClient* client_ptr,
            MqParams& in_params,
            MqParams& out_params,
            QObject* parent)
        : QObject(parent),
          in_mq_params_(in_params),
          out_mq_params_(out_params){
    remote_adapter_client_ptr_ = client_ptr;
}

SDLRemoteTestAdapterQtClient::~SDLRemoteTestAdapterQtClient() {
  if (isconnected_) {
    remote_adapter_client_ptr_->close(in_mq_params_.name);
    remote_adapter_client_ptr_->unlink(in_mq_params_.name);
    remote_adapter_client_ptr_->close(out_mq_params_.name);
    remote_adapter_client_ptr_->unlink(out_mq_params_.name);
  }
}

void SDLRemoteTestAdapterQtClient::connectMq() {
  if (isconnected_) {
    std::cout << "Mq is already connected" << std::endl;
    return;
  }

  int in_open_result = openWithParams(in_mq_params_);
  int out_open_result = openWithParams(out_mq_params_);
  if ((constants::error_codes::SUCCESS == in_open_result)
        && (constants::error_codes::SUCCESS == out_open_result)) {
    try {
      listener_ptr_.reset(new SDLRemoteTestAdapterReceiveThread(this));
      isconnected_ = true;
      emit connected();
      listener_ptr_->start();
    } catch (std::exception& e) {
      std::cout << "Exception occurred: " << e.what() << std::endl;
    }
  } else {
    if (constants::error_codes::SUCCESS == in_open_result) {
      remote_adapter_client_ptr_->close(in_mq_params_.name);
      remote_adapter_client_ptr_->unlink(in_mq_params_.name);
    }

    if (constants::error_codes::SUCCESS == out_open_result) {
      remote_adapter_client_ptr_->close(out_mq_params_.name);
      remote_adapter_client_ptr_->unlink(out_mq_params_.name);
    }
  }
}

int SDLRemoteTestAdapterQtClient::send(const std::string& data) {
  if (isconnected_) {
    int result = remote_adapter_client_ptr_->send(out_mq_params_.name, data);
    if (constants::error_codes::SUCCESS == result) {
      emit bytesWritten(data.length());
    }
    else if (constants::error_codes::NO_CONNECTION == result) {
        connectionLost();
    }
    return result;
  }
  std::cout << "Mq was not connected" << std::endl;
  return constants::error_codes::NO_CONNECTION;
}

std::pair<std::string, int> SDLRemoteTestAdapterQtClient::receive() {
  if (isconnected_) {
    std::pair<std::string, int> result =
          remote_adapter_client_ptr_->receive(in_mq_params_.name);
    if (constants::error_codes::SUCCESS == result.second) {
      QString receivedData(result.first.c_str());
      emit textMessageReceived(receivedData);
    }
    else if (constants::error_codes::NO_CONNECTION == result.second) {
        connectionLost();
    }
    return result;
  }
  std::cout << "Mq was not connected" << std::endl;
  return std::make_pair(std::string(), constants::error_codes::NO_CONNECTION);
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
    listener_ptr_ = nullptr;
    emit disconnected();
  }
}

} // namespace lua_lib
