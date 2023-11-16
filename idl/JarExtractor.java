package java_extractor;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;

public class JarExtractor implements Extractor
{

    @Override
    public JavaInfo extract(String filename) throws Exception
    {
        List<ClassInfo> allClasses = new ArrayList<>();
        BytecodeExtractor bytecodeExtractor = new BytecodeExtractor();

        try (JarFile jarFile = new JarFile(filename))
        {
            java.util.Enumeration<JarEntry> entries = jarFile.entries();
            while (entries.hasMoreElements())
            {
                JarEntry entry = entries.nextElement();
                if (!entry.getName().endsWith(".class"))
                    continue;


                try (InputStream classFileInputStream = jarFile.getInputStream(entry))
                {
                    JavaInfo javaInfo = bytecodeExtractor.extract(classFileInputStream);
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
