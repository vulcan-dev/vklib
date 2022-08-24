local sql = require("sqlite_helper")
local db = sql.open("sqlite.db")

local function player_exists(db, name)
    local query = string.format("SELECT name FROM players WHERE name='%s'", name)
    local result, err = sql.execute(db, query)
    if err then
        error(err)
    end
    
    return result ~= nil
end

local function add_player(db, name)
    local exists, err = sql.execute(db, string.format("SELECT name FROM players WHERE name='%s'", name))
    if err then
        error(err)
    end

    if exists then
        return
    end

    local query = string.format([[INSERT INTO players (
        name,
        playtime,
        kills,
        deaths,
        score
    ) VALUES (
        '%s',
        0,
        0,
        0,
        0
    )]], name)
    local _, err = sql.execute(db, query)
    if err then
        error(err)
    end
end

local function update_player(db, name, playtime, kills, deaths, score)
    local query = string.format([[UPDATE players SET
        playtime = %d,
        kills = %d,
        deaths = %d,
        score = %d
    WHERE name = '%s']], playtime, kills, deaths, score, name)
    local _, err = sql.execute(db, query)
    if err then
        error(err)
    end
end

local function get_player_full_info(db, name)
    local query = string.format("SELECT * FROM players WHERE name='%s'", name)
    local result, err = sql.execute(db, query)
    if err then
        error(err)
    end

    local player = result[1]

    local output = string.format([[
Name: %s
    Playtime: %d
    Kills: %d
    Deaths: %d
    Score: %d
    ]], player.name, player.playtime, player.kills, player.deaths, player.score)
    
    return output
end

if not player_exists(db, "John Doe") then
    add_player(db, "John Doe")
else
    update_player(db, "John Doe", 4819210, 69, 2, 69420)
    print(get_player_full_info(db, "John Doe"))
end

if not sql.table_exists(db, "players") then
    print("Creating table players")
    local _, err = sql.execute(db, [[CREATE TABLE players (
        id INTEGER PRIMARY KEY,
        name TEXT,
        playtime INTEGER,
        kills INTEGER,
        deaths INTEGER,
        score INTEGER
    )]])

    if err then
        print(err)
        return
    end
end

sql.close(db)