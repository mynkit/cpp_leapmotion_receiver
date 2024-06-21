sudo apt update
sudo apt install libboost-all-dev

sudo apt install git cmake

cd
# RapidJSONをクローン
git clone https://github.com/Tencent/rapidjson.git
cd rapidjson

# ビルドとインストール
mkdir build
cd build
cmake ..
make
sudo make install

sudo apt install git cmake libssl-dev

cd
git clone https://github.com/warmcat/libwebsockets.git
cd libwebsockets

# ビルドとインストール
mkdir build
cd build
cmake ..
make
sudo make install
