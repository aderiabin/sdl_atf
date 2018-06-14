#include "qt_impl/sdl_remote_adapter_receive_thread.h"

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
    while(true) {
        std::pair<std::string, int> receivedData = client_->receive();
        if ((receivedData.first).empty()) {
            emit sourceUnreachable();
            break;
        }
        QString result((receivedData.first).c_str());
        emit dataAvailable(result);
    }
}

} // namespace lua_lib
