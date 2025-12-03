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

#ifndef COMBINE_ENGINE_H
#define COMBINE_ENGINE_H

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <functional>
#include <typeindex>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iostream>

namespace Combine {

struct Vector2 {
    float x, y;
    Vector2(float x = 0, float y = 0) : x(x), y(y) {}
    Vector2 operator+(const Vector2& other) const { return Vector2(x + other.x, y + other.y); }
    Vector2 operator-(const Vector2& other) const { return Vector2(x - other.x, y - other.y); }
    Vector2 operator*(float s) const { return Vector2(x * s, y * s); }
    Vector2& operator+=(const Vector2& other) { x += other.x; y += other.y; return *this; }
    Vector2& operator-=(const Vector2& other) { x -= other.x; y -= other.y; return *this; }
    float length() const { return std::sqrt(x * x + y * y); }
    Vector2 normalized() const { float l = length(); return l > 0 ? Vector2(x / l, y / l) : Vector2(); }
};

struct Vector3 {
    float x, y, z;
    Vector3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
    Vector3 operator+(const Vector3& other) const { return Vector3(x + other.x, y + other.y, z + other.z); }
    Vector3 operator-(const Vector3& other) const { return Vector3(x - other.x, y - other.y, z - other.z); }
    Vector3 operator*(float s) const { return Vector3(x * s, y * s, z * s); }
    Vector3& operator+=(const Vector3& other) { x += other.x; y += other.y; z += other.z; return *this; }
    Vector3& operator-=(const Vector3& other) { x -= other.x; y -= other.y; z -= other.z; return *this; }
    float length() const { return std::sqrt(x * x + y * y + z * z); }
    Vector3 normalized() const { float l = length(); return l > 0 ? Vector3(x / l, y / l, z / l) : Vector3(); }
    static Vector3 cross(const Vector3& a, const Vector3& b) {
        return Vector3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
    }
    static float dot(const Vector3& a, const Vector3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
};

struct Vector4 {
    float x, y, z, w;
    Vector4(float x = 0, float y = 0, float z = 0, float w = 1) : x(x), y(y), z(z), w(w) {}
};

struct Color {
    float r, g, b, a;
    Color(float r = 1, float g = 1, float b = 1, float a = 1) : r(r), g(g), b(b), a(a) {}
    static Color white() { return Color(1, 1, 1, 1); }
    static Color black() { return Color(0, 0, 0, 1); }
    static Color red() { return Color(1, 0, 0, 1); }
    static Color green() { return Color(0, 1, 0, 1); }
    static Color blue() { return Color(0, 0, 1, 1); }
    static Color yellow() { return Color(1, 1, 0, 1); }
    static Color cyan() { return Color(0, 1, 1, 1); }
    static Color magenta() { return Color(1, 0, 1, 1); }
};

struct Transform {
    Vector3 position;
    Vector3 rotation;
    Vector3 scale;
    Transform() : scale(1, 1, 1) {}
    
    void translate(const Vector3& delta) { position += delta; }
    void rotate(const Vector3& delta) { rotation += delta; }
};

enum class KeyCode {
    Unknown = -1,
    Space = 32,
    Apostrophe = 39,
    Comma = 44,
    Minus = 45,
    Period = 46,
    Slash = 47,
    Num0 = 48, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    Semicolon = 59,
    Equal = 61,
    A = 65, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    LeftBracket = 91,
    Backslash = 92,
    RightBracket = 93,
    GraveAccent = 96,
    Escape = 256,
    Enter = 257,
    Tab = 258,
    Backspace = 259,
    Insert = 260,
    Delete = 261,
    Right = 262,
    Left = 263,
    Down = 264,
    Up = 265,
    PageUp = 266,
    PageDown = 267,
    Home = 268,
    End = 269,
    CapsLock = 280,
    ScrollLock = 281,
    NumLock = 282,
    PrintScreen = 283,
    Pause = 284,
    F1 = 290, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    LeftShift = 340,
    LeftControl = 341,
    LeftAlt = 342,
    LeftSuper = 343,
    RightShift = 344,
    RightControl = 345,
    RightAlt = 346,
    RightSuper = 347,
    Menu = 348
};

enum class MouseButton {
    Left = 0,
    Right = 1,
    Middle = 2,
    Button4 = 3,
    Button5 = 4,
    Button6 = 5,
    Button7 = 6,
    Button8 = 7
};

class Input {
public:
    static Input& instance() {
        static Input inst;
        return inst;
    }
    
