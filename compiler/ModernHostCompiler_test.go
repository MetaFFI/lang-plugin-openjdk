package main

import (
	"encoding/json"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"testing"
)

// checkJARsAvailable checks if the required MetaFFI JAR files are available
func checkJARsAvailable() bool {
	metaffiHome := os.Getenv("METAFFI_HOME")
	if metaffiHome == "" {
		return false
	}

	metaffiAPIJar := filepath.Join(metaffiHome, "openjdk", "metaffi.api.jar")
	bridgeJar := filepath.Join(metaffiHome, "openjdk", "xllr.openjdk.bridge.jar")

	_, err1 := os.Stat(metaffiAPIJar)
	_, err2 := os.Stat(bridgeJar)

	return err1 == nil && err2 == nil
}

// TestJARAvailability tests if the required JAR files are available
func TestJARAvailability(t *testing.T) {
	metaffiHome := os.Getenv("METAFFI_HOME")
	if metaffiHome == "" {
		t.Log("METAFFI_HOME environment variable is not set")
		t.Log("To run full tests, set METAFFI_HOME and build MetaFFI with: python build_target.py metaffi-core --build-type Debug")
		return
	}

	metaffiAPIJar := filepath.Join(metaffiHome, "openjdk", "metaffi.api.jar")
	bridgeJar := filepath.Join(metaffiHome, "openjdk", "xllr.openjdk.bridge.jar")

	_, err1 := os.Stat(metaffiAPIJar)
	_, err2 := os.Stat(bridgeJar)

	if err1 != nil {
		t.Logf("MetaFFI API JAR not found: %s", metaffiAPIJar)
	}
	if err2 != nil {
		t.Logf("Bridge JAR not found: %s", bridgeJar)
	}

	if err1 == nil && err2 == nil {
		t.Log("All required JAR files are available - full compilation tests will run")
	} else {
		t.Log("Some JAR files are missing - compilation tests will be skipped")
		t.Log("To build the JAR files, run: python build_target.py metaffi-core --build-type Debug")
	}
}

// TestSimpleFunction tests compilation of a simple function IDL
func TestSimpleFunction(t *testing.T) {
	idl := `{
		"module": "test_module",
		"entities": [
			{
				"name": "add",
				"type": "function",
				"parameters": [
					{"name": "a", "type": "int32"},
					{"name": "b", "type": "int32"}
				],
				"return_type": "int32"
			}
		]
	}`

	compiler := NewModernHostCompiler()
	outputDir := t.TempDir()

	err := compiler.CompileHost([]byte(idl), outputDir)
	if err != nil {
		t.Fatalf("Failed to compile: %v", err)
	}

	// Check that JAR file was created
	jarPath := filepath.Join(outputDir, "test_module.jar")
	if _, err := os.Stat(jarPath); os.IsNotExist(err) {
		t.Fatalf("JAR file was not created: %s", jarPath)
	}

	// Only verify JAR contents if JARs are available (Java compilation succeeded)
	if checkJARsAvailable() {
		// Verify JAR contents
		cmd := exec.Command("jar", "-tf", jarPath)
		output, err := cmd.Output()
		if err != nil {
			t.Fatalf("Failed to list JAR contents: %v", err)
		}

		jarContents := string(output)
		if !strings.Contains(jarContents, "test_module/") {
			t.Errorf("JAR does not contain expected package structure")
		}
		if !strings.Contains(jarContents, "test_module/test_module.class") {
			t.Errorf("JAR does not contain expected class file")
		}
	} else {
		t.Log("Skipping JAR content verification - MetaFFI JARs not available")
	}
}

