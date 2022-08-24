local vklib = assert(package.loadlib("build/bin/vklib.dll", "luaopen_vklib"))()

local M = {}

local function delete_table(db, table_name)
    local query = string.format("DELETE FROM %s", table_name)
    vklib.sqlite.exec(db, query)
end

local function table_exists(db, table_name)
    local query = string.format("SELECT name FROM sqlite_master WHERE type='table' AND name='%s'", table_name)
    local result, err = vklib.sqlite.exec(db, query)
    if err then
        error(err)
    end
    
    return result ~= nil
end

local function create_table(db, table_name, fields)
    local query = string.format("CREATE TABLE %s (%s)", table_name, fields)
    local success, err = vklib.sqlite.exec(db, query)
    return success, err
end

local function open(name)
    local db = vklib.sqlite.open(name)
    return db
end

local function close(db)
    vklib.sqlite.close(db)
end

local function execute(db, query)
    return vklib.sqlite.exec(db, query)
end

M.open = open
M.close = close
M.execute = execute

M.delete_table = delete_table
M.table_exists = table_exists
M.create_table = create_table

return M