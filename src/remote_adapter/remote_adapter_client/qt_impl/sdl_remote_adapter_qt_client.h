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
  SDLRemoteTestAdapterQtClient(SDLRemoteTestAdapterClient* client_ptr,
                               MqParams& in_params,
                               MqParams& out_params,
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
  * @brief Perform actions in case underlying client is disconnected
  */
  void connectionLost();

  bool isconnected_ = false;
  MqParams in_mq_params_;
  MqParams out_mq_params_;
  SDLRemoteTestAdapterClient* remote_adapter_client_ptr_;
  std::unique_ptr<SDLRemoteTestAdapterReceiveThread> listener_ptr_;
};

} // namespace lua_lib
