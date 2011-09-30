
rot = 0

camera_set_defaults()
p = entity_loadDummy()
i = world_add(p)
entity_move(i, -1.5, 0.0, -30.0)

bind('a', 'rotateLeft')
bind('d', 'rotateRight')
bind('Escape', 'quit')

set_update_callback("update")
set_move_callback("move")
set_press_callback("press")

function move(dX, dY)
	
end

function press(button, x, y)
	
end

function update(delta)
	entity_rotate(i, 0, rot, 0)
end

function rotateLeft()
	rot = rot + 0.1
end

function rotateRight()
	rot = rot - 0.1
end