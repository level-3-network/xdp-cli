// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <vector>
#include <string>

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

class SDKCommon
{
private:
    std::vector<std::string> application_arguments_;

public:
    SDKCommon() = default;
    ~SDKCommon() = default;

    auto& get_application_arguments() const {
        return this->application_arguments_;
    }

    void initialize_application_arguments(int number_of_arguments, char* arguments[]);

    bool application_arguments_contains_flag(const std::string& flag);
    std::string get_application_argument_value(const std::string& argument, const std::string& default_value);

    bool is_numeric_string(const std::string& string);
};
