import api.MetaFFIModule;
import api.MetaFFIRuntime;
import metaffi.*;
import org.junit.*;
import metaffi.MetaFFIHandle;

import java.util.*;


class PyDict
{
	private MetaFFIModule mod = null;
	private metaffi.MetaFFIHandle instance = null;
	private metaffi.Caller pget = null;
	private metaffi.Caller pset = null;
	private metaffi.Caller plen = null;

	public PyDict(api.MetaFFIRuntime runtime)
	{
		mod = runtime.loadModule("builtins");
		metaffi.Caller pconstructor = mod.load("callable=dict", null, new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)});
		this.instance = (metaffi.MetaFFIHandle)pconstructor.call()[0];

		this.pset = mod.load("callable=dict.__setitem__,instance_required",
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle), new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8), new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIAny)},
				null);

		this.pget = mod.load("callable=dict.__getitem__,instance_required",
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle), new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)},
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIAny)});

		this.plen = mod.load("callable=dict.__len__,instance_required",
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)},
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64)});
	}

	public PyDict(api.MetaFFIRuntime runtime, metaffi.MetaFFIHandle h)
	{
		mod = runtime.loadModule("builtins");
		this.instance = h;

		this.pset = mod.load("callable=dict.__setitem__,instance_required",
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle), new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8), new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIAny)},
				null);

		this.pget = mod.load("callable=dict.__getitem__,instance_required",
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle), new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)},
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIAny)});

		this.plen = mod.load("callable=dict.__len__,instance_required",
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)},
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64)});
	}

	public MetaFFIHandle getHandle()
	{
		return this.instance;
	}

	public Object get(String key)
	{
		return this.pget.call(this.instance, key)[0];
	}

	public void set(String k, Object o)
	{
		this.pset.call(this.instance, k, o);
	}

	public long len()
	{
		return (long)this.plen.call(this.instance)[0];
	}
}



class PyList
{
	private MetaFFIModule mod = null;
	private metaffi.MetaFFIHandle instance = null;
	private metaffi.Caller pget = null;
	private metaffi.Caller pappend = null;
	private metaffi.Caller plen = null;

	public PyList(api.MetaFFIRuntime runtime)
	{
		mod = runtime.loadModule("builtins");
		metaffi.Caller pconstructor = mod.load("callable=list", null, new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)});
		this.instance = (metaffi.MetaFFIHandle)pconstructor.call()[0];

		this.pappend = mod.load("callable=list.append,instance_required",
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle), new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIAny)},
				null);

		this.pget = mod.load("callable=list.__getitem__,instance_required",
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)},
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIAny)});

		this.plen = mod.load("callable=list.__len__,instance_required",
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)},
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64)});
	}

	public PyList(api.MetaFFIRuntime runtime, metaffi.MetaFFIHandle h)
	{
		mod = runtime.loadModule("builtins");
		this.instance = h;

		this.pappend = mod.load("callable=list.append,instance_required",
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle),
						new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIAny)},
				null);

		this.pget = mod.load("callable=list.__getitem__,instance_required",
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle),
											new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64)},
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIAny)});

		this.plen = mod.load("callable=list.__len__,instance_required",
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)},
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64)});
	}

	public MetaFFIHandle getHandle()
	{
		return this.instance;
	}

	Object get(int index)
	{
		return this.pget.call(this.instance, index)[0];
	}

	void append(Object o)
	{
		this.pappend.call(this.instance, o);
	}

	long len()
	{
		return (long)this.plen.call(this.instance)[0];
	}
}

class ExtendedTest
{
	private static api.MetaFFIModule module = null;
	private metaffi.MetaFFIHandle instance = null;
	private api.MetaFFIRuntime runtime = null;
	private metaffi.Caller psetX = null;
	private metaffi.Caller pgetX = null;

