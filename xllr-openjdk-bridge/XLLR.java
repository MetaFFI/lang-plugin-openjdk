package openffi;

import java.io.*;

public class XLLR
{
	private native String init();
	private native String load_runtime_plugin(String runtime_plugin);
	private native String free_runtime_plugin(String runtime_plugin);
	private native Object load_function(String runtime_plugin, String function_path);
	private native String free_function(String runtime_plugin, long function_id);
	private native String call(String runtime_plugin,
							long function_id,
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
                String xllrExtension = System.mapLibraryName("xllr.openjdk.jni.bridge").replace("xllr.openjdk.jni.bridge", "");
				String openffiHome = System.getenv("OPENFFI_HOME");
                System.load(openffiHome+"/xllr.openjdk.jni.bridge"+xllrExtension);
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
    public long loadFunction(String runtimePlugin, String functionPath) throws OpenFFIException
    {
		Object functionID = this.load_function(runtimePlugin, functionPath);
		if(functionID instanceof String)
			throw new OpenFFIException((String)functionID);

		return (long)functionID;
    }
    //--------------------------------------------------------------------
    public void freeModule(String runtimePlugin, long functionID) throws OpenFFIException
    {
		String err = this.free_function(runtimePlugin, functionID);
		if(!err.isEmpty())
			throw new OpenFFIException(err);
    }
    //--------------------------------------------------------------------
    public CallResult call(String runtimePlugin,
	                    long functionID,
	                    byte inParams[]) throws OpenFFIException
    {
		CallResult cr = new CallResult();

		String err = this.call(runtimePlugin, functionID, inParams, cr);
		if(!err.isEmpty())
			throw new OpenFFIException(err);

		return cr;
    }
    //--------------------------------------------------------------------
}