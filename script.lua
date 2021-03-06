
function q()
    core.quit()
end

function update()
    root:rotate(rotA, rot, 0)
    root:move(0, -30.0, posX)
end

function rotateLeft()
    rot = rot + 0.1
    update()
end

function rotateRight()
    rot = rot - 0.1
    update()
end

function rotateTop()
    rotA = rotA - 0.1
    update()
end

function rotateBottom()
    rotA = rotA + 0.1
    update()
end

function moveBack()
    posX = posX - 1
    update()
end

function moveForward()
    posX = posX + 1
    update()
end

function playerEnd(entity, marker)
    -- entity == player
    -- world.disable(entity)
end

rot = 0.0
rotA = 1.57079633
posX = 50.0

-- for n,v in pairs(scene_node_t) do print(n,v) end

-- root = world.addNodeDummy()
root = scene_node_t.new("Dummy Node")
obj = scene_node_t.new("Altair")

obj:loadMesh("data/models/altair/altair.obj")
obj:attachTo(root)
root:attachTo(nil)

l = root:child("Torso Hood head eyes")
l:dump()
-- root:dump()

-- obj = world.addNodeMesh("data/models/altair/altair.obj", root)
-- data/models/bob/boblampclean.md5mesh

-- world.rotate(obj, rotA, rotA * 2.0, 0)

-- world.load("basic")
-- camera.set3D()
-- player.spawnAtMarker("default")
-- world.addTriggerAtMarker(player, "endpoint", "playerEnd")


core.bind('a', 'rotateLeft')
core.bind('d', 'rotateRight')
core.bind('w', 'rotateTop')
core.bind('s', 'rotateBottom')
core.bind('q', 'moveBack')
core.bind('e', 'moveForward')

core.bind('Escape', 'q')

update()

