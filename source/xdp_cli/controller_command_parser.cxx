// SPDX-License-Identifier: GPL-2.0-only

#include <iostream>
#include <functional>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>

#include "xdp_cli/sdk_common.hxx"

#include "xdp_cli/controller_command_parser.hxx"

void Controller_CommandParser::initialize()
{
    this->running_ = true;

    for (const auto& hook : this->hooks_initialize_)
    {
        if (hook) {
            hook();
        }
    }

    std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << "Type /help to view a list of available commands" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

    std::string input;

    while (this->running_)
    {
        if (!std::getline(std::cin, input)) {
            continue;
        }

        this->execute_command(input);
    }
}

void Controller_CommandParser::destroy()
{
    for (const auto& hook : this->hooks_destroy_)
    {
        if (hook) {
            hook();
        }
    }

    this->running_ = false;
}

bool Controller_CommandParser::register_command(const std::string& name, const std::vector<std::string> syntax, std::function<void(const std::vector<std::string>)> callback_function)
{
    if (!this->running_) {
        return false;
    }

    if (name.empty())
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Failed to register command with an empty name" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    auto iterator = this->commands_callback_function_.find(name);

    if (iterator != this->commands_callback_function_.end())
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Failed to register an already registered command" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    this->commands_syntax_[name] = syntax;
    this->commands_callback_function_[name] = callback_function;

    return true;
}

bool Controller_CommandParser::unregister_command(const std::string& name)
{
    if (!this->running_) {
        return false;
    }

    if (name.empty())
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Failed to unregister command with an empty name" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    auto iterator = this->commands_callback_function_.find(name);

    if (iterator == this->commands_callback_function_.end())
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Failed to unregister a command that doesn't exist" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    this->commands_syntax_.erase(name);
    this->commands_callback_function_.erase(name);

    return true;
}

bool Controller_CommandParser::execute_command(const std::string& input)
{
    if (!this->running_) {
        return false;
    }

    if (input.empty()) {
        return false;
    }

    std::stringstream ss(input);

    std::string name;

    ss >> name;

    std::vector<std::string> arguments;

    std::string argument;

    while (ss >> argument) {
        arguments.push_back(argument);
    }

    auto iterator = this->commands_callback_function_.find(name);

    if (iterator != this->commands_callback_function_.end()) {
        iterator->second(arguments);
    }

    else if (name == "/help")
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << "List of available commands:" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        for (const auto& [key, value] : this->commands_syntax_)
        {
            for (const auto& syntax : value) {
                std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_MAGENTA << key << " - " << syntax << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;
            }
        }
    }

    else {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Unknown command [" << name << "]" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;
    }

    return true;
}
