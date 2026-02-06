#include "jvm_wrappers.h"

#include <cstdlib>
#include <cstring>

JvmHandle::JvmHandle(cdt_metaffi_handle* handle)
	: _handle(handle)
{
}

JvmHandle::~JvmHandle()
{
	if(_handle)
	{
		if(_handle->release)
		{
			_handle->release(_handle);
		}
		xllr_free_memory(_handle);
		_handle = nullptr;
	}
}

JvmHandle::JvmHandle(JvmHandle&& other) noexcept
	: _handle(other._handle)
{
	other._handle = nullptr;
}

JvmHandle& JvmHandle::operator=(JvmHandle&& other) noexcept
{
	if(this != &other)
	{
		if(_handle)
		{
			if(_handle->release)
			{
				_handle->release(_handle);
			}
			xllr_free_memory(_handle);
		}
		_handle = other._handle;
		other._handle = nullptr;
	}
	return *this;
}

cdt_metaffi_handle* JvmHandle::get() const
{
	return _handle;
}

bool JvmHandle::is_null() const
{
	return !_handle || !_handle->handle;
}

cdt_metaffi_handle* JvmHandle::release()
{
	cdt_metaffi_handle* released = _handle;
	_handle = nullptr;
	return released;
}

std::string take_string8(metaffi_string8 value)
{
	if(!value)
	{
		return {};
	}
	std::string result(reinterpret_cast<const char*>(value));
	xllr_free_string(reinterpret_cast<char*>(value));
	return result;
}

std::string java_object_to_string(const JvmHandle& obj)
{
	auto& env = jvm_test_env();
	auto to_string = env.guest_module.load_entity(
		"class=java.lang.Object,callable=toString,instance_required",
		{metaffi_handle_type},
		{metaffi_string8_type});
	auto [result] = to_string.call<std::string>(*obj.get());
	return result;
}

std::string java_class_name(const JvmHandle& obj)
{
	auto& env = jvm_test_env();
	auto get_class = env.guest_module.load_entity(
		"class=java.lang.Object,callable=getClass,instance_required",
		{metaffi_handle_type},
		{metaffi_handle_type});
	auto [cls_ptr] = get_class.call<cdt_metaffi_handle*>(*obj.get());
	JvmHandle cls_handle(cls_ptr);

	auto get_name = env.guest_module.load_entity(
		"class=java.lang.Class,callable=getName,instance_required",
		{metaffi_handle_type},
		{metaffi_string8_type});
	auto [name] = get_name.call<std::string>(*cls_handle.get());
	return name;
}

JavaArray::JavaArray(JvmHandle handle)
	: _handle(std::move(handle))
{
}

metaffi_size JavaArray::length() const
{
	auto& env = jvm_test_env();
	auto get_length = env.guest_module.load_entity(
		"class=java.lang.reflect.Array,callable=getLength",
		{metaffi_handle_type},
		{metaffi_int32_type});
	auto [len] = get_length.call<int32_t>(*(_handle.get()));
	return static_cast<metaffi_size>(len);
}

JvmHandle JavaArray::get_handle(metaffi_size index) const
{
	auto& env = jvm_test_env();
	auto get_item = env.guest_module.load_entity(
		"class=java.lang.reflect.Array,callable=get",
		{metaffi_handle_type, metaffi_int32_type},
		{metaffi_handle_type});
	auto [handle] = get_item.call<cdt_metaffi_handle*>(*(_handle.get()), static_cast<int32_t>(index));
	return JvmHandle(handle);
}

metaffi_variant JavaArray::get_any(metaffi_size index) const
{
	auto& env = jvm_test_env();
	auto get_item = env.guest_module.load_entity(
		"class=java.lang.reflect.Array,callable=get",
		{metaffi_handle_type, metaffi_int32_type},
		{metaffi_any_type});
	auto [value] = get_item.call<metaffi_variant>(*(_handle.get()), static_cast<int32_t>(index));
	return value;
}

cdt_metaffi_handle* JavaArray::handle() const
{
	return _handle.get();
}

void set_callback_error(char** out_err, const std::string& msg)
{
	if(!out_err)
	{
		return;
	}
	char* buf = static_cast<char*>(malloc(msg.size() + 1));
	if(!buf)
	{
		return;
	}
	std::memcpy(buf, msg.c_str(), msg.size());
	buf[msg.size()] = '\0';
	*out_err = buf;
}
