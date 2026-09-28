#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <raymath.h>
#include <cassert>
#include <iostream>

void APIENTRY DebugCallback(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam);

int main(void)
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

    GLFWwindow* window = glfwCreateWindow(1600, 900, "Graphics 1", nullptr, nullptr);
    assert(window != nullptr);

    glfwMakeContextCurrent(window);
    assert(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == true);

#ifdef NDEBUG
#else
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(DebugCallback, nullptr);
#endif

    // Face-culling is disabled by default, but we should remember that OpenGL defines front-faces as CCW winding-order
    Vector3 triangle_positions[] =
    {
        {  0.0f,  0.5f, 0.0 },
        { -0.5f, -0.5f, 0.0 },
        {  0.5f, -0.5f, 0.0 }
    };

    Vector3 triangle_colors[] =
    {
        {  1.0f, 0.0f, 0.0 },
        {  0.0f, 1.0f, 0.0 },
        {  0.0f, 0.0f, 1.0 }
    };

    GLuint triangle_vao = GL_NONE;
    GLuint triangle_positions_vbo = GL_NONE;
    GLuint triangle_colors_vbo = GL_NONE;

    glGenVertexArrays(1, &triangle_vao);
    glGenBuffers(1, &triangle_positions_vbo);
    glGenBuffers(1, &triangle_colors_vbo);

    glBindVertexArray(triangle_vao);                            // "Record the following state of vertex buffers within this vertex array"

    glBindBuffer(GL_ARRAY_BUFFER, triangle_positions_vbo);      // The following array (vertex) buffer will be described (triangle_positions_vbo):
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);// Index 0, 3 components, float data, no implicit normalization, no stride, no offset
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(Vector3), triangle_positions, GL_STATIC_DRAW); // Transfer memory of triangle_positions into currently bound vbo
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, triangle_colors_vbo);         // The following array (vertex) buffer will be described (triangle_colors_vbo):
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);// Index 1, 3 components, float data, no implicit normalization, no stride, no offset
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(Vector3), triangle_colors, GL_STATIC_DRAW); // Transfer memory of triangle_colors into currently bound vbo
    glEnableVertexAttribArray(1);

    glBindVertexArray(GL_NONE);                                 // "Stop recording vertex buffer state"

    static const char* vertex_shader_source =
    R"(
        #version 430 core
        layout(location = 0) in vec3 vertex_position;
        layout(location = 1) in vec3 vertex_colour;
        out vec3 colour;
        void main() {
        colour = vertex_colour;
        gl_Position = vec4(vertex_position, 1.0);
        }
    )";

    static const char* fragment_shader_source =
    R"(
        #version 430 core
        in vec3 colour;
        out vec4 frag_colour;
        void main() {
        frag_colour = vec4(colour, 1.0);
        }
    )";

    GLint compile_status = -1;
    GLchar compile_log[512];

    GLuint vertex_shader_handle = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader_handle = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertex_shader_handle, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader_handle);
    glGetShaderiv(vertex_shader_handle, GL_COMPILE_STATUS, &compile_status);
    if (!compile_status)
    {
        glGetShaderInfoLog(vertex_shader_handle, 512, NULL, compile_log);
        std::cout << "Shader failed to compile: \n" << compile_log << std::endl;
    }

    glShaderSource(fragment_shader_handle, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader_handle);
    glGetShaderiv(fragment_shader_handle, GL_COMPILE_STATUS, &compile_status);
    if (!compile_status)
    {
        glGetShaderInfoLog(fragment_shader_handle, 512, NULL, compile_log);
        std::cout << "Shader failed to compile: \n" << compile_log << std::endl;
    }

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader_handle);
    glAttachShader(shader_program, fragment_shader_handle);
    glLinkProgram(shader_program);

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader_program);
        glBindVertexArray(triangle_vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
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
