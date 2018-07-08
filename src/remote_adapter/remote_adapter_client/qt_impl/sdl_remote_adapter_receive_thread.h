#pragma once

#include <QThread>

#include <string>
#include <utility>

#include "qt_impl/sdl_remote_adapter_qt_client.h"

namespace lua_lib {

class SDLRemoteTestAdapterReceiveThread : public QThread {
Q_OBJECT

public:
  SDLRemoteTestAdapterReceiveThread(SDLRemoteTestAdapterQtClient *client,
                                    QObject *parent = Q_NULLPTR);

  ~SDLRemoteTestAdapterReceiveThread();

private:
  void run() Q_DECL_OVERRIDE;

  SDLRemoteTestAdapterQtClient* client_;
};

} // namespace lua_lib
