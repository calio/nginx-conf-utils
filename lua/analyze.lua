local cjson = require("cjson")
local digest = require("digest")
local rewrite = require("rewrite")

local function usage()
    print("Usage:\n\t" .. arg[0] .. " file")
end

local function get_json_data(fname)
    local f = assert(io.open(fname))
    local d = cjson.decode(f:read("*a"))
    f:close()

    return d
end

local function transform_cmd(cmd_json)
    local key = cmd_json[1]
    if #cmd_json == 2 then
        -- key: value format
        return {
            [key] = cmd_json[2]
        }
    else
        table.remove(cmd_json, 1)
        return {
            [key] = cmd_json
        }
    end
end

local function transform_events(events_block)
    local events = {}
    assert(type(events_block) == "table")

    for i, v in ipairs(events_block) do
        events[#events + 1] = transform_cmd(v)
    end

    return events
end

local function transform_location(location_block, l)
    for i, v in ipairs(location_block) do
        local directive = v[1]

        if (directive == "internal") then
            l.internal = true
        end
    end
end

local function transform_server(server_block)
    local server = {}
    local server_names = {}
    local listens = {}
    local locations = {}
    assert(type(server_block) == "table")

    for i, v in ipairs(server_block) do
        local directive = v[1]

        if (directive == "location") then
            local l = {}
            local path
            if v[2] == "=" or v[2] == "~" or v[2] == "~*" or v[2] == "^~" then
                l.op = v[2]
                l.path = v[3]
                assert(type(v[4]) == "table")
                l.block = transform_location(v[4], l)
            else
                l.path = v[2]
                assert(type(v[3]) == "table")
                l.block = transform_location(v[3], l)
            end
            locations[#locations + 1] = l
        elseif (directive == "listen") then
            local addr = v[2]
            local args
            if #v > 2 then
                args = {}
                for i = 3, #v do
                    args[#args + 1] = v[i]
                end
            end
            listens[#listens + 1] = { addr = addr, args = args }
        elseif (directive == "server_name") then
            for i = 2, #v do
                server_names[#server_names + 1] = v[i]
            end
        else
            --server[#server + 1] = transform_cmd(v)
        end
    end

    if (#server_names > 0) then
        server.server_names = server_names
    end
    if (#listens > 0) then
        server.listens = listens
    end
    if (#locations > 0) then
        server.locations = locations
    end
    return server
end

local function transform_upstream(upstream_block)
end

local function transform_http(http_block)
    local http = {}
    local servers = {}
    local upstreams = {}
    assert(type(http_block) == "table")

    for i, v in ipairs(http_block) do
        local directive = v[1]

        if (directive == "server") then
            servers[#servers + 1] = transform_server(v[2])
        elseif (directive == "upstream") then
            upstreams[#upstreams + 1] = transform_upstream(v[2])
        else
        end

    end

    http.servers = servers
    http.upstreams = upstreams

    return http
end

-- transform JSON data get from ncfp to a better tree structure
-- events, http, globals
local function transform(ncfp_json)
    local d = {}
    local globals = {}

    for i, v in ipairs(ncfp_json) do
        local directive = v[1]

        if (directive == "events") then
            local events = transform_events(v[2])
            d.events = events
        elseif (directive == "http") then
            local http = transform_http(v[2])
            d.http = http
        else
            -- global confs
            globals[#globals + 1] = transform_cmd(v)
        end
    end

    d.globals = globals

    return d
end

local function main()
    if not arg[1] then
        usage()
        os.exit()
    end

    local d = get_json_data(arg[1])
    local tree = transform(d)

    local cmd = arg[2]
    if cmd == "digest" then
        digest.print_info(tree)
    elseif cmd =="graph" then
        print(rewrite.get_graph(tree))
    else
        --print(cjson.encode(tree))
        print(rewrite.get_graph(tree))
    end
end

main()
