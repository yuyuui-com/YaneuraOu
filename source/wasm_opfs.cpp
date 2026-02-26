#include <emscripten/wasmfs.h>
#include <emscripten.h>
#include <iostream>
#include "misc.h"

__attribute__((constructor))
void setup_wasmfs_opfs() {
    auto backend = wasmfs_create_opfs_backend();

    int res = wasmfs_create_directory("/opfs", 0777, backend);
    
    if (res == 0) {
        std::cout << "[WasmFS] OPFS successfully mounted at /opfs" << std::endl;
    } else {
        std::cerr << "[WasmFS] Failed to mount OPFS" << std::endl;
    }
}

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    int usi_command(const char* cmd) {
        YaneuraOu::CommandLine::g.add_command(cmd);        
        return 0; 
    }
}
