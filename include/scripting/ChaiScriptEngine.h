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

#ifndef CHAISCRIPT_ENGINE_H
#define CHAISCRIPT_ENGINE_H

#include "../CombineEngine.h"
#include "../MapLoader.h"
#include <chaiscript/chaiscript.hpp>
#include <iostream>
#include <functional>

namespace Combine {

class ChaiScriptEngine : public IScriptEngine {
private:
    std::unique_ptr<chaiscript::ChaiScript> chai;
    std::vector<std::function<void(float)>> updateCallbacks;
    std::vector<std::function<void(float)>> lateUpdateCallbacks;
    std::string currentFile = "script";

public:
    bool initialize() override {
        chai = std::make_unique<chaiscript::ChaiScript>();
        return true;
    }

    void registerAPI() override {
        chai->add(chaiscript::const_var(chaiscript::Boxed_Value()), "null");
        chai->add(chaiscript::fun([this](const std::string& msg) {
            std::cout << "[" << currentFile << "] " << msg << std::endl;
        }), "print");

        chai->add(chaiscript::fun([this](const std::string& msg) {
            std::cerr << "[!] [" << currentFile << "] " << msg << std::endl;
        }), "error");

        chai->add(chaiscript::user_type<Vector2>(), "Vector2");
        chai->add(chaiscript::constructor<Vector2()>(), "Vector2");
        chai->add(chaiscript::constructor<Vector2(float, float)>(), "Vector2");
        chai->add(chaiscript::fun(&Vector2::x), "x");
        chai->add(chaiscript::fun(&Vector2::y), "y");
        chai->add(chaiscript::fun(&Vector2::length), "length");
        chai->add(chaiscript::fun(&Vector2::normalized), "normalized");
        chai->add(chaiscript::fun([](const Vector2& a, const Vector2& b) { return a + b; }), "+");
        chai->add(chaiscript::fun([](const Vector2& a, const Vector2& b) { return a - b; }), "-");
        chai->add(chaiscript::fun([](const Vector2& a, float s) { return a * s; }), "*");
        chai->add(chaiscript::user_type<Vector3>(), "Vector3");
        chai->add(chaiscript::constructor<Vector3()>(), "Vector3");
        chai->add(chaiscript::constructor<Vector3(float, float, float)>(), "Vector3");
        chai->add(chaiscript::fun(&Vector3::x), "x");
        chai->add(chaiscript::fun(&Vector3::y), "y");
        chai->add(chaiscript::fun(&Vector3::z), "z");
        chai->add(chaiscript::fun(&Vector3::length), "length");
        chai->add(chaiscript::fun(&Vector3::normalized), "normalized");
        chai->add(chaiscript::fun([](const Vector3& a, const Vector3& b) { return a + b; }), "+");
        chai->add(chaiscript::fun([](const Vector3& a, const Vector3& b) { return a - b; }), "-");
        chai->add(chaiscript::fun([](const Vector3& a, float s) { return a * s; }), "*");
        chai->add(chaiscript::fun(&Vector3::cross), "cross");
        chai->add(chaiscript::fun(&Vector3::dot), "dot");
        chai->add(chaiscript::user_type<Vector4>(), "Vector4");
        chai->add(chaiscript::constructor<Vector4()>(), "Vector4");
        chai->add(chaiscript::constructor<Vector4(float, float, float, float)>(), "Vector4");
        chai->add(chaiscript::fun(&Vector4::x), "x");
        chai->add(chaiscript::fun(&Vector4::y), "y");
        chai->add(chaiscript::fun(&Vector4::z), "z");
        chai->add(chaiscript::fun(&Vector4::w), "w");
        chai->add(chaiscript::user_type<Color>(), "Color");
        chai->add(chaiscript::constructor<Color()>(), "Color");
        chai->add(chaiscript::constructor<Color(float, float, float, float)>(), "Color");
        chai->add(chaiscript::fun(&Color::r), "r");
        chai->add(chaiscript::fun(&Color::g), "g");
        chai->add(chaiscript::fun(&Color::b), "b");
        chai->add(chaiscript::fun(&Color::a), "a");
        chai->add(chaiscript::fun(&Color::white), "white");
        chai->add(chaiscript::fun(&Color::black), "black");
        chai->add(chaiscript::fun(&Color::red), "red");
        chai->add(chaiscript::fun(&Color::green), "green");
        chai->add(chaiscript::fun(&Color::blue), "blue");
        chai->add(chaiscript::fun(&Color::yellow), "yellow");
        chai->add(chaiscript::fun(&Color::cyan), "cyan");
        chai->add(chaiscript::fun(&Color::magenta), "magenta");
        chai->add(chaiscript::user_type<Transform>(), "Transform");
        chai->add(chaiscript::constructor<Transform()>(), "Transform");
        chai->add(chaiscript::fun(&Transform::position), "position");
        chai->add(chaiscript::fun(&Transform::rotation), "rotation");
        chai->add(chaiscript::fun(&Transform::scale), "scale");
        chai->add(chaiscript::fun(&Transform::translate), "translate");
        chai->add(chaiscript::fun(&Transform::rotate), "rotate");
        chai->add(chaiscript::user_type<Entity>(), "Entity");
        chai->add(chaiscript::fun(&Entity::transform), "transform");
        chai->add(chaiscript::fun(&Entity::name), "name");
        chai->add(chaiscript::fun(&Entity::tag), "tag");
        chai->add(chaiscript::fun(&Entity::active), "active");
        chai->add(chaiscript::fun([](const std::shared_ptr<Entity>& a, const std::shared_ptr<Entity>& b) { return a == b; }), "==");
        chai->add(chaiscript::fun([](const std::shared_ptr<Entity>& a, const std::shared_ptr<Entity>& b) { return a != b; }), "!=");
        chai->add(chaiscript::fun([](const std::shared_ptr<Entity>& a) { return a == nullptr; }), "==");
        chai->add(chaiscript::fun([](const std::shared_ptr<Entity>& a) { return a != nullptr; }), "!=");
        chai->add(chaiscript::user_type<Vertex>(), "Vertex");
        chai->add(chaiscript::constructor<Vertex()>(), "Vertex");
        chai->add(chaiscript::constructor<Vertex(const Vector3&)>(), "Vertex");
        chai->add(chaiscript::fun(&Vertex::position), "position");
        chai->add(chaiscript::fun(&Vertex::normal), "normal");
        chai->add(chaiscript::fun(&Vertex::texCoord), "texCoord");
        chai->add(chaiscript::fun(&Vertex::color), "color");
        chai->add(chaiscript::user_type<Mesh>(), "Mesh");
        chai->add(chaiscript::base_class<Entity, Mesh>());
        chai->add(chaiscript::fun(&Mesh::color), "color");
        chai->add(chaiscript::fun(&Mesh::dirty), "dirty");
        chai->add(chaiscript::fun([](Mesh& m, const Vertex& v) { m.addVertex(v); }), "addVertex");
        chai->add(chaiscript::fun([](Mesh& m, float x, float y, float z) { m.addVertex(x, y, z); }), "addVertex");
        chai->add(chaiscript::fun([](std::shared_ptr<Mesh> m, const Vertex& v) { m->addVertex(v); }), "addVertex");
        chai->add(chaiscript::fun([](std::shared_ptr<Mesh> m, float x, float y, float z) { m->addVertex(x, y, z); }), "addVertex");
        chai->add(chaiscript::fun(&Mesh::addIndex), "addIndex");
        chai->add(chaiscript::fun(&Mesh::addTriangle), "addTriangle");
        chai->add(chaiscript::fun(&Mesh::clear), "clear");
        chai->add(chaiscript::fun(&Mesh::calculateNormals), "calculateNormals");
        chai->add(chaiscript::fun([](const std::string& name) -> std::shared_ptr<Mesh> {
            return std::make_shared<Mesh>(name);
        }), "createMesh");

        chai->add(chaiscript::fun([](const std::string& name) -> std::shared_ptr<Mesh> {
            return Mesh::createCube(name);
        }), "createCube");

        chai->add(chaiscript::fun([](const std::string& name, float width, float height) -> std::shared_ptr<Mesh> {
            return Mesh::createPlane(name, width, height);
        }), "createPlane");

        chai->add(chaiscript::fun([](const std::string& name, int segments, int rings) -> std::shared_ptr<Mesh> {
            return Mesh::createSphere(name, segments, rings);
        }), "createSphere");

        chai->add(chaiscript::fun([]() -> std::shared_ptr<Mesh> {
            return Mesh::createCube();
        }), "createCube");

        chai->add(chaiscript::fun([]() -> std::shared_ptr<Mesh> {
            return Mesh::createPlane();
        }), "createPlane");

        chai->add(chaiscript::fun([]() -> std::shared_ptr<Mesh> {
            return Mesh::createSphere();
        }), "createSphere");

        chai->add(chaiscript::user_type<Camera>(), "Camera");
        chai->add(chaiscript::constructor<Camera()>(), "Camera");
        chai->add(chaiscript::fun([](Camera& c) -> Vector3& { return c.position; }), "position");
        chai->add(chaiscript::fun([](Camera& c) -> Vector3& { return c.rotation; }), "rotation");
        chai->add(chaiscript::fun([](Camera& c) -> float& { return c.fov; }), "fov");
        chai->add(chaiscript::fun([](Camera& c) -> float& { return c.nearPlane; }), "nearPlane");
        chai->add(chaiscript::fun([](Camera& c) -> float& { return c.farPlane; }), "farPlane");
        chai->add(chaiscript::fun([](Camera& c) -> Color& { return c.clearColor; }), "clearColor");
        chai->add(chaiscript::fun([](Camera* c) -> Vector3& { return c->position; }), "position");
        chai->add(chaiscript::fun([](Camera* c) -> Vector3& { return c->rotation; }), "rotation");
        chai->add(chaiscript::fun([](Camera* c) -> float& { return c->fov; }), "fov");
        chai->add(chaiscript::fun([](Camera* c) -> float& { return c->nearPlane; }), "nearPlane");
        chai->add(chaiscript::fun([](Camera* c) -> float& { return c->farPlane; }), "farPlane");
        chai->add(chaiscript::fun([](Camera* c) -> Color& { return c->clearColor; }), "clearColor");
        
        chai->add(chaiscript::fun([](const std::string& name, const std::string& vertexPath, const std::string& fragmentPath) -> bool {
            return g_engine->getRenderer()->loadShader(name, vertexPath, fragmentPath);
        }), "loadRendererShader");
        
        chai->add(chaiscript::fun([](const std::string& name) {
            g_engine->getRenderer()->useShader(name);
        }), "useShader");
        chai->add(chaiscript::fun([](Camera& c) { return c.forward(); }), "forward");
        chai->add(chaiscript::fun([](Camera& c) { return c.right(); }), "right");
        chai->add(chaiscript::fun([](Camera& c) { return c.up(); }), "up");
        chai->add(chaiscript::fun([](Camera* c) { return c->forward(); }), "forward");
        chai->add(chaiscript::fun([](Camera* c) { return c->right(); }), "right");
        chai->add(chaiscript::fun([](Camera* c) { return c->up(); }), "up");
        chai->add(chaiscript::user_type<Light>(), "Light");
        chai->add(chaiscript::constructor<Light()>(), "Light");
        chai->add(chaiscript::fun([](Light& l) -> Light::Type& { return l.type; }), "type");
        chai->add(chaiscript::fun([](Light& l) -> Vector3& { return l.position; }), "position");
        chai->add(chaiscript::fun([](Light& l) -> Vector3& { return l.direction; }), "direction");
        chai->add(chaiscript::fun([](Light& l) -> Color& { return l.color; }), "color");
        chai->add(chaiscript::fun([](Light& l) -> float& { return l.intensity; }), "intensity");
        chai->add(chaiscript::fun([](Light& l) -> float& { return l.range; }), "range");
        chai->add(chaiscript::fun([](Light& l) -> float& { return l.spotAngle; }), "spotAngle");
        chai->add(chaiscript::user_type<Light::Type>(), "LightType");
        chai->add_global_const(chaiscript::const_var(Light::Type::Directional), "LIGHT_DIRECTIONAL");
        chai->add_global_const(chaiscript::const_var(Light::Type::Point), "LIGHT_POINT");
        chai->add_global_const(chaiscript::const_var(Light::Type::Spot), "LIGHT_SPOT");
        chai->add(chaiscript::user_type<Scene>(), "Scene");
        chai->add(chaiscript::fun([](Scene* s) -> Camera* { return &s->camera; }), "getCamera");
        chai->add(chaiscript::fun([](Scene& s) -> Color& { return s.ambientColor; }), "ambientColor");
        chai->add(chaiscript::fun([](Scene* s) -> Color& { return s->ambientColor; }), "ambientColor");
        chai->add(chaiscript::fun([](Scene* s, std::shared_ptr<Entity> e) { s->addEntity(e); }), "addEntity");
        chai->add(chaiscript::fun([](Scene* s, std::shared_ptr<Entity> e) { s->removeEntity(e); }), "removeEntity");
        chai->add(chaiscript::fun([](Scene* s, const std::string& n) { s->removeEntityByName(n); }), "removeEntityByName");
        chai->add(chaiscript::fun([](Scene* s, const std::string& n) { return s->getEntityByName(n); }), "getEntityByName");
        chai->add(chaiscript::fun([](Scene* s, const std::string& t) { return s->getEntitiesByTag(t); }), "getEntitiesByTag");
        chai->add(chaiscript::fun([](Scene* s, const Light& l) { s->addLight(l); }), "addLight");
        chai->add(chaiscript::fun([](Scene* s) { s->clear(); }), "clear");
        chai->add(chaiscript::fun([](Scene* scene) -> std::vector<std::shared_ptr<Entity>>& {
            return scene->entities;
        }), "entities");
        chai->add(chaiscript::fun([](Scene* scene) -> std::vector<Light>& {
            return scene->lights;
        }), "lights");

        chai->add(chaiscript::fun([]() -> Scene* {
            return g_engine->getScene();
        }), "getScene");

        chai->add(chaiscript::fun([](std::shared_ptr<Entity> entity) {
            g_engine->getScene()->addEntity(entity);
        }), "addEntity");

        chai->add(chaiscript::fun([](std::shared_ptr<Mesh> mesh) {
            g_engine->getScene()->addEntity(std::static_pointer_cast<Entity>(mesh));
        }), "addEntity");

        chai->add(chaiscript::fun([]() -> Camera* {
            return &g_engine->getScene()->camera;
        }), "getCamera");

        chai->add(chaiscript::user_type<Input>(), "Input");
        chai->add(chaiscript::fun(&Input::isKeyDown), "isKeyDown");
        chai->add(chaiscript::fun(&Input::isKeyPressed), "isKeyPressed");
        chai->add(chaiscript::fun(&Input::isKeyReleased), "isKeyReleased");
        chai->add(chaiscript::fun(&Input::isMouseButtonDown), "isMouseButtonDown");
        chai->add(chaiscript::fun(&Input::isMouseButtonPressed), "isMouseButtonPressed");
        chai->add(chaiscript::fun(&Input::isMouseButtonReleased), "isMouseButtonReleased");
        chai->add(chaiscript::fun(&Input::getMousePosition), "getMousePosition");
        chai->add(chaiscript::fun(&Input::getMouseDelta), "getMouseDelta");
        chai->add(chaiscript::fun(&Input::getScrollDelta), "getScrollDelta");
        chai->add(chaiscript::fun([]() -> Input& {
            return Input::instance();
        }), "getInput");

        chai->add(chaiscript::fun([](KeyCode key) -> bool {
            return Input::instance().isKeyDown(key);
        }), "isKeyDown");

        chai->add(chaiscript::fun([](KeyCode key) -> bool {
            return Input::instance().isKeyPressed(key);
        }), "isKeyPressed");

        chai->add(chaiscript::fun([](KeyCode key) -> bool {
            return Input::instance().isKeyReleased(key);
        }), "isKeyReleased");

        chai->add(chaiscript::fun([](MouseButton button) -> bool {
            return Input::instance().isMouseButtonDown(button);
        }), "isMouseButtonDown");

        chai->add(chaiscript::fun([](MouseButton button) -> bool {
            return Input::instance().isMouseButtonPressed(button);
        }), "isMouseButtonPressed");

        chai->add(chaiscript::fun([]() -> Vector2 {
            return Input::instance().getMousePosition();
        }), "getMousePosition");

        chai->add(chaiscript::fun([]() -> Vector2 {
            return Input::instance().getMouseDelta();
        }), "getMouseDelta");

        chai->add(chaiscript::fun([]() -> Vector2 {
            return Input::instance().getScrollDelta();
        }), "getScrollDelta");

        chai->add(chaiscript::user_type<KeyCode>(), "KeyCode");
        chai->add_global_const(chaiscript::const_var(KeyCode::Space), "KEY_SPACE");
        chai->add_global_const(chaiscript::const_var(KeyCode::Escape), "KEY_ESCAPE");
        chai->add_global_const(chaiscript::const_var(KeyCode::Enter), "KEY_ENTER");
        chai->add_global_const(chaiscript::const_var(KeyCode::Tab), "KEY_TAB");
        chai->add_global_const(chaiscript::const_var(KeyCode::Backspace), "KEY_BACKSPACE");
        chai->add_global_const(chaiscript::const_var(KeyCode::Up), "KEY_UP");
        chai->add_global_const(chaiscript::const_var(KeyCode::Down), "KEY_DOWN");
        chai->add_global_const(chaiscript::const_var(KeyCode::Left), "KEY_LEFT");
        chai->add_global_const(chaiscript::const_var(KeyCode::Right), "KEY_RIGHT");
        chai->add_global_const(chaiscript::const_var(KeyCode::A), "KEY_A");
        chai->add_global_const(chaiscript::const_var(KeyCode::B), "KEY_B");
        chai->add_global_const(chaiscript::const_var(KeyCode::C), "KEY_C");
        chai->add_global_const(chaiscript::const_var(KeyCode::D), "KEY_D");
        chai->add_global_const(chaiscript::const_var(KeyCode::E), "KEY_E");
        chai->add_global_const(chaiscript::const_var(KeyCode::F), "KEY_F");
        chai->add_global_const(chaiscript::const_var(KeyCode::G), "KEY_G");
        chai->add_global_const(chaiscript::const_var(KeyCode::H), "KEY_H");
        chai->add_global_const(chaiscript::const_var(KeyCode::I), "KEY_I");
        chai->add_global_const(chaiscript::const_var(KeyCode::J), "KEY_J");
        chai->add_global_const(chaiscript::const_var(KeyCode::K), "KEY_K");
        chai->add_global_const(chaiscript::const_var(KeyCode::L), "KEY_L");
        chai->add_global_const(chaiscript::const_var(KeyCode::M), "KEY_M");
        chai->add_global_const(chaiscript::const_var(KeyCode::N), "KEY_N");
        chai->add_global_const(chaiscript::const_var(KeyCode::O), "KEY_O");
        chai->add_global_const(chaiscript::const_var(KeyCode::P), "KEY_P");
        chai->add_global_const(chaiscript::const_var(KeyCode::Q), "KEY_Q");
        chai->add_global_const(chaiscript::const_var(KeyCode::R), "KEY_R");
        chai->add_global_const(chaiscript::const_var(KeyCode::S), "KEY_S");
        chai->add_global_const(chaiscript::const_var(KeyCode::T), "KEY_T");
        chai->add_global_const(chaiscript::const_var(KeyCode::U), "KEY_U");
        chai->add_global_const(chaiscript::const_var(KeyCode::V), "KEY_V");
        chai->add_global_const(chaiscript::const_var(KeyCode::W), "KEY_W");
        chai->add_global_const(chaiscript::const_var(KeyCode::X), "KEY_X");
        chai->add_global_const(chaiscript::const_var(KeyCode::Y), "KEY_Y");
        chai->add_global_const(chaiscript::const_var(KeyCode::Z), "KEY_Z");
        chai->add_global_const(chaiscript::const_var(KeyCode::Num0), "KEY_0");
        chai->add_global_const(chaiscript::const_var(KeyCode::Num1), "KEY_1");
        chai->add_global_const(chaiscript::const_var(KeyCode::Num2), "KEY_2");
        chai->add_global_const(chaiscript::const_var(KeyCode::Num3), "KEY_3");
        chai->add_global_const(chaiscript::const_var(KeyCode::Num4), "KEY_4");
        chai->add_global_const(chaiscript::const_var(KeyCode::Num5), "KEY_5");
        chai->add_global_const(chaiscript::const_var(KeyCode::Num6), "KEY_6");
        chai->add_global_const(chaiscript::const_var(KeyCode::Num7), "KEY_7");
        chai->add_global_const(chaiscript::const_var(KeyCode::Num8), "KEY_8");
        chai->add_global_const(chaiscript::const_var(KeyCode::Num9), "KEY_9");
        chai->add_global_const(chaiscript::const_var(KeyCode::F1), "KEY_F1");
        chai->add_global_const(chaiscript::const_var(KeyCode::F2), "KEY_F2");
        chai->add_global_const(chaiscript::const_var(KeyCode::F3), "KEY_F3");
        chai->add_global_const(chaiscript::const_var(KeyCode::F4), "KEY_F4");
        chai->add_global_const(chaiscript::const_var(KeyCode::F5), "KEY_F5");
        chai->add_global_const(chaiscript::const_var(KeyCode::F6), "KEY_F6");
        chai->add_global_const(chaiscript::const_var(KeyCode::F7), "KEY_F7");
        chai->add_global_const(chaiscript::const_var(KeyCode::F8), "KEY_F8");
        chai->add_global_const(chaiscript::const_var(KeyCode::F9), "KEY_F9");
        chai->add_global_const(chaiscript::const_var(KeyCode::F10), "KEY_F10");
        chai->add_global_const(chaiscript::const_var(KeyCode::F11), "KEY_F11");
        chai->add_global_const(chaiscript::const_var(KeyCode::F12), "KEY_F12");
        chai->add_global_const(chaiscript::const_var(KeyCode::LeftShift), "KEY_LSHIFT");
        chai->add_global_const(chaiscript::const_var(KeyCode::RightShift), "KEY_RSHIFT");
        chai->add_global_const(chaiscript::const_var(KeyCode::LeftControl), "KEY_LCTRL");
        chai->add_global_const(chaiscript::const_var(KeyCode::RightControl), "KEY_RCTRL");
        chai->add_global_const(chaiscript::const_var(KeyCode::LeftAlt), "KEY_LALT");
        chai->add_global_const(chaiscript::const_var(KeyCode::RightAlt), "KEY_RALT");
        chai->add(chaiscript::user_type<MouseButton>(), "MouseButton");
        chai->add_global_const(chaiscript::const_var(MouseButton::Left), "MOUSE_LEFT");
        chai->add_global_const(chaiscript::const_var(MouseButton::Right), "MOUSE_RIGHT");
        chai->add_global_const(chaiscript::const_var(MouseButton::Middle), "MOUSE_MIDDLE");
        chai->add(chaiscript::user_type<Time>(), "Time");
        chai->add(chaiscript::fun(&Time::getDeltaTime), "getDeltaTime");
        chai->add(chaiscript::fun(&Time::getUnscaledDeltaTime), "getUnscaledDeltaTime");
        chai->add(chaiscript::fun(&Time::getTotalTime), "getTotalTime");
        chai->add(chaiscript::fun(&Time::getFPS), "getFPS");
        chai->add(chaiscript::fun(&Time::getFrameCount), "getFrameCount");
        chai->add(chaiscript::fun(&Time::setTimeScale), "setTimeScale");
        chai->add(chaiscript::fun(&Time::getTimeScale), "getTimeScale");
        chai->add(chaiscript::fun([]() -> Time& {
            return Time::instance();
        }), "getTime");

        chai->add(chaiscript::fun([]() -> float {
            return Time::instance().getDeltaTime();
        }), "deltaTime");

        chai->add(chaiscript::fun([]() -> float {
            return Time::instance().getTotalTime();
        }), "totalTime");

        chai->add(chaiscript::fun([]() -> float {
            return Time::instance().getFPS();
        }), "fps");

        auto& callbacks = updateCallbacks;
        chai->add(chaiscript::fun([&callbacks](const std::function<void(float)>& func) {
            callbacks.push_back(func);
        }), "onUpdate");

        auto& lateCallbacks = lateUpdateCallbacks;
        chai->add(chaiscript::fun([&lateCallbacks](const std::function<void(float)>& func) {
            lateCallbacks.push_back(func);
        }), "onLateUpdate");

        chai->add(chaiscript::fun([]() {
            g_engine->stop();
        }), "quit");

        chai->add(chaiscript::fun([](bool enabled) {
            if (g_engine->getRenderer()) {
                g_engine->getRenderer()->setWireframe(enabled);
            }
        }), "setWireframe");

        chai->add(chaiscript::fun([](bool enabled) {
            if (g_engine->getRenderer()) {
                g_engine->getRenderer()->setVSync(enabled);
            }
        }), "setVSync");

        chai->add(chaiscript::fun([](const std::string& filename) -> bool {
            return g_engine->executeScript(filename);
        }), "require");

        chai->add(chaiscript::fun([](float min, float max) -> float {
            return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
        }), "random");

        chai->add(chaiscript::fun([](float value, float min, float max) -> float {
            return std::max(min, std::min(max, value));
        }), "clamp");

        chai->add(chaiscript::fun([](float a, float b, float t) -> float {
            return a + t * (b - a);
        }), "lerp");

        chai->add(chaiscript::fun([](const Vector3& a, const Vector3& b, float t) -> Vector3 {
            return Vector3(a.x + t * (b.x - a.x), a.y + t * (b.y - a.y), a.z + t * (b.z - a.z));
        }), "lerp");

        chai->add(chaiscript::fun([](float degrees) -> float {
            return degrees * 3.14159265358979323846f / 180.0f;
        }), "radians");

        chai->add(chaiscript::fun([](float radians) -> float {
            return radians * 180.0f / 3.14159265358979323846f;
        }), "degrees");

        chai->add(chaiscript::fun([](float value) -> float {
            return std::sin(value);
        }), "sin");

        chai->add(chaiscript::fun([](float value) -> float {
            return std::cos(value);
        }), "cos");

        chai->add(chaiscript::fun([](float value) -> float {
            return std::tan(value);
        }), "tan");

        chai->add(chaiscript::fun([](float value) -> float {
            return std::sqrt(value);
        }), "sqrt");

        chai->add(chaiscript::fun([](float value) -> float {
            return std::abs(value);
        }), "abs");

        chai->add(chaiscript::fun([](float a, float b) -> float {
            return std::min(a, b);
        }), "min");

        chai->add(chaiscript::fun([](float a, float b) -> float {
            return std::max(a, b);
        }), "max");

        chai->add(chaiscript::fun([](float base, float exp) -> float {
            return std::pow(base, exp);
        }), "pow");

        chai->add(chaiscript::fun([](float y, float x) -> float {
            return std::atan2(y, x);
        }), "atan2");

        chai->add(chaiscript::fun([](float value) -> float {
            return std::floor(value);
        }), "floor");

        chai->add(chaiscript::fun([](float value) -> float {
            return std::ceil(value);
        }), "ceil");

        chai->add(chaiscript::fun([](float value) -> float {
            return std::round(value);
        }), "round");
        chai->add(chaiscript::fun([](const std::string& filename) -> bool {
            auto mapData = MapLoader::loadMap(filename);
            if (mapData) {
                MapLoader::loadMapIntoScene(mapData, g_engine->getScene());
                return true;
            }
            return false;
        }), "loadMap");

        chai->add(chaiscript::fun([]() {
            MapLoader::clearScene(g_engine->getScene());
        }), "clearScene");
    }

