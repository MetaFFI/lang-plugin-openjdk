package java_extractor;

import org.objectweb.asm.*;
import org.objectweb.asm.tree.ClassNode;
import org.objectweb.asm.tree.FieldNode;
import org.objectweb.asm.tree.InnerClassNode;
import org.objectweb.asm.tree.MethodNode;

import java.io.FileInputStream;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.*;
import java.util.function.Function;

public class BytecodeExtractor implements Extractor
{
    @Override
    public JavaInfo extract(String filename) throws Exception
    {
        try (InputStream inputStream = new FileInputStream(filename)) {
            return extract(inputStream, null);
        }
    }


    public JavaInfo extract(InputStream s, JarExtractor.ThrowingFunction<Type,Boolean> isPublicMethodArgument) throws Exception
    {
        ClassReader reader = new ClassReader(s);
        ClassNode classNode = new ClassNode();
        reader.accept(classNode, ClassReader.EXPAND_FRAMES);

        return this.extract(classNode, isPublicMethodArgument);
    }

    private void fillPackageAndClassName(ClassNode classNode, ClassInfo classInfo)
    {
        classInfo.Name = classNode.name.replace('/', '.');
        int lastDotIndex = classInfo.Name.lastIndexOf('.');
        if (lastDotIndex != -1)
        {
            classInfo.Package = classInfo.Name.substring(0, lastDotIndex);
        }
        classInfo.Name = classInfo.Name.substring(lastDotIndex+1);
    }

    private List<VariableInfo> getVariables(ClassNode classNode)
    {
        List<VariableInfo> vars = new ArrayList<>();
        for (int i = 0; i < classNode.fields.size(); i++)
        {
            FieldNode fieldNode = classNode.fields.get(i);

            if(!this.isPublic(fieldNode.access)) // skip non-public fields
                continue;

            VariableInfo variableInfo = new VariableInfo();
            variableInfo.Name = (fieldNode.name == null || fieldNode.name.isEmpty()) ? String.format("field%d", i) : fieldNode.name;
            variableInfo.Type = Type.getType(fieldNode.desc).getClassName().replace("java.lang.", "");
            variableInfo.IsStatic = this.isStatic(fieldNode.access);
            variableInfo.IsFinal = this.isFinal(fieldNode.access);
            vars.add(variableInfo);
        }

        return vars;
    }

    private boolean isStatic(int access)
    {
        return (access & Opcodes.ACC_STATIC) != 0;
    }

    private boolean isFinal(int access)
    {
        return (access & Opcodes.ACC_FINAL) != 0;
    }

    private boolean isPublic(int access)
    {
        return (access & Opcodes.ACC_PUBLIC) != 0;
    }

    private boolean isSynthetic(int access)
    {
        return (access & Opcodes.ACC_SYNTHETIC) != 0;
    }

    private boolean isInterface(int access)
    {
        return (access & Opcodes.ACC_INTERFACE) != 0;
    }

    private boolean isAbstract(int access)
    {
        return (access & Opcodes.ACC_ABSTRACT) != 0;
    }

    private void fillConstructorsAndMethods(ClassNode classNode, List<MethodInfo> methods, List<MethodInfo> constructors, JarExtractor.ThrowingFunction<Type,Boolean> isPublicMethodArgument) throws Exception
    {
        for (int i = 0; i < classNode.methods.size(); i++)
        {
            MethodNode methodNode = classNode.methods.get(i);

            if(!this.isPublic(methodNode.access)) // skip non-public methods
                continue;

            if(this.isSynthetic(methodNode.access)) // skip synthetically generated methods
                continue;

            MethodInfo methodInfo = new MethodInfo();
            methodInfo.Name = methodNode.name;
            methodInfo.IsStatic = this.isStatic(methodNode.access);
            Type[] argumentTypes = Type.getArgumentTypes(methodNode.desc);

            List<ParameterInfo> params = new ArrayList<>();
            boolean isSkipMethod = false;
            for (int j = 0; j < argumentTypes.length; j++)
            {
                if(isPublicMethodArgument != null && !isPublicMethodArgument.apply(argumentTypes[j]))
                {
                    isSkipMethod = true;
                    break;
                }

                ParameterInfo parameterInfo = new ParameterInfo();
                parameterInfo.Type = argumentTypes[j].getClassName();

                // get parameter name - if the bytecode generated with debug information
                if(methodNode.localVariables != null &&
                    methodNode.localVariables.size() > j+1 &&
                    methodNode.localVariables.size() > methodNode.localVariables.get(j + 1).index)
                {
                    // generated with debug information
                    int index = methodNode.localVariables.get(j + 1).index;
                    parameterInfo.Name = methodNode.localVariables.get(index).name;
                }
                else
                {
                    // no debug information
                    parameterInfo.Name = String.format("p%d", j);
                }
                params.add(parameterInfo);
            }

            // skip method as one of the parameters is not available
            // from outside class package.
            // it is applicable only in cases the class is part of a JAR.
            if(isSkipMethod)
                continue;

            methodInfo.Parameters = params.toArray(new ParameterInfo[0]);

            ParameterInfo returnValue = new ParameterInfo();
            returnValue.Type = Type.getReturnType(methodNode.desc).getClassName();
            returnValue.Name = "r0";
            methodInfo.ReturnValue = returnValue;

            if(methodInfo.Name.equals("<init>"))
                constructors.add(methodInfo);
            else
                methods.add(methodInfo);
        }
    }

    public JavaInfo extract(ClassNode classNode, JarExtractor.ThrowingFunction<Type,Boolean> isPublicMethodArgument) throws Exception
    {
        if(!this.isPublic(classNode.access)) // skip non-public classes
            return null;

//        if(this.isInterface(classNode.access)) // skip interfaces
//            return null;

//        if(this.isAbstract(classNode.access)) // skip abstract classes
//            return null;

        JavaInfo javaInfo = new JavaInfo();
        ClassInfo classInfo = new ClassInfo();
        javaInfo.Classes = new ClassInfo[]{classInfo};

        this.fillPackageAndClassName(classNode, classInfo);

        classInfo.Fields = getVariables(classNode).toArray(new VariableInfo[0]);

        List<MethodInfo> methods = new ArrayList<>();
        List<MethodInfo> constructors = new ArrayList<>();

        this.fillConstructorsAndMethods(classNode, methods, constructors, isPublicMethodArgument);

        // if the class is interface or abstract, it cannot have constructors
        if(!this.isInterface(classNode.access) && !this.isAbstract(classNode.access))
            classInfo.Constructors = constructors.toArray(new MethodInfo[0]);

        classInfo.Methods = methods.toArray(new MethodInfo[0]);

        return javaInfo;
    }

    private static String convertType(Type type)
    {
        switch (type.getSort()) {
            case Type.FieldType_BOOLEAN:
                return "boolean";
            case Type.FieldType_CHAR:
                return "char";
            case Type.FieldType_BYTE:
                return "byte";
            case Type.FieldType_SHORT:
                return "short";
            case Type.FieldType_INT:
                return "int";
            case Type.FieldType_FLOAT:
                return "float";
            case Type.FieldType_LONG:
                return "long";
            case Type.FieldType_DOUBLE:
                return "double";
            case Type.ARRAY:
                return convertType(type.getElementType()) + "[]";
            case Type.FieldType_OBJECT:
                return type.getClassName().replace("java.lang.", "");
            case Type.VOID:
                return "void";
            default:
                return "unknown";
        }
    }
}
