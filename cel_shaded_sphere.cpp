#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>
#include <iostream>

//press f - For Fullscreen model
//press w a s d for moving camera in space




float cameraX = 0.0f, cameraY = 2.0f, cameraZ = 6.0f;
float lookAtX = 0.0f, lookAtY = 0.0f, lookAtZ = 0.0f;
float cameraSpeed = 0.1f; 
float rotationAngleSphere = 0.0f;
float rotationAngleTorus = 0.0f;
float rotationAnglePineapple = 0.0f;
float rotationAngleBanana = 0.0f;
float rotationSpeed = 0.5f; 
bool isFullScreen = false;
int windowWidth = 1200, windowHeight = 800;


GLuint celShaderProgram, outlineShaderProgram;

glm::mat4 model, view, projection;

GLuint vaoSphere, vboSphere, nboSphere, eboSphere;
int indexCount;
GLuint vaoTorus, vboTorus, nboTorus, eboTorus;
int torusIndexCount;
GLuint vaoPineapple, vboPineapple, nboPineapple, eboPineapple;
int pineappleIndexCount;



void toggleFullScreen();
void handleKeyboard(unsigned char key, int x, int y);


const char* vertexShaderSource = R"(
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

out vec3 fragNormal;
out vec3 fragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(position, 1.0);
    fragPos = vec3(model * vec4(position, 1.0));
    fragNormal = mat3(transpose(inverse(model))) * normal;
}
)";


const char* celFragmentShaderSource = R"(
#version 330 core

in vec3 fragNormal;   
in vec3 fragPos;      

out vec4 fragColor;  


uniform vec3 lightPos = vec3(10.0, 10.0, 10.0); 
uniform vec3 viewPos = vec3(0.0, 2.0, 6.0);     
uniform vec3 objectColor = vec3(1.0, 0.5, 0.3); 
uniform vec3 lightColor = vec3(1.0, 1.0, 1.0);  

void main() {
 
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(lightPos - fragPos);
    vec3 viewDir = normalize(viewPos - fragPos);

    float diff = max(dot(norm, lightDir), 0.0);
    if (diff > 0.75) {
        diff = 1.0;  // Brightest
    } else if (diff > 0.5) {
        diff = 0.75; // Medium bright
    } else if (diff > 0.25) {
        diff = 0.5;  // Medium dark
    } else {
        diff = 0.25; // Darkest
    }

    vec3 diffuse = diff * lightColor * objectColor;

    // Stylized specular highlights
    vec3 reflectDir = reflect(-lightDir, norm); 
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0); 
    spec = step(0.8, spec) * 0.5; 

    vec3 highlight = spec * lightColor;
    vec3 finalColor = diffuse + highlight;

    fragColor = vec4(finalColor, 1.0); 
}
)";



const char* outlineFragmentShaderSource = R"(
#version 330 core

in vec3 fragNormal;
in vec3 fragPos;

out vec4 fragColor;

void main() {
    fragColor = vec4(0.0, 0.0, 0.0, 1.0); // Black outline
}
)";


