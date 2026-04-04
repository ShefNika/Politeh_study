#ifndef MYSTRINGEXCEPTION
#define MYSTRINGEXCEPTION
#include <exception>
#include <cstring>

class MyStringError : public std::exception {
private:
    char message[128];

public:
    MyStringError(const char* msg) noexcept {
        strncpy(message, msg ? msg : "Unknown error", 127);
        message[127] = '\0';
    }

    const char* what() const noexcept override {
        return message;
    }
};

#endif