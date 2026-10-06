//==============================================================================================
//【文件说明】ExposingCPPClassesUsingLuabind/main.cpp —— 用 luabind 把 C++ 类暴露给 Lua
//
//【这个小演示干什么?】
//  用 luabind 把 Animal(动物基类)和 Pet(宠物子类)注册进 Lua:注册构造函数、Speak/
//  NumLegs/GetName 等成员函数,并通过 bases<Animal> 声明 Pet 继承自 Animal。
//  之后 Lua 脚本就能 new Animal/Pet、调它们的方法。
//
//【文件地图】main.cpp + Animal.h(基类)+ Pet.h(子类)+ LuaHelperFunctions.h;
//  luabind 来自第三方库 Common\luabind,Lua 来自 Common\lua-5.1.5。
//  脚本 ExposingCPPClassesToLua.lua 与 exe 同目录。
//【调用流程】main → lua_open/打开库/open luabind → RegisterAnimal/RegisterPet →
//           RunLuaScript(Lua 里创建并使用动物)→ lua_close。
//==============================================================================================
//include the libraries
#pragma comment(lib, "lua5.1.lib")
#pragma comment(lib, "luabind.lib")
//#pragma comment(lib, "lua.lib")
//#pragma comment(lib, "lualib.lib")
#pragma warning (disable : 4786)

extern "C"
{
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
}

#include <string>
#include <iostream>
using namespace std;

//include the luabind headers. Make sure you have the paths set correctly
//to the lua, luabind and Boost files.
#include <luabind/luabind.hpp>
using namespace luabind;

#include "LuaHelperFunctions.h"
#include "Animal.h"
#include "Pet.h"




// RegisterAnimalWithLua:module[ class_<Animal>("名字") .def(构造) .def("方法",&方法)... ]
// —— class_ 声明一个类,def 逐个把成员函数/构造器导出给 Lua。
void RegisterAnimalWithLua(lua_State* pLua)
{
  module(pLua)
  [
    class_<Animal>("Animal")
    .def(constructor<string, int>())
    .def("Speak", &Animal::Speak)
    .def("NumLegs", &Animal::NumLegs)   
  ];  
}

// RegisterPetWithLua:class_<Pet, bases<Animal>> —— bases<Animal> 告诉 luabind
// Pet 是 Animal 的子类,Lua 里就能把 Pet 当 Animal 用(多态)。
void RegisterPetWithLua(lua_State* pLua)
{
  module(pLua)
    [
      class_<Pet, bases<Animal> >("Pet")
      .def(constructor<string, string, int>())
      .def("GetName", &Pet::GetName)  
    
    ];  
}


int main()
{
  //create a lua state
  lua_State* pLua = lua_open();

  //open the lua libaries - new in lua5.1
  luaL_openlibs(pLua);

  //open luabind
  open(pLua);

  RegisterAnimalWithLua(pLua);
  RegisterPetWithLua(pLua);
 
  //load and run the script
  RunLuaScript(pLua, "ExposingCPPClassesToLua.lua");

  lua_close(pLua);



  
    
  return 0;
}