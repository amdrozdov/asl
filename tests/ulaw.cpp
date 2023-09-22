#include "gtest/gtest.h"
#include "slice.h"
#include <vector>
#include <utility>

#include <stdio.h>
#include <stdlib.h>
#include <cassert>

#include "./aux.h"


namespace {
    const std::string test_file = "../samples/addf8-mulaw-GW.wav";
    const std::string test_format = "7 (Microsoft M-LAW (8-bit ITU-T G.711 M-LAW)";

    TEST(AudioUlawFormatTest, TestRead) {
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
    
    TEST(AudioULawFormatTest, TestSlice) {
        // Checking mono recording
        std::vector<chunk> slices = {
            chunk{1, 2, "test_ulaw_one.wav"}
        };
        auto as = AudioSlicer(test_file, true);
        EXPECT_EQ(as.Channels(), 1);
        as.slice(slices);
        // Binary compare the result with expected files
        EXPECT_TRUE(compare(
            "test_ulaw_one.wav", "../tests/expected/test_ulaw_one.wav"));

        // Test stereo recording
        // FIXME: find ulaw stereo recording for testing
    }

    TEST(AudioULawFormatTest, TestSplit) {
        // FIXME TODO Write the test
        // FIXME: find ulaw stereo recording for testing
    }
}
