// SPDX-License-Identifier: GPL-2.0-only

#pragma once

class Controller_CommandParser
{
private:
    bool is_initialized_;

    std::unordered_map<std::string, std::vector<std::string>> commands_syntax_;
    std::unordered_map<std::string, std::function<void(const std::vector<std::string>)>> commands_callback_function_;

public:
    Controller_CommandParser() = default;
    ~Controller_CommandParser() = default;

    auto& is_initialized() const {
        return this->is_initialized_;
    }

    bool initialize();
    bool loop();
    bool destroy();

    bool register_command(const std::string& name, const std::vector<std::string> syntax, std::function<void(const std::vector<std::string>)> callback_function);
    bool unregister_command(const std::string& name);

    bool execute_command(const std::string& input);
};
