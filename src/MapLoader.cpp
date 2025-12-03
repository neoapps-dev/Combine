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

#include "../include/MapLoader.h"
#include <iostream>
#include <algorithm>
#include <memory>

namespace Combine {
std::shared_ptr<MapData> MapLoader::loadMap(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open map file: " << filename << std::endl;
        return nullptr;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return parseComapFile(buffer.str());
}

bool MapLoader::saveMap(const std::shared_ptr<MapData>& map, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to create map file: " << filename << std::endl;
        return false;
    }

    file << serializeMap(map);
    file.close();
    return true;
}

void MapLoader::loadMapIntoScene(const std::shared_ptr<MapData>& map, Scene* scene) {
    if (!scene || !map) return;
    clearScene(scene);
    scene->ambientColor = map->ambientColor;
    scene->camera.position = map->cameraPosition;
    scene->camera.rotation = map->cameraRotation;
    scene->camera.fov = map->cameraFov;
    for (const auto& mapLight : map->lights) {
        Light light;
        light.type = mapLight.type;
        light.position = mapLight.position;
        light.direction = mapLight.direction;
        light.color = mapLight.color;
        light.intensity = mapLight.intensity;
        light.range = mapLight.range;
        light.spotAngle = mapLight.spotAngle;
        scene->addLight(light);
    }

    for (const auto& mapObj : map->objects) {
        std::shared_ptr<Mesh> mesh;
        if (mapObj.type == "cube") {
            mesh = Mesh::createCube(mapObj.name);
        } else if (mapObj.type == "plane") {
            mesh = Mesh::createPlane(mapObj.name);
        } else if (mapObj.type == "sphere") {
            mesh = Mesh::createSphere(mapObj.name);
        } else {
            mesh = Mesh::createCube(mapObj.name);
        }

        mesh->transform.position = mapObj.position;
        mesh->transform.rotation = mapObj.rotation;
        mesh->transform.scale = mapObj.scale;
        mesh->name = mapObj.name;
        mesh->tag = mapObj.tag;
        mesh->color = mapObj.color;
        if (!mapObj.texture.empty()) {
            mesh->texturePath = mapObj.texture;
        }

        scene->addEntity(mesh);
    }
}

void MapLoader::clearScene(Scene* scene) {
    if (!scene) return;
    scene->clear();
}

