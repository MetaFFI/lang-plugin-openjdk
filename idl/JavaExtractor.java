package java_extractor;

import com.github.javaparser.*;
import com.github.javaparser.ast.CompilationUnit;
import com.github.javaparser.ast.body.TypeDeclaration;

import java.io.File;
import java.io.FileNotFoundException;
import java.net.MalformedURLException;
import java.nio.file.Paths;
import java.security.InvalidParameterException;
import java.util.*;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;

import org.objectweb.asm.*;
import org.objectweb.asm.tree.*;

import java.io.IOException;
import java.nio.file.Files;

public class JavaExtractor
{
	public static void main(String[] args) throws Exception
	{
		JavaExtractor je = new JavaExtractor("C:\\src\\github.com\\MetaFFI\\Tests\\Hosts\\Go\\ToJava\\sanity_bytecode\\sanity\\TestMap.class");
		JavaInfo ji = je.extract();

		System.out.println(ji);
	}

	private final String filename;

	public JavaExtractor(String filename) throws RuntimeException
	{
		filename = filename.replace("\\", "/");

		File f = new File(filename);
	    if(!f.exists()) {
	        throw new RuntimeException("\""+filename + "\" to parse does not exist");
	    }
		this.filename = filename;
	}

	public JavaInfo extract() throws Exception
	{
		Extractor ext = null;
		if(this.filename.toLowerCase().endsWith(".java"))
		{
			ext = new JavaSourceExtractor();
		}
		else if(this.filename.toLowerCase().endsWith(".class"))
		{
			ext = new BytecodeExtractor();
		}
		else if(this.filename.toLowerCase().endsWith(".jar"))
		{
			ext = new JarExtractor();
		}
		else
		{
			throw new InvalidParameterException(String.format("Unexpected file type: %s", this.filename));
		}

		return ext.extract(this.filename);
	}


}