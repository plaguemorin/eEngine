
function q()
    core.quit()
end

function update()
	world.rotate(root, rotA, rot, 0)
	world.move(root, 0, -30.0, posX)
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

rot = 0.0
rotA = 1.57079633
posX = -50.0

root = world.addDummy(nil)

obj = world.addAnimatedMesh(root, entity.load("data/models/altair/altair.obj"))
-- md3 = world.addAnimatedMesh(root, entity.load("data/models/bob/boblampclean.md5mesh"))

-- world.move(obj, -50, 0.0, 0.0)
-- world.move(md5, 50, 0.0, 0.0)

world.rotate(obj, rotA, rotA * 2.0, 0)

core.bind('a', 'rotateLeft')
core.bind('d', 'rotateRight')
core.bind('w', 'rotateTop')
core.bind('s', 'rotateBottom')
core.bind('q', 'moveBack')
core.bind('e', 'moveForward')

core.bind('Escape', 'q')

update()

