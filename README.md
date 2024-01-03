# cpp leapmotion receiver

## Install

- boost

```sh
brew install boost
brew install rapidjson
brew install libwebsockets
```

- oscpack

https://code.google.com/archive/p/oscpack/downloads から`oscpack_1_1_0.zip`をダウンロードして解凍し、フォルダ名をoscpackに変更したあとこのプロジェクトディレクトリに配置しています

- nlohmann/json.hpp

https://github.com/nlohmann/json/blob/develop/single_include/nlohmann/json.hpp をダウンロードし、プロジェクトディレクトリに配置しています

## Run

### MacOS Intel (<= Catalina)

Start WebSocket Receiver

```sh
# Makefileの作成
cmake CMakeLists.txt
# build
make
# run
./WebSocketOSC
```

### MacOS Intel (> Catalina)

Start WebSocket Server

```sh
export DYLD_LIBRARY_PATH=./MacOS-Intel:$DYLD_LIBRARY_PATH
./MacOS-Intel/Ultraleap-Tracking-WS
```

Start WebSocket Receiver

```sh
cmake CMakeLists.txt
make
./WebSocketOSC
```

### MacOS Apple Silicon

Start WebSocket Server

```sh
export DYLD_LIBRARY_PATH=./MacOS-Silicon:$DYLD_LIBRARY_PATH
./MacOS-Silicon/Ultraleap-Tracking-WS
```

Start WebSocket Receiver

```sh
cmake -DCMAKE_OSX_ARCHITECTURES="x86_64" CMakeLists.txt
make
./WebSocketOSC
```
