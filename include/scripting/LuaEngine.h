#ifndef LUA_ENGINE_H
#define LUA_ENGINE_H

#include "../CombineEngine.h"
#include <lua.hpp>
#include <iostream>
#include <functional>
#include <vector>
#include <map>

namespace Combine {

class LuaEngine : public IScriptEngine {
private:
    lua_State* L = nullptr;
    std::vector<std::function<void(float)>> updateCallbacks;
    std::vector<std::function<void(float)>> lateUpdateCallbacks;
    std::string currentFile = "script";

    static LuaEngine* instance;

    static int l_print(lua_State* L) {
        int nargs = lua_gettop(L);
        std::string msg;
        for (int i = 1; i <= nargs; i++) {
            if (i > 1) msg += "\t";
            if (lua_isstring(L, i)) {
                msg += lua_tostring(L, i);
            } else if (lua_isnumber(L, i)) {
                msg += std::to_string(lua_tonumber(L, i));
            } else if (lua_isboolean(L, i)) {
                msg += lua_toboolean(L, i) ? "true" : "false";
            } else if (lua_isnil(L, i)) {
                msg += "nil";
            } else {
                msg += lua_typename(L, lua_type(L, i));
            }
        }
        std::cout << "[" << instance->currentFile << "] " << msg << std::endl;
        return 0;
    }

    static int l_error(lua_State* L) {
        const char* msg = luaL_checkstring(L, 1);
        std::cerr << "[!] [" << instance->currentFile << "] " << msg << std::endl;
        return 0;
    }

    static int l_getScene(lua_State* L) {
        Scene** s = (Scene**)lua_newuserdata(L, sizeof(Scene*));
        *s = g_engine->getScene();
        luaL_setmetatable(L, "Scene");
        return 1;
    }

    static int l_getCamera(lua_State* L) {
        Camera** c = (Camera**)lua_newuserdata(L, sizeof(Camera*));
        *c = &g_engine->getScene()->camera;
        luaL_setmetatable(L, "Camera");
        return 1;
    }

    static int l_createMesh(lua_State* L) {
        const char* name = luaL_optstring(L, 1, "Mesh");
        std::shared_ptr<Mesh>* m = (std::shared_ptr<Mesh>*)lua_newuserdata(L, sizeof(std::shared_ptr<Mesh>));
        new (m) std::shared_ptr<Mesh>(std::make_shared<Mesh>(name));
        luaL_setmetatable(L, "Mesh");
        return 1;
    }

    static int l_createCube(lua_State* L) {
        const char* name = luaL_optstring(L, 1, "Cube");
        std::shared_ptr<Mesh>* m = (std::shared_ptr<Mesh>*)lua_newuserdata(L, sizeof(std::shared_ptr<Mesh>));
        new (m) std::shared_ptr<Mesh>(Mesh::createCube(name));
        luaL_setmetatable(L, "Mesh");
        return 1;
    }

    static int l_createPlane(lua_State* L) {
        const char* name = luaL_optstring(L, 1, "Plane");
        float width = luaL_optnumber(L, 2, 1.0f);
        float height = luaL_optnumber(L, 3, 1.0f);
        std::shared_ptr<Mesh>* m = (std::shared_ptr<Mesh>*)lua_newuserdata(L, sizeof(std::shared_ptr<Mesh>));
        new (m) std::shared_ptr<Mesh>(Mesh::createPlane(name, width, height));
        luaL_setmetatable(L, "Mesh");
        return 1;
    }

    static int l_createSphere(lua_State* L) {
        const char* name = luaL_optstring(L, 1, "Sphere");
        int segments = luaL_optinteger(L, 2, 16);
        int rings = luaL_optinteger(L, 3, 16);
        std::shared_ptr<Mesh>* m = (std::shared_ptr<Mesh>*)lua_newuserdata(L, sizeof(std::shared_ptr<Mesh>));
        new (m) std::shared_ptr<Mesh>(Mesh::createSphere(name, segments, rings));
        luaL_setmetatable(L, "Mesh");
        return 1;
    }

    static int l_createLight(lua_State* L) {
        Light* light = (Light*)lua_newuserdata(L, sizeof(Light));
        new (light) Light();
        luaL_setmetatable(L, "Light");
        return 1;
    }

    static int l_addEntity(lua_State* L) {
        std::shared_ptr<Mesh>* m = (std::shared_ptr<Mesh>*)luaL_checkudata(L, 1, "Mesh");
        g_engine->getScene()->addEntity(*m);
        return 0;
    }

    static int l_isKeyDown(lua_State* L) {
        int key = luaL_checkinteger(L, 1);
        lua_pushboolean(L, Input::instance().isKeyDown(static_cast<KeyCode>(key)));
        return 1;
    }

