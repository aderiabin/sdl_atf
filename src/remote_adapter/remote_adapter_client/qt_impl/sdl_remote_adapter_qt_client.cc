#include <iostream>

#include "qt_impl/sdl_remote_adapter_qt_client.h"
#include "common/constants.h"
#include "rpc/detail/log.h"

namespace lua_lib {

namespace error_codes = constants::error_codes;

SDLRemoteTestAdapterQtClient::SDLRemoteTestAdapterQtClient(
            SDLRemoteTestAdapterClient* client_ptr
            ,TCPParams& in_params
            ,QObject* parent)
        : QObject(parent)
         ,tcp_params_(in_params)
{
    LOG_INFO("{0}",__func__);
    remote_adapter_client_ptr_ = client_ptr;
    future_ = exitSignal_.get_future();
}

SDLRemoteTestAdapterQtClient::~SDLRemoteTestAdapterQtClient() {
  LOG_INFO("{0}",__func__);
  if (isconnected_) {    
    exitSignal_.set_value();
    remote_adapter_client_ptr_->tcp_close(tcp_params_.host,tcp_params_.port);    
  }
}

void SDLRemoteTestAdapterQtClient::connect() {
  LOG_INFO("{0}",__func__);
  if (isconnected_) {
    LOG_INFO("{0} Is already connected",__func__);
    return;
  }

  int result = remote_adapter_client_ptr_->tcp_open(tcp_params_.host,tcp_params_.port);    

  if (error_codes::SUCCESS == result) {
    try {
      isconnected_ = true;
      emit connected();   
      auto & future = future_;
      listener_ptr_.reset(new std::thread(
           [this,&future]
            {               	
                while (future.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout){                    
                    this->receive();
                }
            }
          ));         
    } catch (std::exception& e) {
      LOG_ERROR("{0}: Exception occurred: {1} ",__func__,e.what());
    }
  }
}

int SDLRemoteTestAdapterQtClient::send(const std::string& data) {
  if (isconnected_) {
    int result = remote_adapter_client_ptr_->tcp_send(tcp_params_.host,tcp_params_.port,data);
    if(error_codes::SUCCESS == result) {
      emit bytesWritten(data.length());
    }else if(error_codes::NO_CONNECTION == result) {
        connectionLost();
    }
    return result;
  }  
  LOG_ERROR("{0}: Websocket was not connected",__func__);
  return error_codes::NO_CONNECTION;
}

std::pair<std::string, int> SDLRemoteTestAdapterQtClient::receive() {
  if (isconnected_) {
    std::pair<std::string, int> result =
          remote_adapter_client_ptr_->tcp_receive(tcp_params_.host,tcp_params_.port);
    if(error_codes::SUCCESS == result.second) {
      QString receivedData(result.first.c_str());
      emit textMessageReceived(receivedData);
    }else if(error_codes::NO_CONNECTION == result.second) {
        connectionLost();
    }
    return result;
  }
  LOG_ERROR("{0}: Websocket was not connected",__func__);
  return std::make_pair(std::string(),error_codes::NO_CONNECTION);
}

void SDLRemoteTestAdapterQtClient::connectionLost() {
  if (isconnected_) {
    isconnected_ = false;
    listener_ptr_ = nullptr;
    emit disconnected();
  }
}

} // namespace lua_lib
