--[[--------------------------------------------------------------------------
  class.lua - LuaClass-compatible class library (restoration addition)

  Covers the LuaClass semantics used by classes_in_lua.lua in Buckland's
  "Programming Game AI by Example", Chapter 6 (CreatingClassesUsingLuabind):

    class 'Name'                define a class (registered as global Name)
    function Name:__init(...)   constructor
    function Name:Method(...)   class method
    class 'Child' (Parent)      inheritance (parses as class('Child')(Parent),
                                so the returned class table is callable with
                                the parent as its argument)
    super(...)                  call parent __init from a child __init
    obj = Name(...)             instantiate (calls __init automatically)

  The original class.lua shipped with the book was not present in this
  repository; this file provides equivalent behaviour for the API subset
  the script actually uses.
--------------------------------------------------------------------------]]

local classes = {}  -- registered classes: name -> class table

-- Define a class: class 'Name'  or  class 'Name' (Base)
function class(name, base)
  local cls = { __init = function() end }
  local mt  = {}
  mt.__call = function(c, first, ...)
    if type(first) == "table" then
      -- inheritance call: class 'Child' (Parent) => c(Parent)
      mt.__index = first
      return c
    else
      -- instantiation: c(args...) creates an instance and calls __init
      local obj = setmetatable({}, { __index = c })
      c.__init(obj, first, ...)
      return obj
    end
  end

  if base then
    mt.__index = base
  end

  setmetatable(cls, mt)

  classes[name] = cls
  _G[name] = cls
  return cls
end

-- Call the parent __init from inside a child __init: super(args...)
-- Locates the calling method via the debug stack, then invokes the parent's
-- __init with the instance (first argument of the calling method) prepended.
function super(...)
  local level  = 2                              -- frame 2 = the calling method
  local caller = debug.getinfo(level, "f").func -- function that called super
  local name, self = debug.getlocal(level, 1)   -- first parameter = the instance

  for _, c in pairs(classes) do
    for _, v in pairs(c) do
      if v == caller then
        local mt     = getmetatable(c)
        local parent = mt and mt.__index
        if parent and parent.__init then
          parent.__init(self, ...)
          return
        end
      end
    end
  end

  error("super(): unable to locate parent class", 2)
end
