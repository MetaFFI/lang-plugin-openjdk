import api.MetaFFIRuntime;
import metaffi.MetaFFITypeWithAlias;
import org.junit.AfterClass;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;

import java.util.ArrayList;
import java.util.Arrays;

public class APITestGo
{
	private static MetaFFIRuntime runtime = null;
	private static api.MetaFFIModule module = null;

	@BeforeClass
	public static void init()
	{
		runtime = new MetaFFIRuntime("go");
		runtime.loadRuntimePlugin();
		module = runtime.loadModule("./tests/go/TestRuntime_MetaFFIGuest.dll");
	}

	@AfterClass
	public static void fini()
	{
		runtime.releaseRuntimePlugin();
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
				new MetaFFITypeWithAlias[]{ new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIInt64), new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIInt64) },
				new MetaFFITypeWithAlias[]{ new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIFloat32) });

		Object res = pff.call(10L, 2L);

		Assert.assertEquals(5f, (float)((Object[])res)[0], 0);
	}

	@Test
	public void testJoinStrings()
	{
		metaffi.Caller pff = module.load("callable=JoinStrings",
				new MetaFFITypeWithAlias[]{ new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIString8Array) },
				new MetaFFITypeWithAlias[]{ new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIString8) });

		Object res = pff.call((Object)new String[]{"one", "two", "three"});

		Assert.assertEquals((String)((Object[])res)[0], "one,two,three");
	}

	@Test
	public void testMapSetGetContains()
	{
		metaffi.Caller newTestMap = module.load("callable=NewTestMap",
				null,
				new MetaFFITypeWithAlias[]{ new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIHandle) });

		metaffi.Caller testMapSet = module.load("callable=TestMap.Set,instance_required",
				new MetaFFITypeWithAlias[]{ new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIHandle), new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIString8), new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIAny) },
				null);

		metaffi.Caller testMapContains = module.load("callable=TestMap.Contains,instance_required",
				new MetaFFITypeWithAlias[]{ new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIHandle), new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIString8) },
				new MetaFFITypeWithAlias[]{ new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIBool) });

		metaffi.Caller testMapGet = module.load("callable=TestMap.Get,instance_required",
				new MetaFFITypeWithAlias[]{ new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIHandle), new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIString8) },
				new MetaFFITypeWithAlias[]{ new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIAny) });

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
				new MetaFFITypeWithAlias[]{ new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIHandle) });

		metaffi.Caller testMapSetName = module.load("field=TestMap.Name,instance_required,setter",
				new MetaFFITypeWithAlias[]{ new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIHandle), new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIString8)},
				null);

		metaffi.Caller testMapGetName = module.load("field=TestMap.Name,instance_required,getter",
				new MetaFFITypeWithAlias[]{ new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIHandle)},
				new MetaFFITypeWithAlias[]{new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIString8)});

		var testMap = ((Object[])newTestMap.call())[0];
		Assert.assertNotNull(testMap);

		Assert.assertEquals("TestMap Name", ((Object[])testMapGetName.call(testMap))[0]);

		testMapSetName.call(testMap, "NewName");

		Assert.assertEquals("NewName", ((Object[])testMapGetName.call(testMap))[0]);
	}
}
