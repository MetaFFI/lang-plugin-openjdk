import api.MetaFFIModule;
import api.MetaFFIRuntime;
import metaffi.MetaFFIException;
import metaffi.MetaFFIHandle;
import metaffi.MetaFFITypeInfo;
import org.junit.AfterClass;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;


class GoMCache
{
	private metaffi.Caller pinfinity = null;
	private metaffi.Caller plen = null;
	private metaffi.Caller pset = null;
	private metaffi.Caller pget = null;
	private MetaFFIHandle instance = null;

	public GoMCache() throws Exception
	{
		String filename = "mcache_MetaFFIGuest";
		String osName = System.getProperty("os.name");
		if(osName.startsWith("Windows"))
		{
			filename += ".dll";
		}
		else if(osName.startsWith("Linux"))
		{
			filename += ".so";
		}
		else
		{
			throw new Exception("OS not supported");
		}

		MetaFFIModule mcache = GoMCacheTest.runtime.loadModule(filename);

		this.pinfinity = mcache.load("global=TTL_FOREVER,getter",
				null,
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64)});

		var newMCache = mcache.load("callable=New",
				null,
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)});

		this.plen = mcache.load("callable=CacheDriver.Len,instance_required",
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)},
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64)});

		this.pset = mcache.load("callable=CacheDriver.Set,instance_required",
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle),
											new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8),
											new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIAny),
											new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64)},
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)});

		this.pget = mcache.load("callable=CacheDriver.Get,instance_required",
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle),
											new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)},
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle),
											new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIBool)});

		// call constructor
		this.instance = (MetaFFIHandle)newMCache.call()[0];
	}

	public long len()
	{
		return (long)this.plen.call(this.instance)[0];
	}

	public void set(String k, Object v) throws MetaFFIException
	{
		var infinity = (long)this.pinfinity.call()[0];
		this.pset.call(this.instance, k, v, infinity);
	}

	public Object get(String k)
	{
		return this.pget.call(this.instance, k)[0];
	}
}


public class GoMCacheTest
{
	public static api.MetaFFIRuntime runtime = null;

	@BeforeClass
	public static void init()
	{
		runtime = new MetaFFIRuntime("go");
		runtime.loadRuntimePlugin();
	}

	@AfterClass
	public static void fini()
	{
		// TODO
		// runtime.releaseRuntimePlugin();
	}

	@Test
	public void testGoMCache() throws Exception
	{
		var m = new GoMCache();
		m.set("myinteger", 101L);

		Assert.assertTrue(m.len() == 1);

		Object o = m.get("myinteger");
		Assert.assertTrue((long)o == 101L);
	}

}