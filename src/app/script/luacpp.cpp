// Aseprite
// Copyright (C) 2018  David Capello
//
// This program is distributed under the terms of
// the End-User License Agreement for Aseprite.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/script/luacpp.h"

namespace app {
namespace script {

static const char* mt_index_code =
  "__generic_mt_index = function(t, k) "
  "  local mt = getmetatable(t) "
  "  local f = mt[k] "
  "  if f then return f end "
  "  f = mt.__getters[k] "
  "  if f then return f(t) end "
  "  if type(t) == 'table' then return rawget(t, k) end "
  "  error('Field '..tostring(k)..' does not exist')"
  "end "
  "__generic_mt_newindex = function(t, k, v) "
  "  local mt = getmetatable(t) "
  "  local f = mt[k] "
  "  if f then return f end "
  "  f = mt.__setters[k] "
  "  if f then return f(t, v) end "
  "  if type(t) == 'table' then return rawset(t, k, v) end "
  "  error('Cannot set field '..tostring(k))"
  "end";

void run_mt_index_code(lua_State* L)
{
  luaL_dostring(L, mt_index_code);
}

void create_mt_getters_setters(lua_State* L,
                               const char* tname,
                               const Property* properties)
{
  const int top = lua_gettop(L);

  bool withGetters = false;
  bool withSetters = false;
  for (auto p=properties; p->name; ++p) {
    if (p->getter) withGetters = true;
    if (p->setter) withSetters = true;
  }
  ASSERT(withGetters || withSetters);

  luaL_getmetatable(L, tname);
  if (withGetters) {
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setfield(L, -3, "__getters");
    for (auto p=properties; p->name; ++p) {
      if (p->getter) {
        lua_pushcclosure(L, p->getter, 0);
        lua_setfield(L, -2, p->name);
      }
    }
    lua_pop(L, 1);
  }
  if (withSetters) {
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setfield(L, -3, "__setters");
    for (auto p=properties; p->name; ++p) {
      if (p->setter) {
        lua_pushcclosure(L, p->setter, 0);
        lua_setfield(L, -2, p->name);
      }
    }
    lua_pop(L, 1);
  }
  lua_pop(L, 1);

  ASSERT(lua_gettop(L) == top);
}

} // namespace script
} // namespace app