    static int l_isKeyPressed(lua_State* L) {
        int key = luaL_checkinteger(L, 1);
        lua_pushboolean(L, Input::instance().isKeyPressed(static_cast<KeyCode>(key)));
        return 1;
    }

    static int l_isKeyReleased(lua_State* L) {
        int key = luaL_checkinteger(L, 1);
        lua_pushboolean(L, Input::instance().isKeyReleased(static_cast<KeyCode>(key)));
        return 1;
    }

    static int l_isMouseButtonDown(lua_State* L) {
        int button = luaL_checkinteger(L, 1);
        lua_pushboolean(L, Input::instance().isMouseButtonDown(static_cast<MouseButton>(button)));
        return 1;
    }

    static int l_isMouseButtonPressed(lua_State* L) {
        int button = luaL_checkinteger(L, 1);
        lua_pushboolean(L, Input::instance().isMouseButtonPressed(static_cast<MouseButton>(button)));
        return 1;
    }

    static int l_getMousePosition(lua_State* L) {
        Vector2 pos = Input::instance().getMousePosition();
        lua_newtable(L);
        lua_pushnumber(L, pos.x);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, pos.y);
        lua_setfield(L, -2, "y");
        return 1;
    }

    static int l_getMouseDelta(lua_State* L) {
        Vector2 delta = Input::instance().getMouseDelta();
        lua_newtable(L);
        lua_pushnumber(L, delta.x);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, delta.y);
        lua_setfield(L, -2, "y");
        return 1;
    }

    static int l_getScrollDelta(lua_State* L) {
        Vector2 delta = Input::instance().getScrollDelta();
        lua_newtable(L);
        lua_pushnumber(L, delta.x);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, delta.y);
        lua_setfield(L, -2, "y");
        return 1;
    }

    static int l_deltaTime(lua_State* L) {
        lua_pushnumber(L, Time::instance().getDeltaTime());
        return 1;
    }

    static int l_totalTime(lua_State* L) {
        lua_pushnumber(L, Time::instance().getTotalTime());
        return 1;
    }

    static int l_fps(lua_State* L) {
        lua_pushnumber(L, Time::instance().getFPS());
        return 1;
    }

    static int l_quit(lua_State* L) {
        (void)L;
        g_engine->stop();
        return 0;
    }

    static int l_setWireframe(lua_State* L) {
        bool enabled = lua_toboolean(L, 1);
        if (g_engine->getRenderer()) {
            g_engine->getRenderer()->setWireframe(enabled);
        }
        return 0;
    }

    static int l_setVSync(lua_State* L) {
        bool enabled = lua_toboolean(L, 1);
        if (g_engine->getRenderer()) {
            g_engine->getRenderer()->setVSync(enabled);
        }
        return 0;
    }

    static int l_onUpdate(lua_State* L) {
        luaL_checktype(L, 1, LUA_TFUNCTION);
        int ref = luaL_ref(L, LUA_REGISTRYINDEX);
        instance->updateCallbacks.push_back([L, ref](float dt) {
            lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
            lua_pushnumber(L, dt);
            if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
                std::cerr << "Update callback error: " << lua_tostring(L, -1) << std::endl;
                lua_pop(L, 1);
            }
        });
        return 0;
    }

    static int l_onLateUpdate(lua_State* L) {
        luaL_checktype(L, 1, LUA_TFUNCTION);
        int ref = luaL_ref(L, LUA_REGISTRYINDEX);
        instance->lateUpdateCallbacks.push_back([L, ref](float dt) {
            lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
            lua_pushnumber(L, dt);
            if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
                std::cerr << "Late update callback error: " << lua_tostring(L, -1) << std::endl;
                lua_pop(L, 1);
            }
        });
        return 0;
    }

    static int l_random(lua_State* L) {
        float min = luaL_checknumber(L, 1);
        float max = luaL_checknumber(L, 2);
        float result = min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
        lua_pushnumber(L, result);
        return 1;
    }

    static int l_clamp(lua_State* L) {
        float value = luaL_checknumber(L, 1);
        float min = luaL_checknumber(L, 2);
        float max = luaL_checknumber(L, 3);
        lua_pushnumber(L, std::max(min, std::min(max, value)));
        return 1;
    }

    static int l_lerp(lua_State* L) {
        float a = luaL_checknumber(L, 1);
        float b = luaL_checknumber(L, 2);
        float t = luaL_checknumber(L, 3);
        lua_pushnumber(L, a + t * (b - a));
        return 1;
    }

    static int l_radians(lua_State* L) {
        float degrees = luaL_checknumber(L, 1);
        lua_pushnumber(L, degrees * 3.14159265358979323846f / 180.0f);
        return 1;
    }

    static int l_degrees(lua_State* L) {
        float radians = luaL_checknumber(L, 1);
        lua_pushnumber(L, radians * 180.0f / 3.14159265358979323846f);
        return 1;
    }

    static int l_require(lua_State* L) {
        const char* filename = luaL_checkstring(L, 1);
        lua_pushboolean(L, g_engine->executeScript(filename));
        return 1;
    }

    static int l_mesh_gc(lua_State* L) {
        std::shared_ptr<Mesh>* m = (std::shared_ptr<Mesh>*)luaL_checkudata(L, 1, "Mesh");
        m->~shared_ptr();
        return 0;
    }

    static int l_mesh_index(lua_State* L) {
        std::shared_ptr<Mesh>* m = (std::shared_ptr<Mesh>*)luaL_checkudata(L, 1, "Mesh");
        const char* key = luaL_checkstring(L, 2);

        if (strcmp(key, "transform") == 0) {
            Transform* t = &(*m)->transform;
            Transform** tp = (Transform**)lua_newuserdata(L, sizeof(Transform*));
            *tp = t;
            luaL_setmetatable(L, "Transform");
            return 1;
        } else if (strcmp(key, "color") == 0) {
            Color* c = &(*m)->color;
            Color** cp = (Color**)lua_newuserdata(L, sizeof(Color*));
            *cp = c;
            luaL_setmetatable(L, "Color");
            return 1;
        } else if (strcmp(key, "name") == 0) {
            lua_pushstring(L, (*m)->name.c_str());
            return 1;
        } else if (strcmp(key, "active") == 0) {
            lua_pushboolean(L, (*m)->active);
            return 1;
        } else if (strcmp(key, "addVertex") == 0) {
            lua_pushcfunction(L, [](lua_State* L) -> int {
                std::shared_ptr<Mesh>* m = (std::shared_ptr<Mesh>*)luaL_checkudata(L, 1, "Mesh");
                float x = luaL_checknumber(L, 2);
                float y = luaL_checknumber(L, 3);
                float z = luaL_checknumber(L, 4);
                (*m)->addVertex(x, y, z);
                return 0;
            });
            return 1;
        } else if (strcmp(key, "addIndex") == 0) {
            lua_pushcfunction(L, [](lua_State* L) -> int {
                std::shared_ptr<Mesh>* m = (std::shared_ptr<Mesh>*)luaL_checkudata(L, 1, "Mesh");
                unsigned int idx = luaL_checkinteger(L, 2);
                (*m)->addIndex(idx);
                return 0;
            });
            return 1;
        } else if (strcmp(key, "addTriangle") == 0) {
            lua_pushcfunction(L, [](lua_State* L) -> int {
                std::shared_ptr<Mesh>* m = (std::shared_ptr<Mesh>*)luaL_checkudata(L, 1, "Mesh");
                unsigned int i0 = luaL_checkinteger(L, 2);
                unsigned int i1 = luaL_checkinteger(L, 3);
                unsigned int i2 = luaL_checkinteger(L, 4);
                (*m)->addTriangle(i0, i1, i2);
                return 0;
            });
            return 1;
        } else if (strcmp(key, "clear") == 0) {
            lua_pushcfunction(L, [](lua_State* L) -> int {
                std::shared_ptr<Mesh>* m = (std::shared_ptr<Mesh>*)luaL_checkudata(L, 1, "Mesh");
                (*m)->clear();
                return 0;
            });
            return 1;
        } else if (strcmp(key, "calculateNormals") == 0) {
            lua_pushcfunction(L, [](lua_State* L) -> int {
                std::shared_ptr<Mesh>* m = (std::shared_ptr<Mesh>*)luaL_checkudata(L, 1, "Mesh");
                (*m)->calculateNormals();
                return 0;
            });
            return 1;
        }

        return 0;
    }

    static int l_mesh_newindex(lua_State* L) {
        std::shared_ptr<Mesh>* m = (std::shared_ptr<Mesh>*)luaL_checkudata(L, 1, "Mesh");
        const char* key = luaL_checkstring(L, 2);

        if (strcmp(key, "name") == 0) {
            (*m)->name = luaL_checkstring(L, 3);
        } else if (strcmp(key, "active") == 0) {
            (*m)->active = lua_toboolean(L, 3);
        }

        return 0;
    }

    static int l_transform_index(lua_State* L) {
        Transform** t = (Transform**)luaL_checkudata(L, 1, "Transform");
        const char* key = luaL_checkstring(L, 2);

        if (strcmp(key, "position") == 0) {
            Vector3** v = (Vector3**)lua_newuserdata(L, sizeof(Vector3*));
            *v = &(*t)->position;
            luaL_setmetatable(L, "Vector3");
            return 1;
        } else if (strcmp(key, "rotation") == 0) {
            Vector3** v = (Vector3**)lua_newuserdata(L, sizeof(Vector3*));
            *v = &(*t)->rotation;
            luaL_setmetatable(L, "Vector3");
            return 1;
        } else if (strcmp(key, "scale") == 0) {
            Vector3** v = (Vector3**)lua_newuserdata(L, sizeof(Vector3*));
            *v = &(*t)->scale;
            luaL_setmetatable(L, "Vector3");
            return 1;
        }

        return 0;
    }

    static int l_vector3_index(lua_State* L) {
        Vector3** v = (Vector3**)luaL_checkudata(L, 1, "Vector3");
        const char* key = luaL_checkstring(L, 2);

        if (strcmp(key, "x") == 0) {
            lua_pushnumber(L, (*v)->x);
            return 1;
        } else if (strcmp(key, "y") == 0) {
            lua_pushnumber(L, (*v)->y);
            return 1;
        } else if (strcmp(key, "z") == 0) {
            lua_pushnumber(L, (*v)->z);
            return 1;
        } else if (strcmp(key, "length") == 0) {
            lua_pushcfunction(L, [](lua_State* L) -> int {
                Vector3** v = (Vector3**)luaL_checkudata(L, 1, "Vector3");
                lua_pushnumber(L, (*v)->length());
                return 1;
            });
            return 1;
        } else if (strcmp(key, "normalized") == 0) {
            lua_pushcfunction(L, [](lua_State* L) -> int {
                Vector3** v = (Vector3**)luaL_checkudata(L, 1, "Vector3");
                Vector3 n = (*v)->normalized();
                lua_newtable(L);
                lua_pushnumber(L, n.x); lua_setfield(L, -2, "x");
                lua_pushnumber(L, n.y); lua_setfield(L, -2, "y");
                lua_pushnumber(L, n.z); lua_setfield(L, -2, "z");
                return 1;
            });
            return 1;
        }

        return 0;
    }

    static int l_vector3_newindex(lua_State* L) {
        Vector3** v = (Vector3**)luaL_checkudata(L, 1, "Vector3");
        const char* key = luaL_checkstring(L, 2);
        float value = luaL_checknumber(L, 3);

        if (strcmp(key, "x") == 0) {
            (*v)->x = value;
        } else if (strcmp(key, "y") == 0) {
            (*v)->y = value;
        } else if (strcmp(key, "z") == 0) {
            (*v)->z = value;
        }

        return 0;
    }

    static int l_color_index(lua_State* L) {
        Color** c = (Color**)luaL_checkudata(L, 1, "Color");
        const char* key = luaL_checkstring(L, 2);

        if (strcmp(key, "r") == 0) {
            lua_pushnumber(L, (*c)->r);
            return 1;
        } else if (strcmp(key, "g") == 0) {
            lua_pushnumber(L, (*c)->g);
            return 1;
        } else if (strcmp(key, "b") == 0) {
            lua_pushnumber(L, (*c)->b);
            return 1;
        } else if (strcmp(key, "a") == 0) {
            lua_pushnumber(L, (*c)->a);
            return 1;
        }

        return 0;
    }

    static int l_color_newindex(lua_State* L) {
        Color** c = (Color**)luaL_checkudata(L, 1, "Color");
        const char* key = luaL_checkstring(L, 2);
        float value = luaL_checknumber(L, 3);

        if (strcmp(key, "r") == 0) {
            (*c)->r = value;
        } else if (strcmp(key, "g") == 0) {
            (*c)->g = value;
        } else if (strcmp(key, "b") == 0) {
            (*c)->b = value;
        } else if (strcmp(key, "a") == 0) {
            (*c)->a = value;
        }

        return 0;
    }

    static int l_camera_index(lua_State* L) {
        Camera** c = (Camera**)luaL_checkudata(L, 1, "Camera");
        const char* key = luaL_checkstring(L, 2);

        if (strcmp(key, "position") == 0) {
            Vector3** v = (Vector3**)lua_newuserdata(L, sizeof(Vector3*));
            *v = &(*c)->position;
            luaL_setmetatable(L, "Vector3");
            return 1;
        } else if (strcmp(key, "rotation") == 0) {
            Vector3** v = (Vector3**)lua_newuserdata(L, sizeof(Vector3*));
            *v = &(*c)->rotation;
            luaL_setmetatable(L, "Vector3");
            return 1;
        } else if (strcmp(key, "fov") == 0) {
            lua_pushnumber(L, (*c)->fov);
            return 1;
        } else if (strcmp(key, "nearPlane") == 0) {
            lua_pushnumber(L, (*c)->nearPlane);
            return 1;
        } else if (strcmp(key, "farPlane") == 0) {
            lua_pushnumber(L, (*c)->farPlane);
            return 1;
        } else if (strcmp(key, "forward") == 0) {
            lua_pushcfunction(L, [](lua_State* L) -> int {
                Camera** c = (Camera**)luaL_checkudata(L, 1, "Camera");
                Vector3 f = (*c)->forward();
                lua_newtable(L);
                lua_pushnumber(L, f.x); lua_setfield(L, -2, "x");
                lua_pushnumber(L, f.y); lua_setfield(L, -2, "y");
                lua_pushnumber(L, f.z); lua_setfield(L, -2, "z");
                return 1;
            });
            return 1;
        } else if (strcmp(key, "right") == 0) {
            lua_pushcfunction(L, [](lua_State* L) -> int {
                Camera** c = (Camera**)luaL_checkudata(L, 1, "Camera");
                Vector3 r = (*c)->right();
                lua_newtable(L);
                lua_pushnumber(L, r.x); lua_setfield(L, -2, "x");
                lua_pushnumber(L, r.y); lua_setfield(L, -2, "y");
                lua_pushnumber(L, r.z); lua_setfield(L, -2, "z");
                return 1;
            });
            return 1;
        } else if (strcmp(key, "up") == 0) {
            lua_pushcfunction(L, [](lua_State* L) -> int {
                Camera** c = (Camera**)luaL_checkudata(L, 1, "Camera");
                Vector3 u = (*c)->up();
                lua_newtable(L);
                lua_pushnumber(L, u.x); lua_setfield(L, -2, "x");
                lua_pushnumber(L, u.y); lua_setfield(L, -2, "y");
                lua_pushnumber(L, u.z); lua_setfield(L, -2, "z");
                return 1;
            });
            return 1;
        }

        return 0;
    }

    static int l_camera_newindex(lua_State* L) {
        Camera** c = (Camera**)luaL_checkudata(L, 1, "Camera");
        const char* key = luaL_checkstring(L, 2);

        if (strcmp(key, "fov") == 0) {
            (*c)->fov = luaL_checknumber(L, 3);
        } else if (strcmp(key, "nearPlane") == 0) {
            (*c)->nearPlane = luaL_checknumber(L, 3);
        } else if (strcmp(key, "farPlane") == 0) {
            (*c)->farPlane = luaL_checknumber(L, 3);
        }

        return 0;
    }

    static int l_scene_index(lua_State* L) {
        Scene** s = (Scene**)luaL_checkudata(L, 1, "Scene");
        const char* key = luaL_checkstring(L, 2);

        if (strcmp(key, "getCamera") == 0) {
            lua_pushcfunction(L, [](lua_State* L) -> int {
                Scene** s = (Scene**)luaL_checkudata(L, 1, "Scene");
                Camera** c = (Camera**)lua_newuserdata(L, sizeof(Camera*));
                *c = &(*s)->camera;
                luaL_setmetatable(L, "Camera");
                return 1;
            });
            return 1;
        } else if (strcmp(key, "addLight") == 0) {
            lua_pushcfunction(L, [](lua_State* L) -> int {
                Scene** s = (Scene**)luaL_checkudata(L, 1, "Scene");
                Light* light = (Light*)luaL_checkudata(L, 2, "Light");
                (*s)->addLight(*light);
                return 0;
            });
            return 1;
        } else if (strcmp(key, "clear") == 0) {
            lua_pushcfunction(L, [](lua_State* L) -> int {
                Scene** s = (Scene**)luaL_checkudata(L, 1, "Scene");
                (*s)->clear();
                return 0;
            });
            return 1;
        }

        return 0;
    }

    static int l_light_index(lua_State* L) {
        Light* light = (Light*)luaL_checkudata(L, 1, "Light");
        const char* key = luaL_checkstring(L, 2);

        if (strcmp(key, "type") == 0) {
            lua_pushinteger(L, static_cast<int>(light->type));
            return 1;
        } else if (strcmp(key, "position") == 0) {
            Vector3** v = (Vector3**)lua_newuserdata(L, sizeof(Vector3*));
            *v = &light->position;
            luaL_setmetatable(L, "Vector3");
            return 1;
        } else if (strcmp(key, "direction") == 0) {
            Vector3** v = (Vector3**)lua_newuserdata(L, sizeof(Vector3*));
            *v = &light->direction;
            luaL_setmetatable(L, "Vector3");
            return 1;
        } else if (strcmp(key, "color") == 0) {
            Color** c = (Color**)lua_newuserdata(L, sizeof(Color*));
            *c = &light->color;
            luaL_setmetatable(L, "Color");
            return 1;
        } else if (strcmp(key, "intensity") == 0) {
            lua_pushnumber(L, light->intensity);
            return 1;
        } else if (strcmp(key, "range") == 0) {
            lua_pushnumber(L, light->range);
            return 1;
        } else if (strcmp(key, "spotAngle") == 0) {
            lua_pushnumber(L, light->spotAngle);
            return 1;
        }

        return 0;
    }

    static int l_light_newindex(lua_State* L) {
        Light* light = (Light*)luaL_checkudata(L, 1, "Light");
        const char* key = luaL_checkstring(L, 2);

        if (strcmp(key, "type") == 0) {
            light->type = static_cast<Light::Type>(luaL_checkinteger(L, 3));
        } else if (strcmp(key, "intensity") == 0) {
            light->intensity = luaL_checknumber(L, 3);
        } else if (strcmp(key, "range") == 0) {
            light->range = luaL_checknumber(L, 3);
        } else if (strcmp(key, "spotAngle") == 0) {
            light->spotAngle = luaL_checknumber(L, 3);
        }

        return 0;
    }

    void registerMetatable(const char* name, lua_CFunction index, lua_CFunction newindex = nullptr, lua_CFunction gc = nullptr) {
        luaL_newmetatable(L, name);
        lua_pushcfunction(L, index);
        lua_setfield(L, -2, "__index");
        if (newindex) {
            lua_pushcfunction(L, newindex);
            lua_setfield(L, -2, "__newindex");
        }
        if (gc) {
            lua_pushcfunction(L, gc);
            lua_setfield(L, -2, "__gc");
        }
        lua_pop(L, 1);
    }

    void registerKeyConstants() {
        lua_pushinteger(L, static_cast<int>(KeyCode::Space)); lua_setglobal(L, "KEY_SPACE");
        lua_pushinteger(L, static_cast<int>(KeyCode::Escape)); lua_setglobal(L, "KEY_ESCAPE");
        lua_pushinteger(L, static_cast<int>(KeyCode::Enter)); lua_setglobal(L, "KEY_ENTER");
        lua_pushinteger(L, static_cast<int>(KeyCode::Tab)); lua_setglobal(L, "KEY_TAB");
        lua_pushinteger(L, static_cast<int>(KeyCode::Backspace)); lua_setglobal(L, "KEY_BACKSPACE");
        lua_pushinteger(L, static_cast<int>(KeyCode::Up)); lua_setglobal(L, "KEY_UP");
        lua_pushinteger(L, static_cast<int>(KeyCode::Down)); lua_setglobal(L, "KEY_DOWN");
        lua_pushinteger(L, static_cast<int>(KeyCode::Left)); lua_setglobal(L, "KEY_LEFT");
        lua_pushinteger(L, static_cast<int>(KeyCode::Right)); lua_setglobal(L, "KEY_RIGHT");
        lua_pushinteger(L, static_cast<int>(KeyCode::A)); lua_setglobal(L, "KEY_A");
        lua_pushinteger(L, static_cast<int>(KeyCode::B)); lua_setglobal(L, "KEY_B");
        lua_pushinteger(L, static_cast<int>(KeyCode::C)); lua_setglobal(L, "KEY_C");
        lua_pushinteger(L, static_cast<int>(KeyCode::D)); lua_setglobal(L, "KEY_D");
        lua_pushinteger(L, static_cast<int>(KeyCode::E)); lua_setglobal(L, "KEY_E");
        lua_pushinteger(L, static_cast<int>(KeyCode::F)); lua_setglobal(L, "KEY_F");
        lua_pushinteger(L, static_cast<int>(KeyCode::G)); lua_setglobal(L, "KEY_G");
        lua_pushinteger(L, static_cast<int>(KeyCode::H)); lua_setglobal(L, "KEY_H");
        lua_pushinteger(L, static_cast<int>(KeyCode::I)); lua_setglobal(L, "KEY_I");
        lua_pushinteger(L, static_cast<int>(KeyCode::J)); lua_setglobal(L, "KEY_J");
        lua_pushinteger(L, static_cast<int>(KeyCode::K)); lua_setglobal(L, "KEY_K");
        lua_pushinteger(L, static_cast<int>(KeyCode::L)); lua_setglobal(L, "KEY_L");
        lua_pushinteger(L, static_cast<int>(KeyCode::M)); lua_setglobal(L, "KEY_M");
        lua_pushinteger(L, static_cast<int>(KeyCode::N)); lua_setglobal(L, "KEY_N");
        lua_pushinteger(L, static_cast<int>(KeyCode::O)); lua_setglobal(L, "KEY_O");
        lua_pushinteger(L, static_cast<int>(KeyCode::P)); lua_setglobal(L, "KEY_P");
        lua_pushinteger(L, static_cast<int>(KeyCode::Q)); lua_setglobal(L, "KEY_Q");
        lua_pushinteger(L, static_cast<int>(KeyCode::R)); lua_setglobal(L, "KEY_R");
        lua_pushinteger(L, static_cast<int>(KeyCode::S)); lua_setglobal(L, "KEY_S");
        lua_pushinteger(L, static_cast<int>(KeyCode::T)); lua_setglobal(L, "KEY_T");
        lua_pushinteger(L, static_cast<int>(KeyCode::U)); lua_setglobal(L, "KEY_U");
        lua_pushinteger(L, static_cast<int>(KeyCode::V)); lua_setglobal(L, "KEY_V");
        lua_pushinteger(L, static_cast<int>(KeyCode::W)); lua_setglobal(L, "KEY_W");
        lua_pushinteger(L, static_cast<int>(KeyCode::X)); lua_setglobal(L, "KEY_X");
        lua_pushinteger(L, static_cast<int>(KeyCode::Y)); lua_setglobal(L, "KEY_Y");
        lua_pushinteger(L, static_cast<int>(KeyCode::Z)); lua_setglobal(L, "KEY_Z");
        lua_pushinteger(L, static_cast<int>(KeyCode::Num0)); lua_setglobal(L, "KEY_0");
        lua_pushinteger(L, static_cast<int>(KeyCode::Num1)); lua_setglobal(L, "KEY_1");
        lua_pushinteger(L, static_cast<int>(KeyCode::Num2)); lua_setglobal(L, "KEY_2");
        lua_pushinteger(L, static_cast<int>(KeyCode::Num3)); lua_setglobal(L, "KEY_3");
        lua_pushinteger(L, static_cast<int>(KeyCode::Num4)); lua_setglobal(L, "KEY_4");
        lua_pushinteger(L, static_cast<int>(KeyCode::Num5)); lua_setglobal(L, "KEY_5");
        lua_pushinteger(L, static_cast<int>(KeyCode::Num6)); lua_setglobal(L, "KEY_6");
        lua_pushinteger(L, static_cast<int>(KeyCode::Num7)); lua_setglobal(L, "KEY_7");
        lua_pushinteger(L, static_cast<int>(KeyCode::Num8)); lua_setglobal(L, "KEY_8");
        lua_pushinteger(L, static_cast<int>(KeyCode::Num9)); lua_setglobal(L, "KEY_9");
        lua_pushinteger(L, static_cast<int>(KeyCode::F1)); lua_setglobal(L, "KEY_F1");
        lua_pushinteger(L, static_cast<int>(KeyCode::F2)); lua_setglobal(L, "KEY_F2");
        lua_pushinteger(L, static_cast<int>(KeyCode::F3)); lua_setglobal(L, "KEY_F3");
        lua_pushinteger(L, static_cast<int>(KeyCode::F4)); lua_setglobal(L, "KEY_F4");
        lua_pushinteger(L, static_cast<int>(KeyCode::F5)); lua_setglobal(L, "KEY_F5");
        lua_pushinteger(L, static_cast<int>(KeyCode::F6)); lua_setglobal(L, "KEY_F6");
        lua_pushinteger(L, static_cast<int>(KeyCode::F7)); lua_setglobal(L, "KEY_F7");
        lua_pushinteger(L, static_cast<int>(KeyCode::F8)); lua_setglobal(L, "KEY_F8");
        lua_pushinteger(L, static_cast<int>(KeyCode::F9)); lua_setglobal(L, "KEY_F9");
        lua_pushinteger(L, static_cast<int>(KeyCode::F10)); lua_setglobal(L, "KEY_F10");
        lua_pushinteger(L, static_cast<int>(KeyCode::F11)); lua_setglobal(L, "KEY_F11");
        lua_pushinteger(L, static_cast<int>(KeyCode::F12)); lua_setglobal(L, "KEY_F12");
        lua_pushinteger(L, static_cast<int>(KeyCode::LeftShift)); lua_setglobal(L, "KEY_LSHIFT");
        lua_pushinteger(L, static_cast<int>(KeyCode::RightShift)); lua_setglobal(L, "KEY_RSHIFT");
        lua_pushinteger(L, static_cast<int>(KeyCode::LeftControl)); lua_setglobal(L, "KEY_LCTRL");
        lua_pushinteger(L, static_cast<int>(KeyCode::RightControl)); lua_setglobal(L, "KEY_RCTRL");
        lua_pushinteger(L, static_cast<int>(KeyCode::LeftAlt)); lua_setglobal(L, "KEY_LALT");
        lua_pushinteger(L, static_cast<int>(KeyCode::RightAlt)); lua_setglobal(L, "KEY_RALT");

        lua_pushinteger(L, static_cast<int>(MouseButton::Left)); lua_setglobal(L, "MOUSE_LEFT");
        lua_pushinteger(L, static_cast<int>(MouseButton::Right)); lua_setglobal(L, "MOUSE_RIGHT");
        lua_pushinteger(L, static_cast<int>(MouseButton::Middle)); lua_setglobal(L, "MOUSE_MIDDLE");

        lua_pushinteger(L, static_cast<int>(Light::Type::Directional)); lua_setglobal(L, "LIGHT_DIRECTIONAL");
        lua_pushinteger(L, static_cast<int>(Light::Type::Point)); lua_setglobal(L, "LIGHT_POINT");
        lua_pushinteger(L, static_cast<int>(Light::Type::Spot)); lua_setglobal(L, "LIGHT_SPOT");
    }

