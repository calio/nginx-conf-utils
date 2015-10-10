local _M = {}

local cjson = require("cjson")

local insert = table.insert
local format = string.format
local concat = table.concat
local encode = cjson.encode
local gsub   = string.gsub

local function dot_escape(s)
    s = gsub(s, "|", "\\|")
    s = gsub(s, "{", "\\{")
    s = gsub(s, "}", "\\}")
    s = gsub(s, "<", "\\<")
    s = gsub(s, ">", "\\>")

    return s
end

local function gen_locations(t, locations, sid)
    for i, v in ipairs(locations) do
        insert(t, format('location%d_%d [label="{ %s }"]\n', sid, i,
                dot_escape(v)))
        insert(t, format('server%d -> location%d_%d [style=dotted];\n',
                sid, sid, i))
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
            server_listens = server_listens .. "&#92;n" .. v.addr
        else
            server_listens = v.addr
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
        break
    end
end

local function digraph(t, tree)
    insert(t, "digraph structs {\n")
    insert(t, "node [shape=record];\n\n")

    gen_servers(t, tree.http.servers)

    insert(t, "}\n")
end

local function get_graph(tree)
    local t = {}
    digraph(t, tree)

    return table.concat(t)
end

_M.get_graph = get_graph
return _M

