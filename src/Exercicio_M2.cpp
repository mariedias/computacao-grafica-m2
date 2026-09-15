/*
 * Exercício do Módulo 2
 * 
 * Objetivos:
 * - Criação de um cubo 3D a partir de triângulos;
 * - Rotação nos eixos x, y e z;
 * - Translação nos 3 eixos;
 * - Escala uniforme (todos os eixos);
 * - Instanciar mais de um cubo na cena;
 * 
 * Teclas utilizadas:
 * - Rotação eixo x ==> 'X'
 * - Rotação eixo y ==> 'Y'
 * - Rotação eixo z ==> 'Z'
 * - Translação eixo x ==> 'A' (esquerda) e 'D' (direita)
 * - Translação eixo z ==> 'W' (distancia) e 'S' (aproxima)
 * - Translação eixo y ==> 'I' (sobe) e 'J' (desce)
 * - Escala uniforme ==> 'K' (diminui) e 'L' (aumenta)
 * - TAB alterna o objeto selecionado (o não selecionado fica escurecido)
 * 
 * Alunas: Eduarda Fernandes e Maria Eduarda Dias
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

const GLuint WIDTH = 1000, HEIGHT = 800;

struct Object3D
{
    GLuint VAO;
    int nVertices;
    glm::vec3 position;   // translação
    glm::vec3 scale;      // escala
    glm::vec3 rotation;   // ângulos em graus, um por eixo
    glm::vec3 posicaoInicial;
};

vector<Object3D> cena;
int selecionado = 0;


// shaders

const GLchar *vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vColor;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0);
    vColor = color;
}
)";

const GLchar *fragmentShaderSource = R"(
#version 330 core
in vec3 vColor;
uniform int isSelected;
out vec4 color;

void main()
{
    vec3 c = vColor;
    if (isSelected == 0)
    {
        c = c * 0.35;
    }
    color = vec4(c, 1.0);
}
)";

// callbacks

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT)
        return;

    if (key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // alterna entre malha preenchida e wireframe (util para conferir a geometria)
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        static bool wireframe = false;
        wireframe = !wireframe;
        glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
    }

    // seleção
    if (key == GLFW_KEY_TAB)
    {
        selecionado = (selecionado + 1) % cena.size();
        cout << "Objeto selecionado: " << selecionado << endl;
    }

    // aplica no objeto selecionado
    Object3D &obj = cena[selecionado];

    // rotação do objeto nos 3 eixos 
    if (key == GLFW_KEY_X) obj.rotation.x += 5.0f;
    if (key == GLFW_KEY_Y) obj.rotation.y += 5.0f;
    if (key == GLFW_KEY_Z) obj.rotation.z += 5.0f;
    
    // translação do objeto nos 3 eixos
    if (key == GLFW_KEY_A)  obj.position.x -= 0.1f;
    if (key == GLFW_KEY_D)  obj.position.x += 0.1f;
    if (key == GLFW_KEY_W)  obj.position.z -= 0.1f;
    if (key == GLFW_KEY_S)  obj.position.z += 0.1f;
    if (key == GLFW_KEY_I)  obj.position.y += 0.1f;
    if (key == GLFW_KEY_J)  obj.position.y -= 0.1f;
    
    // escala uniforme (K/L em vez de [/] porque em teclado ABNT2
    // o GLFW reporta a posição física da tecla, não o caractere impresso)
    if (key == GLFW_KEY_K) obj.scale *= 1.0f - 0.05f;
    if (key == GLFW_KEY_L) obj.scale *= 1.0f + 0.05f;

    if (key == GLFW_KEY_SPACE)
    {
        for (size_t i = 0; i < cena.size(); i++)
        {
            cena[i].position = cena[i].posicaoInicial;
            cena[i].scale = glm::vec3(1.0f);
            cena[i].rotation = glm::vec3(0.0f);
        }
        cout << "Cena resetada" << endl;
    }
}

// shader

int setupShader()
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        cerr << "ERRO no vertex shader:\n" << infoLog << endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        cerr << "ERRO no fragment shader:\n" << infoLog << endl;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        cerr << "ERRO ao linkar o shader:\n" << infoLog << endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

int setupCubo(int &nVertices) 
{
    GLfloat vertices[] = {
        // ordem: x y z r g b

        // ------------------------ z = 0.5f (rosa)
        // triângulo ABC
        -0.5f, -0.5f,  0.5f,   0.98f, 0.71f, 0.78f, // baixo, esquerda, frente ==> v A
         0.5f, -0.5f,  0.5f,   0.98f, 0.71f, 0.78f, // baixo, direita, frente ==> v B
         0.5f,  0.5f,  0.5f,   0.98f, 0.71f, 0.78f, // cima, direita, frente ==> v C

        // triângulo ACD
        -0.5f, -0.5f,  0.5f,   0.98f, 0.71f, 0.78f, // baixo, esquerda, frente ==> v A
         0.5f,  0.5f,  0.5f,   0.98f, 0.71f, 0.78f, // cima, direita, frente ==> v C
        -0.5f,  0.5f,  0.5f,   0.98f, 0.71f, 0.78f, // cima, esquerda, frente ==> v D

        // ------------------------ z = -0.5f (verde)
        // triângulo EFG
        -0.5f, -0.5f,  -0.5f,   0.70f, 0.87f, 0.66f, // baixo, esquerda, atrás ==> v E
         0.5f, -0.5f,  -0.5f,   0.70f, 0.87f, 0.66f, // baixo, direita, atrás ==> v F
         0.5f,  0.5f,  -0.5f,   0.70f, 0.87f, 0.66f, // cima, direita, atrás ==> v G

        // triângulo EGH
        -0.5f, -0.5f,  -0.5f,   0.70f, 0.87f, 0.66f, // baixo, esquerda, atrás ==> v E
         0.5f,  0.5f,  -0.5f,   0.70f, 0.87f, 0.66f, // cima, direita, atrás ==> v G
        -0.5f,  0.5f,  -0.5f,   0.70f, 0.87f, 0.66f, // cima, esquerda, atrás ==> v H

        // ----------------------- face lateral esquerda (lilás)
        // triângulo EAD
        -0.5f, -0.5f,  -0.5f,   0.79f, 0.72f, 0.92f, // baixo, esquerda, atrás ==> v E
        -0.5f, -0.5f,   0.5f,   0.79f, 0.72f, 0.92f, // baixo, esquerda, frente ==> v A
        -0.5f,  0.5f,   0.5f,   0.79f, 0.72f, 0.92f, // cima, esquerda, frente ==> v D

        // triângulo EDH
        -0.5f, -0.5f,  -0.5f,   0.79f, 0.72f, 0.92f, // baixo, esquerda, atrás ==> v E
        -0.5f,  0.5f,   0.5f,   0.79f, 0.72f, 0.92f, // cima, esquerda, frente ==> v D
        -0.5f,  0.5f,  -0.5f,   0.79f, 0.72f, 0.92f, // cima, esquerda, atrás ==> v H

        // ----------------------- face lateral direita (amarelo)
        // triângulo FBC
        0.5f, -0.5f,  -0.5f,   0.99f, 0.90f, 0.60f, // baixo, direita, atrás ==> v F
        0.5f, -0.5f,   0.5f,   0.99f, 0.90f, 0.60f, // baixo, direita, frente ==> v B
        0.5f,  0.5f,   0.5f,   0.99f, 0.90f, 0.60f, // cima, direita, frente ==> v C

        // triângulo FCG
        0.5f, -0.5f,  -0.5f,   0.99f, 0.90f, 0.60f, // baixo, direita, atrás ==> v F
        0.5f,  0.5f,   0.5f,   0.99f, 0.90f, 0.60f, // cima, direita, frente ==> v C
        0.5f,  0.5f,  -0.5f,   0.99f, 0.90f, 0.60f, // cima, direita, atrás ==> v G

        // ----------------------- face topo (azul)
        // triângulo DCG
        -0.5f, 0.5f,   0.5f,   0.68f, 0.82f, 0.94f, // cima, esquerda, frente ==> v D
         0.5f, 0.5f,   0.5f,   0.68f, 0.82f, 0.94f, // cima, direita, frente ==> v C
         0.5f, 0.5f,  -0.5f,   0.68f, 0.82f, 0.94f, // cima, direita, atrás ==> v G

        // triângulo DGH
        -0.5f, 0.5f,   0.5f,   0.68f, 0.82f, 0.94f, // cima, esquerda, frente ==> v D
         0.5f, 0.5f,  -0.5f,   0.68f, 0.82f, 0.94f, // cima, direita, atrás ==> v G
        -0.5f, 0.5f,  -0.5f,   0.68f, 0.82f, 0.94f, // cima, esquerda, atrás ==> v H

        // ----------------------- face base (laranja)
        // triângulo ABF
        -0.5f, -0.5f,   0.5f,   0.99f, 0.78f, 0.60f, // baixo, esquerda, frente ==> v A
         0.5f, -0.5f,   0.5f,   0.99f, 0.78f, 0.60f, // baixo, direita, frente ==> v B
         0.5f, -0.5f,  -0.5f,   0.99f, 0.78f, 0.60f, // baixo, direita, atrás ==> v F

        // triângulo AFE
        -0.5f, -0.5f,   0.5f,   0.99f, 0.78f, 0.60f, // baixo, esquerda, frente ==> v A
         0.5f, -0.5f,  -0.5f,   0.99f, 0.78f, 0.60f, // baixo, direita, atrás ==> v F
        -0.5f, -0.5f,  -0.5f,   0.99f, 0.78f, 0.60f, // baixo, esquerda, atrás ==> v E
    };

    nVertices = sizeof(vertices) / sizeof(GLfloat) / 6;

    GLuint VBO, VAO;

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}

glm::mat4 getModelMatrix(const Object3D &obj)
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, obj.position);
    model = glm::rotate(model, glm::radians(obj.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(obj.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(obj.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, obj.scale);
    return model;
}


// main

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Exercicio M2", nullptr, nullptr);
    if (!window)
    {
        cerr << "Falha ao criar a janela GLFW" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cerr << "Falha ao inicializar o GLAD" << endl;
        return -1;
    }

    cout << "Placa de video: " << glGetString(GL_RENDERER) << endl;
    cout << "Versao do OpenGL: " << glGetString(GL_VERSION) << endl;

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    GLuint shaderID = setupShader();
    int nVertices = 0;
    GLuint cuboVAO = setupCubo(nVertices);

    Object3D obj1;
    obj1.VAO = cuboVAO;
    obj1.nVertices = nVertices;
    obj1.position = glm::vec3(-1.2f, 0.0f, 0.0f);
    obj1.posicaoInicial = glm::vec3(-1.2f, 0.0f, 0.0f);
    obj1.scale = glm::vec3(1.0f, 1.0f, 1.0f);
    obj1.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    cena.push_back(obj1);

    Object3D obj2 = obj1;                          // mesma geometria
    obj2.position = glm::vec3(1.2f, 0.0f, 0.0f);   // outra posição
    obj2.posicaoInicial = glm::vec3(1.2f, 0.0f, 0.0f);
    cena.push_back(obj2);

    glUseProgram(shaderID);

    GLint modelLoc = glGetUniformLocation(shaderID, "model");
    GLint viewLoc = glGetUniformLocation(shaderID, "view");
    GLint projLoc = glGetUniformLocation(shaderID, "projection");
    GLint selectedLoc = glGetUniformLocation(shaderID, "isSelected");

    // camera (matriz de view) e projecao perspectiva
    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 5.0f), // posicao da camera
        glm::vec3(0.0f, 0.0f, 0.0f), // para onde olha
        glm::vec3(0.0f, 1.0f, 0.0f)  // vetor "up"
    );

    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        (float)width / (float)height,
        0.1f, 100.0f);

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glEnable(GL_DEPTH_TEST);

    // loop principal
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.10f, 0.08f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // desenha cada objeto da cena com sua própria matriz de modelo
        for (size_t i = 0; i < cena.size(); i++)
        {
            glUniform1i(selectedLoc, (int)i == selecionado ? 1 : 0);

            glm::mat4 model = getModelMatrix(cena[i]);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            glBindVertexArray(cena[i].VAO);
            glDrawArrays(GL_TRIANGLES, 0, cena[i].nVertices);
        }
        
        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &cuboVAO);
    glfwTerminate();
    return 0;
}