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



	public metaffi.Caller load(String functionPath, metaffi.MetaFFITypeWithAlias[] parametersTypes, metaffi.MetaFFITypeWithAlias[] retvalsTypes)
	{

		var xcallAndContext = metaffi.MetaFFIBridge.load_function(
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
		return metaffi.Caller.createCaller(xcallAndContext, parametersTypes, (retvalsTypes != null && retvalsTypes.length > 0) ? retvalsTypes[0] : null);
	}
}
