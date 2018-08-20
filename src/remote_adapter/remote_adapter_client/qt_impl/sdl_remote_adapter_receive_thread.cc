#include "qt_impl/sdl_remote_adapter_receive_thread.h"
#include "common/constants.h"

namespace lua_lib {

SDLRemoteTestAdapterReceiveThread::SDLRemoteTestAdapterReceiveThread(
                SDLRemoteTestAdapterQtClient *client,
                QObject *parent)
            : QThread(parent) {
  client_ = client;
}

SDLRemoteTestAdapterReceiveThread::~SDLRemoteTestAdapterReceiveThread() {
  if (isRunning()) {
      terminate();
      wait();
  }
}

void SDLRemoteTestAdapterReceiveThread::run() {
  while(constants::error_codes::PATH_NOT_FOUND != client_->receive().second){ }
}

} // namespace lua_lib
