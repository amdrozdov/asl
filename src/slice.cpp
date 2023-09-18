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


void AudioSlicer::load_formats() {
    this->format_mapping = map<int16_t, string>();
    format_mapping[0x1] = "Linear PCM";
    format_mapping[0x2] = "Microsoft ADPCM";
    format_mapping[0x3] = "IEEE floating-point";
    format_mapping[0x5] = "IBM CVSD";
    format_mapping[0x6] = "Microsoft ALAW (8-bit ITU-T G.711BA ALAW)";
    format_mapping[0x7] = "Microsoft M-LAW (8-bit ITU-T G.711 M-LAW";
    format_mapping[0x11] = "Intel IMA/DVI ADPCM";
    format_mapping[0x16] = "ITU G.723 ADPCM";
    format_mapping[0x17] = "Dialogic OKI ADPCM";
    format_mapping[0x30] = "Dolby AAC";
    format_mapping[0x31] = "Microsoft GSM 6.10";
    format_mapping[0x36] = "Rockwell ADPCM";
    format_mapping[0x40] = "ITU G.721 ADPCM";
    format_mapping[0x42] = "Microsoft MSG723";
    format_mapping[0x45] = "ITU-T G.726";
    format_mapping[0x64] = "APICOM G.726 ADPCM";
    format_mapping[0x101] = "IBM M-LAW";
    format_mapping[0x102] = "IBM A-LAW";
    format_mapping[0x103] = "IBM ADPCM";
}

string AudioSlicer::audio_format() {
    string res = std::to_string(this->header.AudioFormat) + " (";
    res += this->format_prefix + ")";
    return res;
}

void AudioSlicer::init(const string& fname) {
    this->filename = fname;
    this->load_formats();
    this->read();
    this->is_verbose = false;
}

AudioSlicer::AudioSlicer(const string& fname) {
    this->init(fname);
}

AudioSlicer::AudioSlicer(const string& fname, bool is_verbose) {
    this->init(fname);
    this->is_verbose = is_verbose;
}

void AudioSlicer::read() {
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

    this->channels = vector<char*>();
    map<int, int> ch_pos = map<int, int>();
    for (int i=0; i < this->header.NumOfChan; i++) {
        this->channels.push_back(
            reinterpret_cast<char*>(malloc(
                this->num_samples * bytes_per_sample)));
        ch_pos[i] = 0;
    }
    char *buf = reinterpret_cast<char*>(malloc(this->header.Subchunk2Size));
    size_t was_rd = fread(buf, 1, this->header.Subchunk2Size, wavFile);
    assert(was_rd);

    int ch = 0;
    for (int i=0; i < this->header.Subchunk2Size; i+=bytes_per_sample) {
        this->channels[ch][ch_pos[ch]++] = buf[i];
        this->channels[ch][ch_pos[ch]++] = buf[i+1];
        ch = (ch+1) % (this->channels.size());
    }

    free(buf);
    fclose(wavFile);
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
    fwrite(data, 1, total_bytes * this->channels.size(), wavFile);
    fclose(wavFile);

    free(data);
    if (this->is_verbose) {
        cout << "Extracted interval [" << slice.sec_start << ":";
        cout << slice.sec_end << "] into '" << slice.filename << "'" << endl;
    }
}

void AudioSlicer::split_channels(string out_prefix) {
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
    for (int i=0; i < chunks.size(); i++) {
        assert(chunks[i].sec_start >= 0);
        assert(chunks[i].sec_end <= static_cast<int>(this->duration) + 1);
        this->extract_audio(chunks[i]);
    }
}