    bool executeFile(const std::string& filename) override {
        std::string previousFile = currentFile;
        currentFile = filename;
        try {
            chai->eval_file(filename);
            currentFile = previousFile;
            return true;
        } catch (const chaiscript::exception::eval_error& e) {
            std::cerr << "Script eval error in " << filename << ":" << std::endl;
            std::cerr << e.pretty_print() << std::endl;
            return false;
        } catch (const chaiscript::exception::dispatch_error& e) {
            std::cerr << "Script dispatch error in " << filename << ":" << std::endl;
            std::cerr << "  Function: " << e.what() << std::endl;
            std::cerr << "  Parameters:" << std::endl;
            for (const auto& p : e.parameters) {
                std::cerr << "    - " << p.get_type_info().name() << std::endl;
            }
            return false;
        } catch (const std::exception& e) {
            std::cerr << "Script error in " << filename << ": " << e.what() << std::endl;
            currentFile = previousFile;
            return false;
        }
    }

    bool executeString(const std::string& code) override {
        try {
            chai->eval(code);
            return true;
        } catch (const chaiscript::exception::eval_error& e) {
            std::cerr << "Script error: " << e.pretty_print() << std::endl;
            return false;
        } catch (const std::exception& e) {
            std::cerr << "Script error: " << e.what() << std::endl;
            return false;
        }
    }

    void update(float deltaTime) override {
        for (auto& callback : updateCallbacks) {
            try {
                callback(deltaTime);
            } catch (const std::exception& e) {
                std::cerr << "Update callback error: " << e.what() << std::endl;
            }
        }

        for (auto& callback : lateUpdateCallbacks) {
            try {
                callback(deltaTime);
            } catch (const std::exception& e) {
                std::cerr << "Late update callback error: " << e.what() << std::endl;
            }
        }
    }

    void shutdown() override {
        updateCallbacks.clear();
        lateUpdateCallbacks.clear();
        chai.reset();
    }

    std::string getExtension() const override { return ".chai"; }

    chaiscript::ChaiScript* getChai() { return chai.get(); }
};

}

#endif
