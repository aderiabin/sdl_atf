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

signals:
    void dataAvailable(const QString &data);
    void sourceUnreachable();

private:
    SDLRemoteTestAdapterQtClient* client_;

    void run() Q_DECL_OVERRIDE;
};

} // namespace lua_lib
