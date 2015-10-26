#include "realconnection.h"
#include "packet.h"
#include "common.h"
#include "../metadata.h"
#include "../../logger.h"
#include "../../utils/crypto.h"

namespace base
{
    namespace dbo
    {
        namespace internal
        {
            using namespace std;
            using namespace base::dbo::internal;

            RealConnection::RealConnection(EventHandler& handler, memory::MemoryPool& mempool, uint32_t poolid)
                : Client(mempool), handler_(handler), data_handler_(nullptr), conn_phase_(CONN_PHASE_INIT_HANDSHAKE), poolid_(poolid)
            {
            }

            void RealConnection::Connect(const ServerInfo& serverinfo)
            {
                serverinfo_ = serverinfo;
                Client::Connect(serverinfo_.host.c_str(), serverinfo_.port);
            }

            RealConnection::~RealConnection()
            {
                for (map<string, PreparedStatementMetadata*>::iterator it = prepared_stmt_pool_.begin(); it != prepared_stmt_pool_.end(); ++it) {
                    delete it->second;
                }
                prepared_stmt_pool_.clear();
            }

            void RealConnection::OnConnect()
            {
                handler_.OnConnect(this);
            }

            void RealConnection::OnConnectFail(int eno, const char* reason)
            {
                handler_.OnConnectFail(this, reason);
            }

            void RealConnection::OnClose()
            {
                if (conn_phase_ == CONN_PHASE_OK && data_handler_) {
                    data_handler_->OnClose(this);
                }
                handler_.OnClose(this);
                LOG_DEBUG("mysql real connection closed!\n");
            }

            void RealConnection::OnReceive(std::size_t count)
            {
                while (count >= 4) {
                    uint32_t pkt_size = 0;
                    CopyReceive((char*)&pkt_size, 3);
                    pkt_size += 4;   // mysql pkt size = payload + 3 + 1
                    if (count >= pkt_size) {
                        packet_data_t data;
                        size_t got = PopReceive(data, pkt_size);
                        assert(got == pkt_size);
                        PacketIn pktin(data);
                        pktin.ReadHead();
                        OnReceivePacket(pktin);
                        count -= pkt_size;
                    } else {
                        break;
                    }
                }
            }

            static void calculate_mysql_native_password(const string& password, const string& auth_plugin_data_1, const std::string& auth_plugin_data_2, string& token)
            {
                string challenge(auth_plugin_data_1);
                challenge.append(auth_plugin_data_2);

                string stage1;
                base::utils::sha1(password, stage1);
                string stage2;
                base::utils::sha1(stage1, stage2);
                challenge.append(stage2);
                string stage3;
                base::utils::sha1(challenge, stage3);
                token.clear();
                for (uint32_t i = 0; i < 20; ++i) {
                    token.push_back((char)(stage1[i] xor stage3[i]));
                }
            }

            void RealConnection::Send(PacketOut& pktout)
            {
                pktout.WriteHead();
                packet_data_t data = pktout.FetchData();
                PushSend(data);
            }