GLuint compileShader(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void updateRotation() {
    rotationAngleSphere += rotationSpeed;
    rotationAngleTorus += rotationSpeed;
    rotationAnglePineapple += rotationSpeed;
    rotationAngleBanana += rotationSpeed;

    if (rotationAngleSphere > 360.0f) rotationAngleSphere -= 360.0f;
    if (rotationAngleTorus > 360.0f) rotationAngleTorus -= 360.0f;
    if (rotationAnglePineapple > 360.0f) rotationAnglePineapple -= 360.0f;
    if (rotationAngleBanana > 360.0f) rotationAngleBanana -= 360.0f;

    glutPostRedisplay(); 
}

void timer(int value) {
    updateRotation();
    glutTimerFunc(16, timer, 0); 
}



void generateSphere(float radius, int segments, std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<unsigned int>& indices) {
    for (int y = 0; y <= segments; ++y) {
        for (int x = 0; x <= segments; ++x) {
            float xSegment = (float)x / (float)segments;
            float ySegment = (float)y / (float)segments;
            float xPos = radius * cos(xSegment * 2.0f * M_PI) * sin(ySegment * M_PI);
            float yPos = radius * cos(ySegment * M_PI);
            float zPos = radius * sin(xSegment * 2.0f * M_PI) * sin(ySegment * M_PI);

            vertices.emplace_back(xPos, yPos, zPos);
            normals.emplace_back(glm::normalize(glm::vec3(xPos, yPos, zPos)));
        }
    }

    for (int y = 0; y < segments; ++y) {
        for (int x = 0; x < segments; ++x) {
            indices.push_back(y * (segments + 1) + x);
            indices.push_back((y + 1) * (segments + 1) + x);
            indices.push_back(y * (segments + 1) + x + 1);

            indices.push_back(y * (segments + 1) + x + 1);
            indices.push_back((y + 1) * (segments + 1) + x);
            indices.push_back((y + 1) * (segments + 1) + x + 1);
        }
    }
}


void generateTorus(float outerRadius, float innerRadius, int segments, int rings,
                   std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<unsigned int>& indices) {
    for (int ring = 0; ring <= rings; ++ring) {
        float theta = (float)ring / rings * 2.0f * M_PI;
        float cosTheta = cos(theta);
        float sinTheta = sin(theta);

        for (int segment = 0; segment <= segments; ++segment) {
            float phi = (float)segment / segments * 2.0f * M_PI;
            float cosPhi = cos(phi);
            float sinPhi = sin(phi);

            float x = (outerRadius + innerRadius * cosPhi) * cosTheta;
            float y = innerRadius * sinPhi;
            float z = (outerRadius + innerRadius * cosPhi) * sinTheta;

            vertices.emplace_back(x, y, z);

            float nx = cosPhi * cosTheta;
            float ny = sinPhi;
            float nz = cosPhi * sinTheta;
            normals.emplace_back(nx, ny, nz);
        }
    }

    for (int ring = 0; ring < rings; ++ring) {
        for (int segment = 0; segment < segments; ++segment) {
            int first = ring * (segments + 1) + segment;
            int second = (ring + 1) * (segments + 1) + segment;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(first + 1);
            indices.push_back(second);
            indices.push_back(second + 1);
        }
    }
}


void generateCone(float height, float radius, std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals,
                  std::vector<unsigned int>& indices, glm::vec3 basePosition, glm::vec3 direction) {
    int segments = 16;
    glm::vec3 tip = basePosition + direction * height;

    int baseIndex = vertices.size();
    for (int i = 0; i < segments; ++i) {
        float angle = i * 2.0f * M_PI / segments;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        vertices.emplace_back(basePosition.x + x, basePosition.y, basePosition.z + z);
        normals.emplace_back(glm::normalize(glm::vec3(x, height, z)));
    }

    vertices.push_back(tip); 
    normals.push_back(glm::normalize(direction));

    for (int i = 0; i < segments; ++i) {
        indices.push_back(baseIndex + i);
        indices.push_back(baseIndex + (i + 1) % segments);
        indices.push_back(vertices.size() - 1); 
    }
}


void generatePineapple(std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<unsigned int>& indices) {
    generateSphere(0.6f, 32, vertices, normals, indices); 

    int numLeaves = 12; 
    float leafHeight = 0.4f;
    float leafRadius = 0.1f;
    glm::vec3 basePosition(0.0f, 0.6f, 0.0f); 

    for (int i = 0; i < numLeaves; ++i) {
        float angle = i * 2.0f * M_PI / numLeaves;
        glm::vec3 direction(cos(angle), 0.6f, sin(angle));
        glm::vec3 position = basePosition + 0.2f * direction;

        generateCone(leafHeight, leafRadius, vertices, normals, indices, position, glm::normalize(direction));
    }
}



void initializeBuffers() {
    std::vector<glm::vec3> vertices, normals;
    std::vector<unsigned int> indices;
    generateSphere(1.0f, 32, vertices, normals, indices);
    indexCount = indices.size();

    glGenVertexArrays(1, &vaoSphere);
    glGenBuffers(1, &vboSphere);
    glGenBuffers(1, &nboSphere);
    glGenBuffers(1, &eboSphere);

    glBindVertexArray(vaoSphere);

    glBindBuffer(GL_ARRAY_BUFFER, vboSphere);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, nboSphere);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboSphere);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    glBindVertexArray(0);
}



void initializeTorusBuffers() {
    std::vector<glm::vec3> vertices, normals;
    std::vector<unsigned int> indices;
    generateTorus(1.0f, 0.4f, 32, 32, vertices, normals, indices);
    torusIndexCount = indices.size();

    glGenVertexArrays(1, &vaoTorus);
    glGenBuffers(1, &vboTorus);
    glGenBuffers(1, &nboTorus);
    glGenBuffers(1, &eboTorus);

    glBindVertexArray(vaoTorus);

    glBindBuffer(GL_ARRAY_BUFFER, vboTorus);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, nboTorus);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboTorus);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void initializePineappleBuffers() {
    std::vector<glm::vec3> vertices, normals;
    std::vector<unsigned int> indices;

    generatePineapple(vertices, normals, indices);
    pineappleIndexCount = indices.size();

    glGenVertexArrays(1, &vaoPineapple);
    glGenBuffers(1, &vboPineapple);
    glGenBuffers(1, &nboPineapple);
    glGenBuffers(1, &eboPineapple);

    glBindVertexArray(vaoPineapple);

    glBindBuffer(GL_ARRAY_BUFFER, vboPineapple);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, nboPineapple);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboPineapple);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    glBindVertexArray(0);
}



