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
    private String simpleTestClassPath;
    private String complexTestClassPath;
    
    @Before
    public void setUp() throws Exception {
        // Compile test classes
        compileTestClasses();
        
        // Set up paths
        testClassPath = "test/testdata/SimpleTest.class";
        simpleTestClassPath = "test/testdata/SimpleTest.class";
        complexTestClassPath = "test/testdata/ComplexTest.class";
    }
    
    @After
    public void tearDown() throws Exception {
        // Clean up compiled .class files
        cleanupClassFiles();
    }
    
    @Test
    public void testExtractSimpleClass() throws Exception {
        BytecodeExtractor extractor = new BytecodeExtractor(simpleTestClassPath);
        JavaInfo javaInfo = extractor.extract();
        
        assertNotNull("JavaInfo should not be null", javaInfo);
        assertNotNull("Classes should not be null", javaInfo.Classes);
        assertEquals("Should have 1 class", 1, javaInfo.Classes.length);
        
        ClassInfo classInfo = javaInfo.Classes[0];
        assertEquals("Class name should be SimpleTest", "SimpleTest", classInfo.Name);
        assertEquals("Package should be testdata", "testdata", classInfo.Package);
    }
    
    @Test
    public void testExtractComplexClass() throws Exception {
        BytecodeExtractor extractor = new BytecodeExtractor(complexTestClassPath);
        JavaInfo javaInfo = extractor.extract();
        
        assertNotNull("JavaInfo should not be null", javaInfo);
        assertNotNull("Classes should not be null", javaInfo.Classes);
        assertTrue("Should have at least 1 class", javaInfo.Classes.length >= 1);
        
        // Find the main class
        ClassInfo mainClass = null;
        for (ClassInfo classInfo : javaInfo.Classes) {
            if ("ComplexTest".equals(classInfo.Name)) {
                mainClass = classInfo;
                break;
            }
        }
        
        assertNotNull("Should find ComplexTest class", mainClass);
        assertEquals("Class name should be ComplexTest", "ComplexTest", mainClass.Name);
        assertEquals("Package should be testdata", "testdata", mainClass.Package);
    }
    
    @Test
    public void testJSONGeneration() throws Exception {
        BytecodeExtractor extractor = new BytecodeExtractor(simpleTestClassPath);
        JavaInfo javaInfo = extractor.extract();
        
        String json = javaInfo.toMetaFFIJSON();
        assertNotNull("JSON should not be null", json);
        assertFalse("JSON should not be empty", json.trim().isEmpty());
        
        // Basic JSON structure validation
        assertTrue("JSON should contain modules", json.contains("\"modules\""));
        assertTrue("JSON should contain target_language", json.contains("\"target_language\""));
        assertTrue("JSON should contain openjdk", json.contains("\"openjdk\""));
    }
    
    private void compileTestClasses() throws Exception {
        // Compile SimpleTest.java
        String simpleTestSource = "test/testdata/SimpleTest.java";
        String simpleTestClass = "test/testdata/SimpleTest.class";
        
        if (!new File(simpleTestClass).exists()) {
            ProcessBuilder pb = new ProcessBuilder(
                "javac", "-cp", "libs/*", "-d", "test/testdata", simpleTestSource
            );
            Process process = pb.start();
            int exitCode = process.waitFor();
            if (exitCode != 0) {
                throw new Exception("Failed to compile SimpleTest.java");
            }
        }
        
        // Compile ComplexTest.java
        String complexTestSource = "test/testdata/ComplexTest.java";
        String complexTestClass = "test/testdata/ComplexTest.class";
        
        if (!new File(complexTestClass).exists()) {
            ProcessBuilder pb = new ProcessBuilder(
                "javac", "-cp", "libs/*", "-d", "test/testdata", complexTestSource
            );
            Process process = pb.start();
            int exitCode = process.waitFor();
            if (exitCode != 0) {
                throw new Exception("Failed to compile ComplexTest.java");
            }
        }
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