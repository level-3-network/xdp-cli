// SPDX-License-Identifier: GPL-2.0-only

#include "xdp_cli/program_core.hxx"

extern Controller_PluginLoader* controller_plugin_loader;

Controller_PluginLoader_HookTemplate<> hook_initialize;
Controller_PluginLoader_HookTemplate<> hook_destroy;

bool Controller_CommandParser::initialize()
{
    if (this->is_initialized_) {
        return false;
    }

    this->is_initialized_ = true;

    controller_plugin_loader->register_hook(hook_initialize, "hook_command_parser_initialize");
    controller_plugin_loader->register_hook(hook_destroy, "hook_command_parser_destroy");

    hook_initialize.execute([] (const Controller_PluginLoader_HookResult hook_result) {});

    std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << "Type /help to view a list of available commands" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

    return true;
}

bool Controller_CommandParser::loop()
{
    if (!this->is_initialized_) {
        return false;
    }

    std::string input;

    if (!std::getline(std::cin, input)) {
        return false;
    }

    this->execute_command(input);

    return true;
}

bool Controller_CommandParser::destroy()
{
    if (!this->is_initialized_) {
        return false;
    }

    hook_destroy.execute([] (const Controller_PluginLoader_HookResult hook_result) {});

    this->is_initialized_ = false;

    std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << "Destroying command parser controller..." << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

    return true;
}

bool Controller_CommandParser::register_command(const std::string& name, const std::vector<std::string> syntax, std::function<void(const std::vector<std::string>)> callback_function)
{
    if (!this->is_initialized_) {
        return false;
    }

    if (name.empty())
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "[!] Failed to register command with an empty name" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    auto iterator = this->commands_callback_function_.find(name);

    if (iterator != this->commands_callback_function_.end())
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "[!] Failed to register a command that is already registered" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    this->commands_syntax_[name] = syntax;
    this->commands_callback_function_[name] = callback_function;

    return true;
}

bool Controller_CommandParser::unregister_command(const std::string& name)
{
    if (!this->is_initialized_) {
        return false;
    }

    if (name.empty())
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "[!] Failed to unregister command with an empty name" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    auto iterator = this->commands_callback_function_.find(name);

    if (iterator == this->commands_callback_function_.end())
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "[!] Failed to unregister a command that doesn't exist" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    this->commands_syntax_.erase(name);
    this->commands_callback_function_.erase(name);

    return true;
}

bool Controller_CommandParser::execute_command(const std::string& input)
{
    if (!this->is_initialized_) {
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
                std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_MAGENTA << "> " << key << " - " << syntax << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;
            }
        }
    }

    else {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "[!] Unknown command [" << name << "]" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;
    }

    return true;
}
