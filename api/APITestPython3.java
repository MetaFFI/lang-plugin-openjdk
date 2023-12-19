import api.MetaFFIRuntime;
import metaffi.*;
import org.junit.*;
import org.junit.Assert.*;

import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.function.Function;

public class APITestPython3
{
	private static api.MetaFFIRuntime runtime = null;
	private static api.MetaFFIModule module = null;

	@BeforeClass
	public static void init()
	{
		runtime = new MetaFFIRuntime("python3");
		runtime.loadRuntimePlugin();
		module = runtime.loadModule("./tests/python3/runtime_test_target.py");
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
		api.Caller pff = module.load("callable=hello_world", null, null);
		pff.call();
	}

	@Test
	public void testReturnsAnError()
	{
		// Load helloworld
		api.Caller pff = module.load("callable=returns_an_error", null, null);

		try
		{
			pff.call();
			org.junit.Assert.fail("Exception should have been thrown.");
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
		api.Caller pff = module.load("callable=div_integers",
				new MetaFFITypeWithAlias[]{ new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIInt64), new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIInt64) },
				new MetaFFITypeWithAlias[]{ new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIFloat32) });

		Object res = pff.call(10, 2);

		org.junit.Assert.assertEquals(5f, (float)((Object[])res)[0], 0);
	}

	@Test
	public void testJoinStrings()
	{
		api.Caller pff = module.load("callable=join_strings",
				new MetaFFITypeWithAlias[]{ new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIString8Array) },
				new MetaFFITypeWithAlias[]{ new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIString8) });

		Object res = pff.call((Object)new String[]{"one", "two", "three"});

		org.junit.Assert.assertEquals((String)((Object[])res)[0], "one,two,three");
	}

	@Test
	public void testMapSetGetContains()
	{
		api.Caller newTestMap = module.load("callable=testmap",
				null,
				new MetaFFITypeWithAlias[]{ new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIHandle) });

		api.Caller testMapSet = module.load("callable=testmap.set,instance_required",
				new MetaFFITypeWithAlias[]{ new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIHandle), new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIString8), new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIAny) },
				null);

		api.Caller testMapContains = module.load("callable=testmap.contains,instance_required",
				new MetaFFITypeWithAlias[]{ new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIHandle), new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIString8) },
				new MetaFFITypeWithAlias[]{ new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIBool) });

		api.Caller testMapGet = module.load("callable=testmap.get,instance_required",
				new MetaFFITypeWithAlias[]{ new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIHandle), new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIString8) },
				new MetaFFITypeWithAlias[]{ new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIAny) });

		var testMap = ((Object[])newTestMap.call())[0];
		org.junit.Assert.assertNotNull(testMap);

		testMapSet.call(testMap, "key1", 42);
		testMapSet.call(testMap, "key2", new ArrayList<>(Arrays.asList("one", "two", "three")));

		org.junit.Assert.assertTrue((boolean)(((Object[])testMapContains.call(testMap, "key1"))[0]));
		org.junit.Assert.assertTrue((boolean)(((Object[])testMapContains.call(testMap, "key2"))[0]));
		org.junit.Assert.assertFalse((boolean)(((Object[])testMapContains.call(testMap, "key3"))[0]));

		org.junit.Assert.assertEquals((long)(((Object[])testMapGet.call(testMap, "key1"))[0]), 42);

		var arr = testMapGet.call(testMap, "key2");
		var list = (ArrayList<String>)arr[0];
		org.junit.Assert.assertEquals(list.get(0), "one");
		org.junit.Assert.assertEquals(list.get(1), "two");
		org.junit.Assert.assertEquals(list.get(2), "three");
	}

	@Test
	public void testMapName()
	{
		api.Caller newTestMap = module.load("callable=testmap",
				null,
				new MetaFFITypeWithAlias[]{ new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIHandle) });

		api.Caller testMapSetName = module.load("attribute=name,instance_required,setter",
				new MetaFFITypeWithAlias[]{ new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIHandle), new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIString8)},
				null);

		api.Caller testMapGetName = module.load("attribute=name,instance_required,getter",
				new MetaFFITypeWithAlias[]{ new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIHandle)},
				new MetaFFITypeWithAlias[]{new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIString8)});

		var testMap = ((Object[])newTestMap.call())[0];
		org.junit.Assert.assertNotNull(testMap);

		org.junit.Assert.assertEquals("name1", ((Object[])testMapGetName.call(testMap))[0]);

		testMapSetName.call(testMap, "NewName");

		org.junit.Assert.assertEquals("NewName", ((Object[])testMapGetName.call(testMap))[0]);
	}

	@Test
	public void testCallback() throws NoSuchMethodException
	{
		MetaFFIRuntime runtime = new MetaFFIRuntime("openjdk");
		runtime.loadRuntimePlugin();
		Method m = APITestPython3.class.getDeclaredMethod("add", int.class, int.class);
		api.Caller c = api.MetaFFIRuntime.makeMetaFFICallable(m);
		var r = c.call(1, 2);
		org.junit.Assert.assertEquals(3, r[0]);
	}

	public static int add(int x, int y)
	{
		return x+y;
	}
}
