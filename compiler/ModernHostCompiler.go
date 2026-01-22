package main

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"text/template"

	"github.com/MetaFFI/sdk/idl_entities/go/IDL"
)

// MetaFFIHostTemplate is the template for generating Java host wrapper code
const MetaFFIHostTemplate = `package {{.ModuleName}};

import api.MetaFFIModule;
import api.MetaFFIRuntime;
import metaffi.Caller;
import metaffi.MetaFFITypeInfo;
import java.util.Map;

public class {{.ModuleName}} {
    private static MetaFFIRuntime metaffi;
    private static MetaFFIModule module;
    
    static {
        try {
            metaffi = new MetaFFIRuntime("xllr.jvm");
            module = new MetaFFIModule(metaffi, "{{.ModuleName}}");
        } catch (Exception e) {
            throw new RuntimeException("Failed to initialize MetaFFI", e);
        }
    }
    
    {{range .Entities}}
    {{if eq .type "function"}}
    public static {{toJavaType .return_type}} {{.name}}({{range $index, $param := .parameters}}{{if $index}}, {{end}}{{toJavaType $param.type}} {{$param.name}}{{end}}) {
        try {
            {{if ne .return_type "void"}}Object[] result = {{end}}module.load("{{.name}}", new MetaFFITypeInfo[]{ {{range $index, $param := .parameters}}{{if $index}}, {{end}}new MetaFFITypeInfo({{getMetaFFITypeInfo $param.type}}){{end}} }, new MetaFFITypeInfo[]{ new MetaFFITypeInfo({{getMetaFFITypeInfo .return_type}}) }).call(new Object[]{ {{range $index, $param := .parameters}}{{if $index}}, {{end}}{{$param.name}}{{end}} });
            {{if ne .return_type "void"}}return ({{toJavaType .return_type}})result[0];{{end}}
        } catch (Exception e) {
            throw new RuntimeException("Failed to call {{.name}}", e);
        }
    }
    {{else if eq .type "class"}}
    public static class {{.name}} {
        private Object instance;
        
        public {{.name}}(Object instance) {
            this.instance = instance;
        }
        
        {{range .fields}}
        public {{toJavaType .type}} {{.name}};
        {{end}}
    }
    {{else if eq .type "global"}}
    public static {{toJavaType .value_type}} {{.name}};
    {{end}}
    {{end}}
}`

// --------------------------------------------------------------------
type ModernHostCompiler struct{}

// --------------------------------------------------------------------
func NewModernHostCompiler() *ModernHostCompiler {
	return &ModernHostCompiler{}
}

// --------------------------------------------------------------------
// CompileHost compiles IDL to Java host wrapper using the new MetaFFI API
func (this *ModernHostCompiler) CompileHost(idlData []byte, outputDir string) error {
	// Parse IDL
	var idl map[string]interface{}
	if err := json.Unmarshal(idlData, &idl); err != nil {
		return fmt.Errorf("failed to parse IDL: %v", err)
	}

	// Validate IDL structure
	moduleName, ok := idl["module"].(string)
	if !ok {
		return fmt.Errorf("invalid IDL: missing or invalid module name")
	}

	entities, ok := idl["entities"].([]interface{})
	if !ok {
		return fmt.Errorf("invalid IDL: missing or invalid entities array")
	}

	// Validate entities
	for i, entity := range entities {
		entityMap, ok := entity.(map[string]interface{})
		if !ok {
			return fmt.Errorf("invalid IDL: entity %d is not a map", i)
		}

		// Validate entity has required fields
		_, ok = entityMap["name"].(string)
		if !ok {
			return fmt.Errorf("invalid IDL: entity %d missing or invalid name", i)
		}

		entityType, ok := entityMap["type"].(string)
		if !ok {
			return fmt.Errorf("invalid IDL: entity %d missing or invalid type", i)
		}

		// Validate entity type
		switch entityType {
		case "function":
			if err := this.validateFunction(entityMap, i); err != nil {
				return err
			}
		case "class":
			if err := this.validateClass(entityMap, i); err != nil {
				return err
			}
		case "global":
			if err := this.validateGlobal(entityMap, i); err != nil {
				return err
			}
		default:
			return fmt.Errorf("invalid IDL: entity %d has invalid type '%s'", i, entityType)
		}
	}

	// Create output directory if it doesn't exist
	if err := os.MkdirAll(outputDir, 0755); err != nil {
		return fmt.Errorf("failed to create output directory: %v", err)
	}

	// Generate Java source code
	javaCode, err := this.generateJavaCode(moduleName, entities)
	if err != nil {
		return fmt.Errorf("failed to generate Java code: %v", err)
	}

	// Create temporary directory for compilation
	tempDir, err := os.MkdirTemp("", "metaffi_jvm_*")
	if err != nil {
		return fmt.Errorf("failed to create temp directory: %v", err)
	}
	defer os.RemoveAll(tempDir)

	// Write Java source file
	javaFilePath := filepath.Join(tempDir, moduleName+".java")
	if err := ioutil.WriteFile(javaFilePath, []byte(javaCode), 0644); err != nil {
		return fmt.Errorf("failed to write Java source file: %v", err)
	}

	// Compile Java to class files
	if err := this.compileJava(javaFilePath, tempDir); err != nil {
		return fmt.Errorf("failed to compile Java: %v", err)
	}

	// Create JAR file
	jarPath := filepath.Join(outputDir, moduleName+".jar")
	if err := this.createJAR(tempDir, jarPath); err != nil {
		return fmt.Errorf("failed to create JAR: %v", err)
	}

	return nil
}