std::shared_ptr<MapData> MapLoader::parseComapFile(const std::string& content) {
    auto map = std::make_shared<MapData>();
    std::istringstream stream(content);
    std::string line;
    MapObject* currentObject = nullptr;
    MapLight* currentLight = nullptr;
    while (std::getline(stream, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        size_t pos = line.find(':');
        if (pos == std::string::npos) continue;
        std::string key = trim(line.substr(0, pos));
        std::string value = trim(line.substr(pos + 1));
        if (key == "version") {
            map->version = value;
        } else if (key == "name") {
            map->name = value;
        } else if (key == "ambientColor") {
            map->ambientColor = parseColor(value);
        } else if (key == "cameraPosition") {
            map->cameraPosition = parseVector3(value);
        } else if (key == "cameraRotation") {
            map->cameraRotation = parseVector3(value);
        } else if (key == "cameraFov") {
            map->cameraFov = std::stof(value);
        } else if (key == "object") {
            map->objects.push_back(MapObject());
            currentObject = &map->objects.back();
            currentLight = nullptr;
            currentObject->type = value;
        } else if (key == "light") {
            map->lights.push_back(MapLight());
            currentLight = &map->lights.back();
            currentObject = nullptr;
            currentLight->type = parseLightType(value);
        } else if (currentObject) {
            if (key == "name") currentObject->name = value;
            else if (key == "tag") currentObject->tag = value;
            else if (key == "position") currentObject->position = parseVector3(value);
            else if (key == "rotation") currentObject->rotation = parseVector3(value);
            else if (key == "scale") currentObject->scale = parseVector3(value);
            else if (key == "color") currentObject->color = parseColor(value);
            else if (key == "mesh") currentObject->mesh = value;
            else if (key == "texture") currentObject->texture = value;
            else {
                currentObject->properties.push_back({key, value});
            }
        } else if (currentLight) {
            if (key == "position") currentLight->position = parseVector3(value);
            else if (key == "direction") currentLight->direction = parseVector3(value);
            else if (key == "color") currentLight->color = parseColor(value);
            else if (key == "intensity") currentLight->intensity = std::stof(value);
            else if (key == "range") currentLight->range = std::stof(value);
            else if (key == "spotAngle") currentLight->spotAngle = std::stof(value);
        }
    }

    return map;
}

std::string MapLoader::serializeMap(const std::shared_ptr<MapData>& map) {
    std::stringstream ss;
    ss << "# Combine Map File\n";
    ss << "version: " << map->version << "\n";
    ss << "name: " << map->name << "\n";
    ss << "ambientColor: " << map->ambientColor.r << "," << map->ambientColor.g << ","
       << map->ambientColor.b << "," << map->ambientColor.a << "\n";
    ss << "cameraPosition: " << map->cameraPosition.x << "," << map->cameraPosition.y << ","
       << map->cameraPosition.z << "\n";
    ss << "cameraRotation: " << map->cameraRotation.x << "," << map->cameraRotation.y << ","
       << map->cameraRotation.z << "\n";
    ss << "cameraFov: " << map->cameraFov << "\n\n";
    for (const auto& light : map->lights) {
        ss << "light: ";
        switch (light.type) {
            case Light::Type::Directional: ss << "directional"; break;
            case Light::Type::Point: ss << "point"; break;
            case Light::Type::Spot: ss << "spot"; break;
        }
        ss << "\n";
        ss << "  position: " << light.position.x << "," << light.position.y << "," << light.position.z << "\n";
        ss << "  direction: " << light.direction.x << "," << light.direction.y << "," << light.direction.z << "\n";
        ss << "  color: " << light.color.r << "," << light.color.g << "," << light.color.b << "," << light.color.a << "\n";
        ss << "  intensity: " << light.intensity << "\n";
        ss << "  range: " << light.range << "\n";
        ss << "  spotAngle: " << light.spotAngle << "\n\n";
    }

    for (const auto& obj : map->objects) {
        ss << "object: " << obj.type << "\n";
        ss << "  name: " << obj.name << "\n";
        ss << "  tag: " << obj.tag << "\n";
        ss << "  position: " << obj.position.x << "," << obj.position.y << "," << obj.position.z << "\n";
        ss << "  rotation: " << obj.rotation.x << "," << obj.rotation.y << "," << obj.rotation.z << "\n";
        ss << "  scale: " << obj.scale.x << "," << obj.scale.y << "," << obj.scale.z << "\n";
        ss << "  color: " << obj.color.r << "," << obj.color.g << "," << obj.color.b << "," << obj.color.a << "\n";
        if (!obj.mesh.empty()) ss << "  mesh: " << obj.mesh << "\n";
        if (!obj.texture.empty()) ss << "  texture: " << obj.texture << "\n";
        for (const auto& prop : obj.properties) {
            ss << "  " << prop.first << ": " << prop.second << "\n";
        }
        ss << "\n";
    }

    return ss.str();
}

Vector3 MapLoader::parseVector3(const std::string& str) {
    std::vector<std::string> parts = split(str, ',');
    Vector3 result(0, 0, 0);
    if (parts.size() >= 1) result.x = std::stof(trim(parts[0]));
    if (parts.size() >= 2) result.y = std::stof(trim(parts[1]));
    if (parts.size() >= 3) result.z = std::stof(trim(parts[2]));
    return result;
}

Color MapLoader::parseColor(const std::string& str) {
    std::vector<std::string> parts = split(str, ',');
    Color result(1, 1, 1, 1);
    if (parts.size() >= 1) result.r = std::stof(trim(parts[0]));
    if (parts.size() >= 2) result.g = std::stof(trim(parts[1]));
    if (parts.size() >= 3) result.b = std::stof(trim(parts[2]));
    if (parts.size() >= 4) result.a = std::stof(trim(parts[3]));
    return result;
}

Light::Type MapLoader::parseLightType(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower == "directional") return Light::Type::Directional;
    if (lower == "point") return Light::Type::Point;
    if (lower == "spot") return Light::Type::Spot;
    return Light::Type::Directional;
}

std::string MapLoader::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

std::vector<std::string> MapLoader::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

}
