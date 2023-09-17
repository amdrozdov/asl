// Copyright 2023 Andrei Drozdov

#ifndef SRC_WAV_H_
#define SRC_WAV_H_

#include <stdint.h>

typedef struct WAV_HEADER {
    char RIFF[4];             // RIFF section identifier "RIFF"
    int ChunkSize;            // size of section
    char WAVE[4];             // Format identifier "WAVE"
    char fmt[4];              // "fmt" section
    int Subchunk1Size;        // size of fmt section
    int16_t AudioFormat;    // encoding format
    int16_t NumOfChan;      // #of channels in the audio (mono/stereo)
    int SamplesPerSec;        // sample rate Hz
    int bytesPerSec;          // bytes oer second
    int16_t blockAlign;     // alignement
    int16_t bitsPerSample;  // bit depth (8/16/etc bits per sample)
    char Subchunk2ID[4];      // data section
    int Subchunk2Size;        // Size of the data section
} wav_header;

#endif  // SRC_WAV_H_
