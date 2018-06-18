#pragma once

#include <QtCore>
#include <QObject>

#include <string>
#include <utility>

#include "sdl_remote_adapter_client.h"
// #include "qt_impl/sdl_remote_adapter_receive_thread.h"

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
                                 QObject* parent = Q_NULLPTR);

    ~SDLRemoteTestAdapterQtClient();

    // /**
    // * @brief Connect client to server and open queue with default parameters
    // * @param host server host to connect
    // * @param port server port to connect
    // */
    // void connect(const std::string& host, uint32_t port, const std::string& name);

    /**
    * @brief Connect client to server and open queue with custom parameters
    */
    void connectMq();

    /**
    * @brief Sends data to mqueue opened by server
    * @param name mqueue name for data reading
    * @param data - data to be send to mqueue
    * @return 0 in successful case, 1 - if client is not connected,
    * 2 - in case of exception
    */
    int send(const std::string& data);

    // /**
    // * @brief Reads data from mqueue opened by server
    // * @param name mqueue name from which data should be received
    // * @return received data in successful case,
    // * otherwise empty string
    // */
    std::pair<std::string, int> receive();

    // /**
    // * @brief Reads data from mqueue opened by server
    // * @param name mqueue name from which data should be received
    // * @return received data in successful case,
    // * otherwise empty string
    // */
    // std::string receive(const std::string& name);

    // /**
    // * @brief Clears the system from mqueues opened by server
    // * @return 0 in successful case,  - if client is not connected,
    // * 2 - in case of exception
    // */
    // int clear();

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
    * @param name mqueue name which should be opened by server
    * @param max_messages_number -max messages number in mqueue
    * @param max_message_size - max message size in mqueue
    * @param flags - specifies flags that control the operation of the call
    * @param mode - specifies the permissions to be placed on the new queue
    * @return 0 in successful case, 1 - if client is not connected,
    * 2 - in case of exception
    */
    int openWithParams(MqParams& params);

    bool isconnected_ = false;
    const std::string host_;
    uint32_t port_;
    MqParams in_mq_params_;
    MqParams out_mq_params_;
    SDLRemoteTestAdapterClient* remote_adapter_client_ptr_ = nullptr;
    SDLRemoteTestAdapterReceiveThread* listener_ptr_ = nullptr;
};

} // namespace lua_lib
