# cpp leapmotion receiver

## Install

- boost

```sh
brew install boost
brew install rapidjson
```

- oscpack

https://code.google.com/archive/p/oscpack/downloads から`oscpack_1_1_0.zip`をダウンロードして解凍し、フォルダ名をoscpackに変更したあとこのプロジェクトディレクトリに配置する

- nlohmann/json.hpp

https://github.com/nlohmann/json/blob/develop/single_include/nlohmann/json.hpp をダウンロードし、プロジェクトディレクトリに配置

## Run

```sh
# Makefileの作成
cmake CMakeLists.txt
# build
make
# run
./WebSocketOSC
```

Apple Silicon

```
export DYLD_LIBRARY_PATH=.:$DYLD_LIBRARY_PATH
# Start WebSocket Server
./Ultraleap-Tracking-WS
```

```
cmake -DCMAKE_OSX_ARCHITECTURES="x86_64" CMakeLists.txt
make
./WebSocketOSC
```
