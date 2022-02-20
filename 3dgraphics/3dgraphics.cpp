
#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <fstream>
#include <cassert>
#include <array>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define ILUT_USE_OPENGL
#include <il.h>
#include <ilu.h>
#include <ilut.h>

#include <assimp/Importer.hpp>  // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags

inline std::string GLErrorToString(GLenum error) {
    switch (error) {
    case GL_NO_ERROR: return "GL_NO_ERROR";
    case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION: return "GL_INVALID_VALUE";
    case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
    }
    
    return "<Unknown Error Value: " + std::to_string(error) + ">";
}


std::string parent_path(const std::string &str) {
    return str.substr(0, str.find_last_of("/\\")) + "/";
}


std::string replace_all(const std::string &str, const std::string &search, const std::string &replace) {
    std::string result = str;
    
    size_t pos = 0;
    do {
        pos = result.find(search);
        
        if (pos != std::string::npos) {
            result.replace(pos, search.size(), replace);
        }
    }
    while (pos != std::string::npos);
    
    return result;
};


std::vector<std::string> split(const std::string &str, const std::string &delimiter) {
    std::string s = str;
    std::vector<std::string> tokens;
    
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        
        tokens.push_back(token);
        
        s.erase(0, pos + delimiter.length());
    }
    
    tokens.push_back(s);
    
    return tokens;
}


std::string join(const std::vector<std::string> &elements, const std::string &delimiter) {
    std::string str;
    
    for (size_t i = 0; i < elements.size(); i++) {
        if (elements[i].empty()) {
            continue;
        }
        
        str += elements[i];
        
        if (i < elements.size() - 1) {
            str += delimiter;
        }
    }
    
    return str;
}


std::string normalize_path(const std::string &str) {
    std::string result = str;
    
    std::replace(result.begin(), result.end(), '\\', '/');
    
    return result;
}


bool can_be_opened(const std::string &filePath) {
    std::fstream fs;
    
    fs.open(filePath.c_str(), std::ios::in);
    
    return fs.is_open();
}


