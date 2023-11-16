package java_extractor;

import org.objectweb.asm.*;
import org.objectweb.asm.tree.ClassNode;
import org.objectweb.asm.tree.FieldNode;
import org.objectweb.asm.tree.MethodNode;

import java.io.FileInputStream;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.*;

public class BytecodeExtractor implements Extractor
{
    @Override
    public JavaInfo extract(String filename) throws Exception
    {
        try (InputStream inputStream = new FileInputStream(filename)) {
            return extract(inputStream);
        }
    }


    public JavaInfo extract(InputStream s) throws Exception
    {
        ClassReader reader = new ClassReader(s);
        ClassNode classNode = new ClassNode();
        reader.accept(classNode, ClassReader.EXPAND_FRAMES);

        JavaInfo javaInfo = new JavaInfo();
        ClassInfo classInfo = new ClassInfo();
        javaInfo.Classes = new ClassInfo[]{classInfo};

        classInfo.Name = classNode.name.replace('/', '.');
        int lastDotIndex = classInfo.Name.lastIndexOf('.');
        if (lastDotIndex != -1)
        {
            classInfo.Package = classInfo.Name.substring(0, lastDotIndex);
        }
        classInfo.Name = classInfo.Name.substring(lastDotIndex+1);

        List<VariableInfo> vars = new ArrayList<>();
        for (int i = 0; i < classNode.fields.size(); i++)
        {
            FieldNode fieldNode = classNode.fields.get(i);

            if((fieldNode.access & Opcodes.ACC_PUBLIC) == 0) // skip non-public fields
                continue;

            VariableInfo variableInfo = new VariableInfo();
            variableInfo.Name = (fieldNode.name == null || fieldNode.name.isEmpty()) ? String.format("field%d", i) : fieldNode.name;
            variableInfo.Type = Type.getType(fieldNode.desc).getClassName().replace("java.lang.", "");
            variableInfo.IsStatic = (fieldNode.access & Opcodes.ACC_STATIC) != 0;
            variableInfo.IsFinal = (fieldNode.access & Opcodes.ACC_FINAL) != 0;
            vars.add(variableInfo);
        }
        classInfo.Fields = vars.toArray(new VariableInfo[0]);

        List<MethodInfo> methods = new ArrayList<>();
        List<MethodInfo> constructors = new ArrayList<>();
        for (int i = 0; i < classNode.methods.size(); i++)
        {
            MethodNode methodNode = classNode.methods.get(i);

            if((methodNode.access & Opcodes.ACC_PUBLIC) == 0) // skip non-public methods
                continue;

            MethodInfo methodInfo = new MethodInfo();
            methodInfo.Name = methodNode.name;
            methodInfo.IsStatic = (methodNode.access & Opcodes.ACC_STATIC) != 0;
            Type[] argumentTypes = Type.getArgumentTypes(methodNode.desc);

            List<ParameterInfo> params = new ArrayList<>();
            for (int j = 0; j < argumentTypes.length; j++)
            {
                ParameterInfo parameterInfo = new ParameterInfo();
                parameterInfo.Type = convertType(argumentTypes[j]);

                if(methodNode.localVariables != null && methodNode.localVariables.size() > j+1)
                {
                    int index = methodNode.localVariables.get(j + 1).index;
                    parameterInfo.Name = methodNode.localVariables.get(index).name;
                }
                else
                {
                    parameterInfo.Name = String.format("p%d", j);
                }
                params.add(parameterInfo);
            }
            methodInfo.Parameters = params.toArray(new ParameterInfo[0]);

            ParameterInfo returnValue = new ParameterInfo();
            returnValue.Type = convertType(Type.getReturnType(methodNode.desc));
            returnValue.Name = "r0";
            methodInfo.ReturnValue = returnValue;

            if(methodInfo.Name.equals("<init>"))
                constructors.add(methodInfo);
            else
                methods.add(methodInfo);
        }

        classInfo.Constructors = constructors.toArray(new MethodInfo[0]);
        classInfo.Methods = methods.toArray(new MethodInfo[0]);


//        Set<String> referencedTypes = new HashSet<>();
//        ClassVisitor visitor = new ClassVisitor(Opcodes.ASM9)
//        {
//            @Override
//            public FieldVisitor visitField(int access, String name, String descriptor, String signature, Object value)
//            {
//                Type type = Type.getType(descriptor);
//                if (type.getSort() == Type.OBJECT)
//                {
//                    referencedTypes.add(type.getClassName());
//                }
//                return super.visitField(access, name, descriptor, signature, value);
//            }
//
//            @Override
//            public MethodVisitor visitMethod(int access, String name, String descriptor, String signature, String[] exceptions)
//            {
//                Type[] argumentTypes = Type.getArgumentTypes(descriptor);
//                for (Type type : argumentTypes)
//                {
//                    if (type.getSort() == Type.OBJECT)
//                    {
//                        referencedTypes.add(type.getClassName());
//                    }
//                }
//                Type returnType = Type.getReturnType(descriptor);
//                if (returnType.getSort() == Type.OBJECT)
//                {
//                    referencedTypes.add(returnType.getClassName());
//                }
//                return super.visitMethod(access, name, descriptor, signature, exceptions);
//            }
//        };
//        reader.accept(visitor, 0);

        // Here you might want to do something with the referencedTypes

        return javaInfo;
    }

    private static String convertType(Type type) {
        switch (type.getSort()) {
            case Type.BOOLEAN:
                return "boolean";
            case Type.CHAR:
                return "char";
            case Type.BYTE:
                return "byte";
            case Type.SHORT:
                return "short";
            case Type.INT:
                return "int";
            case Type.FLOAT:
                return "float";
            case Type.LONG:
                return "long";
            case Type.DOUBLE:
                return "double";
            case Type.ARRAY:
                return convertType(type.getElementType()) + "[]";
            case Type.OBJECT:
                return type.getClassName().replace("java.lang.", "");
            case Type.VOID:
                return "void";
            default:
                return "unknown";
        }
    }
}
