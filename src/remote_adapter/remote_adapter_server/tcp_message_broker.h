#ifndef __TCP_MESSAGE_BROKER__H__
#define __TCP_MESSAGE_BROKER__H__

#include <thread>
#include <memory>
#include <vector>
#include <queue> 
#include <future>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "rpc/detail/log.h"


namespace tcp_msg_wrappers{

using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
using io_context =  boost::asio::io_context;    // from <boost/asio/io_context.hpp>
namespace websocket = boost::beast::websocket;  // from <boost/beast/websocket.hpp>

template<class Session>
class WebsocketListener;

class WebsocketSession;

template<typename TCPListener = WebsocketListener<WebsocketSession> >
class TCPMessageBroker{
public:    
    typedef std::shared_ptr<TCPListener>  HandleListener; 
    typedef std::vector<std::thread>      ArrayThreads;
    typedef tcp::endpoint ConnectionIdent;
    typedef struct ListenerContext{
        // The io_context is required for all I/O
        io_context ioc_;
        HandleListener listener_;        
        ArrayThreads   aThreads_;
        std::promise<void> exitSignal_;  
        std::shared_future<void> sh_future_;    
        ListenerContext(const int threads,const ConnectionIdent& connectIdent):
            ioc_(threads)
            ,listener_(std::make_shared<TCPListener>(ioc_, connectIdent))
            {
                aThreads_.reserve(threads);
                sh_future_ = exitSignal_.get_future().share();
            }
        }Context;  
    typedef std::shared_ptr<Context> HandleContext;
    typedef std::map<ConnectionIdent,HandleContext> HandlesListeners;    
    typedef int status;
    typedef std::pair<std::string, int> ReceiveResult;

    TCPMessageBroker(){};
    TCPMessageBroker(const TCPMessageBroker&) = delete;
    TCPMessageBroker& operator=(const TCPMessageBroker&) = delete;
    TCPMessageBroker(TCPMessageBroker&&) = delete;
    TCPMessageBroker& operator=(TCPMessageBroker&&) = delete;
    
    ~TCPMessageBroker();

    status        OpenConnection(const std::string& sAddress,const int port,const int threads = 2);
    status        CloseConnection(const std::string& sAddress,const int port);
    status        Send(const std::string& sAddress,const int port,const std::string& sData);
    ReceiveResult Receive(const std::string& sAddress,const int port);    

private:   
    Context* MakeContext(const std::string& sAddress,const int port,int threads);
    Context* GetContext(const std::string& sAddress,const int port);
    ConnectionIdent MakeConnectIdent(const std::string& sAddress,int port);
    
    HandlesListeners aHandles_;

    RPCLIB_CREATE_LOG_CHANNEL(TCPMessageBroker)      
};

template class TCPMessageBroker<WebsocketListener<WebsocketSession> >;

class WebsocketSession : public std::enable_shared_from_this<WebsocketSession>
{
public:
   // Take ownership of the socket
    explicit
    WebsocketSession(tcp::socket socket);
    
    void Run();    
    void OnAccept(boost::system::error_code ec);
    
    void AsyncRead();
    void OnRead(
        boost::system::error_code ec,
        std::size_t bytes_transferred);    
    
    void AsyncWrite(const std::string& sData);
    void OnWrite(
        boost::system::error_code ec,
        std::size_t bytes_transferred);
    
    void AsyncClose();    
    void OnClose(boost::system::error_code ec);  

    std::string GetMessage();  

private:
    websocket::stream<tcp::socket> ws_;
    boost::asio::strand<
        boost::asio::io_context::executor_type> strand_;
    boost::beast::multi_buffer write_buffer_;
    boost::beast::multi_buffer read_buffer_;
    std::queue<std::string> msg_queue_;

    RPCLIB_CREATE_LOG_CHANNEL(WebsocketSession)     
};

template<class Session = WebsocketSession>
class WebsocketListener : public std::enable_shared_from_this<WebsocketListener<Session> >
{    
public:
    explicit
    WebsocketListener(
        boost::asio::io_context& ioc,
        tcp::endpoint endpoint);        
    // Start accepting incoming connections
    void Run();
    void Stop();
    void DoAccept();    
    void OnAccept(boost::system::error_code ec); 

    Session& GetHandler();

private:
    tcp::acceptor acceptor_;
    tcp::socket socket_;
    std::shared_ptr<Session> session_handler_ptr_;
    
    RPCLIB_CREATE_LOG_CHANNEL(WebsocketListener)
};

template class WebsocketListener<WebsocketSession>;

}

#endif //__TCP_MESSAGE_BROKER__H__