#ifndef NDEBUG
#   define M_Assert(Expr, Msg) \
    __M_Assert(#Expr, Expr, __FILE__, __LINE__, Msg)
#else
#   define M_Assert(Expr, Msg) ;
#endif

void __M_Assert(const char* expr_str, bool expr, const char* file, int line, const char* msg)
{
    if (!expr)
    {
        std::cerr << "Assert failed:\t" << msg << "\n"
            << "Expected:\t" << expr_str << "\n"
            << "Source:\t\t" << file << ", line " << line << "\n";
        abort();
    }
}

struct GLErrorRAII {
    GLErrorRAII(const char *file, const int line) : file(file), line(line) {
        check();
    }
    
    ~GLErrorRAII() {
        check();
    }
    
    void check() const {
        const GLenum error = glGetError();
        
        if (error != GL_NO_ERROR) {
            std::cerr
                << "GL Error Detected:\t" << GLErrorToString(error) << "\n"
                << "Source:\t\t" << file << ", line " << line << "\n";
            
            abort();
        }
    }
    
    const char *file;
    const int line;
};


#define GL_SCOPED_ERROR_CHECK GLErrorRAII __gl_error_raii(__FILE__, __LINE__);

struct Material {
    glm::vec4 ambient = {1.0f, 1.0f, 1.0f, 1.0f};
    glm::vec4 diffuse = {1.0f, 1.0f, 1.0f, 1.0f};
    glm::vec4 specular = {1.0f, 1.0f, 1.0f, 1.0f};
    
    GLuint diffuseTexture = 0;
};


struct Light {
    glm::vec3 direction = glm::normalize(glm::vec3{0.5f, 1.0f, 0.25f});
    glm::vec4 ambient = {0.6f, 0.6f, 0.6f, 1.0f};
    glm::vec4 diffuse = {0.8f, 0.8f, 0.8f, 0.8f};
};


GLuint createTexture(GLenum internalFormat, const unsigned width, const unsigned height, const GLenum format, const GLenum type, const void *data) {
    GL_SCOPED_ERROR_CHECK
    
    // Generate a new texture
    GLuint texture = 0;
    glGenTextures(1, &texture);
     
    // Bind the texture to a name
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glTexImage2D( GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    // Set texture clamping method
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Set texture interpolation method to use linear interpolation
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    return texture;
}


class TextureRepository {
public:
    TextureRepository() {
        ilInit();
        iluInit();
        ilutInit();
    }
    
    GLuint getOrCreate(const std::string &filePath) {
        if (filePath.empty()) {
            return 0;
        }
        
        if (auto it = cachedTextureMap.find(filePath); it != cachedTextureMap.end()) {
            return it->second;
        }
        
        GLuint texture = createTexture(filePath.c_str());
        
        if (! texture) {
            return 0;
        }
        
        std::cout << "Loaded texture " << filePath << std::endl;
        
        cachedTextureMap[filePath] = texture;
        
        return texture;
    }
    
private:
    GLuint createTexture(const char* theFileName) {
        ILuint imageID;
        
        ilGenImages(1, &imageID);
        ilBindImage(imageID);
         
        // If we managed to load the image, then we can start to do things with it...
        if (! ilLoadImage(theFileName)) {
            const ILenum error = ilGetError();
            std::cout << "Image load failed: \"" << theFileName << "\" - IL reports error: " << error << " - " << iluErrorString(error) << std::endl;
            
            return 0;
        }
        
        iluFlipImage();
         
        if (!ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE)) {
            ILenum error = ilGetError();
            std::cout << "Image conversion failed: \"" << theFileName << "\" - IL reports error: " << error << " - " << iluErrorString(error) << std::endl;
            
            return 0;
        }
        
        // Specify the texture specification
        const ILint bpp = ilGetInteger(IL_IMAGE_BPP);
        const ILint width = ilGetInteger(IL_IMAGE_WIDTH);
        const ILint height = ilGetInteger(IL_IMAGE_HEIGHT);
        const ILint format = ilGetInteger(IL_IMAGE_FORMAT);
        const void* data = ilGetData();
        
        GLenum internalFormat = 0;

        switch (bpp) {
        case 3: internalFormat = GL_RGB; break;
        case 4: internalFormat = GL_RGBA; break;
        default: internalFormat = format;
        }
        
        const GLuint texture = ::createTexture(internalFormat, width, height, format, GL_UNSIGNED_BYTE, data);
        
        ilDeleteImages(1, &imageID);
        
        return texture;
    }
    
private:
    std::map<std::string, GLuint> cachedTextureMap;
};


GLuint createShader(const std::string &source, const GLenum type) {
    GLuint shader = glCreateShader(type);

    const GLchar * const sources = source.c_str();
    const GLint sourceSizes = source.size();

    glShaderSource(shader, 1, &sources, &sourceSizes);
    glCompileShader(shader);

    GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE) {
        char buffer[2048] = {};
        GLsizei size = 0;

        glGetShaderInfoLog(shader, 2048, &size, buffer);
        const std::string msg = buffer;
        std::cout << msg << std::endl;
        return 0;
    }

    return shader;
}


GLuint createShaderProgram(const std::vector<GLuint> &shaders) {
    GLuint program = glCreateProgram();

    for (const GLuint shader : shaders) {
        assert(shader);
        assert(glIsShader(shader));

        glAttachShader(program, shader);
    }

    glLinkProgram(program);

    GLint status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &status);

    if (status == GL_FALSE) {
        char buffer[2048] = {};
        GLsizei size = 0;

        glGetProgramInfoLog(program, 2048, &size, buffer);
        const std::string msg = buffer;
        std::cout << msg << std::endl;

        return 0;
    }

    return program;
}


GLuint createBuffer(const GLenum target, const GLsizeiptr size, const void *data, GLenum usage) {
    assert(size);
    assert(data);

    GLuint buffer = 0;

    glGenBuffers(1, &buffer);
    glBindBuffer(target, buffer);
    glBufferData(target, size, data, usage);
    glBindBuffer(target, 0);

    assert(glGetError() == GL_NO_ERROR);

    return buffer;
}


template<class ArrayLike>
GLuint createBuffer(const GLenum target, const ArrayLike &values, GLenum usage) {
    using T = typename ArrayLike::value_type;
    
    return createBuffer(target, sizeof(T) * values.size(), values.data(), usage);
}


