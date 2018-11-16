//------------------------------------------------------------------------------
//
// Example: WebSocket server, asynchronous
//
//------------------------------------------------------------------------------
#include "tcp_message_broker.h"
#include <algorithm>
#include <functional>
#include <iostream>
#include <string>
#include "common/constants.h"

namespace tcp_msg_wrappers{

//------------------------------------------------------------------------------
// Report a failure
void Fail(boost::system::error_code ec, char const* what){
    LOG_ERROR("{0}: {1}",what,ec.message());    
}

// //------------------------------------------------------------------------------
 WebsocketSession::WebsocketSession(tcp::socket socket)
        : ws_(std::move(socket))
        , strand_(ws_.get_executor())
{
    LOG_INFO("{0}",__func__);
}

    // Start the asynchronous operation    
void  WebsocketSession::Run(){            
    LOG_INFO("{0}",__func__);
    
    ws_.async_accept(
        boost::asio::bind_executor(
            strand_,
            std::bind(
                &WebsocketSession::OnAccept,
                shared_from_this(),
                std::placeholders::_1)));
}

void WebsocketSession::OnAccept(boost::system::error_code ec){    
    LOG_INFO("{0}",__func__);

    if(ec){
        return Fail(ec,"WebsocketSession::OnAccept");              
    }
    
    // Read a message
    AsyncRead();
}

void WebsocketSession::AsyncRead(){     
    LOG_INFO("{0}",__func__);    
// Read a message into our buffer
    ws_.async_read(
        read_buffer_,
        boost::asio::bind_executor(
            strand_,
            std::bind(
                &WebsocketSession::OnRead,
                shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2)));
}

void WebsocketSession::OnRead(
        boost::system::error_code ec,
        std::size_t bytes_transferred)
{
    LOG_INFO("{0}",__func__);
    
    boost::ignore_unused(bytes_transferred);

    // This indicates that the session was closed
    if(ec == websocket::error::closed)
        return;

    if(ec){
        Fail(ec,"WebsocketSession::AsyncRead");
    }

    if(read_buffer_.size()){
        msg_queue_.push(boost::beast::buffers_to_string(read_buffer_.data()));
        // Clear the buffer
        read_buffer_.consume(read_buffer_.size());
    }
}

 void WebsocketSession::AsyncWrite(const std::string& sData){
    LOG_INFO("{0}",__func__);

    boost::beast::ostream(write_buffer_) << sData;
       
    ws_.text(ws_.got_text());
    ws_.async_write(
        write_buffer_.data(),
        boost::asio::bind_executor(
            strand_,
            std::bind(
                &WebsocketSession::OnWrite,
                shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2)));
}

void WebsocketSession::OnWrite(
        boost::system::error_code ec,
        std::size_t bytes_transferred)
{
    LOG_INFO("{0}",__func__);
    
    boost::ignore_unused(bytes_transferred);

    if(ec){
        return Fail(ec,"WebsocketSession::AsyncWrite");
    }

    // Clear the buffer
    write_buffer_.consume(write_buffer_.size());

    // Do another read
    AsyncRead();
}

void WebsocketSession::AsyncClose(){
    LOG_INFO("{0}",__func__);
    
     // Close the WebSocket connection
     ws_.async_close(websocket::close_code::normal,
        std::bind(
                &WebsocketSession::OnClose,
                shared_from_this(),
                std::placeholders::_1));
}

void WebsocketSession::OnClose(boost::system::error_code ec){
     LOG_INFO("{0}",__func__);

     if(ec){
         return Fail(ec,"WebsocketSession::AsyncClose");
     }    
}

std::string WebsocketSession::GetMessage(){
    LOG_INFO("{0}",__func__);

    if(msg_queue_.empty()){
        return std::string();
    }

    std::string msg = msg_queue_.front();
    msg_queue_.pop();

    return msg;
}

// //------------------------------------------------------------------------------
template<class Session>
WebsocketListener<Session>::WebsocketListener(
        boost::asio::io_context& ioc,
        tcp::endpoint endpoint)
        : acceptor_(ioc)
        , socket_(ioc)
{
    LOG_INFO("{0}",__func__);
    boost::system::error_code ec;

    // Open the acceptor
    acceptor_.open(endpoint.protocol(), ec);
    if(ec){
        Fail(ec,"open");
        return;
    }

    // Allow address reuse
    acceptor_.set_option(boost::asio::socket_base::reuse_address(true), ec);
    if(ec){
        Fail(ec,"set_option");
        return;
    }

    // Bind to the server address
    acceptor_.bind(endpoint, ec);
    if(ec){
        Fail(ec,"bind");
        return;
    }

    // Start listening for connections
    acceptor_.listen(
        boost::asio::socket_base::max_listen_connections, ec);
    if(ec){
        Fail(ec,"listen");
        return;
    }
}

