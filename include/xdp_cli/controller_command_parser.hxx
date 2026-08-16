// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <csignal>
#include <functional>
#include <vector>
#include <string>
#include <unordered_map>

class Controller_CommandParser
{
private:
    inline static Controller_CommandParser* instance_;

    std::vector<std::function<void()>> hooks_initialize_;
    std::vector<std::function<void()>> hooks_destroy_;

    bool running_;

    std::unordered_map<std::string, std::vector<std::string>> commands_syntax_;
    std::unordered_map<std::string, std::function<void(const std::vector<std::string>)>> commands_callback_function_;

    static void signal_handler(int signal)
    {
        if (instance_) {
            instance_->running_ = false;
        }
    }

public:
    Controller_CommandParser()
    {
        this->instance_ = this;

        std::signal(SIGINT, &Controller_CommandParser::signal_handler);
        std::signal(SIGTERM, &Controller_CommandParser::signal_handler);
    };

    ~Controller_CommandParser() = default;

    auto get_running() const {
        return this->running_;
    }

    void hook_initialize(std::function<void()> callback_function) {
        this->hooks_initialize_.push_back(callback_function);
    }

    void hook_destroy(std::function<void()> callback_function) {
        this->hooks_destroy_.push_back(callback_function);
    }

    void initialize();
    void destroy();

    bool register_command(const std::string& name, const std::vector<std::string> syntax, std::function<void(const std::vector<std::string>)> callback_function);
    bool unregister_command(const std::string& name);

    bool execute_command(const std::string& input);
};
