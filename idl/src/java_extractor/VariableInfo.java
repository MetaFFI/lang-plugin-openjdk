package java_extractor;

import com.google.gson.JsonObject;

public class VariableInfo
{
	public String Name;
	public String Type;
	public String TypeAlias;
	public String Comment;
	public int Dimensions;
	public boolean IsStatic;
	public boolean IsFinal;
	public boolean IsPublic;

	/**
	 * Convert VariableInfo to MetaFFI IDL constant JSON format
	 * @param classInfo the class this constant belongs to
	 * @return JsonObject representing the constant
	 * @throws Exception if JSON generation fails
	 */
	public JsonObject toMetaFFIConstantJSON(ClassInfo classInfo) throws Exception {
		JsonObject json = new JsonObject();
		json.addProperty("name", Name);
		json.addProperty("comment", Comment != null ? Comment : "");
		json.add("tags", new JsonObject());
		
		JsonObject entityPath = new JsonObject();
		entityPath.addProperty("module", classInfo.getFullClassName());
		entityPath.addProperty("package", classInfo.Package != null ? classInfo.Package : "");
		json.add("entity_path", entityPath);
		
		JsonObject value = new JsonObject();
		value.addProperty("type", Type);
		value.addProperty("type_alias", TypeAlias != null ? TypeAlias : "");
		value.addProperty("comment", "");
		value.add("tags", new JsonObject());
		value.addProperty("dimensions", Dimensions);
		json.add("value", value);
		
		return json;
	}

	/**
	 * Convert VariableInfo to MetaFFI IDL return value JSON format
	 * @return JsonObject representing the return value
	 * @throws Exception if JSON generation fails
	 */
	public JsonObject toMetaFFIReturnValueJSON() throws Exception {
		JsonObject json = new JsonObject();
		json.addProperty("name", Name);
		json.addProperty("type", Type);
		json.addProperty("type_alias", TypeAlias != null ? TypeAlias : "");
		json.addProperty("comment", Comment != null ? Comment : "");
		json.add("tags", new JsonObject());
		json.addProperty("dimensions", Dimensions);
		return json;
	}
} 