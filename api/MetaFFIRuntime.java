package api;

import metaffi.MetaFFITypeInfo;

import java.io.File;
import java.lang.reflect.Method;
import java.lang.reflect.Type;
import java.util.ArrayList;
import java.util.List;
import java.util.function.Consumer;
import java.util.function.Function;

public class MetaFFIRuntime
{
	public final String runtimePlugin;

	public MetaFFIRuntime(String runtimePlugin)
	{
		String metaFFIHome = System.getenv("METAFFI_HOME");
		if (metaFFIHome == null) {
			throw new IllegalStateException("METAFFI_HOME environment variable is not set");
		}

		String os = System.getProperty("os.name").toLowerCase();
		String libext = "";
		if (os.contains("win")) {
			libext = ".dll";
		}
		else{
			libext = ".so";
		}

		System.load(metaFFIHome+"/openjdk/xllr.openjdk"+libext);

		this.runtimePlugin = runtimePlugin;
	}

	public void loadRuntimePlugin()
	{
		metaffi.MetaFFIBridge.load_runtime_plugin("xllr."+this.runtimePlugin);
	}

	public void releaseRuntimePlugin()
	{
		metaffi.MetaFFIBridge.free_runtime_plugin("xllr."+this.runtimePlugin);
	}

	public api.MetaFFIModule loadModule(String modulePath)
	{
		return new api.MetaFFIModule(this, modulePath);
	}

	public static metaffi.Caller makeMetaFFICallable(Method m) throws NoSuchMethodException
	{
		ArrayList<MetaFFITypeInfo> outParameters = new ArrayList<>();
		ArrayList<MetaFFITypeInfo> outRetvals = new ArrayList<>();
		String jniSignature = MetaFFIRuntime.getJNISignature(m, outParameters, outRetvals);

		var params = outParameters.toArray(new MetaFFITypeInfo[]{});
		var retvals = outRetvals.toArray(new MetaFFITypeInfo[]{});
		long xcall_and_context = metaffi.MetaFFIBridge.load_callable("xllr.openjdk", m, jniSignature, params, retvals);

		return metaffi.Caller.createCaller(xcall_and_context, params, retvals.length > 0 ? retvals[0] : null);
	}

	private static String getJNISignature(Method method, List<MetaFFITypeInfo> outParameters, List<MetaFFITypeInfo> outRetvals) throws NoSuchMethodException
	{
		// Get the parameter types
		Type[] paramTypes = method.getGenericParameterTypes();

		// Get the return type
		Type returnType = method.getGenericReturnType();

		// Convert the types to JNI signatures
		StringBuilder signature = new StringBuilder("(");
		for (Type paramType : paramTypes)
		{
			String jniSig = getJNISignature(paramType);
			outParameters.add(new MetaFFITypeInfo(jniSig));
			signature.append(jniSig);
		}
		signature.append(")");

		String jniSig = getJNISignature(returnType);
		outRetvals.add(new MetaFFITypeInfo(jniSig));
		signature.append(jniSig);

		return signature.toString();
	}

	private static String getJNISignature(Type type)
	{
		String typeName = type.getTypeName();
		StringBuilder signature = new StringBuilder();
		while (typeName.endsWith("[]"))
		{
			// It's an array type
			signature.append("[");
			typeName = typeName.substring(0, typeName.length() - 2);
		}

		switch (typeName)
		{
			case "boolean": signature.append("Z"); break;
			case "byte": signature.append("B"); break;
			case "char": signature.append("C"); break;
			case "short": signature.append("S"); break;
			case "int": signature.append("I"); break;
			case "long": signature.append("J"); break;
			case "float": signature.append("F"); break;
			case "double": signature.append("D"); break;
			default: signature.append("L").append(typeName.replace('.', '/')).append(";"); break;
		}
		return signature.toString();
	}


}

