package.path = mud.luadir() .. "?.lua"

glob_tprintstr=require "tprint"
require "serialize"
glob_util=require "utilities"
require "leaderboard"
require "commands"
require "changelog"
Queue = require "Queue"

envtbl={} -- game object script environments
interptbl={} -- key is game object pointer, table of desc=desc pointer, name=char name
delaytbl={} -- used on the C side mostly

validuds={}
shared_tbls={}

function UdCnt()
    local reg=debug.getregistry()

    local cnt=0
    for k,v in pairs(reg) do
        if type(v)=="userdata" then
            cnt=cnt+1
        end
    end

    return cnt
end

function EnvCnt()
    local cnt=0
    for k,v in pairs(envtbl) do
        cnt=cnt+1
    end
    return cnt
end

function UnregisterDesc(desc)
    for k,v in pairs(interptbl) do
        if v.desc==desc then
            interptbl[k]=nil
        end
    end
end

function glob_rand(pcnt)
    return ( (mt.rand()*100) < pcnt)
end

function glob_randnum(low, high)
    return math.floor( (mt.rand()*(high+1-low) + low)) -- people usually want inclusive
end

function glob_awardptitle( ch, ptitle)
    if not ch.ispc then
        error("Can't add ptitles to NPCs.")
    end

    if string.find(ptitle, "[^a-zA-Z\.]") then
        error("Ptitles can only contain letters and '.'")
    end

    if not ch.ptitles then 
        forceset(ch, "ptitles",{})
    end

    local lst=ch.ptitles

    -- If already has it, do nothing
    for k,v in pairs(lst) do
        if v==ptitle then return end
    end

    table.insert(lst, ptitle)
    table.sort(lst)
end

function SaveTable( name, tbl, areaFname )
  if string.find(name, "[^a-zA-Z0-9_]") then
    error("Invalid character in name.")
  end

  local dir=string.match(areaFname, "(%w+)\.are")
  if not os.rename(dir, dir) then
    os.execute("mkdir '" .. dir .. "'")
  end
  local f=io.open( dir .. "/" .. name .. ".lua", "w")
  out,saved=serialize.save(name,tbl)
  f:write(out)

  f:close()
end

function linenumber( text )
    local cnt=1
    local rtn={}

    for line in text:gmatch(".-\n\r?") do
        table.insert(rtn, string.format("%3d. %s", cnt, line))
        cnt=cnt+1
    end
            
    return table.concat(rtn)
end

function GetScript(subdir, name)
  if string.find(subdir, "[^a-zA-Z0-9_]") then
    error("Invalid character in name.")
  end
  if string.find(name, "[^a-zA-Z0-9_/]") then
    error("Invalid character in name.")
  end


  local fname = mud.userdir() .. subdir .. "/" .. name .. ".lua"
  local f,err=io.open(fname,"r")
  if f==nil then
    error( fname .. "error: " ..  err)
  end

  rtn=f:read("*all")
  f:close()
  return rtn
end

function LoadTable(name, areaFname)
  if string.find(name, "[^a-zA-Z0-9_]") then
    error("Invalid character in name.")
  end

  local dir=string.match(areaFname, "(%w+)\.are")
  local f=loadfile( dir .. "/"  .. name .. ".lua")
  if f==nil then
    return nil
  end

  return f()
end

local script_db = sqlite3.open("script_db.sqlite3")
glob_db = {}
for _,v in ipairs({
  "errcode", "errmsg", "exec", "nrows", "prepare", "rows", "urows"
  }) do
  glob_db[v] = function(ignore, ...)
    return script_db[v](script_db, ...)
  end
end

-- Standard functionality available for any env type
-- doesn't require access to env variables
function MakeLibProxy(tbl)
    local mt={
        __index=tbl,
        __newindex=function(t,k,v)
            error("Cannot alter library functions.")
            end,
        __metatable=0 -- any value here protects it
    }
    local proxy={}
    setmetatable(proxy, mt)
    return proxy
end

