/*
Completed By:   Alexander Mundt 101632886
Class:          GAME2012
Professor:      Connor Smiley
Assignment:     Assignment 4
*/
#define _CRT_SECURE_NO_WARNINGS

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#define FAST_OBJ_IMPLEMENTATION
#include "fast_obj.h"

#define PAR_SHAPES_IMPLEMENTATION
#include "par_shapes.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <raymath.h>

#include <cassert>
#include <iostream>
#include <vector>
#include "Shader.h"

//Constants
constexpr int NUM_MESHES = 5;
constexpr int WINDOW_WIDTH = 1920;
constexpr int WINDOW_HEIGHT = 1080;

enum ShaderVertexAttribute : int
{
    ATTRIBUTE_POSITIONS,
    ATTRIBUTE_NORMALS,
    ATTRIBUTE_TCOORDS,
    ATTRIBUTE_TEXTURED
};

enum RenderChoice : int
{
    PAR_SHAPES_NORMALS = 0,
    OBJ_TCOORDS = 1,
    MESH_TEXTURED = 2,
    MANUAL_GEN = 3,
    WILDCARD = 4
};

struct Camera
{
    Vector3 cameraPosition = { 0.0f, 0.0f, 10.0f };
    float cameraYaw = -90.0f * DEG2RAD;
    float cameraPitch = 0.0f;
    float mouseSensitivity = 0.003f;
    float cameraMoveSpeed = 3.0f;
    Vector3 cameraForward = { cosf(cameraPitch) * cosf(cameraYaw), sinf(cameraPitch), cosf(cameraPitch) * sinf(cameraYaw) };
    Vector3 cameraRight = Vector3Normalize(Vector3CrossProduct(cameraForward, Vector3UnitY));
    Vector3 cameraUp = Vector3Normalize(Vector3CrossProduct(cameraRight, cameraForward));

    //Store the last position of the mouse
    float lastMouseXPos = WINDOW_WIDTH / 2;
    float lastMouseYPos = WINDOW_HEIGHT / 2;
};

struct App
{
    GLFWwindow* window;
    int object_index;
    int attribute_color;
    int color_space;
    float lastFrame;
    float deltaTime;
    Camera mainCam;
};

struct Mesh
{
    std::vector<Vector3> positions;
    std::vector<Vector3> normals;
    std::vector<Vector2> tcoords;
    int index_count;

    GLuint vao;
    GLuint vbo_positions;
    GLuint vbo_normals;
    GLuint vbo_tcoords;
};

