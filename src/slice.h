// Copyright 2023 Andrei Drozdov

#ifndef SRC_SLICE_H_
#define SRC_SLICE_H_

#include <stdint.h>
#include <cstdio>

#include <string>
#include <vector>
#include <map>

#include "./formats/wav.h"
#include "./formats/ulaw.h"

typedef struct {
    int sec_start;
    int sec_end;
        std::string filename;
} chunk;

const int16_t CT_LPCM = 0x1;
const int16_t CT_MSADPCM = 0x2;
const int16_t CT_IEEEFP = 0x3;
const int16_t CT_IBM_CVSD = 0x5;
const int16_t CT_MS_ALAW = 0x6;
const int16_t CT_MS_MLAW = 0x7;


class AudioSlicer{
 private:
        // Class method pointer type f_ptr
        typedef void(AudioSlicer::*f_ptr)();
        // dict(id, func_ptr)
        std::map<int16_t, f_ptr > codecs;

        static std::map<int16_t, std::string> format_mapping;
        bool is_verbose;
        wav_header header;
        std::string filename;
        std::string format_prefix;
        double num_samples;
        std::vector<char*> channels;
        double duration;

        void lpcm_decoder();
        void mu_law_decoder();
        void read_audio();
        FILE* read_header();
        FILE* read_ulaw_header();
        void load_channels(char *buf);
        void extract_audio(chunk slice);
        void init(const std::string& fname);

 public:
        explicit AudioSlicer(const std::string& fname);
        AudioSlicer(const std::string& fname, bool is_verbose);
        int BytesPerSec() { return this->header.bytesPerSec; }
        int SampleRate() { return this->header.SamplesPerSec; }
        std::string audio_format();
        inline int64_t NumSamples() { return int64_t(num_samples); }
        inline int64_t Size() { return int64_t(this->header.Subchunk2Size); }
        inline int BitsPerSample() {
            return int32_t(this->header.bitsPerSample); }
        inline double Duration() { return this->duration; }
        inline std::string Filename() { return this->filename; }
        inline int Channels() { return this->header.NumOfChan; }
        void slice(std::vector<chunk> chunks);
        void split_channels(std::string out_prefix);
};

#endif  // SRC_SLICE_H_
