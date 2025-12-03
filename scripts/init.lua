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
print("Combine Engine - Lua Initialized!")

local scene = getScene()
local cam = getCamera()

cam.position.x = 0.0
cam.position.y = 3.0
cam.position.z = 8.0
cam.rotation.x = 20.0
cam.rotation.y = 0.0
cam.fov = 60.0

local scriptedCube = createCube("ScriptedCube")
scriptedCube.transform.position.x = 0.0
scriptedCube.transform.position.y = 2.5
scriptedCube.transform.position.z = 0.0
scriptedCube.transform.scale.x = 0.5
scriptedCube.transform.scale.y = 0.5
scriptedCube.transform.scale.z = 0.5
scriptedCube.color.r = 1.0
scriptedCube.color.g = 0.5
scriptedCube.color.b = 0.0
addEntity(scriptedCube)

local orbitSphere = createSphere("OrbitSphere", 16, 16)
orbitSphere.transform.scale.x = 0.3
orbitSphere.transform.scale.y = 0.3
orbitSphere.transform.scale.z = 0.3
orbitSphere.color.r = 0.0
orbitSphere.color.g = 1.0
orbitSphere.color.b = 0.5
addEntity(orbitSphere)

local secondOrbit = createCube("OrbitCube")
secondOrbit.transform.scale.x = 0.25
secondOrbit.transform.scale.y = 0.25
secondOrbit.transform.scale.z = 0.25
secondOrbit.color.r = 1.0
secondOrbit.color.g = 0.2
secondOrbit.color.b = 0.8
addEntity(secondOrbit)

local angle = 0.0
local orbitRadius = 2.0
local orbitSpeed = 2.0
local secondAngle = 0.0
local moveSpeed = 5.0
local targetCamY = 0.0
local targetCamX = 20.0

onUpdate(function(dt)
	angle = angle + orbitSpeed * dt
	secondAngle = secondAngle + orbitSpeed * 1.5 * dt

	orbitSphere.transform.position.x = math.cos(angle) * orbitRadius
	orbitSphere.transform.position.y = 2.0
	orbitSphere.transform.position.z = math.sin(angle) * orbitRadius

	secondOrbit.transform.position.x = math.cos(secondAngle) * (orbitRadius * 0.6)
	secondOrbit.transform.position.y = math.sin(secondAngle * 2.0) * 0.5 + 2.0
	secondOrbit.transform.position.z = math.sin(secondAngle) * (orbitRadius * 0.6)
	secondOrbit.transform.rotation.x = secondAngle * 100.0
	secondOrbit.transform.rotation.y = secondAngle * 150.0

	scriptedCube.transform.rotation.y = scriptedCube.transform.rotation.y + 60.0 * dt
	scriptedCube.transform.rotation.x = math.sin(totalTime() * 2.0) * 15.0

	local pulse = (math.sin(totalTime() * 3.0) + 1.0) * 0.5
	scriptedCube.color.r = 1.0
	scriptedCube.color.g = 0.3 + pulse * 0.4
	scriptedCube.color.b = pulse * 0.3

	local forward = cam:forward()
	local right = cam:right()

	if isKeyDown(KEY_W) then
		cam.position.x = cam.position.x + forward.x * moveSpeed * dt
		cam.position.y = cam.position.y + forward.y * moveSpeed * dt
		cam.position.z = cam.position.z + forward.z * moveSpeed * dt
	end
	if isKeyDown(KEY_S) then
		cam.position.x = cam.position.x - forward.x * moveSpeed * dt
		cam.position.y = cam.position.y - forward.y * moveSpeed * dt
		cam.position.z = cam.position.z - forward.z * moveSpeed * dt
	end
	if isKeyDown(KEY_A) then
		cam.position.x = cam.position.x - right.x * moveSpeed * dt
		cam.position.z = cam.position.z - right.z * moveSpeed * dt
	end
	if isKeyDown(KEY_D) then
		cam.position.x = cam.position.x + right.x * moveSpeed * dt
		cam.position.z = cam.position.z + right.z * moveSpeed * dt
	end
	if isKeyDown(KEY_SPACE) then
		cam.position.y = cam.position.y + moveSpeed * dt
	end
	if isKeyDown(KEY_LSHIFT) then
		cam.position.y = cam.position.y - moveSpeed * dt
	end

	if isKeyDown(KEY_LEFT) then
		targetCamY = targetCamY - 90.0 * dt
	end
	if isKeyDown(KEY_RIGHT) then
		targetCamY = targetCamY + 90.0 * dt
	end
	if isKeyDown(KEY_UP) then
		targetCamX = targetCamX - 60.0 * dt
	end
	if isKeyDown(KEY_DOWN) then
		targetCamX = targetCamX + 60.0 * dt
	end

	if targetCamX < -89.0 then
		targetCamX = -89.0
	end
	if targetCamX > 89.0 then
		targetCamX = 89.0
	end

	cam.rotation.y = cam.rotation.y + (targetCamY - cam.rotation.y) * 10.0 * dt
	cam.rotation.x = cam.rotation.x + (targetCamX - cam.rotation.x) * 10.0 * dt

	if isKeyPressed(KEY_F) then
		setWireframe(true)
		print("Wireframe ON")
	end
	if isKeyPressed(KEY_G) then
		setWireframe(false)
		print("Wireframe OFF")
	end

	if isKeyPressed(KEY_ESCAPE) then
		print("Exiting...")
		quit()
	end

	if isKeyPressed(KEY_R) then
		cam.position.x = 0.0
		cam.position.y = 3.0
		cam.position.z = 8.0
		cam.rotation.x = 20.0
		cam.rotation.y = 0.0
		targetCamY = 0.0
		targetCamX = 20.0
		print("Camera reset")
	end
end)

print("Scene setup complete!")
print("Controls: WASD move, Arrows look, Space/Shift up/down, F/G wireframe, R reset, ESC quit")
