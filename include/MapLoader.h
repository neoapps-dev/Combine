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

#ifndef MAP_LOADER_H
#define MAP_LOADER_H
#include "CombineEngine.h"
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <sstream>
namespace Combine {
struct MapObject {
    std::string type;
    std::string name;
    std::string tag;
    Vector3 position;
    Vector3 rotation;
    Vector3 scale;
    Color color;
    std::string mesh;
    std::string texture;
    std::vector<std::pair<std::string, std::string>> properties;
    MapObject() : position(0, 0, 0), rotation(0, 0, 0), scale(1, 1, 1), color(Color::white()) {}
};

struct MapLight {
    Light::Type type;
    Vector3 position;
    Vector3 direction;
    Color color;
    float intensity;
    float range;
    float spotAngle;
    MapLight() : type(Light::Type::Directional), position(0, 0, 0), direction(0, -1, 0),
                 color(Color::white()), intensity(1.0f), range(10.0f), spotAngle(45.0f) {}
};

struct MapData {
    std::string version;
    std::string name;
    Color ambientColor;
    Vector3 cameraPosition;
    Vector3 cameraRotation;
    float cameraFov;
    std::vector<MapObject> objects;
    std::vector<MapLight> lights;
    MapData() : version("1.0"), name("Untitled Map"), ambientColor(0.2f, 0.2f, 0.2f, 1.0f),
                cameraPosition(0, 0, 3), cameraRotation(0, 0, 0), cameraFov(60.0f) {}
};

class MapLoader {
public:
    static std::shared_ptr<MapData> loadMap(const std::string& filename);
    static bool saveMap(const std::shared_ptr<MapData>& map, const std::string& filename);
    static void loadMapIntoScene(const std::shared_ptr<MapData>& map, Scene* scene);
    static void clearScene(Scene* scene);

private:
    static std::shared_ptr<MapData> parseComapFile(const std::string& content);
    static std::string serializeMap(const std::shared_ptr<MapData>& map);
    static Vector3 parseVector3(const std::string& str);
    static Color parseColor(const std::string& str);
    static Light::Type parseLightType(const std::string& str);
    static std::string trim(const std::string& str);
    static std::vector<std::string> split(const std::string& str, char delimiter);
};

}

#endif
