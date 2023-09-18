# asl
Audio analysis tool

![Build status]
(https://github.com/amdrozdov/asl/actions/workflows/build.yml/badge.svg)

### Usage example
```bash
asl info -f samples/sample.wav
asl split -f samples/sample.wav -p ch_split_
asl slice -f samples/sample.wav -s 1 8 55 -e 2 11 65 -o sl_one.wav sl_drums.wav sl_bass.wav
```

### Build
```bash
# C++ 17 
mkdir -p build && cd build && cmake ../ && cmake --build .
```

### Test build
```
pip install cpplint
make rebuild && make lint && make test
```
