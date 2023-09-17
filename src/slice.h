#include <string>
#include <vector>
#include <map>
#include "wav.h"

typedef struct {
	int sec_start;
	int sec_end;
        std::string filename;
} chunk;

class AudioSlicer{
	private:
		bool is_verbose;
		wav_header header;
		std::string filename;
		std::string format_prefix;
		double num_samples;
		std::vector<char*> channels;
		std::map<short int, std::string> format_mapping;
		double duration;

		void load_formats();
		void read();
		void extract_audio(chunk slice);
		void init(const std::string& fname);
	public:
		AudioSlicer(const std::string& fname);
		AudioSlicer(const std::string& fname, bool is_verbose);
		int BytesPerSec() { return this->header.bytesPerSec; };
		int SampleRate() { return this->header.SamplesPerSec; };
		std::string audio_format();
		inline long NumSamples() { return long(num_samples); };
		inline long Size() { return long(this->header.Subchunk2Size); };
		inline int BitsPerSample() { return int(this->header.bitsPerSample); };
		inline double Duration() { return this->duration; };
		inline std::string Filename() { return this->filename; };
		inline int Channels() { return this->channels.size(); };
		void slice(std::vector<chunk> chunks);
		void split_channels(std::string out_prefix);
};
