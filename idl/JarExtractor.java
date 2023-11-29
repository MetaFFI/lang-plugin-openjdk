package java_extractor;

import org.objectweb.asm.ClassReader;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;
import org.objectweb.asm.tree.ClassNode;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.function.Function;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;

public class JarExtractor implements Extractor
{

    private JarFile jarFile;

    public static void main(String[] args) throws Exception
    {
        JavaExtractor je = new JavaExtractor("C:\\src\\github.com\\MetaFFI\\Tests\\Hosts\\Python3\\ToJava\\libraries\\log4j\\log4j-api-2.21.1.jar:org/apache/logging/log4j/LogManager;org/apache/logging/log4j/Logger");
        JavaInfo ji = je.extract();

        System.out.println(ji);
    }

    @FunctionalInterface
    public interface ThrowingFunction<T, R> {
        R apply(T t) throws Exception;
    }

    private boolean isPublicMethodArgument(org.objectweb.asm.Type type) throws IOException
    {
        String pathToType = type.getClassName().replace(".", "/") + ".class";

        JarEntry je = this.jarFile.getJarEntry(pathToType);

        if(je == null) // if not in JAR, assume it is public.
            return true;

        try (InputStream s = jarFile.getInputStream(je))
        {
            ClassReader reader = new ClassReader(s);
            ClassNode classNode = new ClassNode();
            reader.accept(classNode, ClassReader.EXPAND_FRAMES);

            if(type.getDescriptor().lastIndexOf("$") != -1)
            {
                // check parent recursively
                return (classNode.access & Opcodes.ACC_PUBLIC) != 0 &&
                        this.isPublicMethodArgument(Type.getType(type.getDescriptor().substring(0, type.getDescriptor().lastIndexOf("$"))+";"));
            }
            else
            {
                return (classNode.access & Opcodes.ACC_PUBLIC) != 0;
            }
        }
    }

    @Override
    public JavaInfo extract(String filename) throws Exception
    {
        List<ClassInfo> allClasses = new ArrayList<>();
        BytecodeExtractor bytecodeExtractor = new BytecodeExtractor();

        // if the jar path includes path within the jar, extract jar path
        String jarFilename = filename.substring(0, filename.indexOf(".jar")) + ".jar";
        String[] pathesWithinJar = filename.substring(filename.indexOf(".jar")+4).replace(":", "").split(";");

        this.jarFile = new JarFile(jarFilename);

        HashMap<String, Boolean> isPublicClass = new HashMap<String, Boolean>();

        // first iteration without inner-classes
        java.util.Enumeration<JarEntry> entries = jarFile.entries();
        while (entries.hasMoreElements())
        {
            JarEntry entry = entries.nextElement();
            if (!entry.getName().endsWith(".class"))
                continue;

            if(entry.getName().startsWith("META-INF"))
                continue;

            if(entry.getName().contains("$"))
                continue;

            boolean foundPath = false;
            for(String path : pathesWithinJar)
            {
                if(entry.getName().startsWith(path))
                {
                    foundPath = true;
                    break;
                }
            }
            if(!foundPath) // if entry is not is requested path
                continue;

            try (InputStream classFileInputStream = jarFile.getInputStream(entry))
            {
                JavaInfo javaInfo = bytecodeExtractor.extract(classFileInputStream, this::isPublicMethodArgument);
                if (javaInfo != null && javaInfo.Classes != null)
                {
                    isPublicClass.put(entry.getName().replace(".class", ""), true); // only public classes are added to allClasses
                    allClasses.addAll(Arrays.asList(javaInfo.Classes));
                }
                else
                {
                    isPublicClass.put(entry.getName().replace(".class", ""), false);
                }
            }
        }

        // second iteration only inner-classes of PUBLIC classes
        if(false) // TODO: currently ignore inner-classes
        {
            entries = jarFile.entries();
            while (entries.hasMoreElements())
            {
                JarEntry entry = entries.nextElement();
                if (!entry.getName().endsWith(".class"))
                    continue;

                if (!entry.getName().contains("$"))
                    continue;

                boolean foundPath = false;
                for(String path : pathesWithinJar)
                {
                    if(entry.getName().startsWith(path))
                    {
                        foundPath = true;
                        break;
                    }
                }
                if(!foundPath) // if entry is not is requested path
                    continue;

                String parentClassKey = entry.getName().substring(0, entry.getName().indexOf("$"));

                if(!isPublicClass.containsKey(parentClassKey)) // if parent is not public
                    throw new Exception(String.format("%s was not detected during the first iteration", parentClassKey));

                if(!isPublicClass.get(parentClassKey)) // if parent is not public
                    continue;

                try (InputStream classFileInputStream = jarFile.getInputStream(entry))
                {
                    JavaInfo javaInfo = bytecodeExtractor.extract(classFileInputStream, this::isPublicMethodArgument);
                    if (javaInfo != null && javaInfo.Classes != null)
                    {
                        allClasses.addAll(Arrays.asList(javaInfo.Classes));
                    }
                }
            }
        }

        JavaInfo result = new JavaInfo();
        result.Classes = allClasses.toArray(new ClassInfo[0]);
        return result;
    }
}
