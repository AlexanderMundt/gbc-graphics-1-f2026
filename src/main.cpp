/*
Completed By:   Alexander Mundt 101632886
Class:          GAME2012
Professor:      Connor Smiley
Assignment:     Assignment 1
*/
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <raymath.h>
#include <cassert>
#include <iostream>
#include "Shader.h"

void APIENTRY DebugCallback(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

struct App
{
    int object_index;
};

struct DrawCall
{
    GLuint shader;
    GLuint vao;
};

void DrawObject0(App* app, DrawCall draw, Matrix* mvp);
void DrawObject1(App* app, DrawCall draw, Matrix* mvp);
void DrawObject2(App* app, DrawCall draw, Matrix* mvp);
void DrawObject3(App* app, DrawCall draw, Matrix* mvp);
void DrawObject4(App* app, DrawCall draw, Matrix* mvp);

int main(void)
{
    App app{};
    DrawCall draw0{}, draw1{}, draw2{}, draw3{}, draw4{};

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

    GLFWwindow* window = glfwCreateWindow(800, 800, "Graphics 1", nullptr, nullptr);
    assert(window != nullptr);

    glfwSetWindowUserPointer(window, &app);
    glfwSetKeyCallback(window, KeyCallback);

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
        {  0.0f,  0.5f, 0.0f },
        { -0.5f, -0.5f, 0.0f },
        {  0.5f, -0.5f, 0.0f }
    };

    Vector3 triangle_colors[] =
    {
        {  1.0f, 0.0f, 0.0 },
        {  0.0f, 1.0f, 0.0 },
        {  0.0f, 0.0f, 1.0 }
    };

    Vector3 triangle_white[] =
    {
        {  1.0f, 1.0f, 1.0 },
        {  1.0f, 1.0f, 1.0 },
        {  1.0f, 1.0f, 1.0 }
    };

	//Vertex Array Objects
    GLuint triangle_rainbow_vao = GL_NONE;  //VAO1
	GLuint triangle_white_vao = GL_NONE;    //VAO2

    //VAO1
    glGenVertexArrays(1, &triangle_rainbow_vao);
    glBindVertexArray(triangle_rainbow_vao);      

	//Pos for VAO1
    GLuint triangle_positions_vbo = GL_NONE;

    glGenBuffers(1, &triangle_positions_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, triangle_positions_vbo); //The following array (vertex) buffer will be described (triangle_positions_vbo):
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(Vector3), triangle_positions, GL_STATIC_DRAW); //Transfer memory of triangle_positions into currently bound vbo
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr); //Index 0, 3 components, float data, no implicit normalization, no stride, no offset
    glEnableVertexAttribArray(0);

	//Colors for VAO1
    GLuint triangle_colors_vbo = GL_NONE;

    glGenBuffers(1, &triangle_colors_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, triangle_colors_vbo); //The following array (vertex) buffer will be described (triangle_colors_vbo):
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr); //Index 1, 3 components, float data, no implicit normalization, no stride, no offset
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(Vector3), triangle_colors, GL_STATIC_DRAW); //Transfer memory of triangle_colors into currently bound vbo
    glEnableVertexAttribArray(1);

    //VAO2
    glGenVertexArrays(1, &triangle_white_vao);
    glBindVertexArray(triangle_white_vao); //"Record the following state of vertex buffers within this vertex array"

    //Pos for VAO2
    triangle_positions_vbo = GL_NONE;

    glGenBuffers(1, &triangle_positions_vbo); //1 GEN
    glBindBuffer(GL_ARRAY_BUFFER, triangle_positions_vbo); //2 BIND
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(Vector3), triangle_positions, GL_STATIC_DRAW); //3 DATA
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr); //4 ATTRIB
    glEnableVertexAttribArray(0);

    //Colors for VAO2
    GLuint triangle_white_vbo = GL_NONE;

    glGenBuffers(1, &triangle_white_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, triangle_white_vbo);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(Vector3), triangle_white, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);

    //End
    glBindVertexArray(GL_NONE); //"Stop recording vertex buffer state"

    GLuint vs_shader = LoadShader(GL_VERTEX_SHADER, "./assets/shaders/a1_triangle.vert");
    GLuint fs_shader = LoadShader(GL_FRAGMENT_SHADER, "./assets/shaders/a1_triangle.frag");
    GLuint shader_program = LoadProgram(vs_shader, fs_shader);

    draw0.vao = triangle_white_vao;
    draw0.shader = shader_program;

    draw1.vao = triangle_rainbow_vao;
    draw1.shader = shader_program;

    draw2.vao = triangle_rainbow_vao;
    draw2.shader = shader_program;

    draw3.vao = triangle_rainbow_vao;
    draw3.shader = shader_program;

    draw4.vao = triangle_rainbow_vao;
    draw4.shader = shader_program;

    Matrix world = MatrixIdentity();
    Matrix view = MatrixLookAt({ 0.0f, 0.0f, 10.0f }, { 0.0f, 0.0f, 0.0f }, Vector3UnitY);
    Matrix proj = MatrixOrtho(-2.0f, 2.0f, -2.0f, 2.0f, 0.1f, 100.0f);
    Matrix mvp = world * view * proj;

    float time = 0.0f;
    app.object_index = 0;

    while (!glfwWindowShouldClose(window))
    {
        time = static_cast<float>(glfwGetTime());

        //Revert the world matrix
        world = MatrixIdentity();
        mvp = world * view * proj;

        switch (app.object_index)
        {
            case 0:
            {
                //1) White triangle
                DrawObject0(&app, draw0, &mvp);
                break;
            }

            case 1:
            {
                //2) Rainbow triangle
                DrawObject1(&app, draw1, &mvp);
                break;
            }

            case 2:
            {
                //3) Triangle whose colour changes over time
                DrawObject2(&app, draw2, &mvp);
                break;
            }

            case 3:
            {
                //4) Triangle translating between x = -1 and x = +1 (doesn't need to start at x = -1)
				//Translate
				world = MatrixTranslate(sinf(time), 0.0f, 0.0f);
                mvp = world * view * proj;

                DrawObject3(&app, draw3, &mvp);
                break;
            }

            case 4:
            {
                //5) Triangle rotating continuously counter-clockwise about the z-axis
                //Rotate
				world = MatrixRotateZ(time * 0.5f + 0.5f);
                mvp = world * view * proj;

                DrawObject4(&app, draw4, &mvp);
                break;
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

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
        // Bound our array to be [0..4]
        ++app->object_index %= 5;
    }
}

// 1) White triangle
void DrawObject0(App* app, DrawCall draw, Matrix* mvp)
{
    assert(app->object_index == 0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(draw.shader);

    GLint u_mvp = glGetUniformLocation(draw.shader, "u_mvp");
    glUniformMatrix4fv(u_mvp, 1, GL_FALSE, MatrixToFloat(*mvp));

    GLint u_colour = glGetUniformLocation(draw.shader, "u_colour");
    glUniform3f(u_colour, 1.0f, 1.0f, 1.0f);

    glBindVertexArray(draw.vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

// 2) Rainbow triangle
void DrawObject1(App* app, DrawCall draw, Matrix* mvp)
{
    assert(app->object_index == 1);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(draw.shader);

    GLint u_mvp = glGetUniformLocation(draw.shader, "u_mvp");
    glUniformMatrix4fv(u_mvp, 1, GL_FALSE, MatrixToFloat(*mvp));

    GLint u_colour = glGetUniformLocation(draw.shader, "u_colour");
    glUniform3f(u_colour, 1.0f, 1.0f, 1.0f);

    glBindVertexArray(draw.vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

// 3) Triangle whose colour changes over time
void DrawObject2(App* app, DrawCall draw, Matrix* mvp)
{
    assert(app->object_index == 2);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(draw.shader);

    GLint u_mvp = glGetUniformLocation(draw.shader, "u_mvp");
    glUniformMatrix4fv(u_mvp, 1, GL_FALSE, MatrixToFloat(*mvp));

    GLint u_colour = glGetUniformLocation(draw.shader, "u_colour");

    //Clamp sinf between 0.0f and 1.0f
	float rgb = 0.5f + (sinf(glfwGetTime()) * 0.5f);
    glUniform3f(u_colour, rgb, rgb, rgb);

    glBindVertexArray(draw.vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

// 4) Triangle translating between x = -1 and x = +1 (doesn't need to start at x = -1)
void DrawObject3(App* app, DrawCall draw, Matrix* mvp)
{
    assert(app->object_index == 3);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(draw.shader);

    GLint u_mvp = glGetUniformLocation(draw.shader, "u_mvp");
    glUniformMatrix4fv(u_mvp, 1, GL_FALSE, MatrixToFloat(*mvp));

    GLint u_colour = glGetUniformLocation(draw.shader, "u_colour");
    glUniform3f(u_colour, 1.0f, 1.0f, 1.0f);

    glBindVertexArray(draw.vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

// 5) Triangle rotating continuously counter-clockwise about the z-axis
void DrawObject4(App* app, DrawCall draw, Matrix* mvp)
{
    assert(app->object_index == 4);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(draw.shader);

    GLint u_mvp = glGetUniformLocation(draw.shader, "u_mvp");
    glUniformMatrix4fv(u_mvp, 1, GL_FALSE, MatrixToFloat(*mvp));

    GLint u_colour = glGetUniformLocation(draw.shader, "u_colour");
    glUniform3f(u_colour, 1.0f, 1.0f, 1.0f);

    glBindVertexArray(draw.vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
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