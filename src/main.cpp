#include <iostream>
#include <chrono>
#include <vector>

#include <argparse/argparse.hpp>

#include "slice.h"

using namespace std;


void info(string filename, bool is_verbose){
	cout << "Loading..." << endl;
	auto start = chrono::steady_clock::now();
	auto as = AudioSlicer(filename, is_verbose);
	auto cnt = chrono::duration_cast<chrono::milliseconds>(
		chrono::steady_clock::now() - start);
	cout << "Analysis time = " << cnt.count() << " ms\n";

	cout << "Filename: " << as.Filename() << endl;
	cout << "Size: " << as.Size() <<endl;
	cout << "Audio Format: " << as.audio_format() <<endl;
	cout << "Sample rate: " << as.SampleRate() << endl;
	cout << "BitsPerSample: " << as.BitsPerSample() << endl;
	cout << "Channels: " << as.Channels() <<endl;
	cout << "Num samples: " << as.NumSamples() <<endl;
	cout << "Duration: " << as.Duration() << " sec" << endl;
}

void split(string filename, string prefix, bool is_verbose){
	cout << "Loading..." << endl;
	auto as = AudioSlicer(filename, is_verbose);
	auto start = chrono::steady_clock::now();
	as.split_channels(prefix);
	auto cnt = chrono::duration_cast<chrono::milliseconds>(
		chrono::steady_clock::now() - start);
	cout << "Split time = " << cnt.count() << " ms\n";
}

void slice(string filename, vector<chunk> slices, bool is_verbose){
	auto as = AudioSlicer(filename, is_verbose);
	auto start = chrono::steady_clock::now();
	as.slice(slices);
	auto cnt = chrono::duration_cast<chrono::milliseconds>(
		chrono::steady_clock::now() - start);
	cout << "Extraction time = " << cnt.count() << " ms\n";
}

int main(int argc, char* argv[]){
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

	if(program.is_subcommand_used("info")) {
 	       auto input = program.at<argparse::ArgumentParser>(
			"info").get<string>("--file");
	       info(input, is_verbose);

	} else if(program.is_subcommand_used("split")){
 	       auto input = program.at<argparse::ArgumentParser>(
			"split").get<string>("--file");
 	       auto prefix = program.at<argparse::ArgumentParser>(
			"split").get<string>("--prefix");
	       split(input, prefix, is_verbose);
	} else if(program.is_subcommand_used("slice")) {
		auto input = program.at<argparse::ArgumentParser>(
			"slice").get<string>("--file");
		auto starts = program.at<argparse::ArgumentParser>(
			"slice").get<vector<int>>("--start");
		auto ends = program.at<argparse::ArgumentParser>(
			"slice").get<vector<int>>("--end");
		auto outs = program.at<argparse::ArgumentParser>(
			"slice").get<vector<string>>("--output");

		if((starts.size() != ends.size()) ||
				(ends.size() != outs.size())){
		       cout << "Number of slice params should be same" << endl;
		       return 1;
		}

		vector<chunk> slices = vector<chunk>();
		for(int i=0;i<starts.size();i++){
			slices.push_back(chunk{starts[i], ends[i], outs[i]});
		}
		slice(input, slices, is_verbose);
	} else {
		cout << program;
		return 0;
	}

	return 0;
}
