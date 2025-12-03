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
