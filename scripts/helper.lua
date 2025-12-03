print("Lua helper loaded!")

print("Loading Squirrel helper from Lua...")
require("scripts/helper.nut")
print("Squirrel helper loaded from Lua!")

function createColoredCube(name, x, y, z, r, g, b)
    local cube = createCube(name)
    cube.transform.position.x = x
    cube.transform.position.y = y
    cube.transform.position.z = z
    cube.color.r = r
    cube.color.g = g
    cube.color.b = b
    addEntity(cube)
    return cube
end

function createColoredSphere(name, x, y, z, r, g, b)
    local sphere = createSphere(name, 16, 16)
    sphere.transform.position.x = x
    sphere.transform.position.y = y
    sphere.transform.position.z = z
    sphere.color.r = r
    sphere.color.g = g
    sphere.color.b = b
    addEntity(sphere)
    return sphere
end

print("Lua helper functions registered!")