// --------------------------------------------------------------------
// Legacy Compile method for backward compatibility
func (this *ModernHostCompiler) Compile(definition *IDL.IDLDefinition, outputDir string, outputFilename string, hostOptions map[string]string) (err error) {
	// Convert old IDL format to new format
	idl := map[string]interface{}{
		"module":   definition.Modules[0].Name,
		"entities": []interface{}{},
	}

	// Convert functions
	for _, function := range definition.Modules[0].Functions {
		entity := map[string]interface{}{
			"name":        function.Name,
			"type":        "function",
			"parameters":  []interface{}{},
			"return_type": "void",
		}

		// Convert parameters
		for _, param := range function.Parameters {
			paramMap := map[string]interface{}{
				"name": param.Name,
				"type": this.convertOldType(string(param.Type)),
			}
			entity["parameters"] = append(entity["parameters"].([]interface{}), paramMap)
		}

		// Convert return type
		if len(function.ReturnValues) > 0 {
			entity["return_type"] = this.convertOldType(string(function.ReturnValues[0].Type))
		}

		idl["entities"] = append(idl["entities"].([]interface{}), entity)
	}

	// Convert classes
	for _, class := range definition.Modules[0].Classes {
		entity := map[string]interface{}{
			"name":   class.Name,
			"type":   "class",
			"fields": []interface{}{},
		}

		// Convert fields (simplified - would need more complex mapping)
		entity["fields"] = append(entity["fields"].([]interface{}), map[string]interface{}{
			"name": "instance",
			"type": "handle",
		})

		idl["entities"] = append(idl["entities"].([]interface{}), entity)
	}

	// Convert to JSON
	idlData, err := json.Marshal(idl)
	if err != nil {
		return fmt.Errorf("failed to marshal IDL: %v", err)
	}

	// Use new CompileHost method
	return this.CompileHost(idlData, outputDir)
}

// --------------------------------------------------------------------
func (this *ModernHostCompiler) convertOldType(oldType string) string {
	switch oldType {
	case "int32":
		return "int32"
	case "int64":
		return "int64"
	case "float32":
		return "float32"
	case "float64":
		return "float64"
	case "bool":
		return "bool"
	case "string":
		return "string"
	case "handle":
		return "handle"
	case "void":
		return "void"
	default:
		return "object"
	}
}

// --------------------------------------------------------------------
func (this *ModernHostCompiler) generateJavaCode(moduleName string, entities []interface{}) (string, error) {
	// Create template data
	data := map[string]interface{}{
		"ModuleName": moduleName,
		"Entities":   entities,
	}

	// Parse and execute template
	tmpl, err := template.New("host").Funcs(templatesFuncMap).Parse(MetaFFIHostTemplate)
	if err != nil {
		return "", fmt.Errorf("failed to parse template: %v", err)
	}

	var buf strings.Builder
	if err := tmpl.Execute(&buf, data); err != nil {
		return "", fmt.Errorf("failed to execute template: %v", err)
	}

	return buf.String(), nil
}

// --------------------------------------------------------------------
func (this *ModernHostCompiler) compileJava(javaFilePath, outputDir string) error {
	// Get classpath
	classpath := this.getClassPath()

	// Compile Java file
	cmd := exec.Command("javac", "-d", outputDir, "-cp", classpath, javaFilePath)
	output, err := cmd.CombinedOutput()
	if err != nil {
		return fmt.Errorf("javac failed: %v\nOutput: %s", err, string(output))
	}

	return nil
}