void drawPineapple() {
    glUseProgram(celShaderProgram);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-2.0f, 0.0f, -2.0f));
    model = glm::rotate(model, glm::radians(rotationAnglePineapple), glm::vec3(0.0f, 1.0f, 0.0f));
    GLuint modelLoc = glGetUniformLocation(celShaderProgram, "model");
    GLuint viewLoc = glGetUniformLocation(celShaderProgram, "view");
    GLuint projLoc = glGetUniformLocation(celShaderProgram, "projection");
    GLuint colorLoc = glGetUniformLocation(celShaderProgram, "objectColor");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glUniform3f(colorLoc, 1.0f, 0.8f, 0.2f); 
    glBindVertexArray(vaoPineapple);
    glDrawElements(GL_TRIANGLES, pineappleIndexCount / 2, GL_UNSIGNED_INT, 0); 

    glUniform3f(colorLoc, 0.0f, 0.6f, 0.0f); 
    glDrawElements(GL_TRIANGLES, pineappleIndexCount / 2, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * pineappleIndexCount / 2));
    glBindVertexArray(0);

    glUseProgram(outlineShaderProgram);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 
    glBindVertexArray(vaoPineapple);
    glDrawElements(GL_TRIANGLES, pineappleIndexCount, GL_UNSIGNED_INT, 0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); 
    glBindVertexArray(0);
}



void drawSphere() {
    glUseProgram(celShaderProgram);

   
    model = glm::mat4(1.0f); 
    model = glm::rotate(model, glm::radians(rotationAngleSphere), glm::vec3(0.0f, 1.0f, 0.0f));

    GLuint modelLoc = glGetUniformLocation(celShaderProgram, "model");
    GLuint viewLoc = glGetUniformLocation(celShaderProgram, "view");
    GLuint projLoc = glGetUniformLocation(celShaderProgram, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

  
    GLuint colorLoc = glGetUniformLocation(celShaderProgram, "objectColor");
    glUniform3f(colorLoc, 1.0f, 0.5f, 0.3f); 

    
    glBindVertexArray(vaoSphere);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glUseProgram(outlineShaderProgram);

    model = glm::scale(glm::mat4(1.0f), glm::vec3(1.05f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(vaoSphere);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); 
    glBindVertexArray(0);
}




void drawTorus() {
    glUseProgram(celShaderProgram);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotationAngleTorus), glm::vec3(0.0f, 1.0f, 0.0f));
    GLuint modelLoc = glGetUniformLocation(celShaderProgram, "model");
    GLuint viewLoc = glGetUniformLocation(celShaderProgram, "view");
    GLuint projLoc = glGetUniformLocation(celShaderProgram, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    GLuint colorLoc = glGetUniformLocation(celShaderProgram, "objectColor");
    glUniform3f(colorLoc, 0.0f, 1.0f, 0.0f); 

    glBindVertexArray(vaoTorus);
    glDrawElements(GL_TRIANGLES, torusIndexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}





void toggleFullScreen() {
    if (isFullScreen) {
        glutReshapeWindow(windowWidth, windowHeight);
        isFullScreen = false;
    } else {
        glutFullScreen();
        isFullScreen = true;
    }
}


void handleKeyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'w': 
            cameraZ -= cameraSpeed;
            break;
        case 's': 
            cameraZ += cameraSpeed;
            break;
        case 'a': 
            cameraX -= cameraSpeed;
            break;
        case 'd': 
            cameraX += cameraSpeed;
            break;
        case 'f': 
            toggleFullScreen();
            break;
        case 27: 
            exit(0);
            break;
    }
    glutPostRedisplay();
}



void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    view = glm::lookAt(glm::vec3(cameraX, cameraY, cameraZ), glm::vec3(lookAtX, lookAtY, lookAtZ), glm::vec3(0.0f, 1.0f, 0.0f));
    projection = glm::perspective(glm::radians(45.0f), (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);
    drawSphere();
    drawTorus();
    drawPineapple();

    glutSwapBuffers();
}


void reshape(int width, int height) {
    if (!isFullScreen) {
        windowWidth = width;
        windowHeight = height;
    }
    glViewport(0, 0, width, height);
}



void init() {
    glewInit();
    glEnable(GL_DEPTH_TEST); 

    celShaderProgram = compileShader(vertexShaderSource, celFragmentShaderSource);
    outlineShaderProgram = compileShader(vertexShaderSource, outlineFragmentShaderSource);

    initializeBuffers();
    initializeTorusBuffers();
    initializePineappleBuffers();
    glClearColor(0.9f, 0.9f, 0.6f, 1.0f);
}


int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Toon Shading using Opengl- Sphere, Torus, pineapple");

    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(handleKeyboard); 
    glutReshapeFunc(reshape);   
    glutTimerFunc(16, timer, 0); 
	
    glutMainLoop();
    return 0;
}

