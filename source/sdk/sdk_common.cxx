// SPDX-License-Identifier: GPL-2.0-only

#include <iostream>
#include <algorithm>
#include <string>
#include <vector>

#include "sdk/sdk_common.hxx"

void SDKCommon::initialize_application_arguments(int number_of_arguments, char* arguments[])
{
    if (number_of_arguments > 1) {
        this->application_arguments_.assign(arguments + 1, arguments + number_of_arguments);
    }
}

bool SDKCommon::application_arguments_contains_flag(const std::string& flag) {
    return std::find(this->application_arguments_.begin(), this->application_arguments_.end(), flag) != this->application_arguments_.end();
}

std::string SDKCommon::get_application_argument_value(const std::string& argument, const std::string& default_value)
{
    auto iterator = std::find(this->application_arguments_.begin(), this->application_arguments_.end(), argument);

    if (iterator != this->application_arguments_.end() && std::next(iterator) != this->application_arguments_.end())
    {
        std::string next_value = *std::next(iterator);

        if (next_value.empty() || next_value[0] != '-') {
            return next_value;
        }
    }

    return default_value;
}

bool SDKCommon::is_numeric_string(const std::string& string)
{
    return !string.empty() && std::all_of(string.begin(), string.end(), [] (unsigned char character) {
        return std::isdigit(character);
    });
}