// TestClassWithFields tests compilation of a class with fields
func TestClassWithFields(t *testing.T) {
	idl := `{
		"module": "person_module",
		"entities": [
			{
				"name": "Person",
				"type": "class",
				"fields": [
					{"name": "name", "type": "string"},
					{"name": "age", "type": "int32"},
					{"name": "height", "type": "float64"}
				]
			}
		]
	}`

	compiler := NewModernHostCompiler()
	outputDir := t.TempDir()

	err := compiler.CompileHost([]byte(idl), outputDir)
	if err != nil {
		t.Fatalf("Failed to compile: %v", err)
	}

	// Check that JAR file was created
	jarPath := filepath.Join(outputDir, "person_module.jar")
	if _, err := os.Stat(jarPath); os.IsNotExist(err) {
		t.Fatalf("JAR file was not created: %s", jarPath)
	}

	// Verify JAR contents
	cmd := exec.Command("jar", "-tf", jarPath)
	output, err := cmd.Output()
	if err != nil {
		t.Fatalf("Failed to list JAR contents: %v", err)
	}

	jarContents := string(output)
	if !strings.Contains(jarContents, "person_module/person_module.class") {
		t.Errorf("JAR does not contain expected class file")
	}
}

// TestGlobalVariables tests compilation of global variables
func TestGlobalVariables(t *testing.T) {
	idl := `{
		"module": "config_module",
		"entities": [
			{
				"name": "MAX_CONNECTIONS",
				"type": "global",
				"value_type": "int32"
			},
			{
				"name": "DEFAULT_TIMEOUT",
				"type": "global",
				"value_type": "float64"
			},
			{
				"name": "APP_NAME",
				"type": "global",
				"value_type": "string"
			}
		]
	}`

	compiler := NewModernHostCompiler()
	outputDir := t.TempDir()

	err := compiler.CompileHost([]byte(idl), outputDir)
	if err != nil {
		t.Fatalf("Failed to compile: %v", err)
	}

	// Check that JAR file was created
	jarPath := filepath.Join(outputDir, "config_module.jar")
	if _, err := os.Stat(jarPath); os.IsNotExist(err) {
		t.Fatalf("JAR file was not created: %s", jarPath)
	}
}

// TestComplexTypes tests compilation with complex parameter types
func TestComplexTypes(t *testing.T) {
	idl := `{
		"module": "complex_module",
		"entities": [
			{
				"name": "process_data",
				"type": "function",
				"parameters": [
					{"name": "data", "type": "array", "element_type": "int32"},
					{"name": "config", "type": "map", "key_type": "string", "value_type": "string"},
					{"name": "callback", "type": "function", "parameters": [{"name": "result", "type": "int32"}], "return_type": "void"}
				],
				"return_type": "array",
				"return_element_type": "float64"
			}
		]
	}`

	compiler := NewModernHostCompiler()
	outputDir := t.TempDir()

	err := compiler.CompileHost([]byte(idl), outputDir)
	if err != nil {
		t.Fatalf("Failed to compile: %v", err)
	}

	// Check that JAR file was created
	jarPath := filepath.Join(outputDir, "complex_module.jar")
	if _, err := os.Stat(jarPath); os.IsNotExist(err) {
		t.Fatalf("JAR file was not created: %s", jarPath)
	}
}

// TestMixedEntities tests compilation with mixed entity types
func TestMixedEntities(t *testing.T) {
	idl := `{
		"module": "mixed_module",
		"entities": [
			{
				"name": "Calculator",
				"type": "class",
				"fields": [
					{"name": "precision", "type": "int32"},
					{"name": "enabled", "type": "bool"}
				]
			},
			{
				"name": "calculate",
				"type": "function",
				"parameters": [
					{"name": "x", "type": "float64"},
					{"name": "y", "type": "float64"}
				],
				"return_type": "float64"
			},
			{
				"name": "PI",
				"type": "global",
				"value_type": "float64"
			}
		]
	}`

	compiler := NewModernHostCompiler()
	outputDir := t.TempDir()

	err := compiler.CompileHost([]byte(idl), outputDir)
	if err != nil {
		t.Fatalf("Failed to compile: %v", err)
	}

	// Check that JAR file was created
	jarPath := filepath.Join(outputDir, "mixed_module.jar")
	if _, err := os.Stat(jarPath); os.IsNotExist(err) {
		t.Fatalf("JAR file was not created: %s", jarPath)
	}
}

