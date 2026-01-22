package java_extractor;

import org.junit.Test;
import org.junit.Before;
import org.junit.After;
import static org.junit.Assert.*;
import java.io.File;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

public class JarExtractorTest {
    
    private String log4jApiJarPath;
    private String log4jCoreJarPath;
    
    @Before
    public void setUp() throws Exception {
        // Set up paths to log4j JAR files
        log4jApiJarPath = "test/log4j-api-2.21.1.jar";
        log4jCoreJarPath = "test/log4j-core-2.21.1.jar";
        
        // Verify JAR files exist
        assertTrue("log4j-api JAR should exist", new File(log4jApiJarPath).exists());
        assertTrue("log4j-core JAR should exist", new File(log4jCoreJarPath).exists());
    }
    
    @Test
    public void testExtractLog4jApiJar() throws Exception {
        System.out.println("Testing log4j-api JAR extraction...");
        
        JarExtractor extractor = new JarExtractor(log4jApiJarPath);
        JavaInfo javaInfo = extractor.extract();
        
        assertNotNull("JavaInfo should not be null", javaInfo);
        assertNotNull("Classes should not be null", javaInfo.Classes);
        assertTrue("Should have at least 1 class", javaInfo.Classes.length > 0);
        
        System.out.println("Extracted " + javaInfo.Classes.length + " classes from log4j-api JAR");
        
        // Look for specific log4j classes
        boolean foundLogManager = false;
        boolean foundLogger = false;
        
        for (ClassInfo classInfo : javaInfo.Classes) {
            System.out.println("Found class: " + classInfo.Package + "." + classInfo.Name);
            
            if ("LogManager".equals(classInfo.Name) && classInfo.Package.contains("log4j")) {
                foundLogManager = true;
                System.out.println("Found LogManager class");
            }
            
            if ("Logger".equals(classInfo.Name) && classInfo.Package.contains("log4j")) {
                foundLogger = true;
                System.out.println("Found Logger class");
            }
        }
        
        assertTrue("Should find LogManager class", foundLogManager);
        assertTrue("Should find Logger class", foundLogger);
    }
    
    @Test
    public void testExtractLog4jCoreJar() throws Exception {
        System.out.println("Testing log4j-core JAR extraction...");
        
        JarExtractor extractor = new JarExtractor(log4jCoreJarPath);
        JavaInfo javaInfo = extractor.extract();
        
        assertNotNull("JavaInfo should not be null", javaInfo);
        assertNotNull("Classes should not be null", javaInfo.Classes);
        assertTrue("Should have at least 1 class", javaInfo.Classes.length > 0);
        
        System.out.println("Extracted " + javaInfo.Classes.length + " classes from log4j-core JAR");
        
        // Look for specific log4j core classes
        boolean foundLog4jClass = false;
        
        for (ClassInfo classInfo : javaInfo.Classes) {
            System.out.println("Found class: " + classInfo.Package + "." + classInfo.Name);
            
            if (classInfo.Package.contains("log4j")) {
                foundLog4jClass = true;
                System.out.println("Found log4j class: " + classInfo.Package + "." + classInfo.Name);
                break;
            }
        }
        
        assertTrue("Should find at least one log4j class", foundLog4jClass);
    }
    
    @Test
    public void testExtractLog4jApiWithPackageFilter() throws Exception {
        System.out.println("Testing log4j-api JAR extraction with package filter...");
        
        // Test with specific package filter using the constructor that takes separate parameters
        String[] packageFilters = {"org/apache/logging/log4j"};
        JarExtractor extractor = new JarExtractor(log4jApiJarPath, packageFilters);
        JavaInfo javaInfo = extractor.extract();
        
        assertNotNull("JavaInfo should not be null", javaInfo);
        assertNotNull("Classes should not be null", javaInfo.Classes);
        assertTrue("Should have at least 1 class", javaInfo.Classes.length > 0);
        
        System.out.println("Extracted " + javaInfo.Classes.length + " classes with package filter");
        
        // All classes should be from the specified package
        for (ClassInfo classInfo : javaInfo.Classes) {
            assertTrue("All classes should be from org.apache.logging.log4j package", 
                      classInfo.Package.startsWith("org.apache.logging.log4j"));
            System.out.println("Filtered class: " + classInfo.Package + "." + classInfo.Name);
        }
    }
    
    @Test
    public void testJSONGenerationFromLog4jApi() throws Exception {
        System.out.println("Testing JSON generation from log4j-api JAR...");
        
        JarExtractor extractor = new JarExtractor(log4jApiJarPath);
        JavaInfo javaInfo = extractor.extract();
        
        String json = javaInfo.toMetaFFIJSON();
        assertNotNull("JSON should not be null", json);
        assertFalse("JSON should not be empty", json.trim().isEmpty());
        
        System.out.println("Generated JSON length: " + json.length());
        
        // Basic JSON structure validation
        assertTrue("JSON should contain modules", json.contains("\"modules\""));
        assertTrue("JSON should contain target_language", json.contains("\"target_language\""));
        assertTrue("JSON should contain jvm", json.contains("\"jvm\""));
        
        // Should contain log4j-specific content
        assertTrue("JSON should contain log4j references", 
                  json.contains("log4j") || json.contains("LogManager") || json.contains("Logger"));
        
        System.out.println("JSON generation successful");
    }
    
    @Test
    public void testExtractMultipleJars() throws Exception {
        System.out.println("Testing extraction from multiple JARs...");
        
        // Extract from each JAR separately since JarExtractor doesn't support multiple JARs
        JarExtractor apiExtractor = new JarExtractor(log4jApiJarPath);
        JavaInfo apiJavaInfo = apiExtractor.extract();
        
        JarExtractor coreExtractor = new JarExtractor(log4jCoreJarPath);
        JavaInfo coreJavaInfo = coreExtractor.extract();
        
        assertNotNull("API JavaInfo should not be null", apiJavaInfo);
        assertNotNull("Core JavaInfo should not be null", coreJavaInfo);
        assertNotNull("API Classes should not be null", apiJavaInfo.Classes);
        assertNotNull("Core Classes should not be null", coreJavaInfo.Classes);
        assertTrue("API should have at least 1 class", apiJavaInfo.Classes.length > 0);
        assertTrue("Core should have at least 1 class", coreJavaInfo.Classes.length > 0);
        
        int totalClasses = apiJavaInfo.Classes.length + coreJavaInfo.Classes.length;
        System.out.println("Extracted " + apiJavaInfo.Classes.length + " classes from log4j-api JAR");
        System.out.println("Extracted " + coreJavaInfo.Classes.length + " classes from log4j-core JAR");
        System.out.println("Total classes: " + totalClasses);
        
        // Should find classes from both JARs
        boolean foundFromApi = false;
        boolean foundFromCore = false;
        
        // Check API JAR
        for (ClassInfo classInfo : apiJavaInfo.Classes) {
            if (classInfo.Package.contains("log4j")) {
                if (classInfo.Name.equals("LogManager") || classInfo.Name.equals("Logger")) {
                    foundFromApi = true;
                    System.out.println("Found API class: " + classInfo.Package + "." + classInfo.Name);
                }
            }
        }
        
        // Check Core JAR
        for (ClassInfo classInfo : coreJavaInfo.Classes) {
            if (classInfo.Package.contains("log4j")) {
                foundFromCore = true;
                System.out.println("Found Core class: " + classInfo.Package + "." + classInfo.Name);
                break;
            }
        }
        
        assertTrue("Should find classes from log4j-api", foundFromApi);
        assertTrue("Should find classes from log4j-core", foundFromCore);
    }
} 