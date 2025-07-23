package java_extractor;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.jar.JarFile;
import java.util.jar.JarEntry;
import org.objectweb.asm.ClassReader;
import org.objectweb.asm.ClassVisitor;
import org.objectweb.asm.FieldVisitor;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;

public class JarExtractor implements Extractor {
    private String filePath;
    private String[] packageFilters;

    public JarExtractor(String filePath) {
        this.filePath = filePath;
        this.packageFilters = null;
    }

    public JarExtractor(String filePath, String[] packageFilters) {
        this.filePath = filePath;
        this.packageFilters = packageFilters;
    }

    @Override
    public JavaInfo extract() throws Exception {
        try (JarFile jarFile = new JarFile(filePath)) {
            List<ClassInfo> classes = new ArrayList<>();
            
            // Iterate through JAR entries
            jarFile.stream()
                .filter(entry -> entry.getName().endsWith(".class"))
                .filter(entry -> !entry.getName().startsWith("META-INF/"))
                .filter(entry -> !entry.getName().contains("$")) // Skip inner classes for now
                .filter(this::shouldIncludePackage)
                .forEach(entry -> {
                    try {
                        ClassInfo classInfo = extractClassFromJarEntry(jarFile, entry);
                        if (classInfo != null && isPublicClass(classInfo)) {
                            classes.add(classInfo);
                        }
                    } catch (Exception e) {
                        // Skip classes that can't be extracted
                        System.err.println("Warning: Could not extract class " + entry.getName() + ": " + e.getMessage());
                    }
                });
            
            JavaInfo javaInfo = new JavaInfo();
            javaInfo.Classes = classes.toArray(new ClassInfo[0]);
            
            return javaInfo;
        } catch (Exception e) {
            throw new Exception("Failed to read JAR file: " + e.getMessage());
        }
    }
    
    /**
     * Check if the package should be included based on filters
     */
    private boolean shouldIncludePackage(JarEntry entry) {
        if (packageFilters == null || packageFilters.length == 0) {
            return true; // Include all packages if no filters
        }
        
        String className = entry.getName();
        String packageName = className.contains("/") ? 
            className.substring(0, className.lastIndexOf('/')) : "";
        
        for (String filter : packageFilters) {
            if (packageName.startsWith(filter)) {
                return true;
            }
        }
        return false;
    }
    
    /**
     * Check if the class is public
     */
    private boolean isPublicClass(ClassInfo classInfo) {
        // For now, assume all extracted classes are public
        // In a full implementation, we would check the access flags
        return true;
    }
    
