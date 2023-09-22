// Copyright 2023 Andrei Drozdov

#ifndef SRC_FORMATS_ULAW_H_
#define SRC_FORMATS_ULAW_H_

#include <stdint.h>

// Based on
// https://www.dsprelated.com/showthread/speechcoding/601-1.php
// Works onnly for MS Ulaw
typedef struct ULAW_HEADER {
    char RIFF[4];             // RIFF section identifier "RIFF"
    int ChunkSize;            // size of section
    char WAVE[4];             // Format identifier "WAVE"
    char fmt[4];              // "fmt" section
    int Subchunk1Size;        // size of fmt section
    int16_t AudioFormat;      // encoding format
    int16_t NumOfChan;        // #of channels in the audio (mono/stereo)
    int SamplesPerSec;        // sample rate Hz
    int bytesPerSec;          // bytes per second
    int16_t blockAlign;       // alignement
    int16_t bitsPerSample;    // bit depth (8/16/etc bits per sample)

    int16_t extra_params;     // extra mlaw padding
    char FACT[4];             // fact section
    int16_t fact_size;        // section size
    int32_t fact_data;        // section data
    int16_t fact_extra;       // extra mlaw padding
                              //
    char Subchunk2ID[4];      // data section
    int16_t Subchunk2Size;    // Size of the data section
} ulaw_header;

#endif  // SRC_FORMATS_ULAW_H_