    void setKeyState(int key, bool pressed) {
        prevKeyStates[key] = keyStates[key];
        keyStates[key] = pressed;
    }
    
    void setMouseButton(int button, bool pressed) {
        prevMouseStates[button] = mouseStates[button];
        mouseStates[button] = pressed;
    }
    
    void setMousePosition(float x, float y) {
        mouseDelta = Vector2(x - mousePosition.x, y - mousePosition.y);
        mousePosition = Vector2(x, y);
    }
    
    void setScrollDelta(float x, float y) {
        scrollDelta = Vector2(x, y);
    }
    
    void update() {
        prevKeyStates = keyStates;
        prevMouseStates = mouseStates;
        mouseDelta = Vector2(0, 0);
        scrollDelta = Vector2(0, 0);
    }
    
    bool isKeyDown(KeyCode key) const {
        auto it = keyStates.find(static_cast<int>(key));
        return it != keyStates.end() && it->second;
    }
    
    bool isKeyPressed(KeyCode key) const {
        int k = static_cast<int>(key);
        auto curr = keyStates.find(k);
        auto prev = prevKeyStates.find(k);
        bool currState = curr != keyStates.end() && curr->second;
        bool prevState = prev != prevKeyStates.end() && prev->second;
        return currState && !prevState;
    }
    
    bool isKeyReleased(KeyCode key) const {
        int k = static_cast<int>(key);
        auto curr = keyStates.find(k);
        auto prev = prevKeyStates.find(k);
        bool currState = curr != keyStates.end() && curr->second;
        bool prevState = prev != prevKeyStates.end() && prev->second;
        return !currState && prevState;
    }
    
    bool isMouseButtonDown(MouseButton button) const {
        auto it = mouseStates.find(static_cast<int>(button));
        return it != mouseStates.end() && it->second;
    }
    
    bool isMouseButtonPressed(MouseButton button) const {
        int b = static_cast<int>(button);
        auto curr = mouseStates.find(b);
        auto prev = prevMouseStates.find(b);
        bool currState = curr != mouseStates.end() && curr->second;
        bool prevState = prev != prevMouseStates.end() && prev->second;
        return currState && !prevState;
    }
    
    bool isMouseButtonReleased(MouseButton button) const {
        int b = static_cast<int>(button);
        auto curr = mouseStates.find(b);
        auto prev = prevMouseStates.find(b);
        bool currState = curr != mouseStates.end() && curr->second;
        bool prevState = prev != prevMouseStates.end() && prev->second;
        return !currState && prevState;
    }
    
    Vector2 getMousePosition() const { return mousePosition; }
    Vector2 getMouseDelta() const { return mouseDelta; }
    Vector2 getScrollDelta() const { return scrollDelta; }

private:
    Input() = default;
    std::map<int, bool> keyStates;
    std::map<int, bool> prevKeyStates;
    std::map<int, bool> mouseStates;
    std::map<int, bool> prevMouseStates;
    Vector2 mousePosition;
    Vector2 mouseDelta;
    Vector2 scrollDelta;
};

class Entity;

class Component {
public:
    Entity* entity = nullptr;
    bool enabled = true;
    
    virtual ~Component() = default;
    virtual void onAttach() {}
    virtual void onDetach() {}
    virtual void onUpdate(float deltaTime) { (void)deltaTime; }
    virtual void onLateUpdate(float deltaTime) { (void)deltaTime; }
};

class Entity : public std::enable_shared_from_this<Entity> {
public:
    Transform transform;
    std::string name;
    std::string tag;
    bool active = true;
    
    Entity(const std::string& name = "Entity") : name(name) {}
    virtual ~Entity() {
        for (auto& [type, comp] : components) {
            comp->onDetach();
        }
    }
    
    template<typename T, typename... Args>
    std::shared_ptr<T> addComponent(Args&&... args) {
        auto comp = std::make_shared<T>(std::forward<Args>(args)...);
        comp->entity = this;
        components[std::type_index(typeid(T))] = comp;
        comp->onAttach();
        return comp;
    }
    
    template<typename T>
    std::shared_ptr<T> getComponent() {
        auto it = components.find(std::type_index(typeid(T)));
        if (it != components.end()) {
            return std::static_pointer_cast<T>(it->second);
        }
        return nullptr;
    }
    