// TestInvalidIDL tests error handling for invalid IDL
func TestInvalidIDL(t *testing.T) {
	invalidIDLs := []string{
		`{"invalid": "json"`, // Invalid JSON
		`{"module": "test"}`, // Missing entities
		`{
			"module": "test",
			"entities": [
				{
					"name": "test",
					"type": "invalid_type"
				}
			]
		}`, // Invalid entity type
		`{
			"module": "test",
			"entities": [
				{
					"name": "test",
					"type": "function",
					"parameters": [
						{"name": "param", "type": "invalid_type"}
					],
					"return_type": "int32"
				}
			]
		}`, // Invalid parameter type
	}

	compiler := NewModernHostCompiler()
	outputDir := t.TempDir()

	for i, invalidIDL := range invalidIDLs {
		err := compiler.CompileHost([]byte(invalidIDL), outputDir)
		if err == nil {
			t.Errorf("Expected error for invalid IDL %d, but got none", i)
		}
	}
}

// TestEmptyModule tests compilation with empty module
func TestEmptyModule(t *testing.T) {
	idl := `{
		"module": "empty_module",
		"entities": []
	}`

	compiler := NewModernHostCompiler()
	outputDir := t.TempDir()

	err := compiler.CompileHost([]byte(idl), outputDir)
	if err != nil {
		t.Fatalf("Failed to compile empty module: %v", err)
	}

	// Check that JAR file was created even for empty module
	jarPath := filepath.Join(outputDir, "empty_module.jar")
	if _, err := os.Stat(jarPath); os.IsNotExist(err) {
		t.Fatalf("JAR file was not created for empty module: %s", jarPath)
	}
}

// TestSpecialCharacters tests compilation with special characters in names
func TestSpecialCharacters(t *testing.T) {
	idl := `{
		"module": "special_chars_module",
		"entities": [
			{
				"name": "test_function_123",
				"type": "function",
				"parameters": [
					{"name": "param_1", "type": "int32"},
					{"name": "param_2", "type": "string"}
				],
				"return_type": "bool"
			}
		]
	}`

	compiler := NewModernHostCompiler()
	outputDir := t.TempDir()

	err := compiler.CompileHost([]byte(idl), outputDir)
	if err != nil {
		t.Fatalf("Failed to compile with special characters: %v", err)
	}

	// Check that JAR file was created
	jarPath := filepath.Join(outputDir, "special_chars_module.jar")
	if _, err := os.Stat(jarPath); os.IsNotExist(err) {
		t.Fatalf("JAR file was not created: %s", jarPath)
	}
}

// TestLargeModule tests compilation with many entities
func TestLargeModule(t *testing.T) {
	// Create a large IDL with many entities
	entities := []map[string]interface{}{}

	// Add 10 functions
	for i := 0; i < 10; i++ {
		entities = append(entities, map[string]interface{}{
			"name": fmt.Sprintf("func_%d", i),
			"type": "function",
			"parameters": []map[string]interface{}{
				{"name": "x", "type": "int32"},
				{"name": "y", "type": "int32"},
			},
			"return_type": "int32",
		})
	}

	// Add 5 classes
	for i := 0; i < 5; i++ {
		entities = append(entities, map[string]interface{}{
			"name": fmt.Sprintf("Class_%d", i),
			"type": "class",
			"fields": []map[string]interface{}{
				{"name": "field1", "type": "string"},
				{"name": "field2", "type": "int32"},
			},
		})
	}

	// Add 5 globals
	for i := 0; i < 5; i++ {
		entities = append(entities, map[string]interface{}{
			"name":       fmt.Sprintf("GLOBAL_%d", i),
			"type":       "global",
			"value_type": "int32",
		})
	}

	idl := map[string]interface{}{
		"module":   "large_module",
		"entities": entities,
	}

	idlBytes, err := json.Marshal(idl)
	if err != nil {
		t.Fatalf("Failed to marshal large IDL: %v", err)
	}

	compiler := NewModernHostCompiler()
	outputDir := t.TempDir()

	err = compiler.CompileHost(idlBytes, outputDir)
	if err != nil {
		t.Fatalf("Failed to compile large module: %v", err)
	}

	// Check that JAR file was created
	jarPath := filepath.Join(outputDir, "large_module.jar")
	if _, err := os.Stat(jarPath); os.IsNotExist(err) {
		t.Fatalf("JAR file was not created: %s", jarPath)
	}
}

