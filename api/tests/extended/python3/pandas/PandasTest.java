import api.MetaFFIModule;
import api.MetaFFIRuntime;
import metaffi.Caller;
import metaffi.MetaFFIHandle;
import metaffi.MetaFFITypeWithAlias;
import org.junit.AfterClass;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;


class DataFrame
{
	private Caller piloc = null;
	private Caller pget = null;

	private Caller pto_string = null;

	private MetaFFIHandle instance;

	public DataFrame(MetaFFIHandle h)
	{
		MetaFFIModule pandas = PandasTest.runtime.loadModule("pandas");

		this.pto_string = pandas.load("callable=DataFrame.__str__,instance_required",
				new MetaFFITypeWithAlias[]{new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIHandle)},
				new MetaFFITypeWithAlias[]{new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIString8)});

		this.piloc = pandas.load("callable=DataFrame.iloc.fget,instance_required",
				new MetaFFITypeWithAlias[]{new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIHandle)},
				new MetaFFITypeWithAlias[]{new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIHandle)});

		this.pget = pandas.load("callable=core.indexing._iLocIndexer.__getitem__,instance_required",
				new MetaFFITypeWithAlias[]{new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIHandle), new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIInt64)},
				new MetaFFITypeWithAlias[]{new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIHandle)});

		this.instance = h;
	}

	public DataFrame getRow(int index)
	{
		MetaFFIHandle indexerInstance = (MetaFFIHandle)this.piloc.call(this.instance)[0];
		MetaFFIHandle newDF = (MetaFFIHandle)this.pget.call(indexerInstance, index)[0];
		return new DataFrame(newDF);
	}

	public String toString()
	{
		return (String)this.pto_string.call(this.instance)[0];
	}
}

class Pandas
{
	private MetaFFIHandle instance;
	private Caller pread_csv;

	public Pandas()
	{
		MetaFFIModule pandas = PandasTest.runtime.loadModule("pandas");
		this.pread_csv = pandas.load("callable=read_csv",
				new MetaFFITypeWithAlias[]{new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIString8)},
				new MetaFFITypeWithAlias[]{new MetaFFITypeWithAlias(MetaFFITypeWithAlias.MetaFFITypes.MetaFFIHandle)});
	}

	public DataFrame ReadCSV(String pathToCSVFile)
	{
		var dataFrameHandle = (MetaFFIHandle)this.pread_csv.call(pathToCSVFile)[0];
		return new DataFrame(dataFrameHandle);
	}
}

public class PandasTest
{
	public static api.MetaFFIRuntime runtime = null;

	@BeforeClass
	public static void init()
	{
		runtime = new MetaFFIRuntime("python3");
		runtime.loadRuntimePlugin();
	}

	@AfterClass
	public static void fini()
	{
		// TODO
		// runtime.releaseRuntimePlugin();
	}

	@Test
	public void testPandas() throws IOException
	{
		File file = null;
		try
		{
			String csv = "col1,col2,col3\r\n" +
					"r11,r12,r13\r\n" +
					"r21,r22,r23\r\n";

			// get second row of CSV
			file = File.createTempFile("input", ".csv");
			FileWriter writer = new FileWriter(file);
			writer.write(csv);
			writer.close();

			Pandas pd = new Pandas();
			var df = pd.ReadCSV(file.getAbsolutePath());
			var dfSecondRow = df.getRow(1);
			System.out.println(dfSecondRow.toString());
		}
		finally
		{
			if(file != null)
				file.delete();
		}
	}

}

