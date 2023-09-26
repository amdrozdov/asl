#include "gtest/gtest.h"
#include "slice.h"
#include <vector>
#include <utility>

#include <stdio.h>
#include <stdlib.h>
#include <cassert>

#include "./aux.h"


namespace {
    const std::string test_file = "../samples/addf8-Alaw-GW.wav";
    const std::string test_format = "6 (Microsoft ALAW (8-bit ITU-T G.711BA ALAW))";

    TEST(AudioAlawFormatTest, TestRead) {
        auto as = AudioSlicer(test_file);
        EXPECT_TRUE(as.Filename() == test_file);
        EXPECT_EQ(as.Size(), 23808);
        EXPECT_EQ(as.audio_format(), test_format);
        EXPECT_EQ(as.SampleRate(), 8000);
        EXPECT_EQ(as.BitsPerSample(), 8);
        EXPECT_EQ(as.Channels(), 1);
        EXPECT_EQ(as.NumSamples(), 23808);
        EXPECT_EQ(as.Duration(), 2.976);
    }
    
    TEST(AudioALawFormatTest, TestSlice) {
        // Checking mono recording
        std::vector<chunk> slices = {
            chunk{1, 2, "test_alaw_one.wav"}
        };
        auto as = AudioSlicer(test_file, true);
        EXPECT_EQ(as.Channels(), 1);
        as.slice(slices);
        // Binary compare the result with expected files
        EXPECT_TRUE(compare(
            "test_alaw_one.wav", "../tests/expected/test_alaw_one.wav"));
    }
}
