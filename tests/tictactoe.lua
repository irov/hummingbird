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
    server.AllowWorldTurn(name, {0})
end

event.onCreateAvatar = function(name)
    print(name)
end 

api.join = function(data)
    local rating = server.GetCurrentUserPublicData({"rating"})

    server.JoinMatching("game", rating, {rating = rating})
end

api.update = function(data)
    local status, avatar = server.UpdateMatching("game")
    
    return {status = status, avatar = avatar}
end

avatar.turn = function(index, avatars, data)
    local m = server.GetCurrentWorldPublicData({"m"})
    
    if m[data.index] ~= 0 then
        return {successful = false}
        end
        
    m[data.index] = avatars[index]
    
    server.AllowWorldTurn(name, {index + 1 % #avatars})
    
    return {successful = true}
end