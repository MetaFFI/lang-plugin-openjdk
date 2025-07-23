package java_extractor;

import com.google.gson.JsonObject;

public class ParameterInfo
{
	public String Name;
	public String Type;
	public String TypeAlias;
	public String Comment;
	public int Dimensions;

	/**
	 * Convert ParameterInfo to MetaFFI IDL parameter JSON format
	 * @return JsonObject representing the parameter
	 * @throws Exception if JSON generation fails
	 */
	public JsonObject toMetaFFIParameterJSON() throws Exception {
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