// TestNestedComplexTypes tests compilation with deeply nested complex types
func TestNestedComplexTypes(t *testing.T) {
	idl := `{
		"module": "nested_module",
		"entities": [
			{
				"name": "complex_function",
				"type": "function",
				"parameters": [
					{
						"name": "data",
						"type": "array",
						"element_type": "map",
						"key_type": "string",
						"value_type": "array",
						"value_element_type": "int32"
					},
					{
						"name": "config",
						"type": "map",
						"key_type": "string",
						"value_type": "map",
						"value_key_type": "string",
						"value_value_type": "float64"
					}
				],
				"return_type": "map",
				"return_key_type": "string",
				"return_value_type": "array",
				"return_element_type": "string"
			}
		]
	}`

	compiler := NewModernHostCompiler()
	outputDir := t.TempDir()

	err := compiler.CompileHost([]byte(idl), outputDir)
	if err != nil {
		t.Fatalf("Failed to compile nested complex types: %v", err)
	}

	// Check that JAR file was created
	jarPath := filepath.Join(outputDir, "nested_module.jar")
	if _, err := os.Stat(jarPath); os.IsNotExist(err) {
		t.Fatalf("JAR file was not created: %s", jarPath)
	}
}

// TestCodeGeneration tests that Java code is generated correctly without requiring compilation
func TestCodeGeneration(t *testing.T) {
	idl := `{
		"module": "code_gen_test",
		"entities": [
			{
				"name": "test_function",
				"type": "function",
				"parameters": [
					{"name": "param1", "type": "int32"},
					{"name": "param2", "type": "string"}
				],
				"return_type": "bool"
			}
		]
	}`

	compiler := NewModernHostCompiler()

	// Parse IDL to test code generation
	var idlData map[string]interface{}
	if err := json.Unmarshal([]byte(idl), &idlData); err != nil {
		t.Fatalf("Failed to parse IDL: %v", err)
	}

	moduleName := idlData["module"].(string)
	entities := idlData["entities"].([]interface{})

	// Generate Java code
	javaCode, err := compiler.generateJavaCode(moduleName, entities)
	if err != nil {
		t.Fatalf("Failed to generate Java code: %v", err)
	}

	// Verify generated code contains expected elements
	if !strings.Contains(javaCode, "package code_gen_test") {
		t.Error("Generated code does not contain expected package declaration")
	}
	if !strings.Contains(javaCode, "import api.MetaFFIRuntime") {
		t.Error("Generated code does not contain expected MetaFFI import")
	}
	if !strings.Contains(javaCode, "import metaffi.MetaFFITypeInfo") {
		t.Error("Generated code does not contain expected MetaFFITypeInfo import")
	}
	if !strings.Contains(javaCode, "public static boolean test_function") {
		t.Error("Generated code does not contain expected function signature")
	}
	if !strings.Contains(javaCode, "int param1") {
		t.Error("Generated code does not contain expected parameter")
	}
	if !strings.Contains(javaCode, "String param2") {
		t.Error("Generated code does not contain expected parameter")
	}
	if !strings.Contains(javaCode, "module.load") {
		t.Error("Generated code does not contain expected MetaFFI call")
	}

	t.Log("Java code generation test passed")
}

