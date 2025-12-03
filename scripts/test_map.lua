--[[
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
--]]

print("Loading test map in Lua...")
if loadMap("test.comap") then
    print("Map loaded successfully!")
    local scene = getScene()
    local player = scene:getEntityByName("Player")
    if player then
        print("Found player entity")
    end
    print("Total entities in scene: " .. #scene.entities)
    print("Total lights in scene: " .. #scene.lights)

else
    error("Failed to load map!")
end
onUpdate(function(dt)
    local scene = getScene()
    local orb = scene:getEntityByName("Orb")
    if orb then
        orb.transform.rotation.y = orb.transform.rotation.y + dt * 90.0
        orb.transform.position.y = 1.0 + math.sin(totalTime() * 2.0) * 0.2
    end
end)

print("Lua script initialized!")
