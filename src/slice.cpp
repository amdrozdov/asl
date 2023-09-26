// Copyright 2023 Andrei Drozdov

#include "./slice.h"  // NOLINT [build/include]

#include <stdint.h>
#include <string>
#include <iostream>
#include <vector>
#include <cstdio>
#include <cassert>
#include <map>

using namespace std;  // NOLINT [build/namespaces]

map<int16_t, string> AudioSlicer::format_mapping = {
    {CT_LPCM, "Linear PCM"},
    {CT_MSADPCM, "Microsoft ADPCM"},
    {CT_IEEEFP, "IEEE floating-point"},
    {CT_IBM_CVSD, "IBM CVSD"},
    {CT_MS_ALAW, "Microsoft ALAW (8-bit ITU-T G.711BA ALAW)"},
    {CT_MS_MLAW, "Microsoft M-LAW (8-bit ITU-T G.711 M-LAW"},
    {0x11, "Intel IMA/DVI ADPCM"},
    {0x16, "ITU G.723 ADPCM"},
    {0x17, "Dialogic OKI ADPCM"},
    {0x30, "Dolby AAC"},
    {0x31, "Microsoft GSM 6.10"},
    {0x36, "Rockwell ADPCM"},
    {0x40, "ITU G.721 ADPCM"},
    {0x42, "Microsoft MSG723"},
    {0x45, "ITU-T G.726"},
    {0x64, "APICOM G.726 ADPCM"},
    {0x101, "IBM M-LAW"},
    {0x102, "IBM A-LAW"},
    {0x103, "IBM ADPCM"}
};

string AudioSlicer::audio_format() {
    string res = std::to_string(this->header.AudioFormat) + " (";
    res += this->format_prefix + ")";
    return res;
}

void AudioSlicer::init(const string& fname) {
    this->filename = fname;
    FILE* wavFile = this->read_header();
    fclose(wavFile);
    int format = this->header.AudioFormat;
    if (format == CT_MS_MLAW || format == CT_MS_ALAW) {
        wavFile = this->read_ulaw_header();
        fclose(wavFile);
    }
    this->is_verbose = false;

    this->codecs = {
        {CT_LPCM, &AudioSlicer::lpcm_decoder},
        {CT_MS_MLAW, &AudioSlicer::mu_law_decoder},
        {CT_MS_ALAW, &AudioSlicer::a_law_decoder}
    };
}

void AudioSlicer::load_channels(char *buf) {
    int bytes_per_sample = this->header.bitsPerSample / 8.0;
    map<int, int> ch_pos = map<int, int>();
    this->channels = vector<char*>();
    for (int i=0; i < this->header.NumOfChan; i++) {
        this->channels.push_back(
            reinterpret_cast<char*>(malloc(
                this->num_samples * this->header.bitsPerSample / 8.0)));
        ch_pos[i] = 0;
    }

    int ch = 0;
    for (int i=0; i < this->header.Subchunk2Size; i+=bytes_per_sample) {
        for (int j=0; j < bytes_per_sample; j++) {
            this->channels[ch][ch_pos[ch]++] = buf[i+j];
        }
        ch = (ch+1) % (this->channels.size());
    }
    free(buf);
}

void AudioSlicer::lpcm_decoder() {
    FILE* wavFile = this->read_header();
    char *buf = reinterpret_cast<char*>(malloc(this->header.Subchunk2Size));
    size_t was_rd = fread(buf, 1, this->header.Subchunk2Size, wavFile);
    assert(was_rd);
    fclose(wavFile);

    this->load_channels(buf);
}

void AudioSlicer::a_law_decoder() {
    FILE* wavFile = this->read_ulaw_header();
    char *buf = reinterpret_cast<char*>(
        malloc(this->header.Subchunk2Size));
    size_t was_rd = fread(buf, 1, this->header.Subchunk2Size, wavFile);
    assert(was_rd);

    int16_t *decoded_buf = reinterpret_cast<int16_t*>(
        malloc(this->header.Subchunk2Size*2));

    int8_t tmp = 0;
    int8_t segment = 0;
    int8_t sign = 0;
    int16_t decoded = 0;
    for (int i = 0; i < this->header.Subchunk2Size; i++) {
        // invert even bits of sample (0x0005 -> 0b101)
        tmp = buf[i] ^ 0x55;
        // get first bit
        sign = (tmp & 0x80) >> 7;
        // get the data
        decoded = ((tmp & 0x000f) << 1) | 0x0001;
        // get the segment bits data
        segment = (tmp & 0x0070) >> 4;
        // Update segment boundaries
        if ((segment - 1) == 0) {
            decoded |= 0x0020;
        } else if ((segment - 1) > 0) {
            decoded |= 0x0020;
            decoded = decoded << (segment - 1);
        }
        // Remove segment data
        decoded = decoded << 3;
        // Set sign
        if (sign) {
            decoded = 0-decoded;
        }

        // Store the result sample
        decoded_buf[i] = decoded;
    }

    char *result_buf = reinterpret_cast<char*>(decoded_buf);

    // Set audio format Liner PCM
    this->header.AudioFormat = CT_LPCM;
    // Set decoded sizes 8bit -> 16 bit
    this->header.bitsPerSample *= 2;
    this->header.bytesPerSec *= 2;
    this->header.Subchunk2Size *= 2;
    // Set LPCM header size
    this->header.Subchunk1Size = 0x10;

    this->load_channels(result_buf);
}