	private metaffi.Caller ppositional_or_named = null;
	private metaffi.Caller ppositional_or_named_as_named = null;
	private metaffi.Caller plist_args = null;
	private metaffi.Caller plist_args_without_default = null;
	private metaffi.Caller plist_args_without_default_and_varargs = null;
	private metaffi.Caller pdict_args = null;
	private metaffi.Caller pdict_args_without_default = null;
	private metaffi.Caller pdict_args_without_default_and_kwargs = null;
	private metaffi.Caller pnamed_only = null;
	private metaffi.Caller ppositional_only_with_default = null;
	private metaffi.Caller ppositional_only = null;
	private metaffi.Caller parg_positional_arg_named = null;
	private metaffi.Caller parg_positional_arg_named_with_kwargs = null;
	private metaffi.Caller parg_positional_arg_named_without_default = null;
	private metaffi.Caller parg_positional_arg_named_without_default_with_varargs = null;
	private metaffi.Caller parg_positional_arg_named_without_default_with_kwargs = null;
	private metaffi.Caller parg_positional_arg_named_without_default_with_varargs_and_kwargs = null;

	public ExtendedTest(api.MetaFFIRuntime runtime)
	{
		this.runtime = runtime;
		module = runtime.loadModule("extended_test.py");

		// create instance
		metaffi.Caller newExtendedTest = module.load("callable=extended_test",
                        				null,
                        				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle) });

        this.instance = (MetaFFIHandle)((Object[])newExtendedTest.call())[0];
        org.junit.Assert.assertNotNull(instance);

        // load all methods
		this.psetX = module.load("callable=extended_test.x.fset,instance_required",
                    new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle), new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64)},
                    null);

        this.pgetX = module.load("callable=extended_test.x.fget,instance_required",
                    new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)},
                    new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64)});

		this.ppositional_or_named = module.load("callable=extended_test.positional_or_named,instance_required",
                    new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle),
                                                new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)},
                    new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)});

		this.ppositional_or_named_as_named = module.load("callable=extended_test.positional_or_named,instance_required,named_args",
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle),
						new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)},
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)});

		this.plist_args = module.load("callable=extended_test.list_args,instance_required",
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)},
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)});

		this.plist_args_without_default = module.load("callable=extended_test.list_args,instance_required",
					new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle),
							new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)},
					new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)});

        this.plist_args_without_default_and_varargs = module.load("callable=extended_test.list_args,instance_required,varargs",
	                new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle),
	                                            new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8),
	                                            new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)},
	                new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)});

		this.pdict_args = module.load("callable=extended_test.dict_args,instance_required",
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)},
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)});

		this.pdict_args_without_default = module.load("callable=extended_test.dict_args,instance_required",
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle),
						new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)},
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)});

	    this.pdict_args_without_default_and_kwargs = module.load("callable=extended_test.dict_args,instance_required,named_args",
                    new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle),
                                                new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8),
                                                new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)},
                    new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)});

		this.pnamed_only = module.load("callable=extended_test.named_only,instance_required,named_args",
                  new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle),
                                              new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)},
                  new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIAny)});

		this.ppositional_only_with_default = module.load("callable=extended_test.positional_only,instance_required",
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle),
						new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)},
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIAny)});

        this.ppositional_only = module.load("callable=extended_test.positional_only,instance_required",
                  new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle),
                                              new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8),
                                              new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)},
                  new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIAny)});

		this.parg_positional_arg_named = module.load("callable=extended_test.arg_positional_arg_named,instance_required",
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)},
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)});

		this.parg_positional_arg_named_with_kwargs = module.load("callable=extended_test.arg_positional_arg_named,instance_required,named_args",
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle),
						new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)},
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)});

		this.parg_positional_arg_named_without_default = module.load("callable=extended_test.arg_positional_arg_named,instance_required",
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle),
						new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)},
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)});

		this.parg_positional_arg_named_without_default_with_kwargs = module.load("callable=extended_test.arg_positional_arg_named,instance_required,named_args",
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle),
						new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8),
						new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)},
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)});

		this.parg_positional_arg_named_without_default_with_varargs = module.load("callable=extended_test.arg_positional_arg_named,instance_required,varargs",
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle),
						new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8),
						new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)},
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)});

		this.parg_positional_arg_named_without_default_with_varargs_and_kwargs = module.load("callable=extended_test.arg_positional_arg_named,instance_required,varargs,named_args",
                 new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle),
                                             new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8),
                                             new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle),
                                             new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)},
                 new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)});
	}

	public long getX()
	{
		return (long)this.pgetX.call(this.instance)[0];
	}

	public void setX(long i)
	{
		this.psetX.call(this.instance, i);
	}

	public String PositionalOrNamed(String positionalOrNamed)
	{
		return (String)this.ppositional_or_named.call(this.instance, positionalOrNamed)[0];
	}

	public String PositionalOrNamed(Map<String,String> positionalOrNamed)
	{
		PyDict dict = new PyDict(runtime);

		for(Map.Entry<String,String> e : positionalOrNamed.entrySet())
		{
			dict.set(e.getKey(), e.getValue());
		}

		return (String)this.ppositional_or_named_as_named.call(this.instance, dict.getHandle())[0];
	}

	public String[] ListArgs()
	{
		var lstHandle = (MetaFFIHandle)this.plist_args.call(this.instance)[0];
		PyList pylist = new PyList(runtime, lstHandle);

		long len = pylist.len();
		String[] res = new String[(int)len];
		for(int i=0 ; i<len ; i++)
		{
			res[i] = (String)pylist.get(i);
		}

		return res;
	}

	public String[] ListArgs(String noneDefault, String... lst)
	{
		PyList pylst = null;
		if(lst.length > 0)
		{
			pylst = new PyList(runtime);
			for(String s :lst){
				pylst.append(s);
			}

		}

		PyList pylist;
		if(pylst == null)
		{
			var lstHandle = (MetaFFIHandle)this.plist_args_without_default.call(this.instance, noneDefault)[0];
			pylist = new PyList(runtime, lstHandle);
		}
		else
		{
			var lstHandle = (MetaFFIHandle)this.plist_args_without_default_and_varargs.call(this.instance, noneDefault, pylst.getHandle())[0];
			pylist = new PyList(runtime, lstHandle);
		}


		long len = pylist.len();
		String[] res = new String[(int)len];
		for(int i=0 ; i<len ; i++)
		{
			res[i] = (String)(pylist.get(i));
		}

		return res;
	}

	public String[] DictArgs()
	{
		var lstHandle = (MetaFFIHandle)this.pdict_args.call(this.instance)[0];
		PyList pylist = new PyList(runtime, lstHandle);

		long len = pylist.len();
		String[] res = new String[(int)len];
		for(int i=0 ; i<len ; i++)
		{
			res[i] = (String)pylist.get(i);
		}

		return res;
	}

	public String[] DictArgs(String noneDefault)
	{
		var lstHandle = (MetaFFIHandle)this.pdict_args_without_default.call(this.instance, noneDefault)[0];
		PyList pylist = new PyList(runtime, lstHandle);

		long len = pylist.len();
		String[] res = new String[(int)len];
		for(int i=0 ; i<len ; i++)
		{
			res[i] = (String)pylist.get(i);
		}

		return res;
	}

	public String[] DictArgs(String noneDefault, Map<String,String> named)
	{
		PyDict dict = new PyDict(runtime);

		for(Map.Entry<String,String> e : named.entrySet())
		{
			dict.set(e.getKey(), e.getValue());
		}

		var lstHandle = (MetaFFIHandle)this.pdict_args_without_default_and_kwargs.call(this.instance, noneDefault, dict.getHandle())[0];
		PyList pylist = new PyList(runtime, lstHandle);

		long len = pylist.len();
		String[] res = new String[(int)len];
		for(int i=0 ; i<len ; i++)
		{
			res[i] = (String)pylist.get(i);
		}

		return res;
	}

	public String NamedOnly(Map<String, String> named)
	{
		PyDict dict = new PyDict(runtime);

		for(Map.Entry<String,String> e : named.entrySet())
		{
			dict.set(e.getKey(), e.getValue());
		}

		var res = (String)this.pnamed_only.call(this.instance, dict.getHandle())[0];
		return res;
	}

	public String PositionalOnly(String word)
	{
		var res = (String)this.ppositional_only_with_default.call(this.instance, word)[0];
		return res;
	}

	public String PositionalOnly(String word1, String word2)
	{
		var res = (String)this.ppositional_only.call(this.instance, word1, word2)[0];
		return res;
	}

	public String[] ArgPositionalArgNamed()
	{
		var lstHandle = (MetaFFIHandle)this.parg_positional_arg_named.call(this.instance)[0];
		PyList pylist = new PyList(runtime, lstHandle);

		long len = pylist.len();
		String[] res = new String[(int)len];
		for(int i=0 ; i<len ; i++)
		{
			res[i] = (String)pylist.get(i);
		}

		return res;
	}

	public String[] ArgPositionalArgNamed(String val)
	{
		var lstHandle = (MetaFFIHandle)this.parg_positional_arg_named_without_default.call(this.instance, val)[0];
		PyList pylist = new PyList(runtime, lstHandle);

		long len = pylist.len();
		String[] res = new String[(int)len];
		for(int i=0 ; i<len ; i++)
		{
			res[i] = (String)pylist.get(i);
		}

		return res;
	}

	public String[] ArgPositionalArgNamed(Map<String, String> named)
	{
		PyDict dict = new PyDict(runtime);

		for(Map.Entry<String,String> e : named.entrySet())
		{
			dict.set(e.getKey(), e.getValue());
		}

		var lstHandle = (MetaFFIHandle)this.parg_positional_arg_named_with_kwargs.call(this.instance, dict.getHandle())[0];
		PyList pylist = new PyList(runtime, lstHandle);

		long len = pylist.len();
		String[] res = new String[(int)len];
		for(int i=0 ; i<len ; i++)
		{
			res[i] = (String)pylist.get(i);
		}

		return res;
	}

	public String[] ArgPositionalArgNamed(String val, String... lst)
	{
		PyList pylst = null;
		if(lst.length > 0)
		{
			pylst = new PyList(runtime);
			for(String s :lst){
				pylst.append(s);
			}

		}

		var lstHandle = (MetaFFIHandle)this.parg_positional_arg_named_without_default_with_varargs.call(this.instance, val, pylst.getHandle())[0];
		PyList pylist = new PyList(runtime, lstHandle);

		long len = pylist.len();
		String[] res = new String[(int)len];
		for(int i=0 ; i<len ; i++)
		{
			res[i] = (String)pylist.get(i);
		}

		return res;
	}

	public String[] ArgPositionalArgNamed(String val, Map<String, String> named)
	{
		PyDict dict = new PyDict(runtime);

		for(Map.Entry<String,String> e : named.entrySet())
		{
			dict.set(e.getKey(), e.getValue());
		}

		var lstHandle = (MetaFFIHandle)this.parg_positional_arg_named_without_default_with_kwargs.call(this.instance, val, dict.getHandle())[0];
		PyList pylist = new PyList(runtime, lstHandle);

		long len = pylist.len();
		String[] res = new String[(int)len];
		for(int i=0 ; i<len ; i++)
		{
			res[i] = (String)pylist.get(i);
		}

		return res;
	}

	public String[] ArgPositionalArgNamed(String val, Map<String, String> named, String... lst)
	{
		PyList pylst = new PyList(runtime);
		for(String s :lst){
			pylst.append(s);
		}

		PyDict dict = new PyDict(runtime);
		for(Map.Entry<String,String> e : named.entrySet())
		{
			dict.set(e.getKey(), e.getValue());
		}

		var lstHandle = (MetaFFIHandle)this.parg_positional_arg_named_without_default_with_varargs_and_kwargs.call(this.instance, val, dict.getHandle(), pylst.getHandle())[0];
		PyList pylist = new PyList(runtime, lstHandle);

		long len = pylist.len();
		String[] res = new String[(int)len];
		for(int i=0 ; i<len ; i++)
		{
			res[i] = (String)pylist.get(i);
		}

		return res;
	}
}


