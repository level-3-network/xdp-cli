// SPDX-License-Identifier: GPL-2.0-only

#pragma once

enum class Controller_PluginLoader_HookResult
{
    kHookIgnored = 0,
    kHookHandled,
    kHookSupersede,
    kHookContinue
};

template<typename... Args>
class Controller_PluginLoader_HookTemplate
{
public:
    using Controller_PluginLoader_HookTemplate_CallbackFunction = Controller_PluginLoader_HookResult(*)(Args...);

private:
    std::vector<Controller_PluginLoader_HookTemplate_CallbackFunction> hooks_callback_function_;

public:
    Controller_PluginLoader_HookTemplate() = default;
    ~Controller_PluginLoader_HookTemplate() = default;

    void insert(Controller_PluginLoader_HookTemplate_CallbackFunction callback_function)
    {
        if (callback_function) {
            this->hooks_callback_function_.push_back(callback_function);
        }
    }

    Controller_PluginLoader_HookResult execute(std::function<void(const Controller_PluginLoader_HookResult)> callback_function, Args... args)
    {
        auto hook_result = Controller_PluginLoader_HookResult::kHookIgnored;

        for (auto& hook_callback_function : this->hooks_callback_function_)
        {
            hook_result = hook_callback_function(args...);

            if (hook_result == Controller_PluginLoader_HookResult::kHookSupersede)
            {
                callback_function(hook_result);

                return Controller_PluginLoader_HookResult::kHookSupersede;
            }

            if (hook_result == Controller_PluginLoader_HookResult::kHookContinue)
            {
                callback_function(hook_result);

                continue;
            }

            callback_function(hook_result);
        }

        return hook_result;
    }
};

class Controller_PluginLoader
{
private:
    bool is_initialized_;

    std::vector<void*> plugins_;

public:
    Controller_PluginLoader() = default;
    ~Controller_PluginLoader() = default;

    auto& is_initialized() const {
        return this->is_initialized_;
    }

    bool initialize()
    {
        if (this->is_initialized_) {
            return false;
        }

        this->is_initialized_ = true;

        return true;
    }

    bool destroy()
    {
        if (!this->is_initialized_) {
            return false;
        }

        this->is_initialized_ = false;

        for (auto plugin : this->plugins_) {
            dlclose(plugin);
        }

        return true;
    }

    void* load_plugin(const std::string& path)
    {
        if (!this->is_initialized_) {
            return nullptr;
        }

        if (path.empty()) {
            return nullptr;
        }

        void* plugin = dlopen(path.c_str(), RTLD_NOW);

        if (!plugin) {
            return nullptr;
        }

        this->plugins_.push_back(plugin);

        return plugin;
    }

    template<typename... Args>
    bool register_hook(Controller_PluginLoader_HookTemplate<Args...>& hook_template, const std::string& name)
    {
        if (!this->is_initialized_) {
            return false;
        }

        if (name.empty()) {
            return false;
        }

        for (auto plugin : this->plugins_)
        {
            auto symbol = (typename Controller_PluginLoader_HookTemplate<Args...>::Controller_PluginLoader_HookTemplate_CallbackFunction)dlsym(plugin, name.c_str());

            if (symbol) {
                hook_template.insert(symbol);
            }
        }

        return true;
    }
};
