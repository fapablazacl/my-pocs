
#include <iostream>
#include <memory>
#include <vector>
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
#include <assimp/Importer.hpp>  // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags


struct Material {
    glm::vec4 ambient = {1.0f, 1.0f, 1.0f, 1.0f};
    glm::vec4 diffuse = {1.0f, 1.0f, 1.0f, 1.0f};
    glm::vec4 specular = {1.0f, 1.0f, 1.0f, 1.0f};
};


struct Light {
    glm::vec3 direction = glm::normalize(glm::vec3{0.5f, -1.0f, 0.25f});
    glm::vec4 ambient = {0.2f, 0.2f, 0.2f, 1.0f};
    glm::vec4 diffuse = {0.8f, 0.8f, 0.8f, 0.8f};
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


Material createMaterial(const aiMaterial *aimaterial) {
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
    aiString diffuseTextureFilePath, specularTextureFilePath, heightTextureFilePath;
    aimaterial->GetTexture(aiTextureType_DIFFUSE, 0, &diffuseTextureFilePath);
    aimaterial->GetTexture(aiTextureType_SPECULAR, 0, &specularTextureFilePath);
    aimaterial->GetTexture(aiTextureType_HEIGHT, 0, &heightTextureFilePath);
    
    // TODO: perform texture loader via a texture manager
    
    return material;
}

std::vector<Material> createMaterialArray(const aiScene *aiscene) {
    std::vector<Material> materials;
    
    materials.resize(aiscene->mNumMaterials);
    
    for (unsigned int i=0; i<aiscene->mNumMaterials; i++) {
        materials[i] = createMaterial(aiscene->mMaterials[i]);
    }
    
    return materials;
}


struct ShaderLocationMap {
    GLint coord = -1;
    GLint normal = -1;
    
    GLint uModel = -1;
    GLint uView = -1;
    GLint uProj = -1;
    
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
    
    location.uModel = glGetUniformLocation(program, "uModel");
    location.uView = glGetUniformLocation(program, "uView");
    location.uProj = glGetUniformLocation(program, "uProj");
    
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
    
    GLuint indexBuffer = 0;
    if (mesh->HasFaces()) {
        // perform a little post-processing for the mesh indexing
        // merge all triangle-shaped faces into a single index array
        std::vector<unsigned int> indices;
        
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


//GLuint createTriangleMeshVAO(const GLuint program) {
//    assert(program);
//
//    const std::vector<glm::vec3> vertices {
//        glm::vec3{0.0f, 1.0f, 0.0f},
//        glm::vec3{1.0f, -1.0f, 0.0f},
//        glm::vec3{-1.0f, -1.0f, 0.0f},
//    };
//
//    const std::vector<glm::vec4> colors {
//        glm::vec4{1.0f, 0.0f, 0.0f, 1.0f},
//        glm::vec4{0.0f, 1.0f, 0.0f, 1.0f},
//        glm::vec4{0.0f, 0.0f, 1.0f, 1.0f},
//    };
//
//    const GLuint coordBuffer = createBuffer(GL_ARRAY_BUFFER, vertices, GL_STATIC_DRAW);
//
//    const GLuint colorBuffer = createBuffer(
//        GL_ARRAY_BUFFER,
//        sizeof(glm::vec4) * colors.size(),
//        colors.data(), GL_STATIC_DRAW
//    );
//
//    assert(coordBuffer);
//
//    GLuint vao = 0;
//
//    glGenVertexArrays(1, &vao);
//    glBindVertexArray(vao);
//
//    const GLint vertCoord = glGetAttribLocation(program, "vertCoord");
//    assert(vertCoord >= 0);
//
//    const GLint vertColor = glGetAttribLocation(program, "vertColor");
//    assert(vertColor >= 0);
//
//    glEnableVertexAttribArray(vertCoord);
//    glBindBuffer(GL_ARRAY_BUFFER, coordBuffer);
//    glVertexAttribPointer(vertCoord, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
//
//    glEnableVertexAttribArray(vertColor);
//    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
//    glVertexAttribPointer(vertColor, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
//
//    glBindVertexArray(0);
//
//    assert(glGetError() == GL_NO_ERROR);
//
//    return vao;
//}


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


int main(int argc, char **argv) {
    // Create an instance of the Importer class
    Assimp::Importer importer;
    // And have it read the given file with some example postprocessing
    // Usually - if speed is not the most important aspect for you - you'll
    // propably to request more postprocessing than we do in this example.
    const auto flags =  aiProcess_Triangulate |
                        aiProcess_JoinIdenticalVertices |
                        // aiProcess_CalcTangentSpace |
                        // aiProcess_SortByPType |
                        aiProcess_ValidateDataStructure;

    const aiScene *scene = importer.ReadFile(argv[1], flags);

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

    const std::vector<Material> materials = createMaterialArray(scene);
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
        if (glfwGetKey(window, GLFW_KEY_UP)) {
            playerPosition += 0.075f * playerDirection;
        }
        else if (glfwGetKey(window, GLFW_KEY_DOWN)) {
            playerPosition -= 0.075f * playerDirection;
        }
        
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
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