    template<typename T>
    bool hasComponent() const {
        return components.find(std::type_index(typeid(T))) != components.end();
    }
    
    template<typename T>
    void removeComponent() {
        auto it = components.find(std::type_index(typeid(T)));
        if (it != components.end()) {
            it->second->onDetach();
            components.erase(it);
        }
    }
    
    void updateComponents(float deltaTime) {
        for (auto& [type, comp] : components) {
            if (comp->enabled) {
                comp->onUpdate(deltaTime);
            }
        }
    }
    
    void lateUpdateComponents(float deltaTime) {
        for (auto& [type, comp] : components) {
            if (comp->enabled) {
                comp->onLateUpdate(deltaTime);
            }
        }
    }

private:
    std::unordered_map<std::type_index, std::shared_ptr<Component>> components;
};

class Shader {
public:
    std::string name;
    std::string vertexPath;
    std::string fragmentPath;
    std::string vertexSource;
    std::string fragmentSource;
    bool loaded = false;
    
    Shader() = default;
    Shader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath) 
        : name(name), vertexPath(vertexPath), fragmentPath(fragmentPath) {}
    
    bool loadFromFile() {
        std::ifstream vFile(vertexPath);
        std::ifstream fFile(fragmentPath);
        
        if (!vFile.is_open() || !fFile.is_open()) {
            std::cerr << "Failed to load shader files: " << vertexPath << ", " << fragmentPath << std::endl;
            return false;
        }
        
        std::stringstream vStream, fStream;
        vStream << vFile.rdbuf();
        fStream << fFile.rdbuf();
        
        vertexSource = vStream.str();
        fragmentSource = fStream.str();
        
        vFile.close();
        fFile.close();
        
        loaded = true;
        return true;
    }
    
    static std::shared_ptr<Shader> create(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath) {
        auto shader = std::make_shared<Shader>(name, vertexPath, fragmentPath);
        return shader;
    }
};

struct Vertex {
    Vector3 position;
    Vector3 normal;
    Vector2 texCoord;
    Color color;
    
    Vertex() : color(Color::white()) {}
    Vertex(const Vector3& pos) : position(pos), color(Color::white()) {}
    Vertex(const Vector3& pos, const Vector3& norm) : position(pos), normal(norm), color(Color::white()) {}
    Vertex(const Vector3& pos, const Vector3& norm, const Vector2& uv) : position(pos), normal(norm), texCoord(uv), color(Color::white()) {}
};

class Mesh : public Entity {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    Color color;
    bool dirty = true;
    unsigned int renderId = 0;
    std::string texturePath;
    
    Mesh(const std::string& name = "Mesh") : Entity(name), color(Color::white()) {}
    
    void addVertex(const Vertex& v) {
        vertices.push_back(v);
        dirty = true;
    }
    
    void addVertex(float x, float y, float z) {
        vertices.push_back(Vertex(Vector3(x, y, z)));
        dirty = true;
    }
    
    void addVertex(const Vector3& pos, const Vector3& normal, const Vector2& uv) {
        vertices.push_back(Vertex(pos, normal, uv));
        dirty = true;
    }
    
    void addIndex(unsigned int idx) {
        indices.push_back(idx);
        dirty = true;
    }
    
    void addTriangle(unsigned int i0, unsigned int i1, unsigned int i2) {
        indices.push_back(i0);
        indices.push_back(i1);
        indices.push_back(i2);
        dirty = true;
    }
    
    void clear() {
        vertices.clear();
        indices.clear();
        dirty = true;
    }
    
    void calculateNormals() {
        for (auto& v : vertices) {
            v.normal = Vector3(0, 0, 0);
        }
        
        for (size_t i = 0; i + 2 < indices.size(); i += 3) {
            unsigned int i0 = indices[i];
            unsigned int i1 = indices[i + 1];
            unsigned int i2 = indices[i + 2];
            
            Vector3 v0 = vertices[i0].position;
            Vector3 v1 = vertices[i1].position;
            Vector3 v2 = vertices[i2].position;
            
            Vector3 edge1 = v1 - v0;
            Vector3 edge2 = v2 - v0;
            Vector3 normal = Vector3::cross(edge1, edge2).normalized();
            
            vertices[i0].normal += normal;
            vertices[i1].normal += normal;
            vertices[i2].normal += normal;
        }
        
        for (auto& v : vertices) {
            v.normal = v.normal.normalized();
        }
        
        dirty = true;
    }
    