// --------------------------------------------------------------------
func (this *ModernHostCompiler) createJAR(classDir, jarPath string) error {
	// Find all class files recursively
	var classFiles []string
	err := filepath.Walk(classDir, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}
		if !info.IsDir() && strings.HasSuffix(path, ".class") {
			classFiles = append(classFiles, path)
		}
		return nil
	})
	if err != nil {
		return fmt.Errorf("failed to find class files: %v", err)
	}

	if len(classFiles) == 0 {
		return fmt.Errorf("no class files found in %s", classDir)
	}

	// Create JAR file
	args := []string{"cf", jarPath}
	for _, classFile := range classFiles {
		// Get relative path for JAR
		relPath, err := filepath.Rel(classDir, classFile)
		if err != nil {
			return fmt.Errorf("failed to get relative path: %v", err)
		}
		args = append(args, relPath)
	}

	cmd := exec.Command("jar", args...)
	cmd.Dir = classDir
	output, err := cmd.CombinedOutput()
	if err != nil {
		return fmt.Errorf("jar command failed: %v\nOutput: %s", err, string(output))
	}

	return nil
}

// --------------------------------------------------------------------
func (this *ModernHostCompiler) getClassPath() string {
	metaffiHome := os.Getenv("METAFFI_HOME")
	if metaffiHome == "" {
		metaffiHome = "."
	}

	classPath := []string{
		".",
		filepath.Join(metaffiHome, "jvm", "metaffi.api.jar"),
		filepath.Join(metaffiHome, "jvm", "xllr.jvm.bridge.jar"),
	}

	return strings.Join(classPath, string(os.PathListSeparator))
}

// --------------------------------------------------------------------
func (this *ModernHostCompiler) validateFunction(entity map[string]interface{}, index int) error {
	// Validate parameters
	parameters, ok := entity["parameters"].([]interface{})
	if !ok {
		return fmt.Errorf("invalid IDL: function entity %d missing or invalid parameters", index)
	}

	for i, param := range parameters {
		paramMap, ok := param.(map[string]interface{})
		if !ok {
			return fmt.Errorf("invalid IDL: function entity %d parameter %d is not a map", index, i)
		}

		// Validate parameter name
		_, ok = paramMap["name"].(string)
		if !ok {
			return fmt.Errorf("invalid IDL: function entity %d parameter %d missing or invalid name", index, i)
		}

		// Validate parameter type
		paramType, ok := paramMap["type"].(string)
		if !ok {
			return fmt.Errorf("invalid IDL: function entity %d parameter %d missing or invalid type", index, i)
		}

		if !this.isValidType(paramType) {
			return fmt.Errorf("invalid IDL: function entity %d parameter %d has invalid type '%s'", index, i, paramType)
		}
	}

	// Validate return type
	returnType, ok := entity["return_type"].(string)
	if !ok {
		return fmt.Errorf("invalid IDL: function entity %d missing or invalid return_type", index)
	}

	if !this.isValidType(returnType) {
		return fmt.Errorf("invalid IDL: function entity %d has invalid return type '%s'", index, returnType)
	}

	return nil
}

// --------------------------------------------------------------------
func (this *ModernHostCompiler) validateClass(entity map[string]interface{}, index int) error {
	// Validate fields
	fields, ok := entity["fields"].([]interface{})
	if !ok {
		return fmt.Errorf("invalid IDL: class entity %d missing or invalid fields", index)
	}

	for i, field := range fields {
		fieldMap, ok := field.(map[string]interface{})
		if !ok {
			return fmt.Errorf("invalid IDL: class entity %d field %d is not a map", index, i)
		}

		// Validate field name
		_, ok = fieldMap["name"].(string)
		if !ok {
			return fmt.Errorf("invalid IDL: class entity %d field %d missing or invalid name", index, i)
		}

		// Validate field type
		fieldType, ok := fieldMap["type"].(string)
		if !ok {
			return fmt.Errorf("invalid IDL: class entity %d field %d missing or invalid type", index, i)
		}

		if !this.isValidType(fieldType) {
			return fmt.Errorf("invalid IDL: class entity %d field %d has invalid type '%s'", index, i, fieldType)
		}
	}

	return nil
}

// --------------------------------------------------------------------
func (this *ModernHostCompiler) validateGlobal(entity map[string]interface{}, index int) error {
	// Validate value type
	valueType, ok := entity["value_type"].(string)
	if !ok {
		return fmt.Errorf("invalid IDL: global entity %d missing or invalid value_type", index)
	}

	if !this.isValidType(valueType) {
		return fmt.Errorf("invalid IDL: global entity %d has invalid value type '%s'", index, valueType)
	}

	return nil
}

// --------------------------------------------------------------------
func (this *ModernHostCompiler) isValidType(typeName string) bool {
	validTypes := map[string]bool{
		"int32":    true,
		"int64":    true,
		"float32":  true,
		"float64":  true,
		"bool":     true,
		"string":   true,
		"array":    true,
		"map":      true,
		"function": true,
		"void":     true,
		"handle":   true,
		"object":   true,
	}

	return validTypes[typeName]
}

//--------------------------------------------------------------------
