#include <iostream>
#include <string>
#include <cstring>

// Forward declaration of the plugin function
extern "C" {
    int parse_idl(const char* source_code, const char* file_path, 
                  char** out_idl_json, char** out_err);
    void init_plugin();
    void cleanup_plugin();
}

int main() {
    std::cout << "Testing OpenJDK IDL Plugin..." << std::endl;
    
    // Initialize plugin
    init_plugin();
    
    // Test file path (use the compiled ComplexTest.class)
    const char* test_file = "test/testdata/testdata/ComplexTest.class";
    
    // Test parameters
    const char* source_code = ""; // Not used for .class files
    char* idl_json = nullptr;
    char* error = nullptr;
    
    std::cout << "Testing with file: " << test_file << std::endl;
    
    // Call the plugin
    int result = parse_idl(source_code, test_file, &idl_json, &error);
    
    if (result == 0) {
        std::cout << "SUCCESS: Plugin executed successfully" << std::endl;
        std::cout << "Generated IDL JSON:" << std::endl;
        std::cout << idl_json << std::endl;
        
        // Clean up allocated memory
        delete[] idl_json;
    } else {
        std::cout << "ERROR: Plugin failed" << std::endl;
        if (error != nullptr) {
            std::cout << "Error message: " << error << std::endl;
            delete[] error;
        }
    }
    
    // Cleanup plugin
    cleanup_plugin();
    
    std::cout << "Test completed." << std::endl;
    return result == 0 ? 0 : 1;
} 