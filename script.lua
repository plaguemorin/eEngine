
rot = 0
rotA = 0
posX = -90.0

camera_set_defaults()
p = entity_load("data/models/bob/boblampclean.md5mesh")
i = world_add(p)
entity_move(i, -1.5, -30.0, -60.0)

bind('a', 'rotateLeft')
bind('d', 'rotateRight')
bind('w', 'rotateTop')
bind('s', 'rotateBottom')
bind('q', 'moveBack')
bind('e', 'moveForward')

bind('Escape', 'quit')

set_update_callback("update")
set_move_callback("move")
set_press_callback("press")

function move(dX, dY)
	
end

function press(button, x, y)
	
end

function update(delta)
	entity_rotate(i, rotA, rot, 0)
	entity_move(i, 0, -30.0, posX)
end

function rotateLeft()
	rot = rot + 0.1
end

function rotateRight()
	rot = rot - 0.1
end

function rotateTop()
	rotA = rotA - 0.1
end

function rotateBottom()
	rotA = rotA + 0.1
end

function moveBack()
	posX = posX - 0.1
end

function moveForward()
	posX = posX + 0.1
end
