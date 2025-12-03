print("Squirrel helper loaded!")

function createColoredCube(name, x, y, z, r, g, b) {
    local cube = createCube(name)
    cube.transform.position.x = x
    cube.transform.position.y = y
    cube.transform.position.z = z
    cube.color.r = r
    cube.color.g = g
    cube.color.b = b
    addEntity(cube)
    return cube
}

function createColoredSphere(name, x, y, z, r, g, b) {
    local sphere = createSphere(name, 16, 16)
    sphere.transform.position.x = x
    sphere.transform.position.y = y
    sphere.transform.position.z = z
    sphere.color.r = r
    sphere.color.g = g
    sphere.color.b = b
    addEntity(sphere)
    return sphere
}

function createColoredPlane(name, x, y, z, r, g, b, w, h) {
    local plane = createPlane(name, w, h)
    plane.transform.position.x = x
    plane.transform.position.y = y
    plane.transform.position.z = z
    plane.color.r = r
    plane.color.g = g
    plane.color.b = b
    addEntity(plane)
    return plane
}

print("Squirrel helper functions registered!")
