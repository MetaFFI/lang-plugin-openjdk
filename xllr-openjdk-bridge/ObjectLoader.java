package metaffi;

import java.net.*;
import java.io.*;


public class ObjectLoader
{
	public static Object loadObject(String dirOrJar, String className) throws
																			java.lang.IllegalArgumentException,
																			FileNotFoundException,
																			ClassNotFoundException,
																			MalformedURLException
	{
		// TODO: switch to modules, to be able to MetaFFI dynamically, and make sure there are
		// no duplicated objects loaded into the process

		// TODO: if calling from OpenJDK, the guest runtime jar should be specified in the class path
		File file = new File(dirOrJar);
		if(!file.exists())
		{
			file = new File(dirOrJar+".jar");

			if(!file.exists())
				throw new FileNotFoundException(dirOrJar + " is not found");
		}

		URL url = file.toURI().toURL();

		return Class.forName(className, true, new URLClassLoader(new URL[]{ url }));
	}
}
