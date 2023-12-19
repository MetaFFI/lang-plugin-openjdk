package api;

import metaffi.MetaFFIHandle;

import java.util.function.Function;

public class MetaFFIModule
{
    private api.MetaFFIRuntime runtime;
    private String modulePath;

    public MetaFFIModule(api.MetaFFIRuntime runtime, String modulePath)
    {
        this.runtime = runtime;
        this.modulePath = modulePath;
    }



	public api.Caller load(String functionPath, metaffi.MetaFFITypeWithAlias[] parametersTypes, metaffi.MetaFFITypeWithAlias[] retvalsTypes)
	{

		var xcallAndContext = api.MetaFFIRuntime.metaffiBridge.load_function(
																"xllr."+this.runtime.runtimePlugin,
																this.modulePath,
																functionPath,
																parametersTypes,
																retvalsTypes);

		// Return a Caller object that wraps a lambda that calls the foreign object
		byte paramsCount = parametersTypes != null ? (byte)parametersTypes.length : 0;
		byte retvalsCount = retvalsTypes != null ? (byte)retvalsTypes.length : 0;


		// parametersArray - Object[] with parameters
		// return value is an Object of the expected type
		// or null if calling function is void.
		return api.Caller.createCaller(xcallAndContext, parametersTypes, retvalsCount);
	}
}
