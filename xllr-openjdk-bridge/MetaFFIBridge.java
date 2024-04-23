package metaffi;

import java.io.*;

public class MetaFFIBridge
{
	public static native void load_runtime_plugin(String runtime_plugin);
	public static native void free_runtime_plugin(String runtime_plugin);
	public static native long load_function(String runtime_plugin, String module_path, String function_path, MetaFFITypeInfo[] params_types, MetaFFITypeInfo[] retval_types);
	public static native long load_callable(String runtime_plugin, Object method, String jni_signature, MetaFFITypeInfo[] params_types, MetaFFITypeInfo[] retval_types);
	public static native void free_function(String runtime_plugin, long function_id);
	public static native void xcall_params_ret(long pxcallAndContext, long xcall_params);
	public static native long xcall_no_params_ret(long pxcallAndContext, long xcall_params);
	public static native long xcall_params_no_ret(long pxcallAndContext, long xcall_params);
	public static native long xcall_no_params_no_ret(long pxcallAndContext);
	public static native long java_to_cdts(long pcdt, Object[] params, long[] parameterTypes);
	public static native Object[] cdts_to_java(long pcdt, long length);
	public static native long alloc_cdts(byte params_count, byte retval_count);
	public static native long get_pcdt(long pcdts, byte index);
	public static native Object get_object(long phandle);
	public static native void remove_object(long phandle);

	static
	{
		String xllrExtension = System.mapLibraryName("xllr.openjdk.jni.bridge");
		xllrExtension = xllrExtension.substring(xllrExtension.lastIndexOf("."));
		String metaffiHome = System.getenv("METAFFI_HOME");

		System.load(metaffiHome+"/xllr.openjdk.jni.bridge"+xllrExtension);


	}

	//--------------------------------------------------------------------
	private MetaFFIBridge() {}
	//--------------------------------------------------------------------
	public static long getMetaFFIType(Object o)
	{
		// TODO: get numbers directly from C++, and not hard-coded

		if(o instanceof Float) return 1;
		if(o instanceof Float[]) return 1 | 65536;
		else if(o instanceof Double) return 2;
		else if(o instanceof Double[]) return 2 | 65536;
		else if(o instanceof Byte) return 4;
		else if(o instanceof Byte[]) return 4 | 65536;
		else if(o instanceof Short) return 8;
		else if(o instanceof Short[]) return 8 | 65536;
		else if(o instanceof Integer) return 16;
        else if(o instanceof Integer[]) return 16 | 65536;
		else if(o instanceof Long) return 32;
		else if(o instanceof Long[]) return 32 | 65536;
		else if(o instanceof Boolean) return 1024;
		else if(o instanceof Boolean[]) return 1024 | 65536;
		else if(o instanceof String) return 4096;
		else if(o instanceof String[]) return 4096 | 65536;
		else return 32768;
	}
	//--------------------------------------------------------------------
}