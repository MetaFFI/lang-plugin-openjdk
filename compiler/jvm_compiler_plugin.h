#pragma once

#include <runtime_manager/jvm/runtime_manager.h>
#include <runtime_manager/jvm/module.h>
#include <memory>
#include <string>
#include <mutex>

class JVMCompilerPlugin
{
private:
    std::shared_ptr<jvm_runtime_manager> m_runtime;
    std::shared_ptr<Module> m_module;

    jclass m_compiler_class = nullptr;
    jclass m_context_class = nullptr;
    jclass m_gson_class = nullptr;
    jclass m_idl_def_class = nullptr;
    jclass m_hash_map_class = nullptr;

    jmethodID m_context_default = nullptr;
    jmethodID m_context_get_gson = nullptr;
    jmethodID m_compiler_ctor = nullptr;
    jmethodID m_compile_method = nullptr;
    jmethodID m_gson_from_json = nullptr;
    jmethodID m_hash_map_ctor = nullptr;
    jmethodID m_hash_map_put = nullptr;

    std::mutex m_init_mutex;
    bool m_initialized = false;

    void initialize_jvm();
    void cleanup_java_refs();

    void execute_host_compiler(
        const std::string& idl_def_json,
        const std::string& output_path,
        const std::string& host_options);

public:
    JVMCompilerPlugin() = default;
    ~JVMCompilerPlugin();

    JVMCompilerPlugin(const JVMCompilerPlugin&) = delete;
    JVMCompilerPlugin& operator=(const JVMCompilerPlugin&) = delete;
    JVMCompilerPlugin(JVMCompilerPlugin&&) = delete;
    JVMCompilerPlugin& operator=(JVMCompilerPlugin&&) = delete;

    void init();

    void compile_to_guest(
        const std::string& idl_def_json,
        const std::string& output_path,
        const std::string& guest_options);

    void compile_from_host(
        const std::string& idl_def_json,
        const std::string& output_path,
        const std::string& host_options);
};
