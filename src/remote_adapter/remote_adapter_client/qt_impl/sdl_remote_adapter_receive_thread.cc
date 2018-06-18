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
    QString receivedData(client_->receive().first.c_str());

    while(!receivedData.isEmpty()){
        emit dataAvailable(receivedData);
        receivedData = client_->receive().first.c_str();
    }

    emit sourceUnreachable();
}

} // namespace lua_lib
