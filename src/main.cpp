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

#include "CombineEngine.h"
#include "generated/BuildConfig.h"
#include COMBINE_RENDERER_HEADER
#ifdef COMBINE_SCRIPT_CHAISCRIPT
#include "scripting/ChaiScriptEngine.h"
#endif
#ifndef COMBINE_WINDOW_TITLE
#define COMBINE_WINDOW_TITLE "Combine Engine"
#endif

#ifdef COMBINE_SCRIPT_LUA
#include "scripting/LuaEngine.h"
#endif

#ifdef COMBINE_SCRIPT_SQUIRREL
#include "scripting/SquirrelEngine.h"
#endif

#include <iostream>
#include <memory>

namespace Combine {
    Engine* g_engine = nullptr;
}

int main() {
    std::cout << "~~> Combine Engine" << std::endl;
    Combine::Engine engine;
    Combine::g_engine = &engine;
    engine.setRenderer(std::make_unique<Combine::COMBINE_RENDERER_CLASS>());

#ifdef COMBINE_SCRIPT_CHAISCRIPT
    engine.addScriptEngine(std::make_unique<Combine::ChaiScriptEngine>());
#endif

#ifdef COMBINE_SCRIPT_LUA
    engine.addScriptEngine(std::make_unique<Combine::LuaEngine>());
#endif

#ifdef COMBINE_SCRIPT_SQUIRREL
    engine.addScriptEngine(std::make_unique<Combine::SquirrelEngine>());
#endif

    if (!engine.initialize(1280, 720, COMBINE_WINDOW_TITLE)) {
        std::cerr << "Failed to initialize engine" << std::endl;
        return -1;
    }

    engine.executeScript(COMBINE_INIT_SCRIPT);
    engine.run();
    engine.shutdown();
    return 0;
}