void Init(App* app);
void APIENTRY DebugCallback(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void MousePosCallback(GLFWwindow* window, double xInputPos, double yInputPos);

void LoadMeshObj(Mesh* mesh, const char* path);
void LoadMeshTorus(Mesh* mesh);
void LoadMeshPlane(Mesh* mesh);
void LoadMeshPar(Mesh* mesh, par_shapes_mesh* par);
void LoadMeshTriangle(Mesh* mesh);
void LoadMeshGpu(Mesh* mesh);
void UnloadMeshGpu(Mesh* mesh);

void LoadMeshObj(Mesh* mesh, const char* path)
{
    fastObjMesh* obj_mesh;
    obj_mesh = fast_obj_read(path);

    mesh->index_count = obj_mesh->index_count;
    mesh->positions.resize(mesh->index_count);
    mesh->normals.resize(mesh->index_count);
    mesh->tcoords.resize(mesh->index_count);

    for (fastObjUInt i = 0; i < mesh->index_count; i++)
    {
        Vector3* obj_positions_buffer = (Vector3*)obj_mesh->positions;
        Vector3* obj_normals_buffer = (Vector3*)obj_mesh->normals;
        Vector2* obj_tcoords_buffer = (Vector2*)obj_mesh->texcoords;

        fastObjUInt obj_position_index = obj_mesh->indices[i].p;
        fastObjUInt obj_normal_index = obj_mesh->indices[i].n;
        fastObjUInt obj_tcoord_index = obj_mesh->indices[i].t;

        mesh->positions[i] = obj_positions_buffer[obj_position_index];
        mesh->normals[i] = obj_normals_buffer[obj_normal_index];
        mesh->tcoords[i] = obj_tcoords_buffer[obj_tcoord_index];
    }

    LoadMeshGpu(mesh);
    fast_obj_destroy(obj_mesh);
}

void LoadMeshTorus(Mesh* mesh)
{
    par_shapes_mesh* par = par_shapes_create_torus(30, 40, 0.1);

    LoadMeshPar(mesh, par);
    LoadMeshGpu(mesh);

    par_shapes_free_mesh(par);
}

void LoadMeshPlane(Mesh* mesh)
{
    par_shapes_mesh* par = par_shapes_create_plane(1, 1);
    par_shapes_translate(par, -0.5f, -0.5f, 0.0f);

    LoadMeshPar(mesh, par);
    LoadMeshGpu(mesh);

    par_shapes_free_mesh(par);
}

void LoadMeshPar(Mesh* mesh, par_shapes_mesh* par)
{
    mesh->index_count = par->ntriangles * 3;
    mesh->positions.resize(mesh->index_count);
    mesh->normals.resize(mesh->index_count);
    mesh->tcoords.resize(mesh->index_count);

    Vector3* positions = (Vector3*)par->points;
    Vector3* normals = (Vector3*)par->normals;
    Vector2* tcoords = (Vector2*)par->tcoords;
    for (int i = 0; i < mesh->index_count; i++)
    {
        uint16_t index = par->triangles[i];
        Vector3 position = positions[index];
        Vector3 normal = normals[index];
        Vector2 tcoord = tcoords[index];

        mesh->positions[i] = position;
        mesh->normals[i] = normal;
        mesh->tcoords[i] = tcoord;
    }
}

void LoadMeshTriangle(Mesh* mesh)
{
    mesh->index_count = 3;

    mesh->positions =
    {
        { 0.0f,  0.5f, 0.0f },
        { -0.5f, -0.5f, 0.0f },
        { 0.5f, -0.5f, 0.0f }
    };

    mesh->normals =
    {
        { 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 1.0f }
    };

    mesh->tcoords =
    {
        { 0.5f, 1.0f }, //Top vertex
        { 0.0f, 0.0f }, //Bottom left vertex
        { 1.0f, 0.0f }  //Bottom right vertex
    };

    LoadMeshGpu(mesh);
}

void LoadMeshGpu(Mesh* mesh)
{
    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo_positions);
    glGenBuffers(1, &mesh->vbo_normals);
    glGenBuffers(1, &mesh->vbo_tcoords);

    glBindVertexArray(mesh->vao);

    // Note: positions, normals, and tcoords should be identical in size if loaded correctly!
    // Future improvement: optimize with index buffer (glElementArrayBuffer), but probably not cause that's more work 
    // TODO -- Change to assert after A3 late submission expires

    assert(!mesh->positions.empty());
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo_positions);
    glVertexAttribPointer(ATTRIBUTE_POSITIONS, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBufferData(GL_ARRAY_BUFFER, mesh->positions.size() * sizeof(Vector3), mesh->positions.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);

    if (!mesh->normals.empty())
    {
        glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo_normals);
        glVertexAttribPointer(ATTRIBUTE_NORMALS, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glBufferData(GL_ARRAY_BUFFER, mesh->normals.size() * sizeof(Vector3), mesh->normals.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
    }
    else
        printf("Warning: Uploading a mesh without normals to the GPU!\n");

    if (!mesh->tcoords.empty())
    {
        glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo_tcoords);
        glVertexAttribPointer(ATTRIBUTE_TCOORDS, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
        glBufferData(GL_ARRAY_BUFFER, mesh->tcoords.size() * sizeof(Vector2), mesh->tcoords.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
    }
    else
        printf("Warning: Uploading a mesh without tcoords to the GPU!\n");

    glBindVertexArray(GL_NONE);
}

void UnloadMeshGpu(Mesh* mesh)
{
    glDeleteBuffers(1, &mesh->vbo_positions);
    glDeleteBuffers(1, &mesh->vbo_normals);
    glDeleteBuffers(1, &mesh->vbo_tcoords);
    glDeleteVertexArrays(1, &mesh->vao);
}

struct Color
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

struct Image
{
    std::vector<Color> pixels;
    int width;
    int height;
};

Image GenImageTest()
{
    Image img{};
    img.width = 512;
    img.height = 512;

    img.pixels.resize(img.width * img.height);
    for (int x = 0; x < img.width; x++)
    {
        for (int y = 0; y < img.height; y++)
        {
            Color red = { 255, 0, 0, 255 };
            Color blue = { 0, 0, 255, 255 };
            Color color = y >= img.height / 2 ? red : blue;
            img.pixels[y * img.width + x] = color;
        }
    }

    return img;
}

int main()
{
    App app{};
    app.attribute_color = ATTRIBUTE_TEXTURED;
    Init(&app);

    int width, height;
    glfwGetWindowSize(app.window, &width, &height);

    Mesh meshes[NUM_MESHES];
    LoadMeshTorus(&meshes[0]); //1. Mesh from par shapes using the normals shader
    LoadMeshObj(&meshes[1], "./assets/meshes/monkey.obj"); //2. Mesh loaded from an obj file using the tcoords shader
    LoadMeshObj(&meshes[2], "./assets/meshes/trees.obj"); //3. Mesh loaded with a texture applied to it
    LoadMeshTriangle(&meshes[3]); //4. Manually generated vertices
    LoadMeshPlane(&meshes[4]); //5. Manually generated vertices

    GLuint vs_shader = LoadShader(GL_VERTEX_SHADER, "./assets/shaders/a3_mesh.vert");
    GLuint fs_shader = LoadShader(GL_FRAGMENT_SHADER, "./assets/shaders/a3_mesh.frag");
    GLuint shader_program = LoadProgram(vs_shader, fs_shader);

    //Test texture
    Image test_image = GenImageTest();
    stbi_write_png("./assets/textures/test.png", test_image.width, test_image.height, 4, test_image.pixels.data(), 0);
    stbi__vertical_flip(test_image.pixels.data(), test_image.width, test_image.height, sizeof(Color));
    // ^ hack to flip our generated image ([0, 0] in image memory is top-left, but [0, 0] in OpenGL texture-space is bottom-left)
    stbi_set_flip_vertically_on_load(1);

    GLuint test_texture;
    glGenTextures(1, &test_texture);
    glBindTexture(GL_TEXTURE_2D, test_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, test_image.width, test_image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, test_image.pixels.data());
    glBindTexture(GL_TEXTURE_2D, GL_NONE);

    //Tree model texture
    int trees_image_width, trees_image_height, trees_image_channels;
    stbi_uc* trees_image_memory = stbi_load("./assets/textures/trees.png", &trees_image_width, &trees_image_height, &trees_image_channels, 4);

    GLuint trees_texture;
    glGenTextures(1, &trees_texture);
    glBindTexture(GL_TEXTURE_2D, trees_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, trees_image_width, trees_image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, trees_image_memory);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);

    app.lastFrame = (float)glfwGetTime();

    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(app.window))
    {
        //Calculate delta time. Used in mouse movement
        float currentFrame = (float)glfwGetTime();
        app.deltaTime = currentFrame - app.lastFrame;
        app.lastFrame = currentFrame;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        static float animate_z = 0.0f;

        //Default to the test texture
        GLuint textureToUse = test_texture;

        //Default world matrix values
        Matrix translation = MatrixTranslate(0.0f, 0.0f, 5.0f + (sinf(glfwGetTime()) * 3.0f) * animate_z);
        Matrix rotation = MatrixRotateY(glfwGetTime() * 0.0f * DEG2RAD);
        Matrix scale = MatrixScale(1.0, 1.0, 1.0);

        switch (app.object_index)
        {
            //1. Torus
        case PAR_SHAPES_NORMALS:
            app.attribute_color = ATTRIBUTE_NORMALS;
            break;

            //2. Monkey
        case OBJ_TCOORDS:
            app.attribute_color = ATTRIBUTE_TCOORDS;
            rotation = MatrixRotateY(180.0f * DEG2RAD);
            break;

            //3. Trees
        case MESH_TEXTURED:
            app.attribute_color = ATTRIBUTE_TEXTURED;
            textureToUse = trees_texture;
            rotation = MatrixRotateY(90.0f * DEG2RAD);
            scale = MatrixScale(0.02, 0.02, 0.02);
            break;

            //4. Manual Gen
        case MANUAL_GEN:
            app.attribute_color = ATTRIBUTE_TEXTURED;
            textureToUse = test_texture;
            break;

            //5. Wildcard
        case WILDCARD:
            app.attribute_color = ATTRIBUTE_TEXTURED;
            textureToUse = trees_texture;
            break;
        }

        Matrix world = rotation * translation * scale;
        Matrix view = MatrixLookAt(app.mainCam.cameraPosition, app.mainCam.cameraPosition + app.mainCam.cameraForward, Vector3UnitY);
        //Matrix proj = MatrixOrtho(-10.0, 10.0, -10.0, 10.0, 0.1, 10.0);                   // 2D projection (objects appear the same size regardless of distance from the camera)
        Matrix proj = MatrixPerspective(PI * 0.5f, (width / (float)height), 0.1, 100.0);    // 3D projection (objects closer to the camera appear bigger)
        Matrix mvp = world * view * proj;

        glUseProgram(shader_program);
        GLint loc_world = glGetUniformLocation(shader_program, "u_world");
        GLint loc_mvp = glGetUniformLocation(shader_program, "u_mvp");
        GLint loc_style = glGetUniformLocation(shader_program, "u_style");
        GLint loc_color_space = glGetUniformLocation(shader_program, "u_color_space");
        GLint loc_sampler = glGetUniformLocation(shader_program, "u_sampler");

        glUniformMatrix4fv(loc_world, 1, GL_FALSE, MatrixToFloat(world));
        glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, MatrixToFloat(mvp));
        glUniform1i(loc_style, app.attribute_color);
        glUniform1i(loc_color_space, app.color_space);

        int texture_unit = 15;
        glUniform1i(loc_sampler, texture_unit);

        const Mesh& mesh = meshes[app.object_index];
        glBindVertexArray(mesh.vao);

        glActiveTexture(GL_TEXTURE0 + texture_unit);    // 0 + 15 == 15
        glBindTexture(GL_TEXTURE_2D, textureToUse);     // texture is bound to currently active texture unit (15)
        glDrawArrays(GL_TRIANGLES, 0, mesh.index_count);
        glActiveTexture(GL_TEXTURE0);                   // 0 is the default active texture unit (reset to default the same way we unbind buffers and textures)
        glBindTexture(GL_TEXTURE_2D, GL_NONE);

        glfwSwapBuffers(app.window);
        glfwPollEvents();
    }

    // TODO (optional): make image loading function that frees memory after uploading to the GPU
    //stbi_image_free(head_image_memory);
    stbi_image_free(trees_image_memory);
    glfwTerminate();
    return 0;
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    App* app = (App*)glfwGetWindowUserPointer(window);

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
    {
        ++app->object_index %= NUM_MESHES;
    }

    if (key == GLFW_KEY_C && action == GLFW_PRESS)
    {
        ++app->attribute_color %= 4;
    }
    if (key == GLFW_KEY_V && action == GLFW_PRESS)
    {
        ++app->color_space %= 2;
    }

    //Camera controls
    if (key == GLFW_KEY_W)
    {
        app->mainCam.cameraPosition += app->mainCam.cameraForward * (app->mainCam.cameraMoveSpeed * app->deltaTime);
    }
    if (key == GLFW_KEY_A)
    {
        app->mainCam.cameraPosition -= app->mainCam.cameraRight * (app->mainCam.cameraMoveSpeed * app->deltaTime);
    }
    if (key == GLFW_KEY_S)
    {
        app->mainCam.cameraPosition -= app->mainCam.cameraForward * (app->mainCam.cameraMoveSpeed * app->deltaTime);
    }
    if (key == GLFW_KEY_D)
    {
        app->mainCam.cameraPosition += app->mainCam.cameraRight * (app->mainCam.cameraMoveSpeed * app->deltaTime);
    }
    if (key == GLFW_KEY_SPACE)
    {
        app->mainCam.cameraPosition += app->mainCam.cameraUp * (app->mainCam.cameraMoveSpeed * app->deltaTime);
    }
    if (key == GLFW_KEY_LEFT_SHIFT)
    {
        app->mainCam.cameraPosition -= app->mainCam.cameraUp * (app->mainCam.cameraMoveSpeed * app->deltaTime);
    }
}