    // Start accepting incoming connections
template<class Session>
void WebsocketListener<Session>::Run(){
    LOG_INFO("{0}",__func__);
    
    if(! acceptor_.is_open()){
        return;
    }
    
    DoAccept();
}

template<class Session>
void WebsocketListener<Session>::Stop(){
    LOG_INFO("{0}",__func__);
    
    if(!acceptor_.is_open()){
        return;
    }   
    
    if(session_handler_ptr_.use_count()){
        session_handler_ptr_->AsyncClose();
    }

    boost::system::error_code ec;        
    acceptor_.release(ec);

    if(ec){
        Fail(ec,"release");
    }
}

template<class Session>
void WebsocketListener<Session>::DoAccept(){    
    LOG_INFO("{0}",__func__);
    
    acceptor_.async_accept(
        socket_,
        std::bind(
            &WebsocketListener::OnAccept,
            this->shared_from_this(),
            std::placeholders::_1));
}

template<class Session>
void WebsocketListener<Session>::OnAccept(boost::system::error_code ec){
    LOG_INFO("{0}",__func__);
    
    if(ec){
        Fail(ec,"DoAccept");
    }else{
        // Create the session and run it
        session_handler_ptr_ = std::make_shared<Session>(std::move(socket_));
        session_handler_ptr_->Run();
        // Accept another connection
        DoAccept();
    }   
}

template<typename Session>
Session& WebsocketListener<Session>::GetHandler(){    
    LOG_INFO("{0}",__func__);

    if(session_handler_ptr_){
        return *(session_handler_ptr_).get();
    }

    LOG_ERROR("{0} Session can't created returned dummy_socket",__func__);       
    
    boost::asio::io_context ioc{1};       
    tcp::socket dummy_socket(ioc);

    session_handler_ptr_ = std::make_shared<Session>(std::move(dummy_socket));
    
    return *(session_handler_ptr_).get();
}

// //------------------------------------------------------------------------------
template<typename TCPListener>
TCPMessageBroker<TCPListener>::~TCPMessageBroker(){
    LOG_INFO("{0}",__func__);

    for(auto & handle : aHandles_){
        CloseConnection(handle.first.address().to_string(),handle.first.port());
    }    
}

template<typename TCPListener>
typename TCPMessageBroker<TCPListener>::status TCPMessageBroker<TCPListener>::OpenConnection(
    const std::string& sAddress,const int port,const int threads)
{
    LOG_INFO("{0}: Connect to address:{1} Port:{2} Threads:{3}",__func__,sAddress,port,threads);   

    Context * context = MakeContext(sAddress,port,threads);
    
    if(nullptr == context){
        return constants::error_codes::ALREADY_EXISTS;
    }
    
    context->listener_->Run();      
    
    auto & io_context = context->ioc_;
    auto & aThreads = context->aThreads_;
    auto & sh_future = context->sh_future_;
    
    // Run the I/O service on the requested number of threads        
    for(auto i = threads ? threads : 1; i > 0; --i){  
       
       aThreads.emplace_back(
           [&io_context,&sh_future]
            {               	
                while (sh_future.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout){                    
                    io_context.run();
                }
            }
        );
    }
    
    return constants::error_codes::SUCCESS;
}

template<typename TCPListener>
typename TCPMessageBroker<TCPListener>::status TCPMessageBroker<TCPListener>::CloseConnection(
    const std::string& sAddress,const int port)
{
    LOG_INFO("{0}: address:{1} port:{2}",__func__,sAddress,port);   

    const ConnectionIdent connectIdent = MakeConnectIdent(sAddress,port);

    auto itHandler = aHandles_.find(connectIdent);
    
    if(aHandles_.end() == itHandler){
        return constants::error_codes::NO_CONNECTION;   
    }

    if(0 == itHandler->second.use_count()){
        return constants::error_codes::NO_CONNECTION;           
    }

    Context * context = itHandler->second.get();   
    
    context->exitSignal_.set_value();                
    context->listener_->Stop();  
    
    auto & aThreads = context->aThreads_;    
    for(auto & thread : aThreads){
        thread.join();
    }
    
    aHandles_.erase(itHandler);
    
    return constants::error_codes::SUCCESS;
}

template<typename TCPListener>
typename TCPMessageBroker<TCPListener>::status TCPMessageBroker<TCPListener>::Send(
    const std::string& sAddress,const int port,const std::string& sData)
{
    LOG_INFO("{0}: to address:{1} port:{2} data:{3}",__func__,sAddress,port,sData);   

    Context* context = GetContext(sAddress,port);

    if(nullptr == context){
        return constants::error_codes::NO_CONNECTION;   
    }

    context->listener_->GetHandler().AsyncWrite(sData);       
    
    return constants::error_codes::SUCCESS;
}

template<typename TCPListener>
typename TCPMessageBroker<TCPListener>::ReceiveResult TCPMessageBroker<TCPListener>::Receive(
    const std::string& sAddress,const int port)
{
    LOG_INFO("{0}: from address:{1} port:{2}",__func__,sAddress,port);   

    Context* context = GetContext(sAddress,port);

    if(nullptr == context){
        return std::make_pair<std::string,int>("",int(constants::error_codes::NO_CONNECTION));   
    }

    std::string msg = context->listener_->GetHandler().GetMessage();      
    
    return std::make_pair(msg,int(constants::error_codes::SUCCESS));
}

template<typename TCPListener>
typename TCPMessageBroker<TCPListener>::Context* TCPMessageBroker<TCPListener>::MakeContext(
    const std::string& sAddress,const int port,int threads)
{
    LOG_INFO("{0}",__func__);       

    const ConnectionIdent connectIdent = MakeConnectIdent(sAddress,port);    
    
    threads = std::max<int>(1,threads);     

    auto result = aHandles_.insert(std::make_pair(connectIdent,std::make_shared<Context>(threads,connectIdent)));   

    if(result.second){
        return result.first->second.get();       
    }   
    
    return nullptr;
}

template<typename TCPListener>
typename TCPMessageBroker<TCPListener>::Context* TCPMessageBroker<TCPListener>::GetContext(
    const std::string& sAddress,const int port)
{
    LOG_INFO("{0}",__func__);
    
    const ConnectionIdent connectIdent = MakeConnectIdent(sAddress,port);  

    const auto itHandler = aHandles_.find(connectIdent);

    if(itHandler != aHandles_.end()){
        return itHandler->second.get();
    }

    return nullptr;      
}

template<typename TCPListener>
typename TCPMessageBroker<TCPListener>::ConnectionIdent TCPMessageBroker<TCPListener>::MakeConnectIdent(
    const std::string& sAddress,const int port)    
{
    LOG_INFO("{0}",__func__);   
    
    auto const address = boost::asio::ip::make_address(sAddress.c_str());  

    return ConnectionIdent{address, port};
}

};
