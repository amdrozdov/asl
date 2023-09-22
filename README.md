# asl
Audio encoding and analysis tool

![Build status](https://github.com/amdrozdov/asl/actions/workflows/build.yml/badge.svg)

![AudiooSignal](https://upload.wikimedia.org/wikipedia/commons/thumb/b/b8/Quantization_error.png/1200px-Quantization_error.png)
This project is currently my way to learn digital audio encodings. But it may be useful for quick tasks in speech recognition domain (for instance: quick slicing, channels split or PBX decoding).

### Features list
* Basic audio data information
* Linear PCM wave decoder
* u-law decoder
* a-law decoder
* Audio slicing (from any format to LPCM wave)
* Channel splitting (from any format to LPCM wave)
* Advanced audio analysis (soon)

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