    static std::shared_ptr<Mesh> createCube(const std::string& name = "Cube") {
        auto mesh = std::make_shared<Mesh>(name);
        
        Vector3 positions[] = {
            {-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f},
            {-0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}
        };
        
        Vector3 normals[] = {
            {0, 0, -1}, {0, 0, 1}, {-1, 0, 0}, {1, 0, 0}, {0, -1, 0}, {0, 1, 0}
        };
        
        int faceIndices[][4] = {
            {0, 3, 2, 1}, {4, 5, 6, 7}, {0, 4, 7, 3}, {1, 2, 6, 5}, {0, 1, 5, 4}, {3, 7, 6, 2}
        };
        
        for (int face = 0; face < 6; face++) {
            unsigned int baseIdx = mesh->vertices.size();
            for (int i = 0; i < 4; i++) {
                Vertex v;
                v.position = positions[faceIndices[face][i]];
                v.normal = normals[face];
                v.texCoord = Vector2((i == 1 || i == 2) ? 1.0f : 0.0f, (i == 2 || i == 3) ? 1.0f : 0.0f);
                mesh->vertices.push_back(v);
            }
            mesh->addTriangle(baseIdx, baseIdx + 1, baseIdx + 2);
            mesh->addTriangle(baseIdx, baseIdx + 2, baseIdx + 3);
        }
        
        return mesh;
    }
    
    static std::shared_ptr<Mesh> createPlane(const std::string& name = "Plane", float width = 1.0f, float height = 1.0f) {
        auto mesh = std::make_shared<Mesh>(name);
        float hw = width * 0.5f;
        float hh = height * 0.5f;
        
        mesh->addVertex(Vertex(Vector3(-hw, 0, -hh), Vector3(0, 1, 0), Vector2(0, 0)));
        mesh->addVertex(Vertex(Vector3(hw, 0, -hh), Vector3(0, 1, 0), Vector2(1, 0)));
        mesh->addVertex(Vertex(Vector3(hw, 0, hh), Vector3(0, 1, 0), Vector2(1, 1)));
        mesh->addVertex(Vertex(Vector3(-hw, 0, hh), Vector3(0, 1, 0), Vector2(0, 1)));
        
        mesh->addTriangle(0, 1, 2);
        mesh->addTriangle(0, 2, 3);
        
        return mesh;
    }
    
    static std::shared_ptr<Mesh> createSphere(const std::string& name = "Sphere", int segments = 16, int rings = 16) {
        auto mesh = std::make_shared<Mesh>(name);
        
        for (int ring = 0; ring <= rings; ring++) {
            float phi = 3.14159265f * ring / rings;
            float y = std::cos(phi);
            float ringRadius = std::sin(phi);
            
            for (int seg = 0; seg <= segments; seg++) {
                float theta = 2.0f * 3.14159265f * seg / segments;
                float x = ringRadius * std::cos(theta);
                float z = ringRadius * std::sin(theta);
                
                Vector3 pos(x * 0.5f, y * 0.5f, z * 0.5f);
                Vector3 normal = pos.normalized();
                Vector2 uv((float)seg / segments, (float)ring / rings);
                
                mesh->addVertex(Vertex(pos, normal, uv));
            }
        }
        
        for (int ring = 0; ring < rings; ring++) {
            for (int seg = 0; seg < segments; seg++) {
                unsigned int curr = ring * (segments + 1) + seg;
                unsigned int next = curr + segments + 1;
                
                mesh->addTriangle(curr, next, curr + 1);
                mesh->addTriangle(curr + 1, next, next + 1);
            }
        }
        
        return mesh;
    }
};

class Camera {
public:
    Vector3 position;
    Vector3 rotation;
    float fov = 60.0f;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;
    Color clearColor;
    
    Camera() : position(0, 0, 3), clearColor(0.1f, 0.1f, 0.15f, 1.0f) {}
    
    Vector3 forward() const {
        float pitch = rotation.x * 3.14159265f / 180.0f;
        float yaw = rotation.y * 3.14159265f / 180.0f;
        return Vector3(
            std::sin(yaw) * std::cos(pitch),
            -std::sin(pitch),
            -std::cos(yaw) * std::cos(pitch)
        );
    }
    
