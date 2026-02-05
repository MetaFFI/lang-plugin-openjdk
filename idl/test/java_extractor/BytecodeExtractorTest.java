package java_extractor;

import org.junit.Test;
import org.junit.Before;
import org.junit.After;
import static org.junit.Assert.*;
import java.io.File;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

public class BytecodeExtractorTest {
    
    private String testClassPath;
    
    @Before
    public void setUp() throws Exception {
        // Compile test classes
        compileTestClasses();
        
        // Set up paths
        testClassPath = "test/testdata/testdata/AllVariantsTest.class";
    }
    
    @After
    public void tearDown() throws Exception {
        // Clean up compiled .class files
        cleanupClassFiles();
    }
    
    @Test
    public void testExtractSimpleClass() throws Exception {
        BytecodeExtractor extractor = new BytecodeExtractor(testClassPath);
        JavaInfo javaInfo = extractor.extract();
        
        assertNotNull("JavaInfo should not be null", javaInfo);
        assertNotNull("Classes should not be null", javaInfo.Classes);
        assertEquals("Should have 1 class", 1, javaInfo.Classes.length);
        
        ClassInfo classInfo = javaInfo.Classes[0];
        assertEquals("Class name should be AllVariantsTest", "AllVariantsTest", classInfo.Name);
        assertEquals("Package should be testdata", "testdata", classInfo.Package);
    }
    
    @Test
    public void testExtractComplexClass() throws Exception {
        BytecodeExtractor extractor = new BytecodeExtractor(testClassPath);
        JavaInfo javaInfo = extractor.extract();
        
        assertNotNull("JavaInfo should not be null", javaInfo);
        assertNotNull("Classes should not be null", javaInfo.Classes);
        assertTrue("Should have at least 1 class", javaInfo.Classes.length >= 1);
        
        // Find the main class
        ClassInfo mainClass = null;
        for (ClassInfo classInfo : javaInfo.Classes) {
            if ("AllVariantsTest".equals(classInfo.Name)) {
                mainClass = classInfo;
                break;
            }
        }
        
        assertNotNull("Should find AllVariantsTest class", mainClass);
        assertEquals("Class name should be AllVariantsTest", "AllVariantsTest", mainClass.Name);
        assertEquals("Package should be testdata", "testdata", mainClass.Package);

        assertNotNull("Methods should not be null", mainClass.Methods);
        assertTrue("Should have multiple methods", mainClass.Methods.length >= 3);

        assertNotNull("Fields should not be null", mainClass.Fields);

        VariableInfo intField = findField(mainClass, "intField");
        assertNotNull("Should find intField", intField);
        assertEquals("INT32", intField.Type);
        assertEquals(0, intField.Dimensions);
        assertTrue(intField.IsPublic);
        assertFalse(intField.IsStatic);

        VariableInfo staticField = findField(mainClass, "staticField");
        assertNotNull("Should find staticField", staticField);
        assertEquals("STRING8", staticField.Type);
        assertTrue(staticField.IsPublic);
        assertTrue(staticField.IsStatic);

        VariableInfo stringMatrix = findField(mainClass, "stringMatrix");
        assertNotNull("Should find stringMatrix", stringMatrix);
        assertEquals("STRING8_ARRAY", stringMatrix.Type);
        assertEquals(2, stringMatrix.Dimensions);
        assertTrue(stringMatrix.IsPublic);

        VariableInfo listField = findField(mainClass, "listField");
        assertNotNull("Should find listField", listField);
        assertEquals("HANDLE", listField.Type);
        assertEquals("java.util.List", listField.TypeAlias);
        assertTrue(listField.IsPublic);

        VariableInfo mapField = findField(mainClass, "mapField");
        assertNotNull("Should find mapField", mapField);
        assertEquals("HANDLE", mapField.Type);
        assertEquals("java.util.Map", mapField.TypeAlias);
        assertTrue(mapField.IsPublic);

        assertNotNull("Constructors should not be null", mainClass.Constructors);
        assertTrue("Should have at least 2 constructors", mainClass.Constructors.length >= 2);

        MethodInfo addInt = findMethodByNameAndReturn(mainClass, "add", "INT32", 2);
        assertNotNull("Should find add(int,int)", addInt);
        assertEquals("INT32", addInt.Parameters[0].Type);
        assertEquals("INT32", addInt.Parameters[1].Type);

        MethodInfo addLong = findMethodByNameAndReturn(mainClass, "add", "INT64", 2);
        assertNotNull("Should find add(long,long)", addLong);
        assertEquals("INT64", addLong.Parameters[0].Type);
        assertEquals("INT64", addLong.Parameters[1].Type);

        MethodInfo join = findMethodByNameAndReturn(mainClass, "join", "STRING8", 1);
        assertNotNull("Should find join(String...)", join);
        assertEquals("STRING8_ARRAY", join.Parameters[0].Type);
        assertEquals(1, join.Parameters[0].Dimensions);

        MethodInfo returnsArray = findMethodByNameAndReturn(mainClass, "returnsArray", "INT32_ARRAY", 0);
        assertNotNull("Should find returnsArray()", returnsArray);
        assertEquals(1, returnsArray.ReturnValues[0].Dimensions);

        MethodInfo returnsMatrix = findMethodByNameAndReturn(mainClass, "returnsMatrix", "STRING8_ARRAY", 0);
        assertNotNull("Should find returnsMatrix()", returnsMatrix);
        assertEquals(2, returnsMatrix.ReturnValues[0].Dimensions);

        MethodInfo doNothing = findMethodByName(mainClass, "doNothing");
        assertNotNull("Should find doNothing()", doNothing);
        assertNotNull("ReturnValues should not be null", doNothing.ReturnValues);
        assertEquals(0, doNothing.ReturnValues.length);
    }
    
