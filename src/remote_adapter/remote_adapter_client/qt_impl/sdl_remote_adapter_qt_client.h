#pragma once

#include <QObject>

#include <string>
#include <utility>
#include <memory>

#include "sdl_remote_adapter_client.h"

namespace lua_lib {

struct MqParams {
    const std::string name;
    const int max_messages_number;
    const int max_message_size;
    const int flags;
    const int mode;
};

class SDLRemoteTestAdapterReceiveThread;

class SDLRemoteTestAdapterQtClient : public QObject {
Q_OBJECT

public:
    SDLRemoteTestAdapterQtClient(const std::string& host,
                                 uint32_t port,
                                 MqParams& in_params,
                                 MqParams& out_params,
                                 MqParams& control_params,
                                 QObject* parent = Q_NULLPTR);

    ~SDLRemoteTestAdapterQtClient();

    /**
    * @brief Connect client to server and open queue with custom parameters
    */
    void connectMq();

    /**
    * @brief Sends data to mqueue opened by server
    * @param data - data to be send to mqueue
    * @return 0 in successful case, 1 - if client is not connected,
    * 2 - in case of exception
    */
    int send(const std::string& data);

    // /**
    // * @brief Reads data from mqueue opened by server
    // * @return received data in successful case,
    // * otherwise empty string
    // */
    std::pair<std::string, int> receive();

    /**
    * @brief Sends control data to mqueue opened by server
    * @param data - data to be send to mqueue
    * @return 0 in successful case, 1 - if client is not connected,
    * 2 - in case of exception
    */
    int sendControl(const std::string& data);

public slots:
    void connectionLost();

signals:
    void textMessageReceived(const QString &message);
    void bytesWritten(qint64 data);
    void connected();
    void disconnected();

private:
    /**
    * @brief Sends open mqueue parametrized request to server
    * @param params mqueue parameters for mqueue which should be opened by server
    * @return 0 in successful case, 1 - if client is not connected,
    * 2 - in case of exception
    */
    int openWithParams(MqParams& params);

    /**
    * @brief Sends data to mqueue opened by server
    * @param mq_name - name of mq data to be sent into
    * @param data - data to be send to mqueue
    * @return 0 in successful case, 1 - if client is not connected,
    * 2 - in case of exception
    */
    int sendToMq(const std::string& data, bool isControl);

    bool isconnected_ = false;
    const std::string host_;
    uint32_t port_;
    MqParams in_mq_params_;
    MqParams out_mq_params_;
    MqParams control_mq_params_;
    std::unique_ptr<SDLRemoteTestAdapterClient> remote_adapter_client_ptr_;
    std::unique_ptr<SDLRemoteTestAdapterReceiveThread> listener_ptr_;
};

} // namespace lua_lib
