command.create = function(data)
    local m=[
        0,0,0, 
        0,0,0, 
        0,0,0
        ]
        
    server.CreateMatching("game", 2, 50, {m=m})
end

event.onCreateWorld = function(name, avatars, data)
    print("!!!onCreateWorld!!!")
    print(name)
    print(json_dumps(avatars))
    print(json_dumps(data))
    local r = math.random()
    local id = r % #avatars
    
    server.SetAllowWorldTurn(name, {ids = {id}})
end

api.join = function(data)
    local rating = server.GetCurrentUserPublicData({"rating"})

    server.JoinMatching("game", rating, {rating = rating})
end

api.update = function(data)
    local code, avatar = server.GetStatusMatching("game")
    
    return {code = code, avatar = avatar}
end

avatar.turn = function(id, avatars, data)
    local m = server.GetCurrentWorldPublicData({"m"})
    
    if m[data.index] ~= 0 then
        return {successful = false}
        end
        
    m[data.index] = avatars[id]
    
    local next_id = (id + 1) % #avatars
    
    server.SetAllowWorldTurn(name, {ids = {next_id}})
    
    return {successful = true}
end