#include <emscripten/wasmfs.h>
#include <emscripten.h>
#include <iostream>
#include <string>
#include <queue>
#include <mutex>
#include <streambuf>


class USIStreamBuf : public std::streambuf {
public:
    void push_command(const std::string& cmd) {
        std::lock_guard<std::mutex> lock(mtx);
        for (char ch : cmd) {
            buffer.push(ch);
        }
        buffer.push('\n');
    }

protected:
    virtual int underflow() override {
        std::lock_guard<std::mutex> lock(mtx);
        if (buffer.empty()) {
            return EOF;
        }
        
        current_char = buffer.front();
        setg(&current_char, &current_char, &current_char + 1);
        return current_char;
    }

    virtual int uflow() override {
        std::lock_guard<std::mutex> lock(mtx);
        if (buffer.empty()) {
            return EOF;
        }
        
        char ch = buffer.front();
        buffer.pop();
        return (unsigned char)ch;
    }

private:
    std::queue<char> buffer;
    std::mutex mtx;
    char current_char;
};

static USIStreamBuf g_usi_buf;


__attribute__((constructor))
void setup_wasm_extension() {
    auto backend = wasmfs_create_opfs_backend();
    int res = wasmfs_create_directory("/opfs", 0777, backend);
    
    if (res == 0) {
        std::cout << "[WasmFS] OPFS successfully mounted at /opfs" << std::endl;
    }

    std::cin.rdbuf(&g_usi_buf);
}

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    int usi_command(const char* cmd) {
        if (!cmd) return -1;
        
        g_usi_buf.push_command(std::string(cmd));
        
        return 0; 
    }
}
