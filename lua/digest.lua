local _M = {}
-- generate digest information from conf tree
local function print_info(tree)
    local servers = tree.http.servers

    local nservers = #servers

    for _, s in ipairs(servers) do
        print("\nserver names:")
        if (s.server_names) then
            for i, v in ipairs(s.server_names) do
                print("  " .. v)
            end
        end
        if (s.listens) then
            print("listens:")
            for i, v in ipairs(s.listens) do
                print("  " .. v.addr)
            end
        end

        if (s.locations) then
            print("location:")
            for _, l in ipairs(s.locations) do
                print("  " .. (l.op and l.op .. " " or "") .. l.path)
            end
        end
    end
end
_M.print_info = print_info

return _M
