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

#ifndef SQUIRREL_ENGINE_H
#define SQUIRREL_ENGINE_H
#include "../CombineEngine.h"
#include "../MapLoader.h"
#include <squirrel.h>
#include <sqstdio.h>
#include <sqstdaux.h>
#include <sqstdmath.h>
#include <sqstdstring.h>
#include <iostream>
#include <functional>
#include <vector>
#include <cstring>
#include <cstdarg>
namespace Combine {

class SquirrelEngine : public IScriptEngine {
private:
    HSQUIRRELVM vm = nullptr;
    std::vector<std::function<void(float)>> updateCallbacks;
    std::vector<std::function<void(float)>> lateUpdateCallbacks;
    std::string currentFile = "script";

    static SquirrelEngine* instance;

    static void printFunc(HSQUIRRELVM v, const SQChar* fmt, ...) {
        (void)v;
        va_list args;
        va_start(args, fmt);
        char buffer[1024];
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);
        std::cout << "[" << instance->currentFile << "] " << buffer;
    }

    static void errorFunc(HSQUIRRELVM v, const SQChar* fmt, ...) {
        (void)v;
        va_list args;
        va_start(args, fmt);
        char buffer[1024];
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);
        std::cerr << "[!] [" << instance->currentFile << "] " << buffer;
    }

    static SQInteger sq_print(HSQUIRRELVM v) {
        SQInteger nargs = sq_gettop(v);
        std::string msg;
        for (SQInteger i = 2; i <= nargs; i++) {
            if (i > 2) msg += "\t";
            const SQChar* str;
            sq_tostring(v, i);
            sq_getstring(v, -1, &str);
            msg += str;
            sq_pop(v, 1);
        }
        std::cout << "[" << instance->currentFile << "] " << msg << std::endl;
        return 0;
    }

    static SQInteger sq_error(HSQUIRRELVM v) {
        const SQChar* msg;
        sq_getstring(v, 2, &msg);
        std::cerr << "[!] [" << instance->currentFile << "] " << msg << std::endl;
        return 0;
    }

    static SQInteger sq_getScene(HSQUIRRELVM v) {
        Scene** s = (Scene**)sq_newuserdata(v, sizeof(Scene*));
        *s = g_engine->getScene();
        sq_settypetag(v, -1, (SQUserPointer)"Scene");
        return 1;
    }

    static SQInteger sq_getCamera(HSQUIRRELVM v) {
        Camera** c = (Camera**)sq_newuserdata(v, sizeof(Camera*));
        *c = &g_engine->getScene()->camera;
        sq_settypetag(v, -1, (SQUserPointer)"Camera");
        return 1;
    }

    static SQInteger sq_createMesh(HSQUIRRELVM v) {
        const SQChar* name = "Mesh";
        if (sq_gettop(v) >= 2) {
            sq_getstring(v, 2, &name);
        }
        std::shared_ptr<Mesh>* m = (std::shared_ptr<Mesh>*)sq_newuserdata(v, sizeof(std::shared_ptr<Mesh>));
        new (m) std::shared_ptr<Mesh>(std::make_shared<Mesh>(name));
        sq_settypetag(v, -1, (SQUserPointer)"Mesh");
        sq_setreleasehook(v, -1, meshReleaseHook);
        return 1;
    }

    static SQInteger sq_createCube(HSQUIRRELVM v) {
        const SQChar* name = "Cube";
        if (sq_gettop(v) >= 2) {
            sq_getstring(v, 2, &name);
        }
        std::shared_ptr<Mesh>* m = (std::shared_ptr<Mesh>*)sq_newuserdata(v, sizeof(std::shared_ptr<Mesh>));
        new (m) std::shared_ptr<Mesh>(Mesh::createCube(name));
        sq_settypetag(v, -1, (SQUserPointer)"Mesh");
        sq_setreleasehook(v, -1, meshReleaseHook);
        return 1;
    }

    static SQInteger sq_createPlane(HSQUIRRELVM v) {
        const SQChar* name = "Plane";
        SQFloat width = 1.0f, height = 1.0f;
        if (sq_gettop(v) >= 2) sq_getstring(v, 2, &name);
        if (sq_gettop(v) >= 3) sq_getfloat(v, 3, &width);
        if (sq_gettop(v) >= 4) sq_getfloat(v, 4, &height);
        std::shared_ptr<Mesh>* m = (std::shared_ptr<Mesh>*)sq_newuserdata(v, sizeof(std::shared_ptr<Mesh>));
        new (m) std::shared_ptr<Mesh>(Mesh::createPlane(name, width, height));
        sq_settypetag(v, -1, (SQUserPointer)"Mesh");
        sq_setreleasehook(v, -1, meshReleaseHook);
        return 1;
    }

    static SQInteger sq_createSphere(HSQUIRRELVM v) {
        const SQChar* name = "Sphere";
        SQInteger segments = 16, rings = 16;
        if (sq_gettop(v) >= 2) sq_getstring(v, 2, &name);
        if (sq_gettop(v) >= 3) sq_getinteger(v, 3, &segments);
        if (sq_gettop(v) >= 4) sq_getinteger(v, 4, &rings);
        std::shared_ptr<Mesh>* m = (std::shared_ptr<Mesh>*)sq_newuserdata(v, sizeof(std::shared_ptr<Mesh>));
        new (m) std::shared_ptr<Mesh>(Mesh::createSphere(name, segments, rings));
        sq_settypetag(v, -1, (SQUserPointer)"Mesh");
        sq_setreleasehook(v, -1, meshReleaseHook);
        return 1;
    }

    static SQInteger sq_createLight(HSQUIRRELVM v) {
        Light* light = (Light*)sq_newuserdata(v, sizeof(Light));
        new (light) Light();
        sq_settypetag(v, -1, (SQUserPointer)"Light");
        return 1;
    }

    static SQInteger sq_addEntity(HSQUIRRELVM v) {
        SQUserPointer tag;
        sq_gettypetag(v, 2, &tag);
        if (tag == (SQUserPointer)"Mesh") {
            std::shared_ptr<Mesh>* m;
            sq_getuserdata(v, 2, (SQUserPointer*)&m, nullptr);
            g_engine->getScene()->addEntity(*m);
        }
        return 0;
    }

    static SQInteger meshReleaseHook(SQUserPointer p, SQInteger size) {
        (void)size;
        std::shared_ptr<Mesh>* m = (std::shared_ptr<Mesh>*)p;
        m->~shared_ptr();
        return 0;
    }

    static SQInteger sq_isKeyDown(HSQUIRRELVM v) {
        SQInteger key;
        sq_getinteger(v, 2, &key);
        sq_pushbool(v, Input::instance().isKeyDown(static_cast<KeyCode>(key)));
        return 1;
    }

    static SQInteger sq_isKeyPressed(HSQUIRRELVM v) {
        SQInteger key;
        sq_getinteger(v, 2, &key);
        sq_pushbool(v, Input::instance().isKeyPressed(static_cast<KeyCode>(key)));
        return 1;
    }

    static SQInteger sq_isKeyReleased(HSQUIRRELVM v) {
        SQInteger key;
        sq_getinteger(v, 2, &key);
        sq_pushbool(v, Input::instance().isKeyReleased(static_cast<KeyCode>(key)));
        return 1;
    }

    static SQInteger sq_isMouseButtonDown(HSQUIRRELVM v) {
        SQInteger button;
        sq_getinteger(v, 2, &button);
        sq_pushbool(v, Input::instance().isMouseButtonDown(static_cast<MouseButton>(button)));
        return 1;
    }

    static SQInteger sq_isMouseButtonPressed(HSQUIRRELVM v) {
        SQInteger button;
        sq_getinteger(v, 2, &button);
        sq_pushbool(v, Input::instance().isMouseButtonPressed(static_cast<MouseButton>(button)));
        return 1;
    }

    static SQInteger sq_getMousePosition(HSQUIRRELVM v) {
        Vector2 pos = Input::instance().getMousePosition();
        sq_newtable(v);
        sq_pushstring(v, "x", -1);
        sq_pushfloat(v, pos.x);
        sq_newslot(v, -3, SQFalse);
        sq_pushstring(v, "y", -1);
        sq_pushfloat(v, pos.y);
        sq_newslot(v, -3, SQFalse);
        return 1;
    }

    static SQInteger sq_getMouseDelta(HSQUIRRELVM v) {
        Vector2 delta = Input::instance().getMouseDelta();
        sq_newtable(v);
        sq_pushstring(v, "x", -1);
        sq_pushfloat(v, delta.x);
        sq_newslot(v, -3, SQFalse);
        sq_pushstring(v, "y", -1);
        sq_pushfloat(v, delta.y);
        sq_newslot(v, -3, SQFalse);
        return 1;
    }

    static SQInteger sq_getScrollDelta(HSQUIRRELVM v) {
        Vector2 delta = Input::instance().getScrollDelta();
        sq_newtable(v);
        sq_pushstring(v, "x", -1);
        sq_pushfloat(v, delta.x);
        sq_newslot(v, -3, SQFalse);
        sq_pushstring(v, "y", -1);
        sq_pushfloat(v, delta.y);
        sq_newslot(v, -3, SQFalse);
        return 1;
    }

    static SQInteger sq_deltaTime(HSQUIRRELVM v) {
        sq_pushfloat(v, Time::instance().getDeltaTime());
        return 1;
    }

    static SQInteger sq_totalTime(HSQUIRRELVM v) {
        sq_pushfloat(v, Time::instance().getTotalTime());
        return 1;
    }

    static SQInteger sq_fps(HSQUIRRELVM v) {
        sq_pushfloat(v, Time::instance().getFPS());
        return 1;
    }

    static SQInteger sq_quit(HSQUIRRELVM v) {
        (void)v;
        g_engine->stop();
        return 0;
    }

    static SQInteger sq_setWireframe(HSQUIRRELVM v) {
        SQBool enabled;
        sq_getbool(v, 2, &enabled);
        if (g_engine->getRenderer()) {
            g_engine->getRenderer()->setWireframe(enabled);
        }
        return 0;
    }

    static SQInteger sq_setVSync(HSQUIRRELVM v) {
        SQBool enabled;
        sq_getbool(v, 2, &enabled);
        if (g_engine->getRenderer()) {
            g_engine->getRenderer()->setVSync(enabled);
        }
        return 0;
    }

    static SQInteger sq_onUpdate(HSQUIRRELVM v) {
        HSQOBJECT func;
        sq_getstackobj(v, 2, &func);
        sq_addref(v, &func);
        instance->updateCallbacks.push_back([v, func](float dt) {
            sq_pushobject(v, func);
            sq_pushroottable(v);
            sq_pushfloat(v, dt);
            if (SQ_FAILED(sq_call(v, 2, SQFalse, SQTrue))) {
                std::cerr << "Update callback error" << std::endl;
            }
            sq_pop(v, 1);
        });
        return 0;
    }

    static SQInteger sq_onLateUpdate(HSQUIRRELVM v) {
        HSQOBJECT func;
        sq_getstackobj(v, 2, &func);
        sq_addref(v, &func);
        instance->lateUpdateCallbacks.push_back([v, func](float dt) {
            sq_pushobject(v, func);
            sq_pushroottable(v);
            sq_pushfloat(v, dt);
            if (SQ_FAILED(sq_call(v, 2, SQFalse, SQTrue))) {
                std::cerr << "Late update callback error" << std::endl;
            }
            sq_pop(v, 1);
        });
        return 0;
    }

    static SQInteger sq_random(HSQUIRRELVM v) {
        SQFloat min, max;
        sq_getfloat(v, 2, &min);
        sq_getfloat(v, 3, &max);
        float result = min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
        sq_pushfloat(v, result);
        return 1;
    }

    static SQInteger sq_clamp(HSQUIRRELVM v) {
        SQFloat value, min, max;
        sq_getfloat(v, 2, &value);
        sq_getfloat(v, 3, &min);
        sq_getfloat(v, 4, &max);
        sq_pushfloat(v, std::max(min, std::min(max, value)));
        return 1;
    }

    static SQInteger sq_lerp(HSQUIRRELVM v) {
        SQFloat a, b, t;
        sq_getfloat(v, 2, &a);
        sq_getfloat(v, 3, &b);
        sq_getfloat(v, 4, &t);
        sq_pushfloat(v, a + t * (b - a));
        return 1;
    }

    static SQInteger sq_radians(HSQUIRRELVM v) {
        SQFloat degrees;
        sq_getfloat(v, 2, &degrees);
        sq_pushfloat(v, degrees * 3.14159265358979323846f / 180.0f);
        return 1;
    }

    static SQInteger sq_degrees(HSQUIRRELVM v) {
        SQFloat radians;
        sq_getfloat(v, 2, &radians);
        sq_pushfloat(v, radians * 180.0f / 3.14159265358979323846f);
        return 1;
    }

    static SQInteger sq_require(HSQUIRRELVM v) {
        const SQChar* filename;
        sq_getstring(v, 2, &filename);
        sq_pushbool(v, g_engine->executeScript(filename));
        return 1;
    }

    static SQInteger sq_loadMap(HSQUIRRELVM v) {
        const SQChar* filename;
        sq_getstring(v, 2, &filename);
        auto mapData = MapLoader::loadMap(filename);
        if (mapData) {
            MapLoader::loadMapIntoScene(mapData, g_engine->getScene());
            sq_pushbool(v, SQTrue);
        } else {
            sq_pushbool(v, SQFalse);
        }
        return 1;
    }

    static SQInteger sq_clearScene(HSQUIRRELVM v) {
        (void)v;
        MapLoader::clearScene(g_engine->getScene());
        return 0;
    }

    static SQInteger sq_mesh_get(HSQUIRRELVM v) {
        std::shared_ptr<Mesh>* m;
        sq_getuserdata(v, 1, (SQUserPointer*)&m, nullptr);
        const SQChar* key;
        sq_getstring(v, 2, &key);

        if (strcmp(key, "transform") == 0) {
            Transform** t = (Transform**)sq_newuserdata(v, sizeof(Transform*));
            *t = &(*m)->transform;
            sq_settypetag(v, -1, (SQUserPointer)"Transform");
            return 1;
        } else if (strcmp(key, "color") == 0) {
            Color** c = (Color**)sq_newuserdata(v, sizeof(Color*));
            *c = &(*m)->color;
            sq_settypetag(v, -1, (SQUserPointer)"Color");
            return 1;
        } else if (strcmp(key, "name") == 0) {
            sq_pushstring(v, (*m)->name.c_str(), -1);
            return 1;
        } else if (strcmp(key, "active") == 0) {
            sq_pushbool(v, (*m)->active);
            return 1;
        }

        sq_pushnull(v);
        return 1;
    }

    static SQInteger sq_mesh_set(HSQUIRRELVM v) {
        std::shared_ptr<Mesh>* m;
        sq_getuserdata(v, 1, (SQUserPointer*)&m, nullptr);
        const SQChar* key;
        sq_getstring(v, 2, &key);

        if (strcmp(key, "name") == 0) {
            const SQChar* val;
            sq_getstring(v, 3, &val);
            (*m)->name = val;
        } else if (strcmp(key, "active") == 0) {
            SQBool val;
            sq_getbool(v, 3, &val);
            (*m)->active = val;
        }

        return 0;
    }

    static SQInteger sq_transform_get(HSQUIRRELVM v) {
        Transform** t;
        sq_getuserdata(v, 1, (SQUserPointer*)&t, nullptr);
        const SQChar* key;
        sq_getstring(v, 2, &key);

        if (strcmp(key, "position") == 0) {
            Vector3** vec = (Vector3**)sq_newuserdata(v, sizeof(Vector3*));
            *vec = &(*t)->position;
            sq_settypetag(v, -1, (SQUserPointer)"Vector3");
            return 1;
        } else if (strcmp(key, "rotation") == 0) {
            Vector3** vec = (Vector3**)sq_newuserdata(v, sizeof(Vector3*));
            *vec = &(*t)->rotation;
            sq_settypetag(v, -1, (SQUserPointer)"Vector3");
            return 1;
        } else if (strcmp(key, "scale") == 0) {
            Vector3** vec = (Vector3**)sq_newuserdata(v, sizeof(Vector3*));
            *vec = &(*t)->scale;
            sq_settypetag(v, -1, (SQUserPointer)"Vector3");
            return 1;
        }

        sq_pushnull(v);
        return 1;
    }

    static SQInteger sq_vector3_get(HSQUIRRELVM v) {
        Vector3** vec;
        sq_getuserdata(v, 1, (SQUserPointer*)&vec, nullptr);
        const SQChar* key;
        sq_getstring(v, 2, &key);

        if (strcmp(key, "x") == 0) {
            sq_pushfloat(v, (*vec)->x);
            return 1;
        } else if (strcmp(key, "y") == 0) {
            sq_pushfloat(v, (*vec)->y);
            return 1;
        } else if (strcmp(key, "z") == 0) {
            sq_pushfloat(v, (*vec)->z);
            return 1;
        }

        sq_pushnull(v);
        return 1;
    }

    static SQInteger sq_vector3_set(HSQUIRRELVM v) {
        Vector3** vec;
        sq_getuserdata(v, 1, (SQUserPointer*)&vec, nullptr);
        const SQChar* key;
        sq_getstring(v, 2, &key);
        SQFloat value;
        sq_getfloat(v, 3, &value);

        if (strcmp(key, "x") == 0) {
            (*vec)->x = value;
        } else if (strcmp(key, "y") == 0) {
            (*vec)->y = value;
        } else if (strcmp(key, "z") == 0) {
            (*vec)->z = value;
        }

        return 0;
    }

    static SQInteger sq_color_get(HSQUIRRELVM v) {
        Color** c;
        sq_getuserdata(v, 1, (SQUserPointer*)&c, nullptr);
        const SQChar* key;
        sq_getstring(v, 2, &key);

        if (strcmp(key, "r") == 0) {
            sq_pushfloat(v, (*c)->r);
            return 1;
        } else if (strcmp(key, "g") == 0) {
            sq_pushfloat(v, (*c)->g);
            return 1;
        } else if (strcmp(key, "b") == 0) {
            sq_pushfloat(v, (*c)->b);
            return 1;
        } else if (strcmp(key, "a") == 0) {
            sq_pushfloat(v, (*c)->a);
            return 1;
        }

        sq_pushnull(v);
        return 1;
    }

    static SQInteger sq_color_set(HSQUIRRELVM v) {
        Color** c;
        sq_getuserdata(v, 1, (SQUserPointer*)&c, nullptr);
        const SQChar* key;
        sq_getstring(v, 2, &key);
        SQFloat value;
        sq_getfloat(v, 3, &value);

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

    static SQInteger sq_camera_get(HSQUIRRELVM v) {
        Camera** c;
        sq_getuserdata(v, 1, (SQUserPointer*)&c, nullptr);
        const SQChar* key;
        sq_getstring(v, 2, &key);

        if (strcmp(key, "position") == 0) {
            Vector3** vec = (Vector3**)sq_newuserdata(v, sizeof(Vector3*));
            *vec = &(*c)->position;
            sq_settypetag(v, -1, (SQUserPointer)"Vector3");
            return 1;
        } else if (strcmp(key, "rotation") == 0) {
            Vector3** vec = (Vector3**)sq_newuserdata(v, sizeof(Vector3*));
            *vec = &(*c)->rotation;
            sq_settypetag(v, -1, (SQUserPointer)"Vector3");
            return 1;
        } else if (strcmp(key, "fov") == 0) {
            sq_pushfloat(v, (*c)->fov);
            return 1;
        } else if (strcmp(key, "nearPlane") == 0) {
            sq_pushfloat(v, (*c)->nearPlane);
            return 1;
        } else if (strcmp(key, "farPlane") == 0) {
            sq_pushfloat(v, (*c)->farPlane);
            return 1;
        } else if (strcmp(key, "forward") == 0) {
            sq_pushnull(v);
            return 1;
        } else if (strcmp(key, "right") == 0) {
            sq_pushnull(v);
            return 1;
        } else if (strcmp(key, "up") == 0) {
            sq_pushnull(v);
            return 1;
        }

        sq_pushnull(v);
        return 1;
    }

    static SQInteger sq_camera_set(HSQUIRRELVM v) {
        Camera** c;
        sq_getuserdata(v, 1, (SQUserPointer*)&c, nullptr);
        const SQChar* key;
        sq_getstring(v, 2, &key);

        if (strcmp(key, "fov") == 0) {
            SQFloat val;
            sq_getfloat(v, 3, &val);
            (*c)->fov = val;
        } else if (strcmp(key, "nearPlane") == 0) {
            SQFloat val;
            sq_getfloat(v, 3, &val);
            (*c)->nearPlane = val;
        } else if (strcmp(key, "farPlane") == 0) {
            SQFloat val;
            sq_getfloat(v, 3, &val);
            (*c)->farPlane = val;
        }

        return 0;
    }

    static SQInteger sq_camera_forward(HSQUIRRELVM v) {
        Camera** c;
        sq_getuserdata(v, 1, (SQUserPointer*)&c, nullptr);
        Vector3 f = (*c)->forward();
        sq_newtable(v);
        sq_pushstring(v, "x", -1);
        sq_pushfloat(v, f.x);
        sq_newslot(v, -3, SQFalse);
        sq_pushstring(v, "y", -1);
        sq_pushfloat(v, f.y);
        sq_newslot(v, -3, SQFalse);
        sq_pushstring(v, "z", -1);
        sq_pushfloat(v, f.z);
        sq_newslot(v, -3, SQFalse);
        return 1;
    }

    static SQInteger sq_camera_right(HSQUIRRELVM v) {
        Camera** c;
        sq_getuserdata(v, 1, (SQUserPointer*)&c, nullptr);
        Vector3 r = (*c)->right();
        sq_newtable(v);
        sq_pushstring(v, "x", -1);
        sq_pushfloat(v, r.x);
        sq_newslot(v, -3, SQFalse);
        sq_pushstring(v, "y", -1);
        sq_pushfloat(v, r.y);
        sq_newslot(v, -3, SQFalse);
        sq_pushstring(v, "z", -1);
        sq_pushfloat(v, r.z);
        sq_newslot(v, -3, SQFalse);
        return 1;
    }

    static SQInteger sq_camera_up(HSQUIRRELVM v) {
        Camera** c;
        sq_getuserdata(v, 1, (SQUserPointer*)&c, nullptr);
        Vector3 u = (*c)->up();
        sq_newtable(v);
        sq_pushstring(v, "x", -1);
        sq_pushfloat(v, u.x);
        sq_newslot(v, -3, SQFalse);
        sq_pushstring(v, "y", -1);
        sq_pushfloat(v, u.y);
        sq_newslot(v, -3, SQFalse);
        sq_pushstring(v, "z", -1);
        sq_pushfloat(v, u.z);
        sq_newslot(v, -3, SQFalse);
        return 1;
    }

    static SQInteger sq_scene_get(HSQUIRRELVM v) {
        Scene** s;
        sq_getuserdata(v, 1, (SQUserPointer*)&s, nullptr);
        const SQChar* key;
        sq_getstring(v, 2, &key);

        if (strcmp(key, "getCamera") == 0) {
            sq_pushnull(v);
            return 1;
        } else if (strcmp(key, "addLight") == 0) {
            sq_pushnull(v);
            return 1;
        } else if (strcmp(key, "clear") == 0) {
            sq_pushnull(v);
            return 1;
        }

        sq_pushnull(v);
        return 1;
    }

    static SQInteger sq_scene_getCamera(HSQUIRRELVM v) {
        Scene** s;
        sq_getuserdata(v, 1, (SQUserPointer*)&s, nullptr);
        Camera** c = (Camera**)sq_newuserdata(v, sizeof(Camera*));
        *c = &(*s)->camera;
        sq_settypetag(v, -1, (SQUserPointer)"Camera");
        return 1;
    }

    static SQInteger sq_scene_addLight(HSQUIRRELVM v) {
        Scene** s;
        sq_getuserdata(v, 1, (SQUserPointer*)&s, nullptr);
        Light* light;
        sq_getuserdata(v, 2, (SQUserPointer*)&light, nullptr);
        (*s)->addLight(*light);
        return 0;
    }

    static SQInteger sq_light_get(HSQUIRRELVM v) {
        Light* light;
        sq_getuserdata(v, 1, (SQUserPointer*)&light, nullptr);
        const SQChar* key;
        sq_getstring(v, 2, &key);

        if (strcmp(key, "type") == 0) {
            sq_pushinteger(v, static_cast<SQInteger>(light->type));
            return 1;
        } else if (strcmp(key, "position") == 0) {
            Vector3** vec = (Vector3**)sq_newuserdata(v, sizeof(Vector3*));
            *vec = &light->position;
            sq_settypetag(v, -1, (SQUserPointer)"Vector3");
            return 1;
        } else if (strcmp(key, "direction") == 0) {
            Vector3** vec = (Vector3**)sq_newuserdata(v, sizeof(Vector3*));
            *vec = &light->direction;
            sq_settypetag(v, -1, (SQUserPointer)"Vector3");
            return 1;
        } else if (strcmp(key, "color") == 0) {
            Color** c = (Color**)sq_newuserdata(v, sizeof(Color*));
            *c = &light->color;
            sq_settypetag(v, -1, (SQUserPointer)"Color");
            return 1;
        } else if (strcmp(key, "intensity") == 0) {
            sq_pushfloat(v, light->intensity);
            return 1;
        } else if (strcmp(key, "range") == 0) {
            sq_pushfloat(v, light->range);
            return 1;
        } else if (strcmp(key, "spotAngle") == 0) {
            sq_pushfloat(v, light->spotAngle);
            return 1;
        }

        sq_pushnull(v);
        return 1;
    }

    static SQInteger sq_light_set(HSQUIRRELVM v) {
        Light* light;
        sq_getuserdata(v, 1, (SQUserPointer*)&light, nullptr);
        const SQChar* key;
        sq_getstring(v, 2, &key);

        if (strcmp(key, "type") == 0) {
            SQInteger val;
            sq_getinteger(v, 3, &val);
            light->type = static_cast<Light::Type>(val);
        } else if (strcmp(key, "intensity") == 0) {
            SQFloat val;
            sq_getfloat(v, 3, &val);
            light->intensity = val;
        } else if (strcmp(key, "range") == 0) {
            SQFloat val;
            sq_getfloat(v, 3, &val);
            light->range = val;
        } else if (strcmp(key, "spotAngle") == 0) {
            SQFloat val;
            sq_getfloat(v, 3, &val);
            light->spotAngle = val;
        }

        return 0;
    }

    void registerFunction(const char* name, SQFUNCTION func) {
        sq_pushroottable(vm);
        sq_pushstring(vm, name, -1);
        sq_newclosure(vm, func, 0);
        sq_newslot(vm, -3, SQFalse);
        sq_pop(vm, 1);
    }

    void registerConstant(const char* name, SQInteger value) {
        sq_pushroottable(vm);
        sq_pushstring(vm, name, -1);
        sq_pushinteger(vm, value);
        sq_newslot(vm, -3, SQFalse);
        sq_pop(vm, 1);
    }

    void registerDelegate(const char* typetag, SQFUNCTION getter, SQFUNCTION setter) {
        sq_pushregistrytable(vm);
        sq_pushstring(vm, typetag, -1);
        sq_newtable(vm);

        sq_pushstring(vm, "_get", -1);
        sq_newclosure(vm, getter, 0);
        sq_newslot(vm, -3, SQFalse);

        if (setter) {
            sq_pushstring(vm, "_set", -1);
            sq_newclosure(vm, setter, 0);
            sq_newslot(vm, -3, SQFalse);
        }

        sq_newslot(vm, -3, SQFalse);
        sq_pop(vm, 1);
    }

public:
    bool initialize() override {
        vm = sq_open(1024);
        if (!vm) return false;

        instance = this;

        sq_setprintfunc(vm, printFunc, errorFunc);
        sqstd_seterrorhandlers(vm);

        sq_pushroottable(vm);
        sqstd_register_mathlib(vm);
        sqstd_register_stringlib(vm);
        sq_pop(vm, 1);

        return true;
    }

    void registerAPI() override {
        registerFunction("print", sq_print);
        registerFunction("error", sq_error);
        registerFunction("getScene", sq_getScene);
        registerFunction("getCamera", sq_getCamera);
        registerFunction("createMesh", sq_createMesh);
        registerFunction("createCube", sq_createCube);
        registerFunction("createPlane", sq_createPlane);
        registerFunction("createSphere", sq_createSphere);
        registerFunction("createLight", sq_createLight);
        registerFunction("addEntity", sq_addEntity);
        registerFunction("isKeyDown", sq_isKeyDown);
        registerFunction("isKeyPressed", sq_isKeyPressed);
        registerFunction("isKeyReleased", sq_isKeyReleased);
        registerFunction("isMouseButtonDown", sq_isMouseButtonDown);
        registerFunction("isMouseButtonPressed", sq_isMouseButtonPressed);
        registerFunction("getMousePosition", sq_getMousePosition);
        registerFunction("getMouseDelta", sq_getMouseDelta);
        registerFunction("getScrollDelta", sq_getScrollDelta);
        registerFunction("deltaTime", sq_deltaTime);
        registerFunction("totalTime", sq_totalTime);
        registerFunction("fps", sq_fps);
        registerFunction("quit", sq_quit);
        registerFunction("setWireframe", sq_setWireframe);
        registerFunction("setVSync", sq_setVSync);
        registerFunction("onUpdate", sq_onUpdate);
        registerFunction("onLateUpdate", sq_onLateUpdate);
        registerFunction("random", sq_random);
        registerFunction("clamp", sq_clamp);
        registerFunction("lerp", sq_lerp);
        registerFunction("radians", sq_radians);
        registerFunction("degrees", sq_degrees);
        registerFunction("require", sq_require);
        registerFunction("loadMap", sq_loadMap);
        registerFunction("clearScene", sq_clearScene);

        registerConstant("KEY_SPACE", static_cast<SQInteger>(KeyCode::Space));
        registerConstant("KEY_ESCAPE", static_cast<SQInteger>(KeyCode::Escape));
        registerConstant("KEY_ENTER", static_cast<SQInteger>(KeyCode::Enter));
        registerConstant("KEY_TAB", static_cast<SQInteger>(KeyCode::Tab));
        registerConstant("KEY_BACKSPACE", static_cast<SQInteger>(KeyCode::Backspace));
        registerConstant("KEY_UP", static_cast<SQInteger>(KeyCode::Up));
        registerConstant("KEY_DOWN", static_cast<SQInteger>(KeyCode::Down));
        registerConstant("KEY_LEFT", static_cast<SQInteger>(KeyCode::Left));
        registerConstant("KEY_RIGHT", static_cast<SQInteger>(KeyCode::Right));
        registerConstant("KEY_A", static_cast<SQInteger>(KeyCode::A));
        registerConstant("KEY_B", static_cast<SQInteger>(KeyCode::B));
        registerConstant("KEY_C", static_cast<SQInteger>(KeyCode::C));
        registerConstant("KEY_D", static_cast<SQInteger>(KeyCode::D));
        registerConstant("KEY_E", static_cast<SQInteger>(KeyCode::E));
        registerConstant("KEY_F", static_cast<SQInteger>(KeyCode::F));
        registerConstant("KEY_G", static_cast<SQInteger>(KeyCode::G));
        registerConstant("KEY_H", static_cast<SQInteger>(KeyCode::H));
        registerConstant("KEY_I", static_cast<SQInteger>(KeyCode::I));
        registerConstant("KEY_J", static_cast<SQInteger>(KeyCode::J));
        registerConstant("KEY_K", static_cast<SQInteger>(KeyCode::K));
        registerConstant("KEY_L", static_cast<SQInteger>(KeyCode::L));
        registerConstant("KEY_M", static_cast<SQInteger>(KeyCode::M));
        registerConstant("KEY_N", static_cast<SQInteger>(KeyCode::N));
        registerConstant("KEY_O", static_cast<SQInteger>(KeyCode::O));
        registerConstant("KEY_P", static_cast<SQInteger>(KeyCode::P));
        registerConstant("KEY_Q", static_cast<SQInteger>(KeyCode::Q));
        registerConstant("KEY_R", static_cast<SQInteger>(KeyCode::R));
        registerConstant("KEY_S", static_cast<SQInteger>(KeyCode::S));
        registerConstant("KEY_T", static_cast<SQInteger>(KeyCode::T));
        registerConstant("KEY_U", static_cast<SQInteger>(KeyCode::U));
        registerConstant("KEY_V", static_cast<SQInteger>(KeyCode::V));
        registerConstant("KEY_W", static_cast<SQInteger>(KeyCode::W));
        registerConstant("KEY_X", static_cast<SQInteger>(KeyCode::X));
        registerConstant("KEY_Y", static_cast<SQInteger>(KeyCode::Y));
        registerConstant("KEY_Z", static_cast<SQInteger>(KeyCode::Z));
        registerConstant("KEY_0", static_cast<SQInteger>(KeyCode::Num0));
        registerConstant("KEY_1", static_cast<SQInteger>(KeyCode::Num1));
        registerConstant("KEY_2", static_cast<SQInteger>(KeyCode::Num2));
        registerConstant("KEY_3", static_cast<SQInteger>(KeyCode::Num3));
        registerConstant("KEY_4", static_cast<SQInteger>(KeyCode::Num4));
        registerConstant("KEY_5", static_cast<SQInteger>(KeyCode::Num5));
        registerConstant("KEY_6", static_cast<SQInteger>(KeyCode::Num6));
        registerConstant("KEY_7", static_cast<SQInteger>(KeyCode::Num7));
        registerConstant("KEY_8", static_cast<SQInteger>(KeyCode::Num8));
        registerConstant("KEY_9", static_cast<SQInteger>(KeyCode::Num9));
        registerConstant("KEY_F1", static_cast<SQInteger>(KeyCode::F1));
        registerConstant("KEY_F2", static_cast<SQInteger>(KeyCode::F2));
        registerConstant("KEY_F3", static_cast<SQInteger>(KeyCode::F3));
        registerConstant("KEY_F4", static_cast<SQInteger>(KeyCode::F4));
        registerConstant("KEY_F5", static_cast<SQInteger>(KeyCode::F5));
        registerConstant("KEY_F6", static_cast<SQInteger>(KeyCode::F6));
        registerConstant("KEY_F7", static_cast<SQInteger>(KeyCode::F7));
        registerConstant("KEY_F8", static_cast<SQInteger>(KeyCode::F8));
        registerConstant("KEY_F9", static_cast<SQInteger>(KeyCode::F9));
        registerConstant("KEY_F10", static_cast<SQInteger>(KeyCode::F10));
        registerConstant("KEY_F11", static_cast<SQInteger>(KeyCode::F11));
        registerConstant("KEY_F12", static_cast<SQInteger>(KeyCode::F12));
        registerConstant("KEY_LSHIFT", static_cast<SQInteger>(KeyCode::LeftShift));
        registerConstant("KEY_RSHIFT", static_cast<SQInteger>(KeyCode::RightShift));
        registerConstant("KEY_LCTRL", static_cast<SQInteger>(KeyCode::LeftControl));
        registerConstant("KEY_RCTRL", static_cast<SQInteger>(KeyCode::RightControl));
        registerConstant("KEY_LALT", static_cast<SQInteger>(KeyCode::LeftAlt));
        registerConstant("KEY_RALT", static_cast<SQInteger>(KeyCode::RightAlt));

        registerConstant("MOUSE_LEFT", static_cast<SQInteger>(MouseButton::Left));
        registerConstant("MOUSE_RIGHT", static_cast<SQInteger>(MouseButton::Right));
        registerConstant("MOUSE_MIDDLE", static_cast<SQInteger>(MouseButton::Middle));

        registerConstant("LIGHT_DIRECTIONAL", static_cast<SQInteger>(Light::Type::Directional));
        registerConstant("LIGHT_POINT", static_cast<SQInteger>(Light::Type::Point));
        registerConstant("LIGHT_SPOT", static_cast<SQInteger>(Light::Type::Spot));

        registerDelegate("Mesh", sq_mesh_get, sq_mesh_set);
        registerDelegate("Transform", sq_transform_get, nullptr);
        registerDelegate("Vector3", sq_vector3_get, sq_vector3_set);
        registerDelegate("Color", sq_color_get, sq_color_set);
        registerDelegate("Camera", sq_camera_get, sq_camera_set);
        registerDelegate("Scene", sq_scene_get, nullptr);
        registerDelegate("Light", sq_light_get, sq_light_set);
    }

    bool executeFile(const std::string& filename) override {
        std::string previousFile = currentFile;
        currentFile = filename;
        if (SQ_FAILED(sqstd_dofile(vm, filename.c_str(), SQFalse, SQTrue))) {
            std::cerr << "Script error in " << filename << std::endl;
            currentFile = previousFile;
            return false;
        }
        currentFile = previousFile;
        return true;
    }

    bool executeString(const std::string& code) override {
        if (SQ_FAILED(sq_compilebuffer(vm, code.c_str(), code.length(), "string", SQTrue))) {
            std::cerr << "Script compilation error" << std::endl;
            return false;
        }
        sq_pushroottable(vm);
        if (SQ_FAILED(sq_call(vm, 1, SQFalse, SQTrue))) {
            std::cerr << "Script execution error" << std::endl;
            sq_pop(vm, 1);
            return false;
        }
        sq_pop(vm, 1);
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
        if (vm) {
            sq_close(vm);
            vm = nullptr;
        }
        instance = nullptr;
    }

    std::string getExtension() const override { return ".nut"; }

    HSQUIRRELVM getVM() { return vm; }
};

SquirrelEngine* SquirrelEngine::instance = nullptr;

}

#endif
