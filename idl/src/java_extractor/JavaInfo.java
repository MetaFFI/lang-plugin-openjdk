package java_extractor;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.JsonObject;
import com.google.gson.JsonArray;
import java.util.*;

public class JavaInfo
{
	public ClassInfo[] Classes;

	/**
	 * Convert JavaInfo to MetaFFI IDL JSON format using Gson
	 * @return JSON string representing the MetaFFI IDL
	 * @throws Exception if JSON generation fails
	 */
	public String toMetaFFIJSON() throws Exception {
		Gson gson = new GsonBuilder().setPrettyPrinting().create();
		JsonObject root = new JsonObject();
		
		// Set basic IDL properties
		root.addProperty("idl_filename", "");
		root.addProperty("idl_extension", ".class");
		root.addProperty("idl_filename_with_extension", "");
		root.addProperty("idl_full_path", "");
		
		// Create modules array
		JsonArray modules = new JsonArray();
		
		if (Classes != null && Classes.length > 0) {
			// Group classes by package
			Map<String, List<ClassInfo>> packageMap = new HashMap<>();
			for (ClassInfo classInfo : Classes) {
				String packageName = classInfo.Package != null ? classInfo.Package : "";
				packageMap.computeIfAbsent(packageName, k -> new ArrayList<>()).add(classInfo);
			}
			
			// Create modules for each package
			for (Map.Entry<String, List<ClassInfo>> entry : packageMap.entrySet()) {
				String packageName = entry.getKey();
				List<ClassInfo> classList = entry.getValue();
				
				JsonObject module = new JsonObject();
				module.addProperty("name", packageName.isEmpty() ? "default" : packageName);
				module.addProperty("target_language", "openjdk");
				module.addProperty("comment", "");
				module.add("tags", new JsonObject());
				
				// Add static methods as functions
				JsonArray functions = new JsonArray();
				for (ClassInfo classInfo : classList) {
					if (classInfo.Methods != null) {
						for (MethodInfo method : classInfo.Methods) {
							if (method.IsStatic) {
								functions.add(method.toMetaFFIFunctionJSON(classInfo));
							}
						}
					}
				}
				module.add("functions", functions);
				
				// Add classes
				JsonArray classes = new JsonArray();
				for (ClassInfo classInfo : classList) {
					classes.add(classInfo.toMetaFFIClassJSON());
				}
				module.add("classes", classes);
				
				// Add empty arrays for constants and external resources
				module.add("constants", new JsonArray());
				module.add("external_resources", new JsonArray());
				
				modules.add(module);
			}
		}
		
		root.add("modules", modules);
		
		return gson.toJson(root);
	}
} 