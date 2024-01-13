import api.MetaFFIModule;
import api.MetaFFIRuntime;
import metaffi.*;
import org.junit.*;
import metaffi.MetaFFIHandle;

import java.util.HashMap;


class PyDeque
{
	private MetaFFIModule mod = null;
	private metaffi.MetaFFIHandle instance = null;
	private metaffi.Caller pappend = null;
	private metaffi.Caller ppop = null;
	private metaffi.Caller plen = null;

	public PyDeque(api.MetaFFIRuntime runtime)
	{
		mod = runtime.loadModule("collections");
		metaffi.Caller pconstructor = mod.load("callable=deque", null, new MetaFFITypeWithAlias[]{new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIHandle)});
		this.instance = (metaffi.MetaFFIHandle)pconstructor.call()[0];

		this.pappend = mod.load("callable=deque.append,instance_required",
				new MetaFFITypeWithAlias[]{new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIHandle), new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIAny)},
				null);

		this.ppop = mod.load("callable=deque.pop,instance_required",
				new MetaFFITypeWithAlias[]{new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIHandle)},
				new MetaFFITypeWithAlias[]{new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIAny)});

		this.plen = mod.load("callable=deque.__len__,instance_required",
				new MetaFFITypeWithAlias[]{new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIHandle)},
				new MetaFFITypeWithAlias[]{new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIInt64)});
	}

	public MetaFFIHandle getHandle()
	{
		return this.instance;
	}

	public void append(Object o)
	{
		this.pappend.call(this.instance, o);
	}

	public Object pop()
	{
		return this.ppop.call(this.instance)[0];
	}

	public long len()
	{
		return (long)this.plen.call(this.instance)[0];
	}
}


public class CollectionsTest
{
	private static api.MetaFFIRuntime runtime = null;

	@BeforeClass
	public static void init()
	{
		runtime = new MetaFFIRuntime("python311");
		runtime.loadRuntimePlugin();
	}

	@AfterClass
	public static void fini()
	{
		// TODO
		// runtime.releaseRuntimePlugin();
	}

	@Test
	public void test_deque()
	{
		HashMap<String,String> ss = new HashMap<>();
		ss.put("one", "two");

		PyDeque pydeq = new PyDeque(runtime);
		pydeq.append(4L);
		pydeq.append("test");
		pydeq.append(ss);

		HashMap<String,String> poppedMap = (HashMap<String,String>)pydeq.pop();
		Assert.assertEquals(ss.get("one"), poppedMap.get("one"));
		Assert.assertEquals("test", (String)pydeq.pop());
		Assert.assertEquals(4L, (long)pydeq.pop());
	}

}