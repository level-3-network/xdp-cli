// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#define SDK_COMMON_CONSOLE_TEXT_DEFAULT "\033[0m"
#define SDK_COMMON_CONSOLE_TEXT_BOLD "\033[1m"
#define SDK_COMMON_CONSOLE_TEXT_UNDERSCORE "\033[4m"

#define SDK_COMMON_CONSOLE_TEXT_BLACK "\033[30m"
#define SDK_COMMON_CONSOLE_TEXT_RED "\033[31m"
#define SDK_COMMON_CONSOLE_TEXT_GREEN "\033[32m"
#define SDK_COMMON_CONSOLE_TEXT_YELLOW "\033[33m"
#define SDK_COMMON_CONSOLE_TEXT_BLUE "\033[34m"
#define SDK_COMMON_CONSOLE_TEXT_MAGENTA "\033[35m"
#define SDK_COMMON_CONSOLE_TEXT_CYAN "\033[36m"
#define SDK_COMMON_CONSOLE_TEXT_WHITE "\033[37m"

class SDK_Common
{
private:
    std::vector<std::string> program_arguments_;

public:
    SDK_Common() = default;
    ~SDK_Common() = default;

    auto& get_program_arguments() const {
        return this->program_arguments_;
    }

    void initialize_program_arguments(int number_of_arguments, char* arguments[])
    {
        if (number_of_arguments > 1) {
            this->program_arguments_.assign(arguments + 1, arguments + number_of_arguments);
        }
    }

    bool program_arguments_contains_flag(const std::string& flag) {
        return std::find(this->program_arguments_.begin(), this->program_arguments_.end(), flag) != this->program_arguments_.end();
    }

    std::string get_program_argument_value(const std::string& argument, const std::string& default_value)
    {
        auto iterator = std::find(this->program_arguments_.begin(), this->program_arguments_.end(), argument);

        if (iterator != this->program_arguments_.end() && std::next(iterator) != this->program_arguments_.end())
        {
            std::string next_value = *std::next(iterator);

            if (next_value.empty() || next_value[0] != '-') {
                return next_value;
            }
        }

        return default_value;
    }

    bool is_numeric_string(const std::string& string)
    {
        return !string.empty() && std::all_of(string.begin(), string.end(), [] (unsigned char character) {
            return std::isdigit(character);
        });
    }
};
