package api;

public class MetaFFIRuntime
{
	public final String runtimePlugin;
	public static metaffi.MetaFFIBridge metaffiBridge = new metaffi.MetaFFIBridge();

	public MetaFFIRuntime(String runtimePlugin)
	{
		this.runtimePlugin = runtimePlugin;
	}

	public void loadRuntimePlugin()
	{
		metaffiBridge.load_runtime_plugin("xllr."+this.runtimePlugin);
	}

	public void releaseRuntimePlugin()
	{
		metaffiBridge.free_runtime_plugin("xllr."+this.runtimePlugin);
	}

	public api.MetaFFIModule loadModule(String modulePath)
	{
		return new api.MetaFFIModule(this, modulePath);
	}
}
