#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>
#include <vector>
#include <set>
#include <oscpack/osc/OscOutboundPacketStream.h>
#include <oscpack/ip/UdpSocket.h>
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
            char osc_buffer[6144];
            osc::OutboundPacketStream p(osc_buffer, 6144);
            p << osc::BeginBundleImmediate
              << osc::BeginMessage("/lm/finger");

            if (lmData.contains("hands")) {
                json hands = lmData["hands"];
                for (const auto& hand : hands) {
                    if (hand.contains("id") && hand.contains("type")) {
                        string handId = hand["id"].dump();
                        string handType = hand["type"];
                        handTypeList.push_back(handType);
                        handInfo.emplace(handId, handType);
                    }
                }
                // handTypeListの重複削除(leftとrightのみにする)
                set<string> handTypeSet(handTypeList.begin(), handTypeList.end());

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
                                    json tipVelocity = pointable["tipVelocity"];
                                    p << ht.c_str() << stoi(fingerId);
                                    for (const auto& tipPosition_ : tipPosition) {
                                        p << stof(tipPosition_.dump());
                                    }
                                    for (const auto& tipVelocity_ : tipVelocity) {
                                        p << stof(tipVelocity_.dump());
                                    }
                                    p << 1;
                                }
                            }
                        }
                    }
                }
            }
            

            p << osc::EndMessage
              << osc::EndBundle;
            osc_socket.Send(p.Data(), p.Size());
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

