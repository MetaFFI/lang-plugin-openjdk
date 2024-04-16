import api.MetaFFIRuntime;
import metaffi.MetaFFITypeInfo;
import org.junit.AfterClass;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;

import java.util.ArrayList;
import java.util.Arrays;
import java.io.File;

public class APITestGo
{
	private static MetaFFIRuntime runtime = null;
	private static api.MetaFFIModule module = null;

	public static String getDynamicLibraryExtension() {

	    String osName = System.getProperty("os.name");

	    if (osName.startsWith("Windows"))
	        return ".dll";
	    else if (osName.startsWith("Linux"))
	        return ".so";
	    else
	        return "";
	}

	@BeforeClass
	public static void init() throws Exception
	{
		runtime = new MetaFFIRuntime("go");
		runtime.loadRuntimePlugin();

		File f = new File("./go/TestRuntime_MetaFFIGuest"+getDynamicLibraryExtension());
		if(!f.exists())
			throw new Exception("./go/TestRuntime_MetaFFIGuest"+getDynamicLibraryExtension()+" not found");

		module = runtime.loadModule("./go/TestRuntime_MetaFFIGuest"+getDynamicLibraryExtension());
	}

	@AfterClass
	public static void fini()
	{
		// TODO
		//runtime.releaseRuntimePlugin();
	}

	@Test
	public void testHelloWorld()
	{
		// Load helloworld
		metaffi.Caller pff = module.load("callable=HelloWorld", null, null);
		pff.call();
	}

	@Test
	public void testReturnsAnError()
	{
		// Load helloworld
		metaffi.Caller pff = module.load("callable=ReturnsAnError", null, null);

		try
		{
			pff.call();
			Assert.fail("Exception should have been thrown.");
		}
		catch(Exception exp)
		{
			// exception expected
		}
	}

	@Test
	public void testDivIntegers()
	{
		// Load helloworld
		metaffi.Caller pff = module.load("callable=DivIntegers",
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64), new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64) },
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIFloat32) });

		Object res = pff.call(10L, 2L);

		Assert.assertEquals(5f, (float)((Object[])res)[0], 0);
	}

	@Test
	public void testJoinStrings()
	{
		metaffi.Caller pff = module.load("callable=JoinStrings",
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8Array, 1) },
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8) });

		Object res = pff.call((Object)new String[]{"one", "two", "three"});

		Assert.assertEquals((String)((Object[])res)[0], "one,two,three");
	}

	@Test
	public void testMapSetGetContains()
	{
		metaffi.Caller newTestMap = module.load("callable=NewTestMap",
				null,
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle) });

		metaffi.Caller testMapSet = module.load("callable=TestMap.Set,instance_required",
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle), new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8), new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIAny) },
				null);

		metaffi.Caller testMapContains = module.load("callable=TestMap.Contains,instance_required",
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle), new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8) },
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIBool) });

		metaffi.Caller testMapGet = module.load("callable=TestMap.Get,instance_required",
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle), new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8) },
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIAny) });

		var testMap = ((Object[])newTestMap.call())[0];
		Assert.assertNotNull(testMap);

		testMapSet.call(testMap, "key1", 42L);
		testMapSet.call(testMap, "key2", new ArrayList<>(Arrays.asList("one", "two", "three")));

		Assert.assertTrue((boolean)(((Object[])testMapContains.call(testMap, "key1"))[0]));
		Assert.assertTrue((boolean)(((Object[])testMapContains.call(testMap, "key2"))[0]));
		Assert.assertFalse((boolean)(((Object[])testMapContains.call(testMap, "key3"))[0]));

		Assert.assertEquals((long)(((Object[])testMapGet.call(testMap, "key1"))[0]), 42);

		var arr = testMapGet.call(testMap, "key2");
		var list = (ArrayList<String>)arr[0];
		Assert.assertEquals(list.get(0), "one");
		Assert.assertEquals(list.get(1), "two");
		Assert.assertEquals(list.get(2), "three");
	}

	@Test
	public void testMapName()
	{
		metaffi.Caller newTestMap = module.load("callable=NewTestMap",
				null,
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle) });

		metaffi.Caller testMapSetName = module.load("field=TestMap.Name,instance_required,setter",
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle), new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)},
				null);

		metaffi.Caller testMapGetName = module.load("field=TestMap.Name,instance_required,getter",
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)},
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)});

		var testMap = ((Object[])newTestMap.call())[0];
		Assert.assertNotNull(testMap);

		Assert.assertEquals("TestMap Name", ((Object[])testMapGetName.call(testMap))[0]);

		testMapSetName.call(testMap, "NewName");

		Assert.assertEquals("NewName", ((Object[])testMapGetName.call(testMap))[0]);
	}
}
