generate_random_foliage = function(instancing)
    if instancing then
        debug_print("Generating random instanced foliage from Lua...")
    else
        debug_print("Generating random foliage from Lua...")
    end

    math.randomseed(os.time())
    local range = 1000

    -- Trees
    for i=0, 40 do
        local asset_id = math.random(0, 24)
        local x = math.random(-range, range)
        local z = math.random(-range, range)
        local scale = math.random(10, 80) / 100
        add_asset(71, x, 0, z, 180, 0, 0, scale, instancing)
        -- add_asset(i, (i / 10) * 100, 0, (i % 10) * 100, 1)
    end

    -- Grass and flowers
    for i=0, 200 do
        local asset_id = math.random(0, 24)
        local x = math.random(-range, range)
        local z = math.random(-range, range)
        add_asset(asset_id, x, 0, z, 180, 0, 0, 1, instancing)
    end

    -- Rocks
    for i=0, 20 do
        local asset_id = 91 --math.random(0, 24)
        local x = math.random(-range, range)
        local z = math.random(-range, range)
        local scale = math.random(1, 20) / 100
        add_asset(asset_id, x, 0, z, 180, 0, 0, scale, instancing)
    end

    -- Bushes
    for i=0, 70 do
        local asset_id = 133 --math.random(0, 24)
        local x = math.random(-range, range)
        local z = math.random(-range, range)
        local scale = 1--math.random(1, 20) / 100
        add_asset(asset_id, x, 0, z, 180, 0, 0, scale, instancing)
    end
end

load_foliage = function()
    debug_print("Loading assets from Lua...")
    clear_instance_groups()
    generate_random_foliage(true)
    --instancing_testing()
    build_instance_buffers()
end