public class ComplexPrimitivesTest
{
	private static api.MetaFFIRuntime runtime = null;
	private static ExtendedTest extendedTest = null;

	@BeforeClass
	public static void init()
	{
		runtime = new MetaFFIRuntime("python311");
		runtime.loadRuntimePlugin();

		extendedTest = new ExtendedTest(runtime);
	}

	@AfterClass
	public static void fini()
	{
		// TODO
		// runtime.releaseRuntimePlugin();
	}

	@Test
	public void test_property()
    {
		extendedTest.setX(4);
		long x = extendedTest.getX();
		org.junit.Assert.assertEquals(4, x);
    }

	@Test
	public void test_positional_or_named()
	{
		String res = extendedTest.PositionalOrNamed("PositionalOrNamed");
		org.junit.Assert.assertEquals("PositionalOrNamed", res);

		res = extendedTest.PositionalOrNamed(Map.of("value", "PositionalOrNamed"));
		org.junit.Assert.assertEquals("PositionalOrNamed", res);
	}

	@Test
	public void test_list_args()
	{
        String[] res = extendedTest.ListArgs(); // list_args()
		org.junit.Assert.assertEquals("default", res[0]);

        //---------------

        res = extendedTest.ListArgs("None Default"); // list_args()
		org.junit.Assert.assertEquals("None Default", res[0]);

        //---------------

		res = extendedTest.ListArgs("None-Default 2", "arg1", "arg2", "arg3");
		org.junit.Assert.assertEquals("None-Default 2", res[0]);
		org.junit.Assert.assertEquals("arg1", res[1]);
		org.junit.Assert.assertEquals("arg2", res[2]);
		org.junit.Assert.assertEquals("arg3", res[3]);
	}

