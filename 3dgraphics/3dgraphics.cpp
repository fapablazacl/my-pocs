
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


GLuint createTriangleMeshVAO(const GLuint program) {
    assert(program);

    const std::array<glm::vec3, 3> vertices {
        glm::vec3{0.0f, 1.0f, 0.0f},
        glm::vec3{1.0f, -1.0f, 0.0f},
        glm::vec3{-1.0f, -1.0f, 0.0f},
    };

    const std::array<glm::vec4, 3> colors {
        glm::vec4{1.0f, 0.0f, 0.0f, 1.0f},
        glm::vec4{0.0f, 1.0f, 0.0f, 1.0f},
        glm::vec4{0.0f, 0.0f, 1.0f, 1.0f},
    };

    const GLuint coordBuffer = createBuffer(
        GL_ARRAY_BUFFER, 
        sizeof(glm::vec3) * vertices.size(), 
        vertices.data(), GL_STATIC_DRAW
    );

    const GLuint colorBuffer = createBuffer(
        GL_ARRAY_BUFFER, 
        sizeof(glm::vec4) * colors.size(), 
        colors.data(), GL_STATIC_DRAW
    );

    assert(coordBuffer);
    
    GLuint vao = 0;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    const GLint vertCoord = glGetAttribLocation(program, "vertCoord");
    assert(vertCoord >= 0);

    const GLint vertColor = glGetAttribLocation(program, "vertColor");
    assert(vertColor >= 0);

    glEnableVertexAttribArray(vertCoord);
    glBindBuffer(GL_ARRAY_BUFFER, coordBuffer);
    glVertexAttribPointer(vertCoord, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glEnableVertexAttribArray(vertColor);
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glVertexAttribPointer(vertColor, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindVertexArray(0);

    assert(glGetError() == GL_NO_ERROR);

    return vao;
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
    const std::vector<GLuint> shaders = {
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
                        aiProcess_CalcTangentSpace |
                        aiProcess_SortByPType | 
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

    glfwInit();

    const auto monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_DEPTH_BITS, 16);
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "3dgraphics", monitor, nullptr);

    if (!window) {
        std::cout << "Cant open a Window" << std::endl;
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

    const GLint uProj = glGetUniformLocation(program, "uProj");
    assert(uProj >= 0);

    const GLint uView = glGetUniformLocation(program, "uView");
    assert(uView >= 0);

    const GLint uModel = glGetUniformLocation(program, "uModel");
    assert(uModel >= 0);

    const GLuint triangleMeshVAO = createTriangleMeshVAO(program);

    bool running = true;

    const glm::mat4 proj = glm::perspective(
        45.0f, 
        static_cast<float>(windowWidth) / static_cast<float>(windowHeight), 
        0.1f, 
        100.0f);

    const glm::mat4 view = glm::lookAt(
        glm::vec3{0.0f, 0.0f, 10.0f}, 
        glm::vec3{0.0f, 0.0f, 0.0f},
        glm::vec3{0.0f, 1.0f, 0.0f});

    const glm::mat4 model = glm::identity<glm::mat4>();

    while (running) {
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            running = false;
        }

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program);
        glUniformMatrix4fv(uProj, 1, GL_FALSE, glm::value_ptr(proj));
        glUniformMatrix4fv(uView, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(triangleMeshVAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glFlush();
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    // We're done. Everything will be cleaned up by the importer destructor
    return EXIT_SUCCESS;
};
