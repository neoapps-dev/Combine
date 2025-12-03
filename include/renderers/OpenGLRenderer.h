/*
   Copyright 2025 NEOAPPS

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef OPENGL_RENDERER_H
#define OPENGL_RENDERER_H
#include "CombineEngine.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <unordered_map>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
namespace Combine {
struct Texture {
    GLuint id = 0;
    int width = 0;
    int height = 0;
    int channels = 0;
    std::string path;

    Texture() = default;
    Texture(const std::string& filepath) : path(filepath) {}
};

struct MeshBuffers {
    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;
    GLuint textureId = 0;
    size_t vertexCount = 0;
    size_t indexCount = 0;
};

class OpenGLRenderer : public IRenderer {
private:
    GLFWwindow* window = nullptr;
    GLuint shaderProgram = 0;
    int windowWidth = 0;
    int windowHeight = 0;
    glm::mat4 projection;
    glm::mat4 view;
    bool wireframeMode = false;
    bool vsyncEnabled = true;
    std::unordered_map<unsigned int, MeshBuffers> meshBufferCache;
    std::unordered_map<std::string, Texture> textureCache;
    std::unordered_map<std::string, GLuint> shaderPrograms;
    unsigned int nextMeshId = 1;
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        layout (location = 2) in vec2 aTexCoord;
        layout (location = 3) in vec4 aColor;
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        uniform mat3 normalMatrix;
        uniform bool uHasTexture;
        uniform sampler2D uTextureSampler;
        out vec3 FragPos;
        out vec3 Normal;
        out vec2 TexCoord;
        out vec4 VertexColor;
        void main() {
            FragPos = vec3(model * vec4(aPos, 1.0));
            Normal = normalMatrix * aNormal;
            TexCoord = aTexCoord;
            VertexColor = aColor;
            gl_Position = projection * view * vec4(FragPos, 1.0);
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        in vec3 FragPos;
        in vec3 Normal;
        in vec2 TexCoord;
        in vec4 VertexColor;
        out vec4 FragColor;
        uniform vec4 meshColor;
        uniform vec3 viewPos;
        uniform vec4 ambientColor;
        uniform bool uHasTexture;
        uniform sampler2D uTextureSampler;
        struct Light {
            int type;
            vec3 position;
            vec3 direction;
            vec4 color;
            float intensity;
            float range;
            float spotAngle;
        };

        #define MAX_LIGHTS 8
        uniform Light lights[MAX_LIGHTS];
        uniform int numLights;
        void main() {
            vec3 norm = normalize(Normal);
            vec3 viewDir = normalize(viewPos - FragPos);
            vec3 ambient = ambientColor.rgb * ambientColor.a;
            vec3 result = ambient;
            for (int i = 0; i < numLights && i < MAX_LIGHTS; i++) {
                vec3 lightDir;
                float attenuation = 1.0;
                if (lights[i].type == 0) {
                    lightDir = normalize(-lights[i].direction);
                } else if (lights[i].type == 1) {
                    vec3 toLight = lights[i].position - FragPos;
                    float dist = length(toLight);
                    lightDir = normalize(toLight);
                    attenuation = clamp(1.0 - dist / lights[i].range, 0.0, 1.0);
                    attenuation *= attenuation;
                } else {
                    vec3 toLight = lights[i].position - FragPos;
                    float dist = length(toLight);
                    lightDir = normalize(toLight);
                    float theta = dot(lightDir, normalize(-lights[i].direction));
                    float cutoff = cos(radians(lights[i].spotAngle));
                    if (theta > cutoff) {
                        attenuation = clamp(1.0 - dist / lights[i].range, 0.0, 1.0);
                        attenuation *= attenuation;
                        attenuation *= (theta - cutoff) / (1.0 - cutoff);
                    } else {
                        attenuation = 0.0;
                    }
                }

                float diff = max(dot(norm, lightDir), 0.0);
                vec3 diffuse = diff * lights[i].color.rgb * lights[i].intensity;
                vec3 halfwayDir = normalize(lightDir + viewDir);
                float spec = pow(max(dot(norm, halfwayDir), 0.0), 64.0);
                vec3 specular = spec * lights[i].color.rgb * lights[i].intensity * 0.8;
                vec3 ambient = lights[i].color.rgb * 0.05;

                result += (ambient + diffuse + specular) * attenuation;
            }

            if (numLights == 0) {
                result = vec3(1.0);
            }

            vec4 baseColor = meshColor * VertexColor;
            if (uHasTexture) {
                vec4 texColor = texture(uTextureSampler, TexCoord);
                baseColor = baseColor * texColor;
            }

            FragColor = vec4(result * baseColor.rgb, baseColor.a);
        }
    )";

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        (void)window; (void)scancode; (void)mods;
        Input::instance().setKeyState(key, action != GLFW_RELEASE);
    }

    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        (void)window; (void)mods;
        Input::instance().setMouseButton(button, action != GLFW_RELEASE);
    }

    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
        (void)window;
        Input::instance().setMousePosition(static_cast<float>(xpos), static_cast<float>(ypos));
    }

    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
        (void)window;
        Input::instance().setScrollDelta(static_cast<float>(xoffset), static_cast<float>(yoffset));
    }

    GLuint compileShader(GLenum type, const char* source) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);
        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            std::cerr << "Shader compilation error: " << infoLog << std::endl;
        }

        return shader;
    }

    void createMeshBuffers(Mesh* mesh) {
        if (mesh->renderId == 0) {
            mesh->renderId = nextMeshId++;
        }

        auto it = meshBufferCache.find(mesh->renderId);
        if (it != meshBufferCache.end()) {
            glDeleteVertexArrays(1, &it->second.VAO);
            glDeleteBuffers(1, &it->second.VBO);
            glDeleteBuffers(1, &it->second.EBO);
        }

        MeshBuffers buffers;
        glGenVertexArrays(1, &buffers.VAO);
        glGenBuffers(1, &buffers.VBO);
        glGenBuffers(1, &buffers.EBO);
        glBindVertexArray(buffers.VAO);
        std::vector<float> vertexData;
        vertexData.reserve(mesh->vertices.size() * 12);
        for (const auto& v : mesh->vertices) {
            vertexData.push_back(v.position.x);
            vertexData.push_back(v.position.y);
            vertexData.push_back(v.position.z);
            vertexData.push_back(v.normal.x);
            vertexData.push_back(v.normal.y);
            vertexData.push_back(v.normal.z);
            vertexData.push_back(v.texCoord.x);
            vertexData.push_back(v.texCoord.y);
            vertexData.push_back(v.color.r);
            vertexData.push_back(v.color.g);
            vertexData.push_back(v.color.b);
            vertexData.push_back(v.color.a);
        }

        glBindBuffer(GL_ARRAY_BUFFER, buffers.VBO);
        glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);
        if (!mesh->indices.empty()) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers.EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices.size() * sizeof(unsigned int), mesh->indices.data(), GL_STATIC_DRAW);
        }

        size_t stride = 12 * sizeof(float);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, stride, (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glBindVertexArray(0);
        buffers.vertexCount = mesh->vertices.size();
        buffers.indexCount = mesh->indices.size();
        if (!mesh->texturePath.empty()) {
            buffers.textureId = loadTexture(mesh->texturePath);
        }

        meshBufferCache[mesh->renderId] = buffers;
        mesh->dirty = false;
    }

    GLuint loadTexture(const std::string& filepath) {
        auto it = textureCache.find(filepath);
        if (it != textureCache.end()) {
            return it->second.id;
        }

        Texture texture(filepath);
        int width, height, channels;
        unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 4);
        if (!data) {
            std::cerr << "Failed to load texture: " << filepath << std::endl;
            unsigned char whitePixel[] = {255, 255, 255, 255};
            glGenTextures(1, &texture.id);
            glBindTexture(GL_TEXTURE_2D, texture.id);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
        } else {
            texture.width = width;
            texture.height = height;
            texture.channels = 4;
            glGenTextures(1, &texture.id);
            glBindTexture(GL_TEXTURE_2D, texture.id);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        if (data) {
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        glBindTexture(GL_TEXTURE_2D, 0);

        textureCache[filepath] = texture;
        return texture.id;
    }

public:
    bool initialize(int width, int height, const std::string& title) override {
        windowWidth = width;
        windowHeight = height;
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return false;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SAMPLES, 4);
        window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if (!window) {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(window);
        glfwSwapInterval(vsyncEnabled ? 1 : 0);
        glfwSetKeyCallback(window, keyCallback);
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
        glfwSetCursorPosCallback(window, cursorPosCallback);
        glfwSetScrollCallback(window, scrollCallback);
        if (glewInit() != GLEW_OK) {
            std::cerr << "Failed to initialize GLEW" << std::endl;
            return false;
        }

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_MULTISAMPLE);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
        GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        GLint success;
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
            std::cerr << "Shader linking error: " << infoLog << std::endl;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        projection = glm::perspective(glm::radians(60.0f), (float)width / (float)height, 0.1f, 1000.0f);
        return true;
    }

    void beginFrame(const Camera& camera) override {
        glfwPollEvents();
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        if (w != windowWidth || h != windowHeight) {
            windowWidth = w;
            windowHeight = h;
            glViewport(0, 0, w, h);
        }

        projection = glm::perspective(glm::radians(camera.fov), (float)windowWidth / (float)windowHeight, camera.nearPlane, camera.farPlane);
        view = glm::mat4(1.0f);
        view = glm::rotate(view, glm::radians(camera.rotation.x), glm::vec3(1, 0, 0));
        view = glm::rotate(view, glm::radians(camera.rotation.y), glm::vec3(0, 1, 0));
        view = glm::rotate(view, glm::radians(camera.rotation.z), glm::vec3(0, 0, 1));
        view = glm::translate(view, glm::vec3(-camera.position.x, -camera.position.y, -camera.position.z));
        glClearColor(camera.clearColor.r, camera.clearColor.g, camera.clearColor.b, camera.clearColor.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if (wireframeMode) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        glUseProgram(shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), camera.position.x, camera.position.y, camera.position.z);
        glUniform1f(glGetUniformLocation(shaderProgram, "time"), static_cast<float>(glfwGetTime()));
    }

    void renderMesh(Mesh* mesh, const std::vector<Light>& lights, const Color& ambient) override {
        if (mesh->vertices.empty()) return;
        if (mesh->dirty || meshBufferCache.find(mesh->renderId) == meshBufferCache.end()) {
            createMeshBuffers(mesh);
        }

        auto& buffers = meshBufferCache[mesh->renderId];
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(mesh->transform.position.x, mesh->transform.position.y, mesh->transform.position.z));
        model = glm::rotate(model, glm::radians(mesh->transform.rotation.x), glm::vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(mesh->transform.rotation.y), glm::vec3(0, 1, 0));
        model = glm::rotate(model, glm::radians(mesh->transform.rotation.z), glm::vec3(0, 0, 1));
        model = glm::scale(model, glm::vec3(mesh->transform.scale.x, mesh->transform.scale.y, mesh->transform.scale.z));
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix3fv(glGetUniformLocation(shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
        glUniform4f(glGetUniformLocation(shaderProgram, "meshColor"), mesh->color.r, mesh->color.g, mesh->color.b, mesh->color.a);
        glUniform4f(glGetUniformLocation(shaderProgram, "ambientColor"), ambient.r, ambient.g, ambient.b, ambient.a);
        bool hasTexture = false;
        if (buffers.textureId != 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, buffers.textureId);
            glUniform1i(glGetUniformLocation(shaderProgram, "uTextureSampler"), 0);
            hasTexture = true;
        }
        glUniform1i(glGetUniformLocation(shaderProgram, "uHasTexture"), hasTexture ? 1 : 0);
        int numLights = std::min(static_cast<int>(lights.size()), 8);
        glUniform1i(glGetUniformLocation(shaderProgram, "numLights"), numLights);
        for (int i = 0; i < numLights; i++) {
            std::string prefix = "lights[" + std::to_string(i) + "].";
            glUniform1i(glGetUniformLocation(shaderProgram, (prefix + "type").c_str()), static_cast<int>(lights[i].type));
            glUniform3f(glGetUniformLocation(shaderProgram, (prefix + "position").c_str()), lights[i].position.x, lights[i].position.y, lights[i].position.z);
            glUniform3f(glGetUniformLocation(shaderProgram, (prefix + "direction").c_str()), lights[i].direction.x, lights[i].direction.y, lights[i].direction.z);
            glUniform4f(glGetUniformLocation(shaderProgram, (prefix + "color").c_str()), lights[i].color.r, lights[i].color.g, lights[i].color.b, lights[i].color.a);
            glUniform1f(glGetUniformLocation(shaderProgram, (prefix + "intensity").c_str()), lights[i].intensity);
            glUniform1f(glGetUniformLocation(shaderProgram, (prefix + "range").c_str()), lights[i].range);
            glUniform1f(glGetUniformLocation(shaderProgram, (prefix + "spotAngle").c_str()), lights[i].spotAngle);
        }

        glBindVertexArray(buffers.VAO);
        if (buffers.indexCount > 0) {
            glDrawElements(GL_TRIANGLES, buffers.indexCount, GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, buffers.vertexCount);
        }

        glBindVertexArray(0);
    }

    void endFrame() override {
        glfwSwapBuffers(window);
    }

    bool shouldClose() override {
        return glfwWindowShouldClose(window);
    }

    int getWidth() const override { return windowWidth; }
    int getHeight() const override { return windowHeight; }
    void setVSync(bool enabled) override {
        vsyncEnabled = enabled;
        glfwSwapInterval(enabled ? 1 : 0);
    }

    void setWireframe(bool enabled) override {
        wireframeMode = enabled;
    }
    
    bool loadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath) override {
        std::ifstream vFile(vertexPath);
        std::ifstream fFile(fragmentPath);
        
        if (!vFile.is_open() || !fFile.is_open()) {
            std::cerr << "Failed to open shader files: " << vertexPath << ", " << fragmentPath << std::endl;
            return false;
        }
        
        std::stringstream vStream, fStream;
        vStream << vFile.rdbuf();
        fStream << fFile.rdbuf();
        
        std::string vertexSource = vStream.str();
        std::string fragmentSource = fStream.str();
        
        vFile.close();
        fFile.close();
        
        GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource.c_str());
        GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource.c_str());
        
        if (!vertexShader || !fragmentShader) {
            if (vertexShader) glDeleteShader(vertexShader);
            if (fragmentShader) glDeleteShader(fragmentShader);
            return false;
        }
        
        GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);
        
        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(program, 512, nullptr, infoLog);
            std::cerr << "Shader linking error: " << infoLog << std::endl;
            glDeleteProgram(program);
            return false;
        }
        
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        
        shaderPrograms[name] = program;
        return true;
    }
    
    void useShader(const std::string& name) override {
        auto it = shaderPrograms.find(name);
        if (it != shaderPrograms.end()) {
            glUseProgram(it->second);
            glUniform1f(glGetUniformLocation(it->second, "time"), static_cast<float>(glfwGetTime()));
        } else {
            glUseProgram(shaderProgram);
            glUniform1f(glGetUniformLocation(shaderProgram, "time"), static_cast<float>(glfwGetTime()));
        }
    }

    void shutdown() override {
        for (auto& [id, buffers] : meshBufferCache) {
            glDeleteVertexArrays(1, &buffers.VAO);
            glDeleteBuffers(1, &buffers.VBO);
            glDeleteBuffers(1, &buffers.EBO);
        }
        meshBufferCache.clear();
        for (auto& [path, texture] : textureCache) {
            glDeleteTextures(1, &texture.id);
        }
        textureCache.clear();
        glDeleteProgram(shaderProgram);
        glfwDestroyWindow(window);
        glfwTerminate();
    }
};

}

#endif
