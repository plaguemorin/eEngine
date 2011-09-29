
set_update("update")
camera_set_defaults()

p = entity_loadDummy()
i = world_add(p)

rot = 0
move_entity(i, -1.5, 0.0, -6.0)


function update(delta)
	entity_rotate(i, 0, rot, 0)
	rot = rot + 0.01
end