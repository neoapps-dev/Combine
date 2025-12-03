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
print("Loading wavy+rainbow shaders...");
local shaderLoaded = loadRendererShader("wavy", "shaders/wavy.vert", "shaders/rainbow.frag");
if (shaderLoaded) {
    print("Wavy+Rainbow shader loaded successfully!");
    useShader("wavy");
} else {
    print("Using default shader (wavy/rainbow shaders not found)");
}

local cube = createCube("ShaderTestCube");
cube.position = Vector3(0, 0.5, 0);
cube.color = Color(1.0, 0.5, 0.2, 1.0);
addEntity(cube);
local light = createLight();
light.type = Light.Type.Point;
light.position = Vector3(2, 2, 2);
light.color = Color(1.0, 1.0, 0.8, 1.0);
light.intensity = 3.0;
light.range = 10.0;
addLight(light);

print("Shader test scene loaded!");