void MousePosCallback(GLFWwindow* window, double xInputPos, double yInputPos)
{
    App* app = (App*)glfwGetWindowUserPointer(window);

    float xMouseDelta = xInputPos - app->mainCam.lastMouseXPos;
    float yMouseDelta = yInputPos - app->mainCam.lastMouseYPos;
    app->mainCam.lastMouseXPos = xInputPos;
    app->mainCam.lastMouseYPos = yInputPos;

    app->mainCam.cameraYaw += xMouseDelta * app->mainCam.mouseSensitivity;
    app->mainCam.cameraPitch -= yMouseDelta * app->mainCam.mouseSensitivity;

    //Clamp pitch so the camera can not flip over the top/bottom
    const float maxPitch = 89.0f * DEG2RAD;
    if (app->mainCam.cameraPitch > maxPitch)
    {
        app->mainCam.cameraPitch = maxPitch;
    }
    if (app->mainCam.cameraPitch < -maxPitch)
    {
        app->mainCam.cameraPitch = -maxPitch;
    }

    //Recalculate the new camera forward based on the new look direction. Also make the other direction vectors react to this change
    app->mainCam.cameraForward = { cosf(app->mainCam.cameraPitch) * cosf(app->mainCam.cameraYaw), sinf(app->mainCam.cameraPitch), cosf(app->mainCam.cameraPitch) * sinf(app->mainCam.cameraYaw) };
    app->mainCam.cameraRight = Vector3Normalize(Vector3CrossProduct(app->mainCam.cameraForward, Vector3UnitY));
    app->mainCam.cameraUp = Vector3Normalize(Vector3CrossProduct(app->mainCam.cameraRight, app->mainCam.cameraForward));
}

void Init(App* app)
{
    assert(glfwInit() == GLFW_TRUE);

    // Request certain OpenGL features
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef NDEBUG
    // Don't make a debug context if in release mode
#else
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

    app->window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Graphics 1", nullptr, nullptr);
    assert(app->window != nullptr);

    glfwSetWindowUserPointer(app->window, app);
    glfwSetKeyCallback(app->window, KeyCallback);
    glfwSetCursorPosCallback(app->window, MousePosCallback);

    glfwMakeContextCurrent(app->window);
    assert(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == true);

#ifdef NDEBUG
#else
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(DebugCallback, nullptr);
#endif
}

void APIENTRY DebugCallback(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam)
{
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " << message << std::endl;

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    } std::cout << std::endl;
    std::cout << std::endl;
}