Material createMaterial(const std::string &parentPath, TextureRepository &textureRepository, const aiMaterial *aimaterial) {
    if (!aimaterial) {
        return {};
    }
    
    Material material;
    
    // extract material colors
    aiColor3D colorAmbient, colorDiffuse, colorSpecular, colorEmissive;

    aimaterial->Get(AI_MATKEY_COLOR_AMBIENT, colorAmbient);
    aimaterial->Get(AI_MATKEY_COLOR_DIFFUSE, colorDiffuse);
    aimaterial->Get(AI_MATKEY_COLOR_SPECULAR, colorSpecular);
    // aimaterial->Get(AI_MATKEY_COLOR_EMISSIVE, colorEmissive);
    
    material.ambient = glm::vec4{colorAmbient.r, colorAmbient.g, colorAmbient.b, 1.0f};
    material.diffuse = glm::vec4{colorDiffuse.r, colorDiffuse.g, colorDiffuse.b, 1.0f};
    material.specular = glm::vec4{colorSpecular.r, colorSpecular.g, colorSpecular.b, 1.0f};

    // extract material textures
    std::map<aiTextureType, std::string> textureMap {
        { aiTextureType_AMBIENT, "" },
        { aiTextureType_DIFFUSE, "" },
        { aiTextureType_SPECULAR, "" },
        { aiTextureType_AMBIENT, "" },
        { aiTextureType_HEIGHT, "" },
        { aiTextureType_EMISSIVE, "" },
        { aiTextureType_NORMALS, "" },
        { aiTextureType_SHININESS, "" },
        { aiTextureType_OPACITY, "" },
        { aiTextureType_DISPLACEMENT, "" },
        { aiTextureType_LIGHTMAP, "" },
        { aiTextureType_REFLECTION, "" },
        { aiTextureType_BASE_COLOR, "" },
        { aiTextureType_NORMAL_CAMERA, "" },
        { aiTextureType_EMISSION_COLOR, "" },
        { aiTextureType_METALNESS, "" },
        { aiTextureType_DIFFUSE_ROUGHNESS, "" },
        { aiTextureType_AMBIENT_OCCLUSION, "" },
        { aiTextureType_UNKNOWN, "" },
    };
    
    std::cout << aimaterial->GetName().C_Str() << std::endl;
    for (auto &pair : textureMap) {
        aiString fileName;
        aimaterial->GetTexture(pair.first, 0, &fileName);
        
        if (fileName.length == 0) {
            continue;
        }
        
        std::string filePath = join(split(std::string{fileName.C_Str()}, "\\"), "/");
        
        if (filePath[0] == '/') {
            if (! can_be_opened(filePath)) {
                std::string textureFilePath = filePath;
                std::string textureParentPath = parent_path(filePath);
                
                filePath = replace_all(filePath, parent_path(filePath), parentPath);
            }
        }
        else {
            filePath = parentPath + filePath;
        }
        
        assert(can_be_opened(filePath));
        
        pair.second = filePath;
        
        std::cout << "    " << pair.first << " = " << fileName.C_Str() << " -> " << filePath << std::endl;
        // const std::string filePath = parentPath + file;
    }
    
    material.diffuseTexture = textureRepository.getOrCreate(textureMap[aiTextureType_DIFFUSE]);
    
    return material;
}


std::vector<Material> createMaterialArray(const std::string &parentPath, TextureRepository &textureRepository, const aiScene *aiscene) {
    std::vector<Material> materials;
    
    materials.resize(aiscene->mNumMaterials);
    
    for (unsigned int i=0; i<aiscene->mNumMaterials; i++) {
        materials[i] = createMaterial(parentPath, textureRepository, aiscene->mMaterials[i]);
    }
    
    return materials;
}


struct ShaderLocationMap {
    GLint coord = -1;
    GLint normal = -1;
    GLint texCoord = -1;
    
    GLint uModel = -1;
    GLint uView = -1;
    GLint uProj = -1;
    
    GLint uMaterialDiffuseSamplerEnable = -1;
    GLint uMaterialDiffuseSampler = -1;
    GLint uMaterialAmbient = -1;
    GLint uMaterialDiffuse = -1;
    GLint uMaterialSpecular = -1;
    
    GLint uLightAmbient = -1;
    GLint uLightDirection = -1;
    GLint uLightDiffuse = -1;
};


