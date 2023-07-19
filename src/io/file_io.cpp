#include "file_io.hpp"


// TODO: possibly consider lazily loading the file instead to save memory. This is especially important for large files being compiled with limited RAM.
std::string read_file_into_string(const char *const filename) {
    // TODO: double check the effciiency of this for large files (compared to traditional fopoen and fseek implementation in C)

    try {
        std::ifstream file(filename);
        std::stringstream buffer;

        buffer << file.rdbuf();

        return buffer.str();
    } catch(const std::ifstream::failure&) {
        // we should report this error (besides a stack trace) to the user since mistyping a filename is a common user error
        std::cout << "Failed to read file: " << filename << '\n';
        throw; // rethrow since this is a fatal error
    }

    // this code should be unreachable; put logic error exception here to prevent UB if this happens somehow
    throw std::logic_error("read_file_into_string: unknown error");
}

void write_string_into_file(const std::string& contents, const char *const filename) {
    std::ofstream out_file(filename);
    out_file << contents;
}


