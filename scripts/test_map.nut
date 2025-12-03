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
print("Loading test map in Squirrel...");
if (loadMap("test.comap")) {
    print("Map loaded successfully!");
    local scene = getScene();
    local player = scene.getEntityByName("Player");
    if (player != null) {
        print("Found player entity");
    }

    print("Total entities in scene: " + scene.entities.len());
    print("Total lights in scene: " + scene.lights.len());

} else {
    error("Failed to load map!");
}

onUpdate(function(dt) {
    local scene = getScene();
    local orb = scene.getEntityByName("Orb");
    if (orb != null) {
        orb.transform.rotation.y += dt * 90.0;
        orb.transform.position.y = 1.0 + sin(totalTime() * 2.0) * 0.2;
    }
});

print("Squirrel script initialized!");
