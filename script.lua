
function q()
    core.quit()
end

function update()
	entity.rotate(i, rotA, rot, 0)
	entity.move(i, 0, -30.0, posX)
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

i = entity.load("data/models/altair/altair.obj")
entity.move(i, -1.5, -30.0, -60.0)

core.bind('a', 'rotateLeft')
core.bind('d', 'rotateRight')
core.bind('w', 'rotateTop')
core.bind('s', 'rotateBottom')
core.bind('q', 'moveBack')
core.bind('e', 'moveForward')

core.bind('Escape', 'q')

update()

