package openffi;

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
		File file = new File(dirOrJar);
		if(!file.exists())
			throw new FileNotFoundException(dirOrJar + " is not found");

		URL url = file.toURI().toURL();

		return Class.forName(className, true, new URLClassLoader(new URL[]{ url }));
	}
}
