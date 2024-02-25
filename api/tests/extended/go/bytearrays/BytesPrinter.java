import api.MetaFFIModule;
import api.MetaFFIRuntime;
import metaffi.MetaFFITypeInfo;
import org.junit.Test;

import java.util.Arrays;

public class BytesPrinter
{
	@Test
	public void testGoMCache() throws Exception
	{
		api.MetaFFIRuntime runtime = new MetaFFIRuntime("go");
		runtime.loadRuntimePlugin();


		String filename = "BytesPrinter_MetaFFIGuest";
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

        MetaFFIModule bprinter = runtime.loadModule(filename);

		var PrintBytesArrays = bprinter.load("callable=PrintBytesArrays", new MetaFFITypeInfo[]{new MetaFFITypeInfo(MetaFFITypeInfo.MetaFFITypes.MetaFFIUInt8Array, 2) },
															null);


		byte[][] arr = new byte[][]{ new byte[]{1, 2, 3, 4, 5}, new byte[]{6, 7, 8, 9, 10} };

		var result = PrintBytesArrays.call((Object)arr);

		System.out.printf("Returned back to java: " + ((Arrays)result[0]).toString());
		System.out.printf("Returned back to java: " + ((Arrays)result[1]).toString());

	}

}