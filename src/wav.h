typedef struct WAV_HEADER{    
	char RIFF[4];            // RIFF section identifier "RIFF"
	int ChunkSize;           // size of section
	char WAVE[4];            // Format identifier "WAVE"
	char fmt[4];             // "fmt" section
	int Subchunk1Size;       // size of fmt section
	short int AudioFormat;   // encoding format
	short int NumOfChan;     // #of channels in the audio (mono/stereo)
	int SamplesPerSec;       // sample rate Hz
	int bytesPerSec;         // bytes oer second
	short int blockAlign;    // alignement
	short int bitsPerSample; // bit depth (8/16/etc bits per sample)
	char Subchunk2ID[4];     // data section
	int Subchunk2Size;       // Size of the data section
} wav_header;

