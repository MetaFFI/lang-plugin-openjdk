package openffi;

import java.io.*;

public class XLLR
{
	private native String init();
	private native String load_runtime_plugin(String runtime_plugin);
	private native String free_runtime_plugin(String runtime_plugin);
	private native String load_module(String runtime_plugin, String module);
	private native String free_module(String runtime_plugin, String module);
	private native String call(String runtime_plugin,
							String module_name,
							String func_name,
							byte in_params[],
							CallResult out_res);

	private boolean isLoaded = false;

	//--------------------------------------------------------------------
	public XLLR() throws IOException, OpenFFIException
	{
		synchronized(this)
		{
			if(!isLoaded)
			{
                //String xllrExtension = System.mapLibraryName("xllr_java_bridge_jni").replace("xllr_java_bridge_jni", "");

                System.load("/home/tcs/src/github.com/GreenFuze/OpenFFI/examples/openjdk/xllr_java_bridge_jni.so");
                this.init();

                isLoaded = true;
			}
		}
	}
	//--------------------------------------------------------------------
	public void loadRuntimePlugin(String runtimePlugin) throws OpenFFIException
	{
		String err = this.load_runtime_plugin(runtimePlugin);
		if(!err.isEmpty())
			throw new OpenFFIException(err);
	}
	//--------------------------------------------------------------------
	public void freeRuntimePlugin(String runtimePlugin) throws OpenFFIException
    {
		String err = this.free_runtime_plugin(runtimePlugin);
		if(!err.isEmpty())
			throw new OpenFFIException(err);
    }
    //--------------------------------------------------------------------
    public void loadModule(String runtimePlugin, String module) throws OpenFFIException
    {
		String err = this.load_module(runtimePlugin, module);
		if(!err.isEmpty())
			throw new OpenFFIException(err);
    }
    //--------------------------------------------------------------------
    public void freeModule(String runtimePlugin, String module) throws OpenFFIException
    {
		String err = this.free_module(runtimePlugin, module);
		if(!err.isEmpty())
			throw new OpenFFIException(err);
    }
    //--------------------------------------------------------------------
    public CallResult call(String runtimePlugin,
	                    String moduleName,
	                    String funcName,
	                    byte inParams[]) throws OpenFFIException
    {
		CallResult cr = new CallResult();

		String err = this.call(runtimePlugin, moduleName, funcName, inParams, cr);
		if(!err.isEmpty())
			throw new OpenFFIException(err);

		return cr;
    }
    //--------------------------------------------------------------------
}