public:
    bool initialize() override {
        L = luaL_newstate();
        if (!L) return false;
        luaL_openlibs(L);
        instance = this;
        return true;
    }

    void registerAPI() override {
        registerMetatable("Mesh", l_mesh_index, l_mesh_newindex, l_mesh_gc);
        registerMetatable("Transform", l_transform_index);
        registerMetatable("Vector3", l_vector3_index, l_vector3_newindex);
        registerMetatable("Color", l_color_index, l_color_newindex);
        registerMetatable("Camera", l_camera_index, l_camera_newindex);
        registerMetatable("Scene", l_scene_index);
        registerMetatable("Light", l_light_index, l_light_newindex);

        lua_register(L, "print", l_print);
        lua_register(L, "error", l_error);
        lua_register(L, "getScene", l_getScene);
        lua_register(L, "getCamera", l_getCamera);
        lua_register(L, "createMesh", l_createMesh);
        lua_register(L, "createCube", l_createCube);
        lua_register(L, "createPlane", l_createPlane);
        lua_register(L, "createSphere", l_createSphere);
        lua_register(L, "createLight", l_createLight);
        lua_register(L, "addEntity", l_addEntity);
        lua_register(L, "isKeyDown", l_isKeyDown);
        lua_register(L, "isKeyPressed", l_isKeyPressed);
        lua_register(L, "isKeyReleased", l_isKeyReleased);
        lua_register(L, "isMouseButtonDown", l_isMouseButtonDown);
        lua_register(L, "isMouseButtonPressed", l_isMouseButtonPressed);
        lua_register(L, "getMousePosition", l_getMousePosition);
        lua_register(L, "getMouseDelta", l_getMouseDelta);
        lua_register(L, "getScrollDelta", l_getScrollDelta);
        lua_register(L, "deltaTime", l_deltaTime);
        lua_register(L, "totalTime", l_totalTime);
        lua_register(L, "fps", l_fps);
        lua_register(L, "quit", l_quit);
        lua_register(L, "setWireframe", l_setWireframe);
        lua_register(L, "setVSync", l_setVSync);
        lua_register(L, "onUpdate", l_onUpdate);
        lua_register(L, "onLateUpdate", l_onLateUpdate);
        lua_register(L, "random", l_random);
        lua_register(L, "clamp", l_clamp);
        lua_register(L, "lerp", l_lerp);
        lua_register(L, "radians", l_radians);
        lua_register(L, "degrees", l_degrees);
        lua_register(L, "require", l_require);

        registerKeyConstants();
    }

    bool executeFile(const std::string& filename) override {
        std::string previousFile = currentFile;
        currentFile = filename;
        if (luaL_dofile(L, filename.c_str()) != LUA_OK) {
            std::cerr << "Script error in " << filename << ": " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
            currentFile = previousFile;
            return false;
        }
        currentFile = previousFile;
        return true;
    }

    bool executeString(const std::string& code) override {
        if (luaL_dostring(L, code.c_str()) != LUA_OK) {
            std::cerr << "Script error: " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
            return false;
        }
        return true;
    }

    void update(float deltaTime) override {
        for (auto& callback : updateCallbacks) {
            callback(deltaTime);
        }
        for (auto& callback : lateUpdateCallbacks) {
            callback(deltaTime);
        }
    }

    void shutdown() override {
        updateCallbacks.clear();
        lateUpdateCallbacks.clear();
        if (L) {
            lua_close(L);
            L = nullptr;
        }
        instance = nullptr;
    }

    std::string getExtension() const override { return ".lua"; }

    lua_State* getLua() { return L; }
};

LuaEngine* LuaEngine::instance = nullptr;

}

#endif