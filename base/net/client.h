#ifndef BASE_NET_CLIENT_H
#define BASE_NET_CLIENT_H

#include "../event/eventio.h"
#include "../memory/memorychunk.h"
#include <string>
#include <deque>
#include <vector>
#include <sys/uio.h>
namespace base
{
    namespace net
    {
        // 代表一个客户端连接
        // 可以是服务器accept的，也可以是主动connect的
        class Client : public event::EventIO
        {
        public:
            Client(memory::MemoryPool& mempool)
                : mempool_(mempool), connect_(false), connect_pending_(false), recvq_count_(0) {}
            virtual ~Client();

            // IP地址
            const std::string& ipaddr() const {
                return ipaddr_;
            }
            // 端口
            int port() const {
                return port_;
            }
            // 是否处于连接状态
            bool connect() const {
                return connect_;
            }
            memory::MemoryPool& mempool() {
                return mempool_;
            }

            // 关联已有的socket描述符
            // 通常在listener accept后调用
            void Connect(int clientfd);

            // 连接指定地址, 异步操作
            // 连接成功将调用OnConnect, 连接失败将调用OnConnectFail
            void Connect(const char* ipaddr, int port);

            // 投递需要发送的数据
            template<typename C>
            void PushSend(const C& src) {
                if (!connect()) {
                    return;
                }
                for (typename C::const_iterator it = src.begin(); it != src.end(); ++it) {
                    sendq_.push_back(*it);
                }
                InvokeSend();
            }
            

            // 从收到的数据中复制指定长度数据
            // dst 目标地址
            // count 字节数
            // 返回实际拷贝的字符数
            std::size_t CopyReceive(char* dst, std::size_t count);

            // 从收到的数据中弹出指定长度的数据, 同时从接收缓冲池中清除已弹出的数据
            // dst 存放弹出的数据
            // count 字节数
            // 返回实际弹出的字节数
            std::size_t PopReceive(std::vector<memory::RefMemoryChunk>& dst, std::size_t count);
            std::size_t PopReceive(char* dst, std::size_t count);
            
            virtual void Close();

        private:
            // IO事件
            virtual void OnEventIOReadable();
            virtual void OnEventIOWriteable();
            virtual void OnEventIOClose();
            void OnConnectSuccess() {
                connect_ = true;
                OnConnect();
            }
            // 连接成功
            virtual void OnConnect() = 0;
            // 连接失败
            // errno 错误编号
            // reason 原因
            virtual void OnConnectFail(int eno, const char* reason) = 0;
            // 连接关闭时
            virtual void OnClose() = 0;
            // 当接收到新数据时
            // count 当前接收缓冲区的字节数
            virtual void OnReceive(std::size_t count) = 0;
            // 当发送成功, 发送缓冲区为空时触发
            virtual void OnSendCompleted() {}
            // 检查连接是否已完成
            void CheckIfConnectCompleted();

        private:
            // 进行发送操作
            void InvokeSend();
            // 用于接收缓冲区内存分配
            memory::MemoryPool& mempool_;
            std::string ipaddr_;
            int port_;
            bool connect_;
            // 正在连接，连接尚未建立, 等待OnConnect或OnConnectFail事件
            bool connect_pending_;

            static const int32_t SENDIOV_SIZE = 200;    // 可根据性能需求调整, 加大吞吐量
            iovec sendiov_[SENDIOV_SIZE];
            // 发送缓冲区
            std::deque<memory::RefMemoryChunk> sendq_;
            // 将发送队列转换为iovec中
            int Sendq2IOVec() {
                int cnt = 0;
                std::deque<memory::RefMemoryChunk>::iterator it = sendq_.begin();
                while (it != sendq_.end() && cnt < SENDIOV_SIZE) {
                    sendiov_[cnt].iov_base = (*it).data();
                    sendiov_[cnt].iov_len = (*it).count();
                    ++cnt;
                    ++it;
                }
                return cnt;
            }

            static const int32_t RECVIOV_SIZE = 200;    // 可根据性能需求调整, 加大吞吐量, 或根据内存池chunk大小动态设计(TODO)
            iovec recviov_[RECVIOV_SIZE];
            // 已接收的数据缓冲区
            std::deque<memory::RefMemoryChunk> recvq_;
            std::size_t recvq_count_;
            static const uint32_t RECVP_SIZE = 20;       // 可根据实现数据包大小进行调整
            // 可用于接收数据的内存池
            std::deque<memory::RefMemoryChunk> recvp_;
            // 将可用于接收数据池转为iovec
            int Recvp2IOVec() {
                int cnt = 0;
                std::deque<memory::RefMemoryChunk>::iterator it = recvp_.begin();
                while (it != recvp_.end() && cnt < RECVIOV_SIZE) {
                    recviov_[cnt].iov_base = (*it).data();
                    recviov_[cnt].iov_len = (*it).count();
                    ++cnt;
                    ++it;
                }
                return cnt;
            }
        };
    }
}

#endif // CLIENT_H
