if file.open("LEDblink.lua","r") then do print("-- File content start from next line")
repeat _line = file.readline() if (_line~=nil) then print(string.sub(_line,1,-2)) end until _line == nil
file.close() _line=nil collectgarbage() print("-- File content end") end else print("-- Can't open file read_file.lua") end
