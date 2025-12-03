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
