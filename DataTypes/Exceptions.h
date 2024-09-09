#ifndef EXCEPTIONS
#define EXCEPTIONS

#ifndef STDEXCEPTION
#define STDEXCEPTION
#include <exception>
#endif

#ifndef STDSTRING
#define STDSTRING
#include <string>
#endif

class ConfigParseException : std::exception {
    private:
        std::string message;
    public:
        ConfigParseException(std::string input, std::string datatype) {
            this->message = "Error! could not convert " + input + " to " + datatype;
        }

        const char* what() const noexcept override {
            return message.c_str();
        }
};

class InvalidConfigValueException : std::exception {
    private:
        std::string message;
    public:
        InvalidConfigValueException(std::string input, std::string valid_values) {
            this->message = "Error! " + input + " is not a valid value. Value must be in the following set: " + valid_values;
        }

        const char* what() const noexcept override {
            return message.c_str();
        }
};

#endif