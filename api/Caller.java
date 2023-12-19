package api;

import java.util.function.Function;

public class Caller
{
	private final Function<Object, Object[]> f;

	public Caller(Function<Object,Object[]> f)
	{
		this.f = f;
	}

	public Object[] call(Object... parameters)
	{
		return this.f.apply(parameters);
	}

	public static Caller createCaller(long xcallAndContext, metaffi.MetaFFITypeWithAlias[] params, byte retvalsCount)
	{
		byte paramsCount = (params == null)? 0 : (byte)params.length;
		long[] parametersTypesArray = paramsCount > 0 ? convertToLongArray(params) : new long[]{};

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

	public static long[] convertToLongArray(metaffi.MetaFFITypeWithAlias[] metaFFITypesArray)
	{
		long[] longArray = new long[metaFFITypesArray.length];
		for (int i = 0; i < metaFFITypesArray.length; i++) {
			longArray[i] = metaFFITypesArray[i].type.value;
		}
		return longArray;
	}
}
