#include "gtest/gtest.h"
#include "slice.h"
#include <vector>
#include <utility>

#include <stdio.h>
#include <stdlib.h>
#include <cassert>

#include "./aux.h"


namespace {
    const std::string test_file = "../samples/sample.wav";
    const std::string test_file_2ch = "../samples/sample_2ch.wav";
    const std::string test_format = "1 (Linear PCM)";

    TEST(AudioFormatTest, TestRead) {
        auto as = AudioSlicer(test_file);
        EXPECT_TRUE(as.Filename() == test_file);
        EXPECT_EQ(as.Size(), 132300);
        EXPECT_EQ(as.audio_format(), test_format);
        EXPECT_EQ(as.SampleRate(), 22050);
        EXPECT_EQ(as.BitsPerSample(), 16);
        EXPECT_EQ(as.Channels(), 1);
        EXPECT_EQ(as.NumSamples(), 66150);
        EXPECT_EQ(as.Duration(), 3);
    }
    
    TEST(AudioFormatTest, TestSlice) {
        // Checking mono recording
        std::vector<chunk> slices = {
            chunk{0, 1, "test_one.wav"},
            chunk{1, 2, "test_two.wav"}
        };
        auto as = AudioSlicer(test_file, true);
        EXPECT_EQ(as.Channels(), 1);
        as.slice(slices);
        // Binary compare the result with expected files
        EXPECT_TRUE(compare("test_one.wav", "../tests/expected/test_one.wav"));
        EXPECT_TRUE(compare("test_two.wav", "../tests/expected/test_two.wav"));
        
        // Same check for stereo recording
        slices = {
            chunk{0, 1, "test_one_2ch.wav"},
            chunk{1, 2, "test_two_2ch.wav"}
        };
        as = AudioSlicer(test_file_2ch, true);
        EXPECT_EQ(as.Channels(), 2);
        as.slice(slices);
        EXPECT_TRUE(compare("test_one_2ch.wav", "../tests/expected/test_one_2ch.wav"));
        EXPECT_TRUE(compare("test_two_2ch.wav", "../tests/expected/test_two_2ch.wav"));
    }

    TEST(AudioFormatTest, TestSplit) {
        auto as = AudioSlicer(test_file_2ch, true);
        EXPECT_EQ(as.Channels(), 2);

        // Split 2 channels into 2 mono wav files
        as.split_channels("split_test_");

        // 2 channels are different, check it
        EXPECT_TRUE(!compare("split_test_0.wav", "split_test_1.wav"));

        // Validate each channel
        EXPECT_TRUE(compare("split_test_0.wav", "../tests/expected/split_test_0.wav"));
        EXPECT_TRUE(compare("split_test_1.wav", "../tests/expected/split_test_1.wav"));
    }
}