ShaderLocationMap createShaderLocationMap(const GLuint program) {
    assert(program);
    assert(glIsProgram(program));
    
    ShaderLocationMap location;
    
    location.coord = glGetAttribLocation(program, "vertCoord");
    location.normal = glGetAttribLocation(program, "vertNormal");
    location.texCoord = glGetAttribLocation(program, "vertTexCoord");
    
    location.uModel = glGetUniformLocation(program, "uModel");
    location.uView = glGetUniformLocation(program, "uView");
    location.uProj = glGetUniformLocation(program, "uProj");
    
    location.uMaterialDiffuseSamplerEnable = glGetUniformLocation(program, "uMaterialDiffuseSamplerEnable");
    location.uMaterialDiffuseSampler = glGetUniformLocation(program, "uMaterialDiffuseSampler");
    location.uMaterialAmbient = glGetUniformLocation(program, "uMaterialAmbient");
    location.uMaterialDiffuse = glGetUniformLocation(program, "uMaterialDiffuse");
    location.uMaterialSpecular = glGetUniformLocation(program, "uMaterialSpecular");
    
    location.uLightAmbient = glGetUniformLocation(program, "uLightAmbient");
    location.uLightDirection = glGetUniformLocation(program, "uLightDirection");
    location.uLightDiffuse = glGetUniformLocation(program, "uLightDiffuse");
    
    return location;
}


struct Mesh {
    GLuint vao = 0;
    GLenum primitiveType = GL_TRIANGLES;
    bool indexed = false;
    unsigned int count = 0;
    GLenum indexDataType = GL_UNSIGNED_INT;
    
    int material = -1;
    
    Mesh() {}
    
    bool empty() const {
        return vao == 0;
    }
};