            void RealConnection::HandleAuthInitHandshake(PacketIn& pktin)
            {
                uint8_t protocol_version;
                pktin.ReadFixedInteger<1>(&protocol_version);

                if (protocol_version == PACKET_ERR) {
                    // error packet
                    uint16_t code;
                    pktin.ReadFixedInteger<2>(&code);
                    std::string reason;
                    pktin.ReadRestOfPacketString(reason);
                    handler_.OnAuthFail(this, code, reason.c_str());
                    return;
                }

                if (protocol_version != 10u) {
                    throw Exception("not support mysql server protocol version");
                }
                std::string server_version;
                pktin.ReadNulTerminateString(server_version);
                uint32_t connection_id;
                pktin.ReadFixedInteger<4>(&connection_id);
                std::string auth_plugin_data_part_1;
                pktin.ReadFixedString(auth_plugin_data_part_1, 8);
                uint8_t filter;
                pktin.ReadFixedInteger<1>(&filter);

                uint16_t capability_flag_1 = 0;
                uint8_t charset = 0;
                uint16_t status_flag = 0;
                uint16_t capability_flag_2 = 0;

                if (pktin.FreeCount() >= 2) {
                    pktin.ReadFixedInteger<2>(&capability_flag_1);
                }
                if (pktin.FreeCount() >= 18) {
                    pktin.ReadFixedInteger<1>(&charset);
                    pktin.ReadFixedInteger<2>(&status_flag);
                    pktin.ReadFixedInteger<2>(&capability_flag_2);
                }
                uint32_t capability = (capability_flag_2 << 16) | capability_flag_1;
                uint8_t auth_plugin_data_length = 0;
                if (capability & CLIENT_PLUGIN_AUTH) {
                    pktin.ReadFixedInteger<1>(&auth_plugin_data_length);
                } else {
                    pktin.ReadFixedInteger<1>(&auth_plugin_data_length); // == 0
                }
                pktin.Skip(10);     // skip reserved string[10]

                string auth_plugin_data_part_2;
                string auth_plugin_name;
                if (capability & CLIENT_SECURE_CONNECTION) {
                    uint32_t len = max(13, auth_plugin_data_length - 8);
                    pktin.ReadFixedString(auth_plugin_data_part_2, len);
#ifdef HAS_CXX11
                    auth_plugin_data_part_2.pop_back();     // remove the null terminate
#else
                    auth_plugin_data_part_2.erase(auth_plugin_data_part_2.length() - 1 , 1);
#endif
                    if (capability & CLIENT_PLUGIN_AUTH) {
                        pktin.ReadRestOfPacketString(auth_plugin_name);
                    }
                }
                /*
                cout << "protocol_version:" << (uint32_t)protocol_version << endl;
                cout << "server_version:" << server_version << endl;
                cout << "connection_id:" << connection_id << endl;
                cout << "auth_plugin_data_part_1:" << auth_plugin_data_part_1 << endl;
                cout << "filter:" << (uint32_t)filter << endl;
                cout << "capability:" << capability << endl;
                cout << "auth_plugin_data_part_2:" << auth_plugin_data_part_2 << endl;
                cout << "charset:" << (uint32_t)charset << endl;
                cout << "status_flag:" << status_flag << endl;
                cout << "auth_plugin_name:" << auth_plugin_name << endl;
                */

                if (!(capability & CLIENT_PROTOCOL_41)) {
                    throw Exception("the server is not support CLIENT_PROTOCOL_41");
                }

                uint32_t client_capability = CLIENT_PROTOCOL_41
                                             | CLIENT_CONNECT_WITH_DB | CLIENT_TRANSACTIONS
                                             | CLIENT_SECURE_CONNECTION
                                             | CLIENT_PLUGIN_AUTH
                                             | CLIENT_PLUGIN_AUTH_LENENC_CLIENT_DATA;
                // cout << "client_capability:" << client_capability << endl;
                string token;
                calculate_mysql_native_password(serverinfo_.password, auth_plugin_data_part_1, auth_plugin_data_part_2, token);

                PacketOut pktout(mempool(), 30, 1);
                pktout.WriteFixedInteger<4>(client_capability);
                uint32_t max_packet_size = 1024 * 1024 * 5;
                pktout.WriteFixedInteger<4>(max_packet_size);
                uint8_t default_charset = 33;       // utf-8
                pktout.WriteFixedInteger<1>(default_charset);
                static char response_reserved[23] = {0};
                pktout.WriteFixedString(response_reserved, 23);
                pktout.WriteNulTerminateString(serverinfo_.username);
                if (capability & CLIENT_SECURE_CONNECTION) {
                    pktout.WriteVariableInteger(token.length());
                    pktout.WriteFixedString(token);
                } else if (capability & CLIENT_SECURE_CONNECTION) {
                    pktout.WriteFixedInteger<1>(token.length());
                    pktout.WriteFixedString(token);
                } else {
                    pktout.WriteNulTerminateString(token);
                }
                if (capability & CLIENT_CONNECT_WITH_DB) {
                    pktout.WriteNulTerminateString(serverinfo_.dbname);
                }
                if (capability & CLIENT_PLUGIN_AUTH) {
                    pktout.WriteNulTerminateString(auth_plugin_name);
                }

                conn_phase_ = CONN_PHASE_HANDSHAKE_RESPONSE;
                Send(pktout);
            }

            void RealConnection::HandleAuthResponse(PacketIn& pktin)
            {
                uint8_t flag = 0;
                pktin.ReadFixedInteger<1>(&flag);
                if (flag == PACKET_ERR) {
                    // ERR_Packet
                    uint16_t errorcode;
                    pktin.ReadFixedInteger<2>(&errorcode);
                    string reason;
                    pktin.ReadRestOfPacketString(reason);
                    handler_.OnAuthFail(this, errorcode, reason.c_str());
                } else if (flag == PACKET_OK) {
                    // OK_Packet
                    handler_.OnAuthSuccess(this);
                    conn_phase_ = CONN_PHASE_OK;
                } else if (flag == PACKET_EOF) {
                    // EOF ??
                    uint16_t warning_count;
                    pktin.ReadFixedInteger<2>(&warning_count);
                    uint16_t status_flag;
                    pktin.ReadFixedInteger<2>(&status_flag);
                    cout << "warning_count:" << warning_count << ", status_flag:" << status_flag << endl;
                    handler_.OnAuthFail(this, -1, "unknown");
                }
            }

            void RealConnection::OnReceivePacket(PacketIn& pktin)
            {
                switch (conn_phase_) {
                    case CONN_PHASE_INIT_HANDSHAKE:
                        HandleAuthInitHandshake(pktin);
                        break;
                    case CONN_PHASE_HANDSHAKE_RESPONSE:
                        HandleAuthResponse(pktin);
                        break;
                    case CONN_PHASE_OK:
                        if (data_handler_) {
                            data_handler_->OnReceivePacket(this, pktin);
                        }
                        break;
                }
            }
        }
    }
}

