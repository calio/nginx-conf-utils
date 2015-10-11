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

        insert(t, format('location%d_%d [label="{ %s|%s|%s}"]\n',
                sid, i,
                v.internal and "internal" or "-",
                dot_escape(v.path),
                v.content_handler and v.content_handler[1] or "-"))
        if (v.content_handler) then
            insert(t, format('dest%d_%d [label="%s"]\n', sid, i,
                    dot_escape(v.content_handler[2])))
            insert(t, format('location%d_%d -> dest%d_%d;\n', sid, i, sid, i))
        end

        insert(t, format('server%d -> location%d_%d [style=%s];\n',
                sid, sid, i,
                v.internal and "dotted" or "solid"))
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

