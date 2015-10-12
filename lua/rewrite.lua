local _M = {}

local cjson = require("cjson")

local insert = table.insert
local format = string.format
local concat = table.concat
local encode = cjson.encode
local gsub   = string.gsub
local sub    = string.sub

local function dot_escape(s)
    s = gsub(s, "|", "\\|")
    s = gsub(s, "{", "\\{")
    s = gsub(s, "}", "\\}")
    s = gsub(s, "<", "\\<")
    s = gsub(s, ">", "\\>")
    s = gsub(s, "\"", "\\\"")

    return s
end

local function is_match(url, location)
    if #url < #location then
        return false
    end

    if sub(url, 1, #location) == location then
        return #location
    end

    return false
end

local function get_dest_location_id(locations, pattern)
    local id
    local len, min

    min = 0
    for i, v in ipairs(locations) do
        local len = is_match(pattern, v.path)
        if (len and len > min) then
            min = len
            id = v.id
        end
        --print(format("pattern %s, v.path %s, len %s", pattern, v.path, tostring(len)))
    end

    return id
end

local function gen_rewrites(t, location, locations)
    local block = location.block

    for i, v in ipairs(block) do
        if v.name == "if" then
            for i1, v1 in ipairs(v.block) do
                if v1.name == "rewrite" then
                    local name, from, to, opt = v1.name, v1.arg[1], v1.arg[2], v1.arg[3]
                    --print(name, " ", from, " ", to, " ", opt)
                    if opt == "last" then
                        local id = get_dest_location_id(locations, to)
                        if id then
                            --print("insert", location.id, id)
                            insert(t, format('location%d -> location%d [label="%s", style="bold"];\n', location.id, id, dot_escape(concat(v.condition, " "))))
                        end
                        break
                    end
                end
            end
        end
    end
end

local function gen_locations(t, locations, sid)
    if not locations then
        return
    end
    for i, v in ipairs(locations) do

        insert(t, format('location%d [label="{ %s|%s|%s}"]\n',
                v.id,
                v.internal and "internal" or "-",
                dot_escape(v.path),
                v.content_handler and v.content_handler[1] or "-"))
        if (v.content_handler) then
            insert(t, format('dest%d_%d [label="%s"]\n', sid, i,
                    dot_escape(v.content_handler[2])))
            insert(t, format('location%d -> dest%d_%d;\n', v.id, sid, i))
        end

        insert(t, format('server%d -> location%d [style=%s];\n',
                sid, v.id,
                v.internal and "dotted" or "solid"))

        gen_rewrites(t, v, locations)
    end
end

local function gen_server(t, server, i)
    local server_names, server_listens

    if type(server.server_name) == "table" then
        server_names = concat(server.server_name, "&#92;n")
    else
        server_names = "-"
    end

    server_listens = ""
    for i, v in ipairs(server.listen) do
        if (server_listens ~= "") then
            server_listens = server_listens .. "&#92;n" .. v
        else
            server_listens = v
        end
    end

    insert(t, format('server%d [label="{server|%s|%s|locations}"];\n', i,
            server_names,
            server_listens))

    gen_locations(t, server.location, i)

    insert(t, "\n")
end

local function gen_servers(t, servers)
    for i, s in ipairs(servers) do
        gen_server(t, s, i)
        -- TODO remove this
        --break
    end
end

local function digraph(t, tree)
    insert(t, "digraph structs {\n")
    insert(t, "node [shape=record];\n\n")

    gen_servers(t, tree.http.server)

    insert(t, "}\n")
end

local function get_graph(tree)
    local t = {}
    digraph(t, tree)

    return table.concat(t)
end

_M.get_graph = get_graph
return _M

