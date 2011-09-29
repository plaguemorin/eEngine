
set_update("update")
camera_set_defaults()

p = entity_loadDummy()
i = world_add(p)

rot = 0
move_entity(i, -1.5, 0.0, -30.0)

function update(delta)
	entity_rotate(i, 0, rot, delta / 1000.0)
	rot = rot + 0.001
end