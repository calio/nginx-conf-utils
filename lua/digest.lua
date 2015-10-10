local _M = {}
-- generate digest information from conf tree
local function print_info(tree)
    local servers = tree.http.servers

    local nservers = #servers

    for _, s in ipairs(servers) do
        print("\nserver names:")
        if (s.server_name) then
            for i, v in ipairs(s.server_name) do
                print("  " .. v)
            end
        end
        if (s.listen) then
            print("listen:")
            for i, v in ipairs(s.listen) do
                print("  " .. v.addr)
            end
        end

        if (s.location) then
            print("location:")
            for _, l in ipairs(s.location) do
                print("  " .. l)
            end
        end
    end
end
_M.print_info = print_info

return _M
