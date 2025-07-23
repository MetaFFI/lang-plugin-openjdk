package java_extractor;

import java.io.File;

public class JavaExtractor implements Extractor {
    private String filePath;
    private Extractor extractor;

    public JavaExtractor(String filePath) throws Exception {
        this.filePath = filePath;
        
        File file = new File(filePath);
        if (!file.exists()) {
            throw new Exception("File not found: " + filePath);
        }
        
        String fileName = file.getName().toLowerCase();
        if (fileName.endsWith(".class")) {
            this.extractor = new BytecodeExtractor(filePath);
        } else if (fileName.endsWith(".jar")) {
            this.extractor = new JarExtractor(filePath);
        } else {
            throw new Exception("Unsupported file type: " + fileName);
        }
    }

    @Override
    public JavaInfo extract() throws Exception {
        return extractor.extract();
    }
} 