    @Test
    public void testJSONGeneration() throws Exception {
        BytecodeExtractor extractor = new BytecodeExtractor(testClassPath);
        JavaInfo javaInfo = extractor.extract();
        
        String json = javaInfo.toMetaFFIJSON();
        assertNotNull("JSON should not be null", json);
        assertFalse("JSON should not be empty", json.trim().isEmpty());
        
        // Basic JSON structure validation
        assertTrue("JSON should contain modules", json.contains("\"modules\""));
        assertTrue("JSON should contain target_language", json.contains("\"target_language\""));
        assertTrue("JSON should contain jvm", json.contains("\"jvm\""));
    }
    
    private void compileTestClasses() throws Exception {
        String testSource = "test/testdata/AllVariantsTest.java";
        String testClass = "test/testdata/testdata/AllVariantsTest.class";

        if (!new File(testClass).exists()) {
            ExecResult result = runJavac(new String[] {"javac", "-cp", "libs/*", "-d", "test/testdata", testSource});
            if (result.exitCode != 0) {
                String javaHome = System.getenv("JAVA_HOME");
                if (javaHome == null || javaHome.isEmpty()) {
                    throw new Exception("Failed to compile AllVariantsTest.java (javac not found on PATH and JAVA_HOME not set). Output: " + result.output);
                }

                String javacPath = Paths.get(javaHome, "bin", isWindows() ? "javac.exe" : "javac").toString();
                if (!new File(javacPath).exists()) {
                    throw new Exception("Failed to compile AllVariantsTest.java (javac not found on PATH and JAVA_HOME bin missing: " + javacPath + ")");
                }

                ExecResult fallback = runJavac(new String[] {javacPath, "-cp", "libs/*", "-d", "test/testdata", testSource});
                if (fallback.exitCode != 0) {
                    throw new Exception("Failed to compile AllVariantsTest.java (javac exit code " + fallback.exitCode + "). Output: " + fallback.output);
                }
            }
        }
    }

    private static class ExecResult {
        final int exitCode;
        final String output;
        ExecResult(int exitCode, String output) {
            this.exitCode = exitCode;
            this.output = output;
        }
    }

    private static ExecResult runJavac(String[] command) throws Exception {
        ProcessBuilder pb = new ProcessBuilder(command);
        pb.redirectErrorStream(true);
        Process process = pb.start();
        byte[] output = process.getInputStream().readAllBytes();
        int exitCode = process.waitFor();
        return new ExecResult(exitCode, new String(output));
    }

    private static boolean isWindows() {
        return System.getProperty("os.name").toLowerCase().contains("win");
    }

    private static VariableInfo findField(ClassInfo cls, String name) {
        if (cls.Fields == null) {
            return null;
        }
        for (VariableInfo field : cls.Fields) {
            if (name.equals(field.Name)) {
                return field;
            }
        }
        return null;
    }

    private static MethodInfo findMethodByName(ClassInfo cls, String name) {
        if (cls.Methods == null) {
            return null;
        }
        for (MethodInfo method : cls.Methods) {
            if (name.equals(method.Name)) {
                return method;
            }
        }
        return null;
    }

    private static MethodInfo findMethodByNameAndReturn(ClassInfo cls, String name, String returnType, int paramCount) {
        if (cls.Methods == null) {
            return null;
        }
        for (MethodInfo method : cls.Methods) {
            if (!name.equals(method.Name)) {
                continue;
            }
            if (method.Parameters == null || method.Parameters.length != paramCount) {
                continue;
            }
            if (method.ReturnValues == null || method.ReturnValues.length != 1) {
                continue;
            }
            if (returnType.equals(method.ReturnValues[0].Type)) {
                return method;
            }
        }
        return null;
    }
    
    private void cleanupClassFiles() throws Exception {
        // Remove compiled .class files
        Path testDataPath = Paths.get("test/testdata");
        if (Files.exists(testDataPath)) {
            Files.walk(testDataPath)
                .filter(path -> path.toString().endsWith(".class"))
                .forEach(path -> {
                    try {
                        Files.delete(path);
                    } catch (Exception e) {
                        // Ignore cleanup errors
                    }
                });
        }
    }
} 
