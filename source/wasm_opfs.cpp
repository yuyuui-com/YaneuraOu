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
        for (char ch : cmd) buffer.push(ch);
        buffer.push('\n');
    }

protected:
    virtual int underflow() override {
        std::lock_guard<std::mutex> lock(mtx);
        if (buffer.empty()) return EOF;
        
        char_buffer = buffer.front();
        setg(&char_buffer, &char_buffer, &char_buffer + 1);
        return char_buffer;
    }

    virtual int uflow() override {
        std::lock_guard<std::mutex> lock(mtx);
        if (buffer.empty()) return EOF;
        
        char ch = buffer.front();
        buffer.pop();
        return ch;
    }

private:
    std::queue<char> buffer;
    std::mutex mtx;
    char char_buffer;
};

static USIStreamBuf g_usi_buf;
static std::streambuf* g_old_cin_buf = nullptr;


__attribute__((constructor))
void setup_wasm_extension() {
    auto backend = wasmfs_create_opfs_backend();
    wasmfs_create_directory("/opfs", 0777, backend);
    std::cout << "[WasmFS] OPFS mounted at /opfs" << std::endl;

    g_old_cin_buf = std::cin.rdbuf(&g_usi_buf);
}


extern "C" {
    EMSCRIPTEN_KEEPALIVE
    int usi_command(const char* cmd) {
        g_usi_buf.push_command(std::string(cmd));
        return 0; 
    }
}
