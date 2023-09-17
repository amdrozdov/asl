// Copyright 2023 Andrei Drozdov

#include <iostream>
#include <chrono>  // NOLINT [build/c++11]`
#include <vector>

#include <argparse/argparse.hpp>

#include "./slice.h"


void info(std::string filename, bool is_verbose) {
    std::cout << "Loading..." << std::endl;
    auto start = std::chrono::steady_clock::now();
    auto as = AudioSlicer(filename, is_verbose);
    auto cnt = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);
    std::cout << "Analysis time = " << cnt.count() << " ms\n";

    std::cout << "Filename: " << as.Filename() << std::endl;
    std::cout << "Size: " << as.Size() <<std::endl;
    std::cout << "Audio Format: " << as.audio_format() <<std::endl;
    std::cout << "Sample rate: " << as.SampleRate() << std::endl;
    std::cout << "BitsPerSample: " << as.BitsPerSample() << std::endl;
    std::cout << "Channels: " << as.Channels() <<std::endl;
    std::cout << "Num samples: " << as.NumSamples() <<std::endl;
    std::cout << "Duration: " << as.Duration() << " sec" << std::endl;
}

void split(std::string filename, std::string prefix, bool is_verbose) {
    std::cout << "Loading..." << std::endl;
    auto as = AudioSlicer(filename, is_verbose);
    auto start = std::chrono::steady_clock::now();
    as.split_channels(prefix);
    auto cnt = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);
    std::cout << "Split time = " << cnt.count() << " ms\n";
}

void slice(std::string filename, std::vector<chunk> slices, bool is_verbose) {
    auto as = AudioSlicer(filename, is_verbose);
    auto start = std::chrono::steady_clock::now();
    as.slice(slices);
    auto cnt = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);
    std::cout << "Extraction time = " << cnt.count() << " ms\n";
}

int main(int argc, char* argv[]) {
    argparse::ArgumentParser program("asl - Audio SLicer");
    program.add_argument("--verbose")
        .help("Enable verbose mode")
        .default_value(false)
        .implicit_value(true);
    argparse::ArgumentParser cmd_info("info");
    cmd_info.add_description("Get audio file information");
    cmd_info.add_argument("-f", "--file")
        .required()
        .help("Input audio file");

    argparse::ArgumentParser cmd_split("split");
    cmd_split.add_description("Split audio channels into separate files");
    cmd_split.add_argument("-f", "--file")
        .required()
        .help("Input audio file");
    cmd_split.add_argument("-p", "--prefix")
        .required()
        .help("Output filename prefix");

    argparse::ArgumentParser cmd_slice("slice");
    cmd_slice.add_argument("-f", "--file")
        .required()
        .help("Input audio file");
    cmd_slice.add_argument("-s", "--start")
        .help("Beginnig of the slice in seconds")
        .required()
        .scan<'i', int>()
        .nargs(argparse::nargs_pattern::at_least_one);
    cmd_slice.add_argument("-e", "--end")
        .help("End of the slice in seconds")
        .required()
        .scan<'i', int>()
        .nargs(argparse::nargs_pattern::at_least_one);
    cmd_slice.add_argument("-o", "--output")
        .required()
        .nargs(argparse::nargs_pattern::at_least_one)
        .help("Output filename");

    program.add_subparser(cmd_info);
    program.add_subparser(cmd_split);
    program.add_subparser(cmd_slice);

    try {
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    bool is_verbose = program.get<bool>("--verbose");

    if (program.is_subcommand_used("info")) {
            auto input = program.at<argparse::ArgumentParser>(
            "info").get<std::string>("--file");
           info(input, is_verbose);

    } else if (program.is_subcommand_used("split")) {
            auto input = program.at<argparse::ArgumentParser>(
            "split").get<std::string>("--file");
            auto prefix = program.at<argparse::ArgumentParser>(
            "split").get<std::string>("--prefix");
           split(input, prefix, is_verbose);
    } else if (program.is_subcommand_used("slice")) {
        auto input = program.at<argparse::ArgumentParser>(
            "slice").get<std::string>("--file");
        auto starts = program.at<argparse::ArgumentParser>(
            "slice").get<std::vector<int>>("--start");
        auto ends = program.at<argparse::ArgumentParser>(
            "slice").get<std::vector<int>>("--end");
        auto outs = program.at<argparse::ArgumentParser>(
            "slice").get<std::vector<std::string>>("--output");

        if ((starts.size() != ends.size()) ||
                (ends.size() != outs.size())) {
               std::cout << "Number of slice params should be same";
               std::cout << std::endl;
               return 1;
        }

        std::vector<chunk> slices = std::vector<chunk>();
        for (int i=0; i < starts.size(); i++) {
            slices.push_back(chunk{starts[i], ends[i], outs[i]});
        }
        slice(input, slices, is_verbose);
    } else {
        std::cout << program;
        return 0;
    }

    return 0;
}
