#include <cstdlib>
#include <iostream>
#include <string>

#include "io_data.h"
#include "muxer_core.h"

static void usage(const char *program_name) {
    std::cout << "usage: " << std::string(program_name) << " input_file output_video_file output_audio_file" << std::endl;
}

int main(int argc, char **argv) {
    if(argc < 4) {
        usage(argv[0]);
        return 1;
    }

    do {
        int32_t result = init_muxer(argv[1], argv[2], argv[3]);
        if(result < 0) {
            break;
        }
        result = muxing();
    } while(0);

    destroy_muxer();
    return 0;
}