main_lib={  require=require,
		assert=assert,
		error=error,
		ipairs=ipairs,
		next=next,
		pairs=pairs,
		--pcall=pcall, -- remove so can't bypass infinite loop check
        print=print,
        select=select,
		tonumber=tonumber,
		tostring=tostring,
		type=type,
		unpack=unpack,
		_VERSION=_VERSION,
		--xpcall=xpcall, -- remove so can't bypass infinite loop check
		coroutine={create=coroutine.create,
					resume=coroutine.resume,
					running=coroutine.running,
					status=coroutine.status,
					wrap=coroutine.wrap,
					yield=coroutine.yield},
		string= {byte=string.byte,
				char=string.char,
				find=string.find,
				format=string.format,
				gmatch=string.gmatch,
				gsub=string.gsub,
				len=string.len,
				lower=string.lower,
				match=string.match,
				rep=string.rep,
				reverse=string.reverse,
				sub=string.sub,
				upper=string.upper},
				
		table={insert=table.insert,
				maxn=table.maxn,
				remove=table.remove,
				sort=table.sort,
				getn=table.getn,
				concat=table.concat},
				
		math={abs=math.abs,
				acos=math.acos,
				asin=math.asin,
				atan=math.atan,
				atan2=math.atan2,
				ceil=math.ceil,
				cos=math.cos,
				cosh=math.cosh,
				deg=math.deg,
				exp=math.exp,
				floor=math.floor,
				fmod=math.fmod,
				frexp=math.frexp,
				huge=math.huge,
				ldexp=math.ldexp,
				log=math.log,
				log10=math.log10,
				max=math.max,
				min=math.min,
				modf=math.modf,
				pi=math.pi,
				pow=math.pow,
				rad=math.rad,
				random=math.random,
				sin=math.sin,
				sinh=math.sinh,
				sqrt=math.sqrt,
				tan=math.tan,
				tanh=math.tanh},
		os={time=os.time,
            date=os.date,
			clock=os.clock,
			difftime=os.difftime},
        -- this is safe because we protected the game object and main lib
        --  metatables.
		setmetatable=setmetatable,
        --getmetatable=getmetatable
}
local sqlite3_codes = {}
for k,v in pairs(sqlite3) do
  if type(v) == "number" then 
    sqlite3_codes[k] = v
  end
end
main_lib.sqlite3 = sqlite3_codes 

-- add script_globs to main_lib
for k,v in pairs(script_globs) do
    print("Adding "..k.." to main_lib.")
    if type(v)=="table" then
        for j,w in pairs(v) do
            print(j)
        end
    end
    main_lib[k]=v
end

-- Need to protect our library funcs from evil scripters
function ProtectLib(lib)
    for k,v in pairs(lib) do
        if type(v) == "table" then
            ProtectLib(v) -- recursion in case we add some nested libs
            lib[k]=MakeLibProxy(v)
        end
    end
    return MakeLibProxy(lib)
end

-- Before we protect it, we want to make a lit of names for syntax highligting
main_lib_names={}
for k,v in pairs(main_lib) do
    if type(v) == "function" then
        table.insert(main_lib_names, k)
    elseif type(v) == "table" then
        for l,w in pairs(v) do
            table.insert(main_lib_names, k.."."..l)
        end
    end
end
main_lib=ProtectLib(main_lib)


-- First look for main_lib funcs, then mob/area/obj funcs
env_meta={
    __index=function(tbl,key)
        if main_lib[key] then
            return main_lib[key]
        elseif type(tbl.self[key])=="function" then 
            return function(...) 
                        table.insert(arg, 1, tbl.self)
                        return tbl.self[key](unpack(arg, 1, table.maxn(arg))) 
                   end
        end
    end
}


function new_script_env(ud, objname, share_index)
    local env = {}
    setmetatable( env, {
            __index=function(t,k)
                if k=="self" or k==objname then
                    return ud
                elseif k=="shared" then
                    return shared_tbls[share_index]
                elseif k=="_G" then
                    return env 
                else
                    return env_meta.__index(t,k)
                end
            end,
            __newindex=share_index and function(t,k,v)
                if k=="shared" then
                    if not(type(v)=="table") then
                        error("'shared' value must be a table.")
                    end
                    shared_tbls[share_index]=v
                else
                    rawset(t,k,v)
                end
            end,
            __metatable=0 })
    
    if share_index and not(shared_tbls[share_index]) then
        shared_tbls[share_index] = {}
    end

    return env
end

