#pragma once

struct openjdk_context
{
    jclass cls = nullptr;
    jmethodID method = nullptr;
    jfieldID field = nullptr;
    bool instance_required = false;
    bool is_getter = false;
    bool constructor = false;
    metaffi_type field_or_return_type = metaffi_null_type;
    std::set<uint8_t> any_type_indices;
};