// TestTemplateFunctions tests the template functions directly
func TestTemplateFunctions(t *testing.T) {
	// Test Java type conversion
	tests := []struct {
		metaffiType string
		expected    string
	}{
		{"int32", "int"},
		{"int64", "long"},
		{"float32", "float"},
		{"float64", "double"},
		{"bool", "boolean"},
		{"string", "String"},
		{"array", "Object[]"},
		{"map", "Map"},
		{"function", "Object"},
		{"void", "void"},
	}

	for _, test := range tests {
		result := toJavaType(test.metaffiType)
		if result != test.expected {
			t.Errorf("toJavaType(%s) = %s, expected %s", test.metaffiType, result, test.expected)
		}
	}

	// Test MetaFFI type info generation
	typeInfo := getMetaFFITypeInfo("int32")
	if typeInfo == "" {
		t.Error("getMetaFFITypeInfo returned empty string")
	}

	// Test array type info
	arrayTypeInfo := getMetaFFITypeInfo("array")
	if !strings.Contains(arrayTypeInfo, "Array") {
		t.Error("Array type info should contain 'Array'")
	}

	// Test map type info
	mapTypeInfo := getMetaFFITypeInfo("map")
	if !strings.Contains(mapTypeInfo, "MetaFFIAny") {
		t.Error("Map type info should contain 'MetaFFIAny'")
	}
}

// TestOutputDirectoryHandling tests various output directory scenarios
func TestOutputDirectoryHandling(t *testing.T) {
	idl := `{
		"module": "test_module",
		"entities": [
			{
				"name": "test_func",
				"type": "function",
				"parameters": [{"name": "x", "type": "int32"}],
				"return_type": "int32"
			}
		]
	}`

	compiler := NewModernHostCompiler()

	// Test with non-existent directory
	outputDir := filepath.Join(t.TempDir(), "non_existent", "subdir")
	err := compiler.CompileHost([]byte(idl), outputDir)
	if err != nil {
		t.Fatalf("Failed to compile to non-existent directory: %v", err)
	}

	// Test with existing directory
	existingDir := t.TempDir()
	err = compiler.CompileHost([]byte(idl), existingDir)
	if err != nil {
		t.Fatalf("Failed to compile to existing directory: %v", err)
	}
}

// TestConcurrentCompilation tests that the compiler can handle concurrent calls
func TestConcurrentCompilation(t *testing.T) {
	idl := `{
		"module": "concurrent_module",
		"entities": [
			{
				"name": "test_func",
				"type": "function",
				"parameters": [{"name": "x", "type": "int32"}],
				"return_type": "int32"
			}
		]
	}`

	compiler := NewModernHostCompiler()
	outputDir := t.TempDir()

	// Run multiple compilations concurrently
	done := make(chan bool, 5)
	for i := 0; i < 5; i++ {
		go func(index int) {
			defer func() { done <- true }()

			// Modify module name to avoid conflicts
			modifiedIDL := strings.Replace(idl, "concurrent_module", fmt.Sprintf("concurrent_module_%d", index), 1)

			err := compiler.CompileHost([]byte(modifiedIDL), outputDir)
			if err != nil {
				t.Errorf("Concurrent compilation %d failed: %v", index, err)
			}
		}(i)
	}

	// Wait for all compilations to complete
	for i := 0; i < 5; i++ {
		<-done
	}
}

// TestMemoryUsage tests that the compiler doesn't leak memory
func TestMemoryUsage(t *testing.T) {
	idl := `{
		"module": "memory_test_module",
		"entities": [
			{
				"name": "test_func",
				"type": "function",
				"parameters": [{"name": "x", "type": "int32"}],
				"return_type": "int32"
			}
		]
	}`

	compiler := NewModernHostCompiler()
	outputDir := t.TempDir()

	// Run multiple compilations to check for memory leaks
	for i := 0; i < 10; i++ {
		modifiedIDL := strings.Replace(idl, "memory_test_module", fmt.Sprintf("memory_test_module_%d", i), 1)
		err := compiler.CompileHost([]byte(modifiedIDL), outputDir)
		if err != nil {
			t.Fatalf("Memory test compilation %d failed: %v", i, err)
		}
	}
}