Mesh createMeshVAO(const ShaderLocationMap &location, const aiMesh *mesh) {
    if (! mesh) {
        return {};
    }
    
    Mesh meshVAO;
    
    meshVAO.material = mesh->mMaterialIndex;
    
    GLuint coordBuffer = createBuffer(GL_ARRAY_BUFFER, mesh->mNumVertices * sizeof(aiVector3D), mesh->mVertices, GL_STATIC_DRAW);
    
    GLuint normalBuffer = 0;
    if (mesh->HasNormals()) {
        normalBuffer = createBuffer(GL_ARRAY_BUFFER, mesh->mNumVertices * sizeof(aiVector3D), mesh->mNormals, GL_STATIC_DRAW);
    }
    
    GLuint texCoordBuffer = 0;
    if (mesh->mTextureCoords[0]) {
        assert(mesh->mNumUVComponents[0] == 2);
        
        std::vector<glm::vec2> texCoords;
        texCoords.resize(mesh->mNumVertices);
        
        for (int i = 0; i < texCoords.size(); i++) {
            const auto &tc = mesh->mTextureCoords[0][i];
            
            texCoords[i] = glm::vec2{tc.x, tc.y};
        }
        
        texCoordBuffer = createBuffer(GL_ARRAY_BUFFER, texCoords, GL_STATIC_DRAW);
    }
    
    GLuint indexBuffer = 0;
    if (mesh->HasFaces()) {
        std::vector<unsigned int> indices;
        
        indices.reserve(mesh->mNumFaces);
        
        for (unsigned int j=0; j<mesh->mNumFaces; j++) {
            const aiFace &face = mesh->mFaces[j];
            
            assert("This function requires the aiTriangulate postprocessing flag" && face.mNumIndices == 3);
            
            indices.push_back(face.mIndices[0]);
            indices.push_back(face.mIndices[1]);
            indices.push_back(face.mIndices[2]);
        }
        
        indexBuffer = createBuffer(GL_ELEMENT_ARRAY_BUFFER, indices, GL_STATIC_DRAW);
        
        meshVAO.indexDataType = GL_UNSIGNED_INT;
        meshVAO.indexed = true;
        meshVAO.count = indices.size();
        meshVAO.primitiveType = GL_TRIANGLES;
    } else {
        meshVAO.indexDataType = GL_UNSIGNED_INT;
        meshVAO.indexed = false;
        meshVAO.count = mesh->mNumVertices;
        meshVAO.primitiveType = GL_TRIANGLES;
    }
    
    glGenVertexArrays(1, &meshVAO.vao);
    glBindVertexArray(meshVAO.vao);
    
    glEnableVertexAttribArray(location.coord);
    glBindBuffer(GL_ARRAY_BUFFER, coordBuffer);
    glVertexAttribPointer(location.coord, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    if (normalBuffer) {
        assert(location.normal >= 0);
        
        glEnableVertexAttribArray(location.normal);
        glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
        glVertexAttribPointer(location.normal, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    }
    
    if (texCoordBuffer) {
        assert(location.texCoord >= 0);
        
        glEnableVertexAttribArray(location.texCoord);
        glBindBuffer(GL_ARRAY_BUFFER, texCoordBuffer);
        glVertexAttribPointer(location.texCoord, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    }
    
    if (indexBuffer) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    }
    
    glBindVertexArray(0);

    assert(glGetError() == GL_NO_ERROR);
    
    return meshVAO;
}


std::vector<Mesh> createMeshArray(const ShaderLocationMap &location, const aiScene *aiscene) {
    if (!aiscene) {
        return {};
    }
    
    std::vector<Mesh> meshes;
    meshes.resize(aiscene->mNumMeshes);
    
    for (unsigned i = 0; i < aiscene->mNumMeshes; i++) {
        meshes[i] = createMeshVAO(location, aiscene->mMeshes[i]);
    }
    
    return meshes;
}


std::string loadTextFile(const std::string &file) {
    assert(!file.empty());

    std::fstream fs;

    fs.open(file.c_str(), std::ios::in);
    assert(fs.is_open());

    std::string content;

    std::string line;

    while(fs.good()) {
        std::getline(fs, line);
        line += "\n";
        content += line;
    }

    return content;
}


GLuint createProgram(const std::string &vertFile, const std::string &fragFile) {
    const std::vector<GLuint> shaders {
        createShader(loadTextFile(vertFile), GL_VERTEX_SHADER), 
        createShader(loadTextFile(fragFile), GL_FRAGMENT_SHADER)
    };

    return createShaderProgram(shaders);
}


std::vector<GLuint> createTextureArray(const aiScene* scene, const std::string& pModelPath) {
    if(!scene || !scene->HasTextures()) {
        return {};
    }
    
    std::vector<GLuint> textures;
    textures.resize(scene->mNumTextures);
    
    glGenTextures(scene->mNumTextures, textures.data());
    
    for(size_t ti = 0; ti < scene->mNumTextures; ti++) {
        glBindTexture(GL_TEXTURE_2D, textures[ti]);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (scene->mTextures[ti]->achFormatHint[0] & 0x01) ? GL_REPEAT : GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (scene->mTextures[ti]->achFormatHint[0] & 0x01) ? GL_REPEAT : GL_CLAMP_TO_EDGE);
        
        const unsigned width = scene->mTextures[ti]->mWidth;
        const unsigned height = scene->mTextures[ti]->mHeight;
        const void *data = scene->mTextures[ti]->pcData;
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
    }
    
    return textures;
}


int main(int argc, char **argv) {
    TextureRepository textureRepository;
    
    // Create an instance of the Importer class
    Assimp::Importer importer;
    // And have it read the given file with some example postprocessing
    // Usually - if speed is not the most important aspect for you - you'll
    // propably to request more postprocessing than we do in this example.
    const auto flags =  aiProcess_Triangulate |
                        aiProcess_JoinIdenticalVertices |
                        aiProcess_GenNormals |
                        // aiProcess_CalcTangentSpace |
                        // aiProcess_SortByPType |
                        aiProcess_ValidateDataStructure;

    const std::string sceneFilePath = argv[1];
    const std::string sceneFileParentPath = parent_path(sceneFilePath);
    const aiScene *scene = importer.ReadFile(sceneFilePath, flags);

    // If the import failed, report it
    if (!scene) {
        std::cout << importer.GetErrorString() << std::endl;

        return EXIT_FAILURE;
    }

    if (! scene->HasMeshes()) {
        std::cout << "scene doesn't have meshes" << std::endl;

        return EXIT_FAILURE;
    }

    for (int i = 0; i<scene->mNumMeshes; i++) {
        std::cout << "Mesh: " << scene->mMeshes[i]->mName.C_Str() << std::endl;
    }
    
    if (scene->mNumMeshes <= 0) {
        std::cout << "The object doesn't have any meshes" << std::endl;
        return EXIT_FAILURE;
    }

    glfwInit();

    const auto monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "3dgraphics", nullptr, nullptr);
    
    if (!window) {
        std::cout << "Cant open a Window" << std::endl;
        
        const char description[1024] = {};
        const char *desc = &description[0];
        
        glfwGetError(&desc);
        
        std::cout << description << std::endl;

        return EXIT_FAILURE;
    }

    int windowWidth = 0;
    int windowHeight = 0;

    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return EXIT_FAILURE;
    }

    const GLuint program = createProgram("gouraud.vert", "gouraud.frag");
    assert(program);
    
    const ShaderLocationMap location = createShaderLocationMap(program);
    const std::vector<Mesh> meshes = createMeshArray(location, scene);
    const std::vector<GLuint> textures = createTextureArray(scene, "");

    const std::vector<Material> materials = createMaterialArray(sceneFileParentPath, textureRepository, scene);
    const Light light;
    
    bool running = true;

    glm::vec3 playerPosition = {0.0f, 0.0f, 10.0f};
    float angle = 0.0f;
    
    while (running) {
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            running = false;
        }

        if (glfwGetKey(window, GLFW_KEY_LEFT)) {
            angle += 0.02f;
        }
        else if (glfwGetKey(window, GLFW_KEY_RIGHT)) {
            angle -= 0.02f;
        }
        
        // compute player direction
        glm::mat4 rotationY = glm::identity<glm::mat4>();
        rotationY = glm::rotate(rotationY, angle, glm::vec3{0.0f, 1.0f, 0.0f});
        
        const glm::vec3 playerDirection = rotationY * glm::vec4{0.0f, 0.0f, -1.0f, 0.0f};

        // compute player movement
        if (! (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)) {
            if (glfwGetKey(window, GLFW_KEY_UP)) {
                playerPosition += 0.075f * playerDirection;
            }
            else if (glfwGetKey(window, GLFW_KEY_DOWN)) {
                playerPosition -= 0.075f * playerDirection;
            }
        }
        
        glClearColor(0.1f, 0.1f, 0.6f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        
        glUseProgram(program);

        // setup transformation matrices
        const glm::mat4 proj = glm::perspective(
            45.0f,
            static_cast<float>(windowWidth) / static_cast<float>(windowHeight),
            0.1f,
            100.0f);

        const glm::mat4 view = glm::lookAt(
            playerPosition,
            playerPosition + playerDirection,
            glm::vec3{0.0f, 1.0f, 0.0f});

        const glm::mat4 model = glm::identity<glm::mat4>();
        
        glUniformMatrix4fv(location.uProj, 1, GL_FALSE, glm::value_ptr(proj));
        glUniformMatrix4fv(location.uView, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(location.uModel, 1, GL_FALSE, glm::value_ptr(model));
        
        // setup lighting
        glUniform3fv(location.uLightDirection, 1, glm::value_ptr(light.direction));
        glUniform4fv(location.uLightAmbient, 1, glm::value_ptr(light.ambient));
        glUniform4fv(location.uLightDiffuse, 1, glm::value_ptr(light.diffuse));
        
        for (const Mesh &mesh : meshes) {
            const Material material = mesh.material >= 0 ? materials[mesh.material] : Material{};
            
            glUniform4fv(location.uMaterialAmbient, 1, glm::value_ptr(material.ambient));
            glUniform4fv(location.uMaterialDiffuse, 1, glm::value_ptr(material.diffuse));
            glUniform4fv(location.uMaterialSpecular, 1, glm::value_ptr(material.specular));
            
            if (material.diffuseTexture) {
                glActiveTexture(GL_TEXTURE0 + 0);
                glBindTexture(GL_TEXTURE_2D, material.diffuseTexture);
                glUniform1f(location.uMaterialDiffuseSamplerEnable, 1.0f);
            }
            else {
                glActiveTexture(GL_TEXTURE0 + 0);
                glBindTexture(GL_TEXTURE_2D, 0);
                glUniform1f(location.uMaterialDiffuseSamplerEnable, 0.0f);
            }
            
            glUniform1i(location.uMaterialDiffuseSampler, 0);
            
            // render the mesh
            glBindVertexArray(mesh.vao);
            if (mesh.indexed) {
                glDrawElements(mesh.primitiveType, mesh.count, mesh.indexDataType, nullptr);
            }
            else {
                glDrawArrays(mesh.primitiveType, 0, mesh.count);
            }
        }
        
        glFlush();
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
};