	@Test
	public void test_dict_args()
    {
		String[] res = extendedTest.DictArgs();
		org.junit.Assert.assertEquals("default", res[0]);

        //-------

		res = extendedTest.DictArgs("none-default");
	    org.junit.Assert.assertEquals("none-default", res[0]);

        //-------

	    res = extendedTest.DictArgs("none-default", Map.of (
											    "key1", "val1",
											    "key2", "val2"
										    ));

		var al = new ArrayList<String>();
	    Collections.addAll(al, res);

	    org.junit.Assert.assertTrue(al.containsAll(Arrays.asList("none-default", "key1", "val1", "val2", "key2")));
    }

	@Test
	public void test_named_only()
    {
		String res = extendedTest.NamedOnly(Map.of (
				"named", "test"
		));

	    org.junit.Assert.assertEquals("test", res);
    }

	@Test
	public void test_positional_only()
    {
        String res = extendedTest.PositionalOnly("word1");
	    org.junit.Assert.assertEquals("word1 default", res);

        res = extendedTest.PositionalOnly("word1", "word2");
	    org.junit.Assert.assertEquals("word1 word2", res);
    }

	@Test
	public void test_arg_positional_arg_named()
    {
		String[] res = extendedTest.ArgPositionalArgNamed();
	    org.junit.Assert.assertEquals("default", res[0]);

		//-----

	    res = extendedTest.ArgPositionalArgNamed("positional arg");
	    org.junit.Assert.assertEquals("positional arg", res[0]);

        //-----

	    res = extendedTest.ArgPositionalArgNamed("positional arg", "var positional arg");
	    org.junit.Assert.assertEquals("positional arg", res[0]);
	    org.junit.Assert.assertEquals("var positional arg", res[1]);

        //-----

	    res = extendedTest.ArgPositionalArgNamed("positional arg", Map.of("key1", "val1"));
	    org.junit.Assert.assertEquals("positional arg", res[0]);
	    org.junit.Assert.assertEquals("key1", res[1]);
	    org.junit.Assert.assertEquals("val1", res[2]);

		//-----

	    res = extendedTest.ArgPositionalArgNamed(Map.of("key1", "val1"));
	    org.junit.Assert.assertEquals("default", res[0]);
		org.junit.Assert.assertEquals("key1", res[1]);
	    org.junit.Assert.assertEquals("val1", res[2]);

	    //-----

        res = extendedTest.ArgPositionalArgNamed("positional arg", Map.of("key1", "val1"), "var positional arg");
	    org.junit.Assert.assertEquals("positional arg", res[0]);
	    org.junit.Assert.assertEquals("var positional arg", res[1]);
	    org.junit.Assert.assertEquals("key1", res[2]);
	    org.junit.Assert.assertEquals("val1", res[3]);
    }
}