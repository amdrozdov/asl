#include <string>
#include <utility>

std::pair<int, char*> read_file(std::string filename) {
    FILE* wavFile = fopen(filename.c_str(), "r");
    assert(wavFile != nullptr);

    fseek(wavFile, 0, SEEK_END);
    long fa_size = ftell(wavFile);
    fseek(wavFile, 0, SEEK_SET);

    char* data = (char*)malloc(fa_size+1);

    fread(data, 1, fa_size, wavFile);
    fclose(wavFile);

    return std::pair<int, char*>{fa_size, data};
}

bool compare(std::string a, std::string b) {
    std::pair<int, char*> file_a = read_file(a);
    std::pair<int, char*> file_b = read_file(b);

    if (file_a.first != file_b.first) {
        std::cout << "Wrong size assertion:";
        std::cout << " " << file_a.first << " vs";
        std::cout << " " << file_b.first << std::endl;
        return false;
    }

    for (int i=0; i<file_a.first; i++){
        if (file_a.second[i] != file_b.second[i]){
            std::cout << "Wrong byte assertion at " << i << std::endl;
            return false;
        }
    }
    return true;
}