void AudioSlicer::mu_law_decoder() {
    FILE* wavFile = this->read_ulaw_header();
    char *buf = reinterpret_cast<char*>(
        malloc(this->header.Subchunk2Size));
    size_t was_rd = fread(buf, 1, this->header.Subchunk2Size, wavFile);
    assert(was_rd);

    // allocate decoded space 8 bit -> 16 bit = size * 2
    int16_t *decoded_buf = reinterpret_cast<int16_t*>(
        malloc(this->header.Subchunk2Size*2));
    int16_t decoded = 0;
    int16_t sign = 0;
    int16_t segment = 0;
    for (int i = 0; i < this->header.Subchunk2Size; i++) {
        // invert sample
        decoded = ~buf[i] & 0x00FF;
        // get first bit (0x80 -> 0b10000000) + shift 7 = first bit
        sign = (buf[i] & 0x80) >> 7;
        // get segment 2,3,4 bits, 0x0070 = 0b111000 + shift 4 = 111
        segment = (decoded & 0x0070) >> 4;
        // get last 4 bits 0x000f = 0b1111
        decoded = (decoded & 0x000f) << 1;

        // The value 33 is the amount the end- points
        // of the segments are offset from even powers of two.
        // 0x0021 = 33
        decoded += 0x0021;
        // shift by segment and apply sign
        decoded = decoded << segment;
        if (sign) {
            decoded -= 0x0021;
        } else {
            decoded = 0x0021 - decoded;
        }
        // normalize to 16 bit
        decoded = decoded << 2;

        // write decoded sample
        decoded_buf[i] = decoded;
    }

    char *result_buf = reinterpret_cast<char*>(decoded_buf);

    // Set audio format Liner PCM
    this->header.AudioFormat = CT_LPCM;
    // Set decoded sizes 8bit -> 16 bit
    this->header.bitsPerSample *= 2;
    this->header.bytesPerSec *= 2;
    this->header.Subchunk2Size *= 2;
    // Set LPCM header size
    this->header.Subchunk1Size = 0x10;

    this->load_channels(result_buf);
}

AudioSlicer::AudioSlicer(const string& fname) {
    this->init(fname);
}

AudioSlicer::AudioSlicer(const string& fname, bool is_verbose) {
    this->init(fname);
    this->is_verbose = is_verbose;
}

FILE* AudioSlicer::read_header() {
    this->header = wav_header{};

    FILE* wavFile = fopen(this->filename.c_str(), "r");
    if (wavFile == nullptr) {
        cout << "Unable to read wave file: " << this->filename << endl;
    }
    assert(wavFile != nullptr);

    size_t was_read = fread(&header, 1, sizeof(wav_header), wavFile);
    assert(was_read);
    int bytes_per_sample = this->header.bitsPerSample / 8.0;

    this->num_samples = static_cast<double>(
        header.Subchunk2Size) / static_cast<double>(
        (header.NumOfChan) * bytes_per_sample);
    this->format_prefix = "Unknown";
    if (this->format_mapping.find(
            header.AudioFormat) != this->format_mapping.end()) {
        this->format_prefix = this->format_mapping[header.AudioFormat];
    }
    this->duration = this->num_samples / static_cast<double>(
        this->header.SamplesPerSec);
    return wavFile;
}

