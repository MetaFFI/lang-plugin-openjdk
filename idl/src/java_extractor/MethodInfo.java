package java_extractor;

import com.google.gson.JsonObject;
import com.google.gson.JsonArray;

public class MethodInfo
{
	public String Name;
	public String Comment;
	public ParameterInfo[] Parameters;
	public VariableInfo[] ReturnValues;
	public boolean IsStatic;
	public boolean IsPublic;
	public boolean IsConstructor;

	/**
	 * Convert MethodInfo to MetaFFI IDL function JSON format
	 * @param classInfo the class this method belongs to
	 * @return JsonObject representing the function
	 * @throws Exception if JSON generation fails
	 */
	public JsonObject toMetaFFIFunctionJSON(ClassInfo classInfo) throws Exception {
		JsonObject json = new JsonObject();
		json.addProperty("name", Name);
		json.addProperty("comment", Comment != null ? Comment : "");
		json.add("tags", new JsonObject());
		
		JsonObject entityPath = new JsonObject();
		entityPath.addProperty("module", classInfo.getFullClassName());
		entityPath.addProperty("package", classInfo.Package != null ? classInfo.Package : "");
		json.add("entity_path", entityPath);
		
		// Add parameters
		JsonArray parameters = new JsonArray();
		if (Parameters != null) {
			for (ParameterInfo param : Parameters) {
				parameters.add(param.toMetaFFIParameterJSON());
			}
		}
		json.add("parameters", parameters);
		
		// Add return values
		JsonArray returnValues = new JsonArray();
		if (ReturnValues != null) {
			for (VariableInfo returnValue : ReturnValues) {
				returnValues.add(returnValue.toMetaFFIReturnValueJSON());
			}
		}
		json.add("return_values", returnValues);
		
		return json;
	}

	/**
	 * Convert MethodInfo to MetaFFI IDL method JSON format
	 * @param classInfo the class this method belongs to
	 * @return JsonObject representing the method
	 * @throws Exception if JSON generation fails
	 */
	public JsonObject toMetaFFIMethodJSON(ClassInfo classInfo) throws Exception {
		JsonObject json = new JsonObject();
		json.addProperty("name", Name);
		json.addProperty("comment", Comment != null ? Comment : "");
		json.add("tags", new JsonObject());
		
		JsonObject entityPath = new JsonObject();
		entityPath.addProperty("module", classInfo.getFullClassName());
		entityPath.addProperty("package", classInfo.Package != null ? classInfo.Package : "");
		json.add("entity_path", entityPath);
		
		// Add instance parameter
		JsonArray parameters = new JsonArray();
		JsonObject instanceParam = new JsonObject();
		instanceParam.addProperty("name", "instance");
		instanceParam.addProperty("type", "HANDLE");
		instanceParam.addProperty("type_alias", classInfo.getFullClassName());
		instanceParam.addProperty("comment", "");
		instanceParam.add("tags", new JsonObject());
		instanceParam.addProperty("dimensions", 0);
		parameters.add(instanceParam);
		
		// Add method parameters
		if (Parameters != null) {
			for (ParameterInfo param : Parameters) {
				parameters.add(param.toMetaFFIParameterJSON());
			}
		}
		json.add("parameters", parameters);
		
		// Add return values
		JsonArray returnValues = new JsonArray();
		if (ReturnValues != null) {
			for (VariableInfo returnValue : ReturnValues) {
				returnValues.add(returnValue.toMetaFFIReturnValueJSON());
			}
		}
		json.add("return_values", returnValues);
		
		return json;
	}

	/**
	 * Convert MethodInfo to MetaFFI IDL constructor JSON format
	 * @param classInfo the class this constructor belongs to
	 * @return JsonObject representing the constructor
	 * @throws Exception if JSON generation fails
	 */
	public JsonObject toMetaFFIConstructorJSON(ClassInfo classInfo) throws Exception {
		JsonObject json = new JsonObject();
		json.addProperty("name", Name);
		json.addProperty("comment", Comment != null ? Comment : "");
		json.add("tags", new JsonObject());
		
		JsonObject entityPath = new JsonObject();
		entityPath.addProperty("module", classInfo.getFullClassName());
		entityPath.addProperty("package", classInfo.Package != null ? classInfo.Package : "");
		json.add("entity_path", entityPath);
		
		// Add parameters
		JsonArray parameters = new JsonArray();
		if (Parameters != null) {
			for (ParameterInfo param : Parameters) {
				parameters.add(param.toMetaFFIParameterJSON());
			}
		}
		json.add("parameters", parameters);
		
		// Add return value (instance of the class)
		JsonArray returnValues = new JsonArray();
		JsonObject returnValue = new JsonObject();
		returnValue.addProperty("name", "instance");
		returnValue.addProperty("type", "HANDLE");
		returnValue.addProperty("type_alias", classInfo.getFullClassName());
		returnValue.addProperty("comment", "");
		returnValue.add("tags", new JsonObject());
		returnValue.addProperty("dimensions", 0);
		returnValues.add(returnValue);
		json.add("return_values", returnValues);
		
		return json;
	}
} 