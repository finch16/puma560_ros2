#ifndef H_CLIENTTCP_HPP_H
#define H_CLIENTTCP_HPP_H

#include <boost/asio.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>
#include <cstring>
#include <array>
#include <thread>
#include <stop_token>
#include <mutex>
#include <iostream>
#include <string>

extern "C"
{
#include "frame_codec.h"
}

namespace net
{

class tcp_client
{
    boost::asio::io_context io;
    boost::asio::ip::tcp::socket socket;
    boost::asio::experimental::concurrent_channel<void(boost::system::error_code, std::array<float, 6>)> send_channel;
    std::array<float, 6> recv_buf = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}; 
    std::mutex recv_mtx;
    std::jthread worker;
    bool running = false;

public:
    tcp_client() : io(), send_channel(io, 1), socket(io) {}

    void start(const std::string& host, const std::string& port)
    {
        if(running) return;

        boost::asio::co_spawn(io, connect(host, port), boost::asio::detached);
        worker = std::jthread([this](std::stop_token st)
        {
            std::stop_callback cb(st, [this]()
            {
                io.stop();
            });

            io.run();
        });
        running = true;
    }

    void stop()
    {
        if(!running) return;
       
        boost::asio::post(io, [this] { socket.close(); });
        worker.request_stop();
        running = false;
    }

    bool send(std::array<float, 6> data)
    {
        return send_channel.try_send(boost::system::error_code{}, data);
    }

    std::array<float, 6> get()
    {
        std::scoped_lock sl{recv_mtx};
        return recv_buf; 
    }

private:
    boost::asio::awaitable<void> connect(std::string host, std::string port)
    {
        boost::asio::ip::tcp::resolver resolver(io);
        auto endpoints = co_await resolver.async_resolve(host, port, boost::asio::use_awaitable);
        co_await boost::asio::async_connect(socket, endpoints, boost::asio::use_awaitable);
        
        boost::asio::co_spawn(io, sender(), boost::asio::detached);
        boost::asio::co_spawn(io, receiver(), boost::asio::detached);
    }
    
    boost::asio::awaitable<void> sender()
    {
        while(socket.is_open())
        {
            std::array<uint8_t, 30> buf;
            std::array<float, 6> data = co_await send_channel.async_receive(boost::asio::use_awaitable);

            frame_encode((uint8_t*)data.data(), 24, (uint8_t*)buf.data(), 30);
            co_await boost::asio::async_write(socket, boost::asio::buffer(buf), boost::asio::use_awaitable);
        }
    }
    
    boost::asio::awaitable<void> receiver()
    {
        while(socket.is_open())
        {
            boost::asio::streambuf buf;
            std::size_t size = co_await boost::asio::async_read_until(socket, buf, '\0', boost::asio::use_awaitable);
            if(size == 29 + 1)
            {
                std::array<uint8_t, 29> lbuf;
                boost::asio::buffer_copy(boost::asio::buffer(lbuf), buf.data(), 29);

                std::array<float, 6> data;
                if(frame_decode((uint8_t*)lbuf.data(), 29, (uint8_t*)data.data(), 24))
                { 
                    std::scoped_lock sl{recv_mtx};
                    std::memcpy(recv_buf.data(), data.data(), 24);
                }
            }
        }
    }
};

} //namespace net

#endif //H_CLIENTTCP_HPP_H
