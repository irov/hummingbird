command.create = function(data)
    print(data.a, data.b, data.c)
    
    local entity = server.CreateProjectEntity("match", "", {a=1,b=2,c=3})

    local public_data = {a = 1, b = "test", e = entity}
    server.SetProjectPublicData(public_data)
    
    server.CreateMatching("game", 2, 50, {})
end

event.onEventsTopicUpdate = function(data)
    print(data.name, data.index)
    
    return {name=data.name, index=data.index, update=true}
end

event.onCreateUser = function(data)
    local entity = server.CreateUserEntity("test", "", {a=1,b=2,c=3})

    local public_data = {a = 1, b = "test", e = entity, rating = 50}
    server.SetCurrentUserPublicData(public_data)
    print(string.format("onCreateUser"))
end

event.onLoginUser = function(data)
    local a, b, e = server.GetCurrentUserPublicData({"a"})

    local update_data = {a = a + 3}
    server.UpdateCurrentUserPublicData(update_data)
    print(string.format("onLoginUser"))
end

event.onCreateWorld = function(name, pid, avatars, data)
    print("!!!onCreateWorld!!!")
    print(name)
    print(pid)
    print(json_dumps(avatars))
    print(json_dumps(data))
end

api.test = function(data)
    print(data.a, data.b, data.c)
    
    local a, b, e = server.GetCurrentUserPublicData({"a", "b", "e"})
    
    print(string.format("a = %s b = %s e = %s", a, b, e))
    
    local a, b, c = server.GetUserEntityPublicData(e, {"a", "b", "c"})

    print(string.format("a = %s b = %s c = %s", a, b, c))
    
    --[[
    local status, a, b, c = server.GetProjectEntityPublicData(e, {"a", "b", "c"})

    print(status)
    print(a)
    print(b)    
    print(c)
    ]]--
    
    return {a=a, b=b, c=c, e=e}
end

api.join = function(data)
    local rating = server.GetCurrentUserPublicData({"rating"})

    server.JoinMatching("game", rating, {rating = rating})
end