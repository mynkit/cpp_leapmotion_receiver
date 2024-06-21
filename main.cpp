#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>
#include <vector>
#include <set>
#include "oscpack/osc/OscOutboundPacketStream.h"
#include "oscpack/ip/UdpSocket.h"
#include "json.hpp"

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http;   // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>
using namespace std;
using json = nlohmann::json;


int main() {
    // WebSocketサーバのIPアドレスとポートを指定
    const string server_ip = "127.0.0.1";
    const unsigned short server_port = 6437;
    const string endpoint = "/v6.json";
    // OSC server information
    const string osc_server_address = "127.0.0.1";
    const unsigned short osc_server_port = 12345;

    try {
        boost::asio::io_context io_context;

        // WebSocketエンドポイントの作成
        tcp::resolver resolver(io_context);
        websocket::stream<tcp::socket> ws(io_context);

        // WebSocketエンドポイントへの接続
        auto const results = resolver.resolve(server_ip, to_string(server_port));
        boost::asio::connect(ws.next_layer(), results.begin(), results.end());
        ws.handshake(server_ip, "/v6.json");

        // v6からはレスポンスを受け取るために{focused: true}の送信が必要
        json sendData;
        sendData["focused"] = true;
        ws.write(boost::asio::buffer(sendData.dump()));
        sendData.clear();
        sendData["background"] = true;
        ws.write(boost::asio::buffer(sendData.dump()));


        // OSC client setup
        UdpTransmitSocket osc_socket(IpEndpointName(osc_server_address.c_str(), osc_server_port));

        // メッセージの受信ループ
        for (;;) {
            beast::flat_buffer buffer;
            ws.read(buffer); // メッセージを受信

            // 受信したデータを文字列に変換して表示
            // cout << beast::make_printable(buffer.data()) << endl;
            
            json lmData = json::parse(beast::buffers_to_string(buffer.data()));
            json handInfo = json::object();
            string LR[] = {"left", "right"};
            vector<string> handTypeList;

            // Send received message via OSC
            char osc_bufferf[6144];
            char osc_bufferh[6144];
            osc::OutboundPacketStream p(osc_bufferf, 6144);
            osc::OutboundPacketStream q(osc_bufferh, 6144);
            p << osc::BeginBundleImmediate
              << osc::BeginMessage("/lm/finger");
            q << osc::BeginBundleImmediate
              << osc::BeginMessage("/lm/hand");

            if (lmData.contains("hands")) {
                json hands = lmData["hands"];
                for (const auto& hand : hands) {
                    if (hand.contains("id") && hand.contains("type") && hand.contains("palmNormal") && hand.contains("palmPosition")) {
                        string handId = hand["id"].dump();
                        string handType = hand["type"];
                        json palmNormal = hand["palmNormal"];
                        json palmPosition = hand["palmPosition"];
                        handTypeList.push_back(handType);
                        handInfo.emplace(handId, handType);
                        if (handType == "left") {
                            for (const auto& palmPosition_ : palmPosition) {
                                q << stof(palmPosition_.dump());
                            }
                            for (const auto& palmNormal_ : palmNormal) {
                                q << stof(palmNormal_.dump());
                            }
                            q << 0;
                        }
                    }
                }

                // handTypeListの重複削除(leftとrightのみにする)
                set<string> handTypeSet(handTypeList.begin(), handTypeList.end());

                if (handTypeSet.find("left") == handTypeSet.end()) {
                    q << 0.0 << 0.0 << 0.0 << 0.0 << -1.0 << 0.0 << 0;
                }


                for (int k=0; k<2; k++) {
                    string ht = LR[k];
                    if (handTypeSet.find(LR[k]) == handTypeSet.end()) {
                        // 左手or右手がない場合
                        for (int i=0; i<5; i++) {
                            p << ht.c_str() << i
                              << 0.0 << 0.0 << 0.0
                              << 0.0 << 0.0 << 0.0
                              << 0;
                        }
                    } else {
                        // 左手or右手があった場合
                        if(lmData.contains("pointables")) {
                            json pointables = lmData["pointables"];
                            for (const auto& pointable : pointables) {
                                string handId = pointable["handId"].dump();
                                string handType = handInfo[handId];
                                if (handType != ht) {
                                    continue;
                                } else {
                                    string fingerId = pointable["type"].dump();
                                    json tipPosition = pointable["tipPosition"];
                                    p << ht.c_str() << stoi(fingerId);
                                    for (const auto& tipPosition_ : tipPosition) {
                                        p << stof(tipPosition_.dump());
                                    }
                                    if (pointables.contains("tipVelocity")) {
                                      json tipVelocity = pointable["tipVelocity"];
                                      for (const auto& tipVelocity_ : tipVelocity) {
                                          p << stof(tipVelocity_.dump());
                                      }
                                    } else {
                                      p << 0.0 << 0.0 << 0.0;
                                    }
                                    p << 1;

                                    // if (fingerId == "1") {
                                    //     json mcpPosition = pointable["mcpPosition"];
                                    //     json carpPosition = pointable["carpPosition"];
                                    // }
                                }
                            }
                        }
                    }
                }
            }
            

            p << osc::EndMessage
              << osc::EndBundle;
            osc_socket.Send(p.Data(), p.Size());
            q << osc::EndMessage
              << osc::EndBundle;
            osc_socket.Send(q.Data(), q.Size());
        }

        // エンドポイントのクローズ
        ws.close(websocket::close_code::normal);
    }
    catch (exception const& e) {
        cerr << "エラー: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

