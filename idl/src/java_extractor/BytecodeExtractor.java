package java_extractor;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import org.objectweb.asm.ClassReader;
import org.objectweb.asm.ClassVisitor;
import org.objectweb.asm.FieldVisitor;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;

public class BytecodeExtractor implements Extractor {
    private String filePath;

    public BytecodeExtractor(String filePath) {
        this.filePath = filePath;
    }

    @Override
    public JavaInfo extract() throws Exception {
        try (FileInputStream fis = new FileInputStream(filePath)) {
            ClassReader reader = new ClassReader(fis);
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
            
            JavaInfo javaInfo = new JavaInfo();
            javaInfo.Classes = new ClassInfo[]{classInfo};
            
            return javaInfo;
        } catch (IOException e) {
            throw new Exception("Failed to read class file: " + e.getMessage());
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