FILE* AudioSlicer::read_ulaw_header() {
    // Read modified structure for MS u-law and a-law format
    // Probably need to refactor it
    // Explanation: MS a-law and mu-law formats
    // are extended with fact section
    this->header = wav_header{};

    ulaw_header head = ulaw_header{};

    FILE* wavFile = fopen(this->filename.c_str(), "r");
    if (wavFile == nullptr) {
        cout << "Unable to read wave file: " << this->filename << endl;
    }
    assert(wavFile != nullptr);

    size_t was_read = fread(&head, 1, sizeof(ulaw_header), wavFile);
    assert(was_read);

    this->header.RIFF[0] = head.RIFF[0];
    this->header.RIFF[1] = head.RIFF[1];
    this->header.RIFF[2] = head.RIFF[2];
    this->header.RIFF[3] = head.RIFF[3];
    this->header.ChunkSize = head.ChunkSize;
    this->header.WAVE[0] = head.WAVE[0];
    this->header.WAVE[1] = head.WAVE[1];
    this->header.WAVE[2] = head.WAVE[2];
    this->header.WAVE[3] = head.WAVE[3];

    this->header.fmt[0] = head.fmt[0];
    this->header.fmt[1] = head.fmt[1];
    this->header.fmt[2] = head.fmt[2];
    this->header.fmt[3] = head.fmt[3];

    this->header.Subchunk1Size = head.Subchunk1Size;
    this->header.AudioFormat = head.AudioFormat;
    this->header.NumOfChan = head.NumOfChan;
    this->header.SamplesPerSec = head.SamplesPerSec;
    this->header.bytesPerSec = head.bytesPerSec;
    this->header.blockAlign = head.blockAlign;
    this->header.bitsPerSample = head.bitsPerSample;

    this->header.Subchunk2ID[0] = head.Subchunk2ID[0];
    this->header.Subchunk2ID[1] = head.Subchunk2ID[1];
    this->header.Subchunk2ID[2] = head.Subchunk2ID[2];
    this->header.Subchunk2ID[3] = head.Subchunk2ID[3];
    this->header.Subchunk2Size = head.Subchunk2Size;

    int bytes_per_sample = this->header.bitsPerSample / 8.0;

    this->num_samples = static_cast<double>(
        header.Subchunk2Size) / static_cast<double>(
        (header.NumOfChan) * bytes_per_sample);
    this->format_prefix = "Unknown";
    if (this->format_mapping.find(
            header.AudioFormat) != this->format_mapping.end()) {
        this->format_prefix = this->format_mapping[header.AudioFormat];
    }
    this->duration = this->num_samples / static_cast<double>(
        this->header.SamplesPerSec);
    return wavFile;
}

void AudioSlicer::read_audio() {
    if (this->codecs.find(this->header.AudioFormat) == this->codecs.end()) {
        cout << "Unsupported format ";
        cout << this->format_mapping[header.AudioFormat] << endl;
        exit(1);
    }

    // Load required codec and call it (class method pointer)
    f_ptr codec = this->codecs[header.AudioFormat];
    (this->*codec)();
}

void AudioSlicer::extract_audio(chunk slice) {
    // Do not forget bitsPerSample
    // we need to write bps/8 bytes per sample
    int byte_per_sec = this->header.bitsPerSample / 8;
    int start = slice.sec_start * this->header.SamplesPerSec* byte_per_sec;
    int end = slice.sec_end * this->header.SamplesPerSec* byte_per_sec;
    int total_bytes = (end-start) * this->channels.size();

    char * data = reinterpret_cast<char*>(malloc(total_bytes));
    int p = 0;

    // wav format
    // [1b 1b] <- sample 1 ch 1, [1b 1b] sample 1 ch 2, ...
    // l11 l12 r11 r12 l21 l22 r21 r22
    int ptr = 0;
    while (p <= total_bytes) {
        for (int i=0; i < this->channels.size(); i++) {
            for (int b=0; b < byte_per_sec; b++) {
                data[p++] = channels[i][start+ptr+b];
            }
        }
        ptr+=byte_per_sec;
    }

    wav_header new_header = this->header;
    new_header.Subchunk2Size = total_bytes;

    FILE* wavFile = fopen(slice.filename.c_str(), "wb");
    if (wavFile == nullptr) {
        cout << "Unable to read wave file: " << this->filename << endl;
    }
    assert(wavFile != nullptr);
    fwrite(&new_header, 1, sizeof(new_header), wavFile);
    fwrite(data, 1, total_bytes, wavFile);
    fclose(wavFile);

    free(data);
    if (this->is_verbose) {
        cout << "Extracted interval [" << slice.sec_start << ":";
        cout << slice.sec_end << "] into '" << slice.filename << "'" << endl;
    }
}

void AudioSlicer::split_channels(string out_prefix) {
    this->read_audio();
    int byte_per_sec = this->header.bitsPerSample / 8;
    for (int i=0; i < this->channels.size(); i++) {
        wav_header new_header = this->header;
        new_header.Subchunk2Size = this->num_samples * byte_per_sec;
        new_header.NumOfChan = 1;

        string fname = out_prefix + std::to_string(i) + ".wav";
        FILE* wavFile = fopen(fname.c_str(), "wb");
        if (wavFile == nullptr) {
            cout << "Unable to read wave file: " << this->filename << endl;
        }
        assert(wavFile != nullptr);

        fwrite(&new_header, 1, sizeof(new_header), wavFile);
        fwrite(this->channels[i], 1, this->num_samples * byte_per_sec, wavFile);
        fclose(wavFile);
        if (this->is_verbose) {
            cout << "Extracted channel " << i << " into '";
            cout << out_prefix << i << ".wav'" << endl;
        }
    }
}

void AudioSlicer::slice(vector<chunk> chunks) {
    this->read_audio();
    for (int i=0; i < chunks.size(); i++) {
        assert(chunks[i].sec_start >= 0);
        assert(chunks[i].sec_end <= static_cast<int>(this->duration) + 1);
        this->extract_audio(chunks[i]);
    }
}
