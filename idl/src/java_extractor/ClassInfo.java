package java_extractor;

import com.google.gson.JsonObject;
import com.google.gson.JsonArray;

public class ClassInfo
{
	public String Name;
	public String Comment;
	public String Package;
	public VariableInfo[] Fields;
	public MethodInfo[] Constructors;
	public MethodInfo[] Methods;

	/**
	 * Convert ClassInfo to MetaFFI IDL class JSON format using Gson
	 * @return JsonObject representing the class
	 * @throws Exception if JSON generation fails
	 */
	public JsonObject toMetaFFIClassJSON() throws Exception {
		JsonObject json = new JsonObject();
		json.addProperty("name", Name);
		json.addProperty("comment", Comment != null ? Comment : "");
		json.add("tags", new JsonObject());
		
		// Add constructors
		JsonArray constructors = new JsonArray();
		if (Constructors != null && Constructors.length > 0) {
			for (MethodInfo constructor : Constructors) {
				constructors.add(constructor.toMetaFFIConstructorJSON(this));
			}
		}
		json.add("constructors", constructors);
		
		// Add release method
		JsonObject release = new JsonObject();
		release.addProperty("name", "release");
		release.addProperty("comment", "");
		release.add("tags", new JsonObject());
		
		JsonObject releaseEntityPath = new JsonObject();
		releaseEntityPath.addProperty("module", getFullClassName());
		releaseEntityPath.addProperty("package", Package != null ? Package : "");
		release.add("entity_path", releaseEntityPath);
		
		JsonArray releaseParams = new JsonArray();
		JsonObject instanceParam = new JsonObject();
		instanceParam.addProperty("name", "instance");
		instanceParam.addProperty("type", "HANDLE");
		instanceParam.addProperty("type_alias", getFullClassName());
		instanceParam.addProperty("comment", "");
		instanceParam.add("tags", new JsonObject());
		instanceParam.addProperty("dimensions", 0);
		releaseParams.add(instanceParam);
		release.add("parameters", releaseParams);
		release.add("return_values", new JsonArray());
		json.add("release", release);
		
		// Add instance methods (non-static)
		JsonArray methods = new JsonArray();
		if (Methods != null) {
			for (MethodInfo method : Methods) {
				if (!method.IsStatic) {
					methods.add(method.toMetaFFIMethodJSON(this));
				}
			}
		}
		json.add("methods", methods);
		
		// Add getters/setters for public instance fields
		JsonArray fieldMethods = new JsonArray();
		if (Fields != null) {
			for (VariableInfo field : Fields) {
				if (!field.IsStatic && field.IsPublic) {
					// Add getter
					fieldMethods.add(createFieldGetter(field));
					// Add setter (if not final)
					if (!field.IsFinal) {
						fieldMethods.add(createFieldSetter(field));
					}
				}
			}
		}
		json.add("field_methods", fieldMethods);
		
		// Add static final fields as constants
		JsonArray constants = new JsonArray();
		if (Fields != null) {
			for (VariableInfo field : Fields) {
				if (field.IsStatic && field.IsFinal) {
					constants.add(field.toMetaFFIConstantJSON(this));
				}
			}
		}
		json.add("constants", constants);
		
		return json;
	}
	
	/**
	 * Create a getter method for a field
	 */
	private JsonObject createFieldGetter(VariableInfo field) {
		JsonObject getter = new JsonObject();
		getter.addProperty("name", "get" + capitalize(field.Name));
		getter.addProperty("comment", "Getter for field " + field.Name);
		getter.add("tags", new JsonObject());
		
		JsonObject entityPath = new JsonObject();
		entityPath.addProperty("module", getFullClassName());
		entityPath.addProperty("package", Package != null ? Package : "");
		getter.add("entity_path", entityPath);
		
		getter.add("parameters", new JsonArray()); // No parameters for getter
		
		JsonArray returnValues = new JsonArray();
		JsonObject returnValue = new JsonObject();
		returnValue.addProperty("name", "value");
		returnValue.addProperty("type", field.Type);
		returnValue.addProperty("type_alias", field.TypeAlias != null ? field.TypeAlias : "");
		returnValue.addProperty("comment", "");
		returnValue.add("tags", new JsonObject());
		returnValue.addProperty("dimensions", field.Dimensions);
		returnValues.add(returnValue);
		getter.add("return_values", returnValues);
		
		return getter;
	}
	
	/**
	 * Create a setter method for a field
	 */
	private JsonObject createFieldSetter(VariableInfo field) {
		JsonObject setter = new JsonObject();
		setter.addProperty("name", "set" + capitalize(field.Name));
		setter.addProperty("comment", "Setter for field " + field.Name);
		setter.add("tags", new JsonObject());
		
		JsonObject entityPath = new JsonObject();
		entityPath.addProperty("module", getFullClassName());
		entityPath.addProperty("package", Package != null ? Package : "");
		setter.add("entity_path", entityPath);
		
		JsonArray parameters = new JsonArray();
		JsonObject param = new JsonObject();
		param.addProperty("name", "value");
		param.addProperty("type", field.Type);
		param.addProperty("type_alias", field.TypeAlias != null ? field.TypeAlias : "");
		param.addProperty("comment", "");
		param.add("tags", new JsonObject());
		param.addProperty("dimensions", field.Dimensions);
		parameters.add(param);
		setter.add("parameters", parameters);
		
		setter.add("return_values", new JsonArray()); // No return value for setter
		
		return setter;
	}
	
	/**
	 * Capitalize the first letter of a string
	 */
	private String capitalize(String str) {
		if (str == null || str.isEmpty()) {
			return str;
		}
		return str.substring(0, 1).toUpperCase() + str.substring(1);
	}
	
	/**
	 * Get the fully qualified class name
	 * @return fully qualified class name
	 */
	public String getFullClassName() {
		if (Package != null && !Package.isEmpty()) {
			return Package + "." + Name;
		}
		return Name;
	}
} 