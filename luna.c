#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>

// we need this since we're not defining a main() function.
// if we didn't #define this, SDL would try to #define main SDL_main
// and put parachute code in main and also die.
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

// Metatable defines
static const char *EVENTMETA = "luna:event";


// use for userdata for window object; it's easier to keep them both in one
typedef struct luna_Display {
	SDL_Window *window;
	SDL_Renderer *renderer;
} luna_Display;

static int l_luna_init(lua_State *L)
{
	SDL_SetMainReady(); // needed because we're using SDL_MAIN_HANDLED
	SDL_Init(SDL_INIT_EVERYTHING); // just init everything (even if unused)
	return 0;
}

static int l_luna_quit(lua_State *L)
{
	SDL_Quit();
	return 0;
}

/* EVENT CODE */

static void lh_luna_set_keycode(lua_State *L, SDL_Keysym *ks)
{

}

static void lh_luna_make_key_event(lua_State *L, SDL_Event *e)
{
	// add our table
	lua_newtable(L);

	// set the table's "type" field so user can switch on it
	if (e->type == SDL_KEYDOWN) {
		lua_pushliteral(L,"key_down");
	} else {
		lua_pushliteral(L,"key_up");
	}
	lua_setfield(L,-2,"type");

	// add "timestamp" field
	lua_pushinteger(L, e->key.timestamp);
	lua_setfield(L,-2,"timestamp");

	// set the "repeat" field; true if a repeated press
	lua_pushboolean(L,e->key.repeat);
	lua_setfield(L,-2,"repeat");

	// set the "state" field to "pressed" or "released"
	if (e->key.state == SDL_PRESSED) {
		lua_pushliteral(L,"pressed");
	} else {
		lua_pushliteral(L,"released");
	}
	lua_setfield(L,-2,"state");

	// set keysym info. It gets ugly here and we're using helper fns.

}

static int l_luna_event_poll(lua_State *L)
{
	SDL_Event event;
	SDL_PollEvent(&event);
	// push a table to use as our return value
	switch (event.type) {
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			lh_luna_make_key_event(L, &event);
			break;
		default:
			break;
	}
}

// module defs
static const luaL_Reg l_luna_module_fns[] = {
	{"init", &l_luna_init},
	{"quit", &l_luna_quit},
	{NULL,NULL}
}



int luaopen_luna(lua_State *L)
{

}

