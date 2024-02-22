import api.MetaFFIModule;
import api.MetaFFIRuntime;
import metaffi.Caller;
import metaffi.MetaFFIHandle;
import metaffi.MetaFFITypeInfo;
import org.junit.AfterClass;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;

import java.util.Arrays;


class NumpyArray
{
	private MetaFFIHandle instance;
	private Caller pmean;
	private Caller pstr;

	public NumpyArray(long[] array)
	{
		MetaFFIModule np = NumpyTest.runtime.loadModule("numpy");
		Caller constructor = np.load("callable=array",
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIInt64Array)},
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)});

		this.pmean = np.load("callable=ndarray.mean,instance_required",
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)},
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIFloat64)});

		this.pstr = np.load("callable=ndarray.mean.__str__,instance_required",
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIHandle)},
				new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIString8)});

		Long[] longarray = Arrays.stream(array)
				.mapToObj(Long::valueOf)
				.toArray(Long[]::new);

		this.instance = (MetaFFIHandle)constructor.call((Object)longarray)[0];
	}

	public float mean()
	{
		return (float)this.pmean.call(this.instance)[0];
	}
}

public class NumpyTest
{
	public static api.MetaFFIRuntime runtime = null;

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
	public void testNumpy()
	{
		var input = new long[]{1,2,3,4};
		var arr = new NumpyArray(input);
		Assert.assertEquals(2.5f, arr.mean(), 0);
	}

}
