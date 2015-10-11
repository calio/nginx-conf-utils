local cjson = require("cjson")
local digest = require("digest")
local rewrite = require("rewrite")

local insert = table.insert

local trans_func = {}


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
            name = key,
            arg = cmd_json[2]
        }
    else
        table.remove(cmd_json, 1)
        return {
            name = key,
            arg = cmd_json
        }
    end
end

local function transform(items, node, parent)

    for i, v in ipairs(items) do
        local directive = v[1]

        local trans = trans_func[directive]
        if (trans) then
            local func = trans.func
            if trans.multi then
                if not node[directive] then
                    node[directive] = {}
                end
                node[directive][#node[directive] + 1] = func(v, {}, node)
            else
                node[directive] = func(v, {}, node)
            end
        else
            if not node.block then
                node.block = {}
            end
            node.block[#node.block + 1] = transform_cmd(v)
        end
    end

    return node
end

local function transform_events(items, node)
    node.name = items[1]
    return transform(items[2], node)
end

local function transform_http(items, node)
    node.name = items[1]
    return transform(items[2], node)
end

local function transform_server(items, node)
    node.name = items[1]
    return transform(items[2], node)
end

local function transform_location(items, node)
    node.name = items[1]

    local v = items
    if v[2] == "=" or v[2] == "~" or v[2] == "~*" or v[2] == "^~" then
        node.op = v[2]
        node.path = v[3]

        return transform(v[4], node)
    else
        node.path = v[2]

        return transform(v[3], node)
    end
end

local function transform_listen(items, node)
    return items[2]
end

local function transform_server_name(items, node)
    return items[2]
end

local function transform_content_handler(items, node, parent)
    parent.content_handler = items

    if not parent.block then
        parent.block = {}
    end

    insert(parent.block, transform_cmd(items))
end


trans_func.events = { func = transform_events, multi = false }
trans_func.http = { func = transform_http, multi = false }
trans_func.server = { func = transform_server, multi = true }
trans_func.location = { func = transform_location, multi = true }
trans_func.listen = { func = transform_listen, multi = true }
trans_func.server_name = { func = transform_server_name, multi = true }
trans_func.proxy_pass = { func = transform_content_handler, multi = false }
trans_func.content_by_lua = { func = transform_content_handler, multi = false }
trans_func.content_by_lua_file = { func = transform_content_handler,
        multi = false }
trans_func.root = { func = transform_content_handler, multi = false }

local function main()
    if not arg[1] then
        usage()
        os.exit()
    end

    local d = get_json_data(arg[1])
    local tree = transform(d, {})

    local cmd = arg[2]
    if cmd == "digest" then
        digest.print_info(tree)
    elseif cmd == "graph" then
        print(rewrite.get_graph(tree))
    elseif cmd == "json" then
        print(cjson.encode(tree))
    else
        print(rewrite.get_graph(tree))
    end
end

main()