    /**
     * Extract class information from a JAR entry
     */
    private ClassInfo extractClassFromJarEntry(JarFile jarFile, JarEntry entry) throws Exception {
        try (InputStream is = jarFile.getInputStream(entry)) {
            ClassReader reader = new ClassReader(is);
            ClassInfo classInfo = new ClassInfo();
            
            // Create a class visitor to extract information
            ClassVisitor visitor = new ClassVisitor(Opcodes.ASM9) {
                @Override
                public void visit(int version, int access, String name, String signature, 
                                 String superName, String[] interfaces) {
                    classInfo.Name = name.substring(name.lastIndexOf('/') + 1);
                    classInfo.Package = name.contains("/") ? 
                        name.substring(0, name.lastIndexOf('/')).replace('/', '.') : "";
                    classInfo.Comment = "";
                }
                
                @Override
                public FieldVisitor visitField(int access, String name, String descriptor, 
                                             String signature, Object value) {
                    // Skip synthetic fields and non-public fields
                    if ((access & Opcodes.ACC_SYNTHETIC) != 0) {
                        return null;
                    }
                    
                    VariableInfo field = new VariableInfo();
                    field.Name = name;
                    field.Comment = "";
                    field.IsStatic = (access & Opcodes.ACC_STATIC) != 0;
                    field.IsFinal = (access & Opcodes.ACC_FINAL) != 0;
                    field.IsPublic = (access & Opcodes.ACC_PUBLIC) != 0;
                    
                    // Parse type information
                    Type type = Type.getType(descriptor);
                    field.Type = mapJavaTypeToMetaFFI(type);
                    field.TypeAlias = type.getClassName();
                    field.Dimensions = type.getSort() == Type.ARRAY ? type.getDimensions() : 0;
                    
                    // Add to fields list
                    if (classInfo.Fields == null) {
                        classInfo.Fields = new VariableInfo[0];
                    }
                    VariableInfo[] newFields = new VariableInfo[classInfo.Fields.length + 1];
                    System.arraycopy(classInfo.Fields, 0, newFields, 0, classInfo.Fields.length);
                    newFields[classInfo.Fields.length] = field;
                    classInfo.Fields = newFields;
                    
                    return null;
                }
                
                @Override
                public MethodVisitor visitMethod(int access, String name, String descriptor, 
                                               String signature, String[] exceptions) {
                    // Skip synthetic methods and non-public methods
                    if ((access & Opcodes.ACC_SYNTHETIC) != 0) {
                        return null;
                    }
                    
                    MethodInfo method = new MethodInfo();
                    method.Name = name;
                    method.Comment = "";
                    method.IsStatic = (access & Opcodes.ACC_STATIC) != 0;
                    method.IsPublic = (access & Opcodes.ACC_PUBLIC) != 0;
                    method.IsConstructor = "<init>".equals(name);
                    
                    // Parse method signature
                    Type methodType = Type.getMethodType(descriptor);
                    Type[] paramTypes = methodType.getArgumentTypes();
                    Type returnType = methodType.getReturnType();
                    
                    // Extract parameters
                    List<ParameterInfo> parameters = new ArrayList<>();
                    for (int i = 0; i < paramTypes.length; i++) {
                        ParameterInfo param = new ParameterInfo();
                        param.Name = "p" + i;
                        param.Comment = "";
                        param.Type = mapJavaTypeToMetaFFI(paramTypes[i]);
                        param.TypeAlias = paramTypes[i].getClassName();
                        param.Dimensions = paramTypes[i].getSort() == Type.ARRAY ? paramTypes[i].getDimensions() : 0;
                        parameters.add(param);
                    }
                    method.Parameters = parameters.toArray(new ParameterInfo[0]);
                    
                    // Extract return values
                    if (returnType.getSort() != Type.VOID) {
                        VariableInfo returnValue = new VariableInfo();
                        returnValue.Name = "result";
                        returnValue.Comment = "";
                        returnValue.Type = mapJavaTypeToMetaFFI(returnType);
                        returnValue.TypeAlias = returnType.getClassName();
                        returnValue.Dimensions = returnType.getSort() == Type.ARRAY ? returnType.getDimensions() : 0;
                        returnValue.IsStatic = false;
                        returnValue.IsFinal = false;
                        returnValue.IsPublic = true;
                        method.ReturnValues = new VariableInfo[]{returnValue};
                    } else {
                        method.ReturnValues = new VariableInfo[0];
                    }
                    
                    // Add to appropriate method list
                    if (method.IsConstructor) {
                        if (classInfo.Constructors == null) {
                            classInfo.Constructors = new MethodInfo[0];
                        }
                        MethodInfo[] newConstructors = new MethodInfo[classInfo.Constructors.length + 1];
                        System.arraycopy(classInfo.Constructors, 0, newConstructors, 0, classInfo.Constructors.length);
                        newConstructors[classInfo.Constructors.length] = method;
                        classInfo.Constructors = newConstructors;
                    } else {
                        if (classInfo.Methods == null) {
                            classInfo.Methods = new MethodInfo[0];
                        }
                        MethodInfo[] newMethods = new MethodInfo[classInfo.Methods.length + 1];
                        System.arraycopy(classInfo.Methods, 0, newMethods, 0, classInfo.Methods.length);
                        newMethods[classInfo.Methods.length] = method;
                        classInfo.Methods = newMethods;
                    }
                    
                    return null;
                }
            };
            
            reader.accept(visitor, 0);
            
            return classInfo;
        }
    }
    
    /**
     * Map Java type to MetaFFI type
     */
    private String mapJavaTypeToMetaFFI(Type type) {
        switch (type.getSort()) {
            case Type.BOOLEAN:
                return "BOOL";
            case Type.BYTE:
                return "INT8";
            case Type.SHORT:
                return "INT16";
            case Type.INT:
                return "INT32";
            case Type.LONG:
                return "INT64";
            case Type.FLOAT:
                return "FLOAT32";
            case Type.DOUBLE:
                return "FLOAT64";
            case Type.CHAR:
                return "INT16";
            case Type.ARRAY:
                Type elementType = type.getElementType();
                String baseType = mapJavaTypeToMetaFFI(elementType);
                return baseType + "_ARRAY";
            case Type.OBJECT:
                String className = type.getClassName();
                if ("java.lang.String".equals(className)) {
                    return "STRING8";
                } else if ("java.lang.Object".equals(className)) {
                    return "HANDLE";
                } else {
                    return "HANDLE";
                }
            default:
                return "HANDLE";
        }
    }
} 