function mob_program_setup(ud, f)
    if envtbl[ud]==nil then
        envtbl[ud]=new_script_env(ud, "mob", (ud.isnpc or nil) and ud.proto.area) 
    end
    setfenv(f, envtbl[ud])
    return f
end

function obj_program_setup(ud, f)
    if envtbl[ud]==nil then
        envtbl[ud]=new_script_env(ud, "obj", ud.proto.area)
    end
    setfenv(f, envtbl[ud])
    return f
end

function area_program_setup(ud, f)
    if envtbl[ud]==nil then
        envtbl[ud]=new_script_env(ud, "area", ud)
    end
    setfenv(f, envtbl[ud])
    return f
end

function room_program_setup(ud, f)
    if envtbl[ud]==nil then
        envtbl[ud]=new_script_env(ud, "room", ud.area)
    end
    setfenv(f, envtbl[ud])
    return f
end

function interp_setup( ud, typ, desc, name)
    if interptbl[ud] then
        return 0, interptbl[ud].name
    end

    if envtbl[ud]== nil then
        if typ=="mob" then
            envtbl[ud]=new_script_env(ud,"mob",(ud.isnpc or nil) and ud.proto.area )
        elseif typ=="obj" then
            envtbl[ud]=new_script_env(ud,"obj",ud.proto.area )
        elseif typ=="area" then
            envtbl[ud]=new_script_env(ud,"area",ud )
        elseif typ=="room" then
            envtbl[ud]=new_script_env(ud,"room",ud.area )
        else
            error("Invalid type in interp_setup: "..typ)
        end
    end

    interptbl[ud]={name=name, desc=desc}
    return 1,nil
end

function run_lua_interpret(env, str )
    local f,err
    interptbl[env.self].incmpl=interptbl[env.self].incmpl or {}

    table.insert(interptbl[env.self].incmpl, str)
    f,err=loadstring(table.concat(interptbl[env.self].incmpl, "\n"))

    if not(f) then
        -- Check if incomplete, same way the real cli checks
        local ss,sf=string.find(err, "<eof>")
        if sf==err:len()-1 then
            return 1 -- incomplete
        else
           interptbl[env.self].incmpl=nil
           error(err)
        end
    end

    interptbl[env.self].incmpl=nil
    setfenv(f, env)
    f()
    return 0
end

function list_files ( path )
    local f=assert(io.popen('find ../player -type f -printf "%f\n"', 'r'))
    
    local txt=f:read("*all")
    f:close()
    
    local rtn={}
    for nm in string.gmatch( txt, "(%a+)\n") do
        table.insert(rtn, nm)
    end

    return rtn
end

-- Dijkstra style shortest path algorithm
function findpath( start, finish )
    local dist={}
    dist[start]=0
    local previous={}
    local dirs={}

    local Q = Queue.new()
    Queue.pushleft( Q, start )
    local finished={}
    local found

    local lowest
    while not( Queue.isempty( Q ) ) do
      local lowest=Queue.popleft( Q )
        
      if not(finished[lowest]) then
 
        finished[lowest]=true

        if lowest==finish then found=true break end

        -- any exit has a length of 1
        local alt = dist[lowest] + 1
        -- handle normal exits
        for _,dirname in pairs(lowest.exits) do
            local toroom = lowest[dirname].toroom
            if not(finished[toroom]) then
                Queue.pushright( Q, toroom )
            end

            dist[toroom] = dist[toroom] or math.huge
            if alt < dist[toroom] then
                dist[toroom] = alt
                previous[toroom] = lowest
                dirs[toroom] = dirname
            end
        end
        -- handle portals
        for _,obj in pairs(lowest.contents) do
            if obj.otype=="portal" then
                local toroom=getroom(obj.toroom) 
                if toroom then
                    if not(finished[toroom]) then
                        Queue.pushright( Q, toroom )
                    end

                    dist[toroom] = dist[toroom] or math.huge
                    if alt < dist[toroom] then
                        dist[toroom] = alt
                        previous[toroom] = lowest
                        dirs[toroom] = "enter "..obj.name
                    end
                end
            end
        end
      end
    end

    if not found then
        return nil
    end

    local result={}
    local rm=finish
    while previous[rm] do
        table.insert( result, 1, dirs[rm] )
        rm=previous[rm]
    end

    return result
end

