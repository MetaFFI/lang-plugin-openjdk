package metaffi;

import java.io.*;

public class MetaFFIBridge
{
	private static native void init();
	public native void load_runtime_plugin(String runtime_plugin);
	public native void free_runtime_plugin(String runtime_plugin);
	public native long load_function(String runtime_plugin, String function_path, byte params_count, byte retval_count);
	public native void free_function(String runtime_plugin, long function_id);
	public native void xcall_params_ret(long function_id, long xcall_params);
	public native long xcall_no_params_ret(long function_id, long xcall_params);
	public native long xcall_params_no_ret(long function_id, long xcall_params);
	public native long xcall_no_params_no_ret(long function_id);
	public native long java_to_cdts(long pcdt, Object[] params, long[] parameterTypes);
	public native Object[] cdts_to_java(long pcdt, long length);
	public native long alloc_cdts(byte params_count, byte retval_count);
	public native long get_pcdt(long pcdts, byte index);
	public native Object get_object(long phandle);
	public native void remove_object(long phandle);

	static
	{
		String xllrExtension = System.mapLibraryName("xllr.openjdk.jni.bridge");
		xllrExtension = xllrExtension.substring(xllrExtension.lastIndexOf("."));
		String metaffiHome = System.getenv("METAFFI_HOME");
		System.load(metaffiHome+"/xllr.openjdk.jni.bridge"+xllrExtension);
		init();
	}

	//--------------------------------------------------------------------
	public MetaFFIBridge() {}
	//--------------------------------------------------------------------
    public Object[] XCallParamsRet(long pff, long[] parameterTypes, long[] retvalTypes, Object[] params) throws MetaFFIException
    {
// 	    long xcall_params = java_to_cdts(params, parameterTypes, retvalTypes.length);
//         this.xcall_params_ret(pff, xcall_params);
// 	    return cdts_to_java(xcall_params, retvalTypes);
		return null;
    }
    //--------------------------------------------------------------------
    public void XCallParamsNoRet(long pff, long[] parameterTypes, Object[] params) throws MetaFFIException
    {
	    //long xcall_params = java_to_cdts(params, parameterTypes, 0);
	    //this.xcall_params_no_ret(pff, xcall_params);
    }
    //--------------------------------------------------------------------
    public Object[] XCallNoParamsRet(long pff, long[] retvalTypes) throws MetaFFIException
    {
// 	    long xcall_params = java_to_cdts(null, null, retvalTypes.length);
// 		this.xcall_no_params_ret(pff, xcall_params);
// 		return cdts_to_java(xcall_params, retvalTypes);
		return null;
    }
    //--------------------------------------------------------------------
    public void XCallNoParamsNoRet(long pff) throws MetaFFIException
    {
// 	    this.xcall_no_params_no_ret(pff);
    }
    //--------------------------------------------------------------------
}