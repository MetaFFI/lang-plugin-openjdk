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

	private static long[] convertToLongArray(metaffi.MetaFFITypeWithAlias[] metaFFITypesArray)
	{
		long[] longArray = new long[metaFFITypesArray.length];
		for (int i = 0; i < metaFFITypesArray.length; i++) {
			longArray[i] = metaFFITypesArray[i].type.value;
		}
		return longArray;
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

		// convert MetaFFITypeWithAlias to long[]
		long[] parametersTypesArray = parametersTypes != null ? convertToLongArray(parametersTypes) : null;
		long[] retvalTypesArray = retvalsTypes != null ? convertToLongArray(retvalsTypes) : null;


		// parametersArray - Object[] with parameters
		// return value is an Object of the expected type
		// or null if calling function is void.
		Function<Object, Object[]> f = (Object parametersArray) ->
		{
			int actualsCount = ((Object[])parametersArray).length;

			if(paramsCount != actualsCount)
				throw new IllegalArgumentException(String.format("Expected %d parameters, received %d parameters", paramsCount, actualsCount));

			// allocate CDTS
			long xcall_params = 0;

			if(paramsCount > 0 || retvalsCount > 0)
				xcall_params = api.MetaFFIRuntime.metaffiBridge.alloc_cdts(paramsCount, retvalsCount);

			// get parameters CDTS and returnValue CDTS
			long parametersCDTS = 0;
			long returnValuesCDTS = 0;

			if(paramsCount > 0)
				parametersCDTS = api.MetaFFIRuntime.metaffiBridge.get_pcdt(xcall_params, (byte)0);

			if(retvalsCount > 0)
				returnValuesCDTS = api.MetaFFIRuntime.metaffiBridge.get_pcdt(xcall_params, (byte)1);

			// fill CDTS
			if(paramsCount > 0)
				api.MetaFFIRuntime.metaffiBridge.java_to_cdts(parametersCDTS, (Object[])parametersArray, parametersTypesArray);

			if(paramsCount > 0 && retvalsCount > 0)
				api.MetaFFIRuntime.metaffiBridge.xcall_params_ret(xcallAndContext, xcall_params);
			if(paramsCount > 0 && retvalsCount == 0)
				api.MetaFFIRuntime.metaffiBridge.xcall_params_no_ret(xcallAndContext, xcall_params);
			if(paramsCount == 0 && retvalsCount > 0)
				api.MetaFFIRuntime.metaffiBridge.xcall_no_params_ret(xcallAndContext, xcall_params);
			if(paramsCount == 0 && retvalsCount == 0)
				api.MetaFFIRuntime.metaffiBridge.xcall_no_params_no_ret(xcallAndContext);

			// fill result
			if(retvalsCount == 0)
				return null;

			Object[] res = api.MetaFFIRuntime.metaffiBridge.cdts_to_java(returnValuesCDTS, retvalsCount);
			return res;
		};

		return new api.Caller(f);

	}
}
