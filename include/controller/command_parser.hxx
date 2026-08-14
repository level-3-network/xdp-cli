// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <csignal>
#include <functional>
#include <vector>
#include <string>
#include <unordered_map>

class CommandParserController
{
private:
    inline static CommandParserController* instance_;

    bool running_;

    std::vector<std::function<void()>> forwards_on_initialize_;
    std::vector<std::function<void()>> forwards_on_destroy_;

    static void signal_handler(int signal)
    {
        if (instance_) {
            instance_->running_ = false;
        }
    }

public:
    CommandParserController()
    {
        this->instance_ = this;

        std::signal(SIGINT, &CommandParserController::signal_handler);
        std::signal(SIGTERM, &CommandParserController::signal_handler);

        this->running_ = true;
    };

    ~CommandParserController() = default;

    auto get_running() const {
        return this->running_;
    }

    void set_running(const bool value) {
        this->running_ = value;
    }

    bool initialize();
    void destroy();

    bool register_command(const std::string& name, const std::vector<std::string> syntax, std::function<void(const std::vector<std::string>)> callback_function);
    bool unregister_command(const std::string& name);

    bool execute_command(const std::string& input);

    bool set_forward_on_initialize(std::function<void()> callback_function);
    bool set_forward_on_destroy(std::function<void()> callback_function);

    bool execute_forward_on_initialize();
    bool execute_forward_on_destroy();

private:
    std::unordered_map<std::string, std::vector<std::string>> commands_syntax_;
    std::unordered_map<std::string, std::function<void(const std::vector<std::string>)>> commands_callback_function_;
};
