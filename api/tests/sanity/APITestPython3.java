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

	// for the callable test
	private static api.MetaFFIRuntime javaRuntime = null;

	@BeforeClass
	public static void init()
	{
		runtime = new MetaFFIRuntime("python311");
		runtime.loadRuntimePlugin();
		module = runtime.loadModule("./python3/runtime_test_target.py");


		javaRuntime = new MetaFFIRuntime("openjdk");
		javaRuntime.loadRuntimePlugin();
	}

	@AfterClass
	public static void fini()
	{
		// TODO
		// runtime.releaseRuntimePlugin();
	}

	@Test
	public void testHelloWorld()
	{
		// Load helloworld
		metaffi.Caller pff = module.load("callable=hello_world", null, null);
		pff.call();
	}

	@Test
	public void testReturnsAnError()
	{
		// Load helloworld
		metaffi.Caller pff = module.load("callable=returns_an_error", null, null);

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
		metaffi.Caller pff = module.load("callable=div_integers",
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64), new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64) },
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIFloat32) });

		Object res = pff.call(10L, 2L);

		org.junit.Assert.assertEquals(5f, (float)((Object[])res)[0], 0);
	}

	@Test
	public void testJoinStrings()
	{
		metaffi.Caller pff = module.load("callable=join_strings",
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8Array, 1) },
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8) });

		try {
            // Pause for 30 seconds
            Thread.sleep(30000);
        } catch (InterruptedException e) {
            // Handle the exception
            e.printStackTrace();
        }

		Object res = pff.call((Object)new String[]{"one", "two", "three"});

		org.junit.Assert.assertEquals((String)((Object[])res)[0], "one,two,three");
	}

	@Test
	public void testMapSetGetContains()
	{
		metaffi.Caller newTestMap = module.load("callable=testmap",
				null,
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle) });

		metaffi.Caller testMapSet = module.load("callable=testmap.set,instance_required",
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle), new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8), new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIAny) },
				null);

		metaffi.Caller testMapContains = module.load("callable=testmap.contains,instance_required",
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle), new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8) },
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIBool) });

		metaffi.Caller testMapGet = module.load("callable=testmap.get,instance_required",
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle), new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8) },
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIAny) });

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
		metaffi.Caller newTestMap = module.load("callable=testmap",
				null,
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle) });

		metaffi.Caller testMapSetName = module.load("attribute=name,instance_required,setter",
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle), new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)},
				null);

		metaffi.Caller testMapGetName = module.load("attribute=name,instance_required,getter",
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)},
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)});

		var testMap = ((Object[])newTestMap.call())[0];
		org.junit.Assert.assertNotNull(testMap);

		org.junit.Assert.assertEquals("name1", ((Object[])testMapGetName.call(testMap))[0]);

		testMapSetName.call(testMap, "NewName");

		org.junit.Assert.assertEquals("NewName", ((Object[])testMapGetName.call(testMap))[0]);
	}

	@Test
	public void testCallback() throws NoSuchMethodException
	{
		// use reflection to get the "add" method
		Method m = APITestPython3.class.getDeclaredMethod("add", int.class, int.class);

		// make the method callable from MetaFFI
		metaffi.Caller callbackAdd = api.MetaFFIRuntime.makeMetaFFICallable(m);

		// load the python "call_callback_add" function
		metaffi.Caller callCallback = module.load("callable=call_callback_add",
				new MetaFFITypeInfo[]{ new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFICallable) },
				null);

		// call python
		callCallback.call(callbackAdd);
	}

	public static int add(int x, int y)
	{
		System.out.println("Callback add called with " + x + " and " + y);
		return x+y;
	}

}
