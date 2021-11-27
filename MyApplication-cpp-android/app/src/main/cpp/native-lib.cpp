
#include <string>
#include <vector>
#include <optional>

#include <GLES2/gl2.h>
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>

float clamp(float a, float x) {
    return a > x ? x : a;
}


float lerp(float a, float b, float s) {
    return a + s*(b - a);
}


struct color_t { float r, g, b; };


color_t color(int c) {
    switch (c) {
        case 7:
        case 0: return {0.0f, 0.0f, 0.0f};
        case 1: return {1.0f, 0.0f, 0.0f};
        case 2: return {0.0f, 1.0f, 0.0f};
        case 3: return {0.0f, 0.0f, 1.0f};
        case 4: return {1.0f, 1.0f, 0.0f};
        case 5: return {0.0f, 1.0f, 1.0f};
        case 6: return {1.0f, 0.0f, 1.0f};
        // case 7: return {1.0f, 1.0f, 1.0f};
    }

    return { 0.0f, 0.0f, 0.0f };
};


static const float triangle_coords[] = {
        0.0f, 0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f
};


static const float triangle_colors[] = {
        1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
};


static AAssetManager *assetManager = nullptr;

static GLuint programId = 0;
static GLuint coordVertexBuffer = 0;
static GLuint colorVertexBuffer = 0;


std::string loadTextFile(const std::string &fileName) {
    assert(assetManager);
    assert(fileName != "");

    /*
    AAssetDir *dir = AAssetManager_openDir(assetManager, "");
    assert(dir);
    const char *dirFileName = nullptr;

    do {
        dirFileName = AAssetDir_getNextFileName(dir);
        __android_log_print(ANDROID_LOG_ERROR, "TRACKERS", "%s", dirFileName);
    } while (dirFileName);
    */
    AAsset *asset = AAssetManager_open(assetManager, fileName.c_str(), 0);

    assert(asset);

    const size_t fileSize = AAsset_seek(asset, 0, SEEK_END);

    std::string content;
    content.resize(fileSize);

    AAsset_seek(asset, 0, SEEK_SET);
    AAsset_read(asset, (void*)content.c_str(), fileSize);
    AAsset_close(asset);

    return content;
}

GLuint loadShader(GLuint shaderType, const std::string &fileName) {
    const std::string content = loadTextFile(fileName);
    const char *contentData = content.c_str();
    const int contentSize = content.size();

    GLuint shaderId = glCreateShader(shaderType);
    glShaderSource(shaderId, 1, &contentData, &contentSize);
    glCompileShader(shaderId);

    GLint status;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE) {
        const GLint size = 4094;
        char infoLog[size];

        glGetShaderInfoLog(shaderId, size, nullptr, infoLog);

        __android_log_print(ANDROID_LOG_ERROR, "MYAPPLICATION", "%s", infoLog);
    }

    return shaderId;
}


void drawTriangle() {
    const GLint coordLocation = glGetAttribLocation(programId, "vCoord");
    const GLint colorLocation = glGetAttribLocation(programId, "vColor");

    // glDisable(GL_CULL_FACE);

    glEnableVertexAttribArray(coordLocation);
    glVertexAttribPointer(coordLocation, 3, GL_FLOAT, GL_FALSE, 0, triangle_coords);

    glEnableVertexAttribArray(colorLocation);
    glVertexAttribPointer(colorLocation, 3, GL_FLOAT, GL_FALSE, 0, triangle_colors);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDisableVertexAttribArray(colorLocation);
    glDisableVertexAttribArray(coordLocation);
}


GLuint createShaderProgram() {
    GLuint programId = glCreateProgram();

    GLuint shaders[] = {
            loadShader(GL_VERTEX_SHADER, "shader.vert"),
            loadShader(GL_FRAGMENT_SHADER, "shader.frag")
    };

    for (const GLuint shader : shaders) {
        assert(glIsShader(shader));
        glAttachShader(programId, shader);
    }

    glLinkProgram(programId);

    GLint status;
    glGetProgramiv(programId, GL_LINK_STATUS, &status);

    if (status == GL_FALSE) {
        const GLint size = 4094;
        char infoLog[size];

        glGetProgramInfoLog(programId, size, nullptr, infoLog);

        __android_log_print(ANDROID_LOG_ERROR, "MYAPPLICATION", "%s", infoLog);
    }

    assert(glIsProgram(programId) == GL_TRUE);

    return programId;
}


GLuint createVertexBuffer(const float *data, const size_t count) {
    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(float), data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    assert(glIsBuffer(vertexBuffer));

    return vertexBuffer;
}

void on_surface_created() {
    // coordVertexBuffer = createVertexBuffer(triangle_coords, 3);
    // colorVertexBuffer = createVertexBuffer(triangle_colors, 3);

    programId = createShaderProgram();
    glUseProgram(programId);
}


void on_surface_changed(const int width, const int height) {
    glViewport(0, 0, width, height);
}


color_t compute_next_color() {
    const float step = 0.5f;
    static float current = 0.0f;
    static int i = 0;

    const color_t c1 = color(i);
    const color_t c2 = color(7);

    const color_t c = {
            lerp(c1.r, c2.r, current),
            lerp(c1.g, c2.g, current),
            lerp(c1.b, c2.b, current)
    };

    current += step;

    if (current  > 1.0f) {
        current = 0.0f;
        i++;
    }

    if (i >= 6) {
        i = 0;
    }

    return c;
}

void on_draw_frame() {
    color_t c = compute_next_color();

    glClearColor(c.r, c.g, c.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    drawTriangle();
}


extern "C" JNIEXPORT jstring JNICALL
Java_com_example_myapplication_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {

    const char msg[] = "OpenGL ES 2 Rendering not supported!";
    return env->NewStringUTF(msg);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_myapplication_NativeGameLogic_on_1surface_1created(JNIEnv *env, jclass clazz) {
    on_surface_created();
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_myapplication_NativeGameLogic_on_1surface_1changed(JNIEnv *env, jclass clazz,
                                                                    jint width, jint height) {
    on_surface_changed(width, height);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_myapplication_NativeGameLogic_on_1draw_1frame(JNIEnv *env, jclass clazz) {
    on_draw_frame();
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_myapplication_NativeGameLogic_on_1initial_1setup(JNIEnv *env, jclass clazz,
                                                                  jobject asset_manager) {
    assetManager = AAssetManager_fromJava(env, asset_manager);
}