    Vector3 right() const {
        float yaw = rotation.y * 3.14159265f / 180.0f;
        return Vector3(std::cos(yaw), 0, std::sin(yaw));
    }
    
    Vector3 up() const {
        return Vector3::cross(right(), forward()).normalized();
    }
};

struct Light {
    enum class Type { Directional, Point, Spot };
    
    Type type = Type::Directional;
    Vector3 position;
    Vector3 direction;
    Color color;
    float intensity = 1.0f;
    float range = 10.0f;
    float spotAngle = 45.0f;
    
    Light() : direction(0, -1, 0), color(Color::white()) {}
};

class Scene {
public:
    std::vector<std::shared_ptr<Entity>> entities;
    std::vector<Light> lights;
    std::vector<std::shared_ptr<Shader>> shaders;
    Camera camera;
    Color ambientColor;
    
    Scene() : ambientColor(0.2f, 0.2f, 0.2f, 1.0f) {}
    
    void addEntity(std::shared_ptr<Entity> entity) {
        entities.push_back(entity);
    }
    
    void removeEntity(std::shared_ptr<Entity> entity) {
        entities.erase(std::remove(entities.begin(), entities.end(), entity), entities.end());
    }
    
    void removeEntityByName(const std::string& name) {
        entities.erase(std::remove_if(entities.begin(), entities.end(),
            [&name](const std::shared_ptr<Entity>& e) { return e->name == name; }), entities.end());
    }
    
    std::shared_ptr<Entity> getEntityByName(const std::string& name) {
        for (auto& entity : entities) {
            if (entity->name == name) return entity;
        }
        return nullptr;
    }
    
    std::vector<std::shared_ptr<Entity>> getEntitiesByTag(const std::string& tag) {
        std::vector<std::shared_ptr<Entity>> result;
        for (auto& entity : entities) {
            if (entity->tag == tag) result.push_back(entity);
        }
        return result;
    }
    
    void addLight(const Light& light) {
        lights.push_back(light);
    }
    
    void addShader(std::shared_ptr<Shader> shader) {
        shaders.push_back(shader);
    }
    
    std::shared_ptr<Shader> getShaderByName(const std::string& name) {
        for (auto& shader : shaders) {
            if (shader->name == name) return shader;
        }
        return nullptr;
    }
    
    void update(float deltaTime) {
        for (auto& entity : entities) {
            if (entity->active) {
                entity->updateComponents(deltaTime);
            }
        }
    }
    
    void lateUpdate(float deltaTime) {
        for (auto& entity : entities) {
            if (entity->active) {
                entity->lateUpdateComponents(deltaTime);
            }
        }
    }
    
    void clear() {
        entities.clear();
        lights.clear();
    }
};

class IRenderer {
public:
    virtual ~IRenderer() = default;
    virtual bool initialize(int width, int height, const std::string& title) = 0;
    virtual void beginFrame(const Camera& camera) = 0;
    virtual void renderMesh(Mesh* mesh, const std::vector<Light>& lights, const Color& ambient) = 0;
    virtual void endFrame() = 0;
    virtual bool shouldClose() = 0;
    virtual void shutdown() = 0;
    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;
    virtual void setVSync(bool enabled) = 0;
    virtual void setWireframe(bool enabled) = 0;
    virtual bool loadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath = "") = 0;
    virtual void useShader(const std::string& name) = 0;
};

class IScriptEngine {
public:
    virtual ~IScriptEngine() = default;
    virtual bool initialize() = 0;
    virtual void registerAPI() = 0;
    virtual bool executeFile(const std::string& filename) = 0;
    virtual bool executeString(const std::string& code) = 0;
    virtual void update(float deltaTime) = 0;
    virtual void shutdown() = 0;
    virtual std::string getExtension() const = 0;
};

class Time {
public:
    static Time& instance() {
        static Time inst;
        return inst;
    }
    
    void update() {
        auto now = std::chrono::high_resolution_clock::now();
        deltaTime = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;
        totalTime += deltaTime;
        frameCount++;
        
        fpsTimer += deltaTime;
        fpsFrameCount++;
        if (fpsTimer >= 1.0f) {
            fps = fpsFrameCount / fpsTimer;
            fpsTimer = 0;
            fpsFrameCount = 0;
        }
    }
    
    float getDeltaTime() const { return deltaTime * timeScale; }
    float getUnscaledDeltaTime() const { return deltaTime; }
    float getTotalTime() const { return totalTime; }
    float getFPS() const { return fps; }
    unsigned long long getFrameCount() const { return frameCount; }
    
