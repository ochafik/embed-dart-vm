#ifndef _DART_JVM_STRING_FILE_H
#define _DART_JVM_STRING_FILE_H

#include <fstream>
#include <memory>

/**
 * Simple file structure.
 */
struct File {
  std::shared_ptr<std::ofstream> out_;
  std::shared_ptr<std::ifstream> in_;
  File(std::shared_ptr<std::ofstream> out, std::shared_ptr<std::ifstream> in) :
      out_(out), in_(in) {}

  void read(const uint8_t** data, intptr_t* length) {
    std::string content(std::istreambuf_iterator<char>(*in_.get()),
                        std::istreambuf_iterator<char>());
    *length = content.size() + 1;
    uint8_t* buffer = reinterpret_cast<uint8_t*>(malloc(*length));
    memcpy(buffer, content.c_str(), *length);
    *data = buffer;
  }

  void write(const void* data, intptr_t length) {
    std::string content;
    content.resize(length);
    memcpy(&content[0], data, length + 1);

    *out_ << content;
  }

  static File* openOutput(const std::string& path) {
    std::shared_ptr<std::ofstream> out(new std::ofstream());
    out->open(path);
    return out->is_open() ? new File(out, nullptr) : nullptr;
  }

  static File* openInput(const std::string& path) {
    std::shared_ptr<std::ifstream> in(new std::ifstream());
    in->open(path);
    return in->is_open() ? new File(nullptr, in) : nullptr;
  }
};

#endif  // _DART_JVM_STRING_FILE_H

