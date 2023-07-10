package main

import (
	"fmt"
	. "github.com/MetaFFI/idl-plugin-java/javaextractor"
	"github.com/MetaFFI/plugin-sdk/compiler/go/IDL"
	"strings"
)

var classes map[string]*IDL.ClassDefinition

//--------------------------------------------------------------------
func ExtractClasses(javainfo *JavaInfo) ([]*IDL.ClassDefinition, error) {
	
	classes = make(map[string]*IDL.ClassDefinition)
	
	javaclses, err := javainfo.Classes_MetaFFIGetter()
	if err != nil {
		return nil, err
	}
	
	for _, c := range javaclses {
		clsName, err := c.Name_MetaFFIGetter()
		if err != nil {
			return nil, err
		}
		
		javacls := IDL.NewClassDefinition(clsName)
		
		packageName, err := c.Package_MetaFFIGetter()
		if err != nil {
			return nil, err
		}
		
		//if packageName != "" {
		//	javacls.SetFunctionPath("package", packageName)
		//}
		
		comment, err := c.Comment_MetaFFIGetter()
		if err != nil {
			return nil, err
		}
		javacls.Comment = comment
		
		fields, err := c.Fields_MetaFFIGetter()
		if err != nil {
			return nil, err
		}
		
		// Fields
		for _, f := range fields {
			name, err := f.Name_MetaFFIGetter()
			if err != nil {
				return nil, err
			}
			
			typy, err := f.Type_MetaFFIGetter()
			if err != nil {
				return nil, err
			}
			
			isStatic, err := f.IsStatic_MetaFFIGetter()
			if err != nil {
				return nil, err
			}
			
			isFinal, err := f.IsFinal_MetaFFIGetter()
			if err != nil {
				return nil, err
			}
			
			var fdecl *IDL.FieldDefinition
			if !isFinal {
				fdecl = IDL.NewFieldDefinition(javacls, name, javaTypeToMFFI(typy), "Get"+name, "Set"+name, !isStatic)
				fdecl.Getter.SetFunctionPath("entrypoint_function", fmt.Sprintf("EntryPoint_%v_get_%v", javacls.Name, fdecl.Getter.Name))
				fdecl.Setter.SetFunctionPath("entrypoint_function", fmt.Sprintf("EntryPoint_%v_set_%v", javacls.Name, fdecl.Setter.Name))
			} else {
				fdecl = IDL.NewFieldDefinition(javacls, name, javaTypeToMFFI(typy), "Get"+name, "", !isStatic)
				fdecl.Getter.SetFunctionPath("entrypoint_function", fmt.Sprintf("EntryPoint_%v_get_%v", javacls.Name, fdecl.Getter.Name))
			}
			
			javacls.AddField(fdecl)
		}
		
		javaconstructors, err := c.Constructors_MetaFFIGetter()
		if err != nil {
			return nil, err
		}
		
		for _, f := range javaconstructors {
			javaconst := IDL.NewFunctionDefinition(javacls.Name)
			javaconst.SetFunctionPath("entrypoint_function", fmt.Sprintf("EntryPoint_%v_%v", javacls.Name, javaconst.Name))
			
			comment, err := f.Comment_MetaFFIGetter()
			if err != nil {
				return nil, err
			}
			javaconst.Comment = comment
			
			params, err := f.Parameters_MetaFFIGetter()
			if err != nil {
				return nil, err
			}
			
			for _, p := range params {
				
				name, err := p.Name_MetaFFIGetter()
				if err != nil {
					return nil, err
				}
				
				javaType, err := p.Type_MetaFFIGetter()
				if err != nil {
					return nil, err
				}
				
				mffiType := javaTypeToMFFI(javaType)
				var talias string
				if mffiType == IDL.HANDLE || mffiType == IDL.HANDLE_ARRAY {
					talias = javaType
				}
				
				dims := 0
				if strings.LastIndex(string(mffiType), "_array") >= 0 {
					dims = 1
				}
				
				javaconst.AddParameter(IDL.NewArgArrayDefinitionWithAlias(name, mffiType, dims, talias))
			}
			
			javaconst.AddReturnValues(IDL.NewArgDefinitionWithAlias("instance", IDL.HANDLE, javacls.Name))
			
			javacls.AddConstructor(IDL.NewConstructorDefinitionFromFunctionDefinition(javaconst))
		}
		
		javamethods, err := c.Methods_MetaFFIGetter()
		if err != nil {
			return nil, err
		}
		
		for _, f := range javamethods {
			name, err := f.Name_MetaFFIGetter()
			if err != nil {
				return nil, err
			}
			
			javameth := IDL.NewFunctionDefinition(name)
			
			javameth.SetFunctionPath("entrypoint_function", fmt.Sprintf("EntryPoint_%v_%v", javacls.Name, javameth.Name))
			
			comment, err := f.Comment_MetaFFIGetter()
			if err != nil {
				return nil, err
			}
			javameth.Comment = comment
			
			isStatic, err := f.IsStatic_MetaFFIGetter()
			if err != nil {
				return nil, err
			}
			
			params, err := f.Parameters_MetaFFIGetter()
			if err != nil {
				return nil, err
			}
			
			for _, p := range params {
				
				name, err := p.Name_MetaFFIGetter()
				if err != nil {
					return nil, err
				}
				
				javaType, err := p.Type_MetaFFIGetter()
				if err != nil {
					return nil, err
				}
				
				mffiType := javaTypeToMFFI(javaType)
				var talias string
				if mffiType == IDL.HANDLE || mffiType == IDL.HANDLE_ARRAY {
					talias = javaType
				}
				
				dims := 0
				if strings.LastIndex(string(mffiType), "_array") >= 0 {
					dims = 1
				}
				
				javameth.AddParameter(IDL.NewArgArrayDefinitionWithAlias(name, mffiType, dims, talias))
			}
			
			retval, err := f.ReturnValue_MetaFFIGetter()
			if err != nil {
				return nil, err
			}
			
			retval_name, err := retval.Name_MetaFFIGetter()
			if err != nil {
				return nil, err
			}
			
			typename, err := retval.Type_MetaFFIGetter()
			if err != nil {
				return nil, err
			}
			
			if typename != "void" {
				mffiType := javaTypeToMFFI(typename)
				
				alias := ""
				if mffiType == IDL.HANDLE || mffiType == IDL.HANDLE_ARRAY {
					alias = typename
				}
				
				dims := 0
				if strings.LastIndex(string(mffiType), "_array") >= 0 {
					dims = 1
				}
				
				javameth.AddReturnValues(IDL.NewArgArrayDefinitionWithAlias(retval_name, mffiType, dims, alias))
			}
			
			if name == clsName {
				cstr := IDL.NewConstructorDefinitionFromFunctionDefinition(javameth)
				javacls.AddConstructor(cstr)
			} else {
				meth := IDL.NewMethodDefinitionWithFunction(javacls, javameth, !isStatic)
				javacls.AddMethod(meth)
			}
			
		}
		
		javacls.Releaser.SetFunctionPath("entrypoint_function", fmt.Sprintf("EntryPoint_%v_%v", javacls.Name, javacls.Releaser.Name))
		
		// set function path items
		if packageName != "" {
			javacls.SetFunctionPath("package", packageName)
		}
		
		javacls.SetFunctionPath("entrypoint_class", javacls.Name+"_Entrypoints")
		
		classes[javacls.Name] = javacls
	}
	
	res := make([]*IDL.ClassDefinition, 0)
	for _, c := range classes {
		res = append(res, c)
	}
	
	return res, nil
}

//--------------------------------------------------------------------
