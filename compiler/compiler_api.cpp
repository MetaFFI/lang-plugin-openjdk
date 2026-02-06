/**
 * C-compatible API for JVM Compiler Plugin
 */

#include "jvm_compiler_plugin.h"
#include <runtime/xllr_capi_loader.h>
#include <cstdlib>
#include <cstring>

namespace
{
    JVMCompilerPlugin g_plugin;
    bool g_initialized = false;

    void set_error(char** out_err, uint32_t* out_err_len, const char* msg)
    {
        if(!out_err || !out_err_len)
        {
            return;
        }

        if(!msg)
        {
            msg = "";
        }

        size_t len = strlen(msg);
        *out_err_len = static_cast<uint32_t>(len);

        if(len == 0)
        {
            *out_err = nullptr;
            return;
        }

        char* err_buf = xllr_alloc_string(msg, len);
        if(!err_buf)
        {
            err_buf = static_cast<char*>(calloc(len + 1, 1));
            if(err_buf)
            {
                memcpy(err_buf, msg, len);
            }
            else
            {
                *out_err_len = 0;
            }
        }

        *out_err = err_buf;
    }

    void clear_error(char** out_err, uint32_t* out_err_len)
    {
        *out_err = nullptr;
        *out_err_len = 0;
    }
}

extern "C"
{
    void init_plugin()
    {
        if(!g_initialized)
        {
            g_plugin.init();
            g_initialized = true;
        }
    }

    void compile_to_guest(
        const char* idl_def_json, uint32_t idl_def_json_length,
        const char* output_path, uint32_t output_path_length,
        const char* guest_options, uint32_t guest_options_length,
        char** out_err, uint32_t* out_err_len)
    {
        try
        {
            if(!g_initialized)
            {
                g_plugin.init();
                g_initialized = true;
            }

            std::string idl_json(idl_def_json, idl_def_json_length);
            std::string out_path(output_path, output_path_length);
            std::string options(guest_options, guest_options_length);

            g_plugin.compile_to_guest(idl_json, out_path, options);

            clear_error(out_err, out_err_len);
        }
        catch(const std::exception& e)
        {
            set_error(out_err, out_err_len, e.what());
        }
        catch(...)
        {
            set_error(out_err, out_err_len, "Unknown error in compile_to_guest");
        }
    }

    void compile_from_host(
        const char* idl_def_json, uint32_t idl_def_json_length,
        const char* output_path, uint32_t output_path_length,
        const char* host_options, uint32_t host_options_length,
        char** out_err, uint32_t* out_err_len)
    {
        try
        {
            if(!g_initialized)
            {
                g_plugin.init();
                g_initialized = true;
            }

            std::string idl_json(idl_def_json, idl_def_json_length);
            std::string out_path(output_path, output_path_length);
            std::string options(host_options, host_options_length);

            g_plugin.compile_from_host(idl_json, out_path, options);

            clear_error(out_err, out_err_len);
        }
        catch(const std::exception& e)
        {
            set_error(out_err, out_err_len, e.what());
        }
        catch(...)
        {
            set_error(out_err, out_err_len, "Unknown error in compile_from_host");
        }
    }
}
