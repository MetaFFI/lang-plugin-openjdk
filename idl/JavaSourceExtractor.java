package java_extractor;

import com.github.javaparser.JavaParser;
import com.github.javaparser.ParseResult;
import com.github.javaparser.ast.CompilationUnit;
import com.github.javaparser.ast.body.TypeDeclaration;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.List;

public class JavaSourceExtractor implements Extractor
{
    @Override
    public JavaInfo extract(String filename) throws Exception
    {
        JavaParser jp = new JavaParser();
        System.out.println("Parsing "+filename);
        ParseResult<CompilationUnit> res = jp.parse(new File(filename));

        if(!res.isSuccessful())
        {
            StringBuilder msg = new StringBuilder("Failed to parse " + filename + " due to the following:\n");

            for(var prob : res.getProblems())
            {
                msg.append(prob.getMessage()).append("\n");
            }

            throw new RuntimeException(msg.toString());
        }

        if(res.getResult().isEmpty())
        {
            throw new RuntimeException("Failed to find Java classes or interfaces");
        }

        JavaInfo ji = new JavaInfo();
        List<ClassInfo> classes = new ArrayList<>();

        CompilationUnit cp = res.getResult().get();

        String packageName = "";
        if(!cp.getPackageDeclaration().isEmpty()){
            packageName = cp.getPackageDeclaration().get().getName().asString();
        }

        for(var type : cp.getTypes())
        {
            if(!type.isClassOrInterfaceDeclaration()){ // skip non-class / interface declaration
                continue;
            }

            classes.add(fillClassInfo(type, packageName));
        }

        ji.Classes = classes.toArray(new ClassInfo[0]);

        return ji;
    }

    private ClassInfo fillClassInfo(TypeDeclaration<?> classType, String packageName)
    {
        ClassInfo ci = new ClassInfo();
        ci.Name = classType.getNameAsString();
        ci.Package = packageName;
        if(classType.getComment().isPresent())
        {
            ci.Comment = classType.getComment().get().getContent();
        }

        List<VariableInfo> fields = new ArrayList<>();
        for(var field : classType.getFields())
        {
            if(!field.isPublic())
                continue;

            VariableInfo vi = new VariableInfo();
            vi.Name = field.asFieldDeclaration().getVariables().get(0).getNameAsString();
            vi.Type = field.asFieldDeclaration().getCommonType().asString();
            vi.IsStatic = field.asFieldDeclaration().isStatic();
            vi.IsFinal = field.asFieldDeclaration().isFinal();

            fields.add(vi);
        }
        ci.Fields = fields.toArray(new VariableInfo[0]);

        List<MethodInfo> constructors = new ArrayList<>();

        if(classType.getConstructors().size() == 0)
        {
            // if there are no constructors - Java creates a default one
            MethodInfo fi = new MethodInfo();
            fi.Comment = "Default constructor";
            fi.Name = ci.Name;
            fi.Parameters = new ParameterInfo[]{};
            constructors.add(fi);
        }
        else
        {
            for(var constructor : classType.getConstructors())
            {
                if(!constructor.isPublic())
                    continue;

                MethodInfo fi = new MethodInfo();
                if(constructor.getComment().isPresent())
                {
                    fi.Comment = constructor.getComment().get().getContent();
                }

                fi.Name = ci.Name;

                List<ParameterInfo> params = new ArrayList<>();
                for(var param : constructor.getParameters())
                {
                    ParameterInfo pi = new ParameterInfo();
                    pi.Name = param.getNameAsString();
                    pi.Type = param.getType().asString();
                    params.add(pi);
                }
                fi.Parameters = params.toArray(new ParameterInfo[0]);

                constructors.add(fi);
            }
            ci.Constructors = constructors.toArray(new MethodInfo[0]);
        }

        List<MethodInfo> methods = new ArrayList<>();
        for(var method : classType.getMethods())
        {
            if(!method.isPublic())
                continue;

            MethodInfo fi = new MethodInfo();
            fi.IsStatic = method.isStatic();
            if(method.getComment().isPresent())
            {
                fi.Comment = method.getComment().get().getContent();
            }

            fi.Name = method.asMethodDeclaration().getName().asString();

            List<ParameterInfo> params = new ArrayList<>();
            for(var param : method.getParameters())
            {
                ParameterInfo pi = new ParameterInfo();
                pi.Name = param.getNameAsString();
                pi.Type = param.getType().asString();
                params.add(pi);
            }
            fi.Parameters = params.toArray(new ParameterInfo[0]);

            fi.ReturnValue = new ParameterInfo();
            fi.ReturnValue.Type = method.getType().asString();
            fi.ReturnValue.Name = "result";

            methods.add(fi);
        }
        ci.Methods = methods.toArray(new MethodInfo[0]);

        return ci;
    }
}