    void setTimeScale(float scale) { timeScale = scale; }
    float getTimeScale() const { return timeScale; }
    
    void reset() {
        lastTime = std::chrono::high_resolution_clock::now();
        deltaTime = 0;
        totalTime = 0;
        frameCount = 0;
    }

private:
    Time() { reset(); }
    std::chrono::high_resolution_clock::time_point lastTime;
    float deltaTime = 0;
    float totalTime = 0;
    float timeScale = 1.0f;
    unsigned long long frameCount = 0;
    float fps = 0;
    float fpsTimer = 0;
    int fpsFrameCount = 0;
};

class Engine {
public:
    using UpdateCallback = std::function<void(float)>;
    
    Engine() : running(false) {}
    
    void setRenderer(std::unique_ptr<IRenderer> r) { renderer = std::move(r); }
    
    void addScriptEngine(std::unique_ptr<IScriptEngine> se) {
        scriptEngines.push_back(std::move(se));
    }
    
    void setScriptEngine(std::unique_ptr<IScriptEngine> se) {
        scriptEngines.clear();
        scriptEngines.push_back(std::move(se));
    }
    
    bool initialize(int width, int height, const std::string& title) {
        if (!renderer || !renderer->initialize(width, height, title)) {
            return false;
        }
        
        scene = std::make_unique<Scene>();
        
        for (auto& se : scriptEngines) {
            if (!se->initialize()) {
                return false;
            }
            se->registerAPI();
        }
        
        Time::instance().reset();
        running = true;
        return true;
    }
    
    void run() {
        while (running && !renderer->shouldClose()) {
            Time::instance().update();
            Input::instance().update();
            
            float dt = Time::instance().getDeltaTime();
            
            scene->update(dt);
            
            for (auto& callback : updateCallbacks) {
                callback(dt);
            }
            
            for (auto& se : scriptEngines) {
                se->update(dt);
            }
            
            scene->lateUpdate(dt);
            
            for (auto& callback : lateUpdateCallbacks) {
                callback(dt);
            }
            
            renderer->beginFrame(scene->camera);
            
            for (auto& entity : scene->entities) {
                if (!entity->active) continue;
                if (auto mesh = dynamic_cast<Mesh*>(entity.get())) {
                    renderer->renderMesh(mesh, scene->lights, scene->ambientColor);
                }
            }
            
            renderer->endFrame();
        }
    }
    
    void shutdown() {
        for (auto& se : scriptEngines) {
            se->shutdown();
        }
        if (renderer) renderer->shutdown();
        running = false;
    }
    
    void stop() { running = false; }
    
    Scene* getScene() { return scene.get(); }
    IRenderer* getRenderer() { return renderer.get(); }
    
    IScriptEngine* getScriptEngine(const std::string& extension = "") {
        if (scriptEngines.empty()) return nullptr;
        if (extension.empty()) return scriptEngines[0].get();
        for (auto& se : scriptEngines) {
            if (se->getExtension() == extension) return se.get();
        }
        return nullptr;
    }
    
    IScriptEngine* getScriptEngineForFile(const std::string& filename) {
        size_t dotPos = filename.rfind('.');
        if (dotPos == std::string::npos) return nullptr;
        std::string ext = filename.substr(dotPos);
        return getScriptEngine(ext);
    }
    
    bool executeScript(const std::string& filename) {
        IScriptEngine* se = getScriptEngineForFile(filename);
        return se ? se->executeFile(filename) : false;
    }
    
    bool executeScriptString(const std::string& code, const std::string& extension = "") {
        IScriptEngine* se = getScriptEngine(extension);
        return se ? se->executeString(code) : false;
    }
    
    void onUpdate(UpdateCallback callback) {
        updateCallbacks.push_back(callback);
    }
    
    void onLateUpdate(UpdateCallback callback) {
        lateUpdateCallbacks.push_back(callback);
    }
    
    bool isRunning() const { return running; }

private:
    std::unique_ptr<IRenderer> renderer;
    std::vector<std::unique_ptr<IScriptEngine>> scriptEngines;
    std::unique_ptr<Scene> scene;
    std::vector<UpdateCallback> updateCallbacks;
    std::vector<UpdateCallback> lateUpdateCallbacks;
    bool running;
};

extern Engine* g_engine;

}

#endif
