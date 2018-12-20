#include <stdlib.h>

#include <lua/lua.h>
#include <lua/lauxlib.h>

// we need this since we're not defining a main() function.
// if we didn't #define this, SDL would try to #define main SDL_main
// and put startup code in main and also die becasue we aren't calling main.
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

// //////////////////
// Metatable names //
// //////////////////
static const char *LUNA_WINDOW_MT = "luna.Window";
static const char *LUNA_TEXTURE_MT = "luna.Texture";
static const char *LUNA_SOUND_MT = "luna.Sound";
static const char *LUNA_MUSIC_MT = "luna.Music";
static const char *LUNA_SDLK_TO_STR_TBL = "luna.sdlk_to_str";
static const char *LUNA_STR_TO_SDLK_TBL = "luna.str_to_sdlk";
static const char *LUNA_KEYMOD_TO_STR_TBL = "luna.keymod_to_str";
static const char *LUNA_STR_TO_KEYMOD_TBL = "luna.str_to_keymod";

// ////////////////////////
// Structure definitions //
// ////////////////////////

// use for userdata for window object; it's easier to keep them both in one
typedef struct luna_Window {
	SDL_Window *window;
	SDL_Renderer *renderer;
	int closed;
} luna_Window;

typedef struct luna_Texture {
	SDL_Texture *texture;
	int raw_w;
	int raw_h;
	SDL_Rect frame;
	int is_framed;
} luna_Texture;

typedef struct luna_Sound {
	Mix_Chunk *chunk;
	int channel;
} luna_Sound;

typedef struct luna_Music {
	Mix_Music *music;
} luna_Music;

// /////////////////////
// SDL Keyboard state //
// /////////////////////
const Uint8 *KEYBOARD_STATE;

// ///////////////////
// luna.* functions //
// ///////////////////

// luna.init()
static int luna_init(lua_State *L)
{
	SDL_SetMainReady(); // needed because we're using SDL_MAIN_HANDLED
	SDL_Init(SDL_INIT_EVERYTHING); // just init everything (even if unused)
	int flags = IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF;
	int initted = IMG_Init(flags);
	if ((initted&flags) != flags) {
		lua_pushfstring(L,"SDL_image init failed: %s\n",IMG_GetError());
		lua_error(L);
	}

	//int mixflags = MIX_INIT_FLAC | MIX_INIT_OGG | MIX_INIT_MP3;
	int mixflags = MIX_INIT_OGG;
	int mixinitted = Mix_Init(mixflags);
	if ((mixinitted&mixflags) != mixflags) {
		lua_pushfstring(L, "luna.init-Mix_Init: %s\n", Mix_GetError());
		lua_error(L);
	}

	Mix_AllocateChannels(100);

	int merr = Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT,2,1024);
	if (merr) {
		lua_pushfstring(L,"luna.init-Mix_OpenAudio: %s\n",Mix_GetError());
		lua_error(L);
	}

	return 0;
}

// luna.quit()
static int luna_quit(lua_State *L)
{
	Mix_CloseAudio();
	Mix_Quit();
	IMG_Quit();
	SDL_Quit();
	return 0;
}

// luna.delay(ms:int)
static int luna_delay(lua_State *L)
{
	int ms = luaL_checkinteger(L,1);
	SDL_Delay(ms);
	return 0;
}

static const luaL_Reg luna_module_fns[] = {
	{"init", &luna_init},
	{"quit", &luna_quit},
	{"delay", &luna_delay},
	{NULL,NULL}
};

// //////////////////
// luna.event code //
// //////////////////

//

static void lh_luna_init_keymaps(lua_State *L)
{
	// SDLK_* const-> string
	lua_newtable(L);
	// string -> SDLK_* const
	lua_newtable(L);

	// number keys //
	// SDLK_0
	lua_pushliteral(L,"0");
	lua_pushinteger(L,SDLK_0);
	lua_rawset(L,-3);
	lua_pushliteral(L,"0");
	lua_rawseti(L,-3,SDLK_0);
	// SDLK_1
	lua_pushliteral(L,"1");
	lua_pushinteger(L,SDLK_1);
	lua_rawset(L,-3);
	lua_pushliteral(L,"1");
	lua_rawseti(L,-3,SDLK_1);
	// SDLK_2
	lua_pushliteral(L,"2");
	lua_pushinteger(L,SDLK_2);
	lua_rawset(L,-3);
	lua_pushliteral(L,"2");
	lua_rawseti(L,-3,SDLK_2);
	// SDLK_3
	lua_pushliteral(L,"3");
	lua_pushinteger(L,SDLK_3);
	lua_rawset(L,-3);
	lua_pushliteral(L,"3");
	lua_rawseti(L,-3,SDLK_3);
	// SDLK_4
	lua_pushliteral(L,"4");
	lua_pushinteger(L,SDLK_4);
	lua_rawset(L,-3);
	lua_pushliteral(L,"4");
	lua_rawseti(L,-3,SDLK_4);
	// SDLK_5
	lua_pushliteral(L,"5");
	lua_pushinteger(L,SDLK_5);
	lua_rawset(L,-3);
	lua_pushliteral(L,"5");
	lua_rawseti(L,-3,SDLK_5);
	// SDLK_6
	lua_pushliteral(L,"6");
	lua_pushinteger(L,SDLK_6);
	lua_rawset(L,-3);
	lua_pushliteral(L,"6");
	lua_rawseti(L,-3,SDLK_6);
	// SDLK_7
	lua_pushliteral(L,"7");
	lua_pushinteger(L,SDLK_7);
	lua_rawset(L,-3);
	lua_pushliteral(L,"7");
	lua_rawseti(L,-3,SDLK_7);
	// SDLK_8
	lua_pushliteral(L,"8");
	lua_pushinteger(L,SDLK_8);
	lua_rawset(L,-3);
	lua_pushliteral(L,"8");
	lua_rawseti(L,-3,SDLK_8);
	// SDLK_9
	lua_pushliteral(L,"9");
	lua_pushinteger(L,SDLK_9);
	lua_rawset(L,-3);
	lua_pushliteral(L,"9");
	lua_rawseti(L,-3,SDLK_9);

	// letters //
	// SDLK_a
	lua_pushliteral(L,"a");
	lua_pushinteger(L,SDLK_a);
	lua_rawset(L,-3);
	lua_pushliteral(L,"a");
	lua_rawseti(L,-3,SDLK_a);
	// SDLK_b
	lua_pushliteral(L,"b");
	lua_pushinteger(L,SDLK_b);
	lua_rawset(L,-3);
	lua_pushliteral(L,"b");
	lua_rawseti(L,-3,SDLK_b);
	// SDLK_c
	lua_pushliteral(L,"c");
	lua_pushinteger(L,SDLK_c);
	lua_rawset(L,-3);
	lua_pushliteral(L,"c");
	lua_rawseti(L,-3,SDLK_c);
	// SDLK_d
	lua_pushliteral(L,"d");
	lua_pushinteger(L,SDLK_d);
	lua_rawset(L,-3);
	lua_pushliteral(L,"d");
	lua_rawseti(L,-3,SDLK_d);
	// SDLK_e
	lua_pushliteral(L,"e");
	lua_pushinteger(L,SDLK_e);
	lua_rawset(L,-3);
	lua_pushliteral(L,"e");
	lua_rawseti(L,-3,SDLK_e);
	// SDLK_f
	lua_pushliteral(L,"f");
	lua_pushinteger(L,SDLK_f);
	lua_rawset(L,-3);
	lua_pushliteral(L,"f");
	lua_rawseti(L,-3,SDLK_f);
	// SDLK_g
	lua_pushliteral(L,"g");
	lua_pushinteger(L,SDLK_g);
	lua_rawset(L,-3);
	lua_pushliteral(L,"g");
	lua_rawseti(L,-3,SDLK_g);
	// SDLK_h
	lua_pushliteral(L,"h");
	lua_pushinteger(L,SDLK_h);
	lua_rawset(L,-3);
	lua_pushliteral(L,"h");
	lua_rawseti(L,-3,SDLK_h);
	// SDLK_i
	lua_pushliteral(L,"i");
	lua_pushinteger(L,SDLK_i);
	lua_rawset(L,-3);
	lua_pushliteral(L,"i");
	lua_rawseti(L,-3,SDLK_i);
	// SDLK_j
	lua_pushliteral(L,"j");
	lua_pushinteger(L,SDLK_j);
	lua_rawset(L,-3);
	lua_pushliteral(L,"j");
	lua_rawseti(L,-3,SDLK_j);
	// SDLK_k
	lua_pushliteral(L,"k");
	lua_pushinteger(L,SDLK_k);
	lua_rawset(L,-3);
	lua_pushliteral(L,"k");
	lua_rawseti(L,-3,SDLK_k);
	// SDLK_l
	lua_pushliteral(L,"l");
	lua_pushinteger(L,SDLK_l);
	lua_rawset(L,-3);
	lua_pushliteral(L,"l");
	lua_rawseti(L,-3,SDLK_l);
	// SDLK_m
	lua_pushliteral(L,"m");
	lua_pushinteger(L,SDLK_m);
	lua_rawset(L,-3);
	lua_pushliteral(L,"m");
	lua_rawseti(L,-3,SDLK_m);
	// SDLK_n
	lua_pushliteral(L,"n");
	lua_pushinteger(L,SDLK_n);
	lua_rawset(L,-3);
	lua_pushliteral(L,"n");
	lua_rawseti(L,-3,SDLK_n);
	// SDLK_o
	lua_pushliteral(L,"o");
	lua_pushinteger(L,SDLK_o);
	lua_rawset(L,-3);
	lua_pushliteral(L,"o");
	lua_rawseti(L,-3,SDLK_o);
	// SDLK_p
	lua_pushliteral(L,"p");
	lua_pushinteger(L,SDLK_p);
	lua_rawset(L,-3);
	lua_pushliteral(L,"p");
	lua_rawseti(L,-3,SDLK_p);
	// SDLK_q
	lua_pushliteral(L,"q");
	lua_pushinteger(L,SDLK_q);
	lua_rawset(L,-3);
	lua_pushliteral(L,"q");
	lua_rawseti(L,-3,SDLK_q);
	// SDLK_r
	lua_pushliteral(L,"r");
	lua_pushinteger(L,SDLK_r);
	lua_rawset(L,-3);
	lua_pushliteral(L,"r");
	lua_rawseti(L,-3,SDLK_r);
	// SDLK_s
	lua_pushliteral(L,"s");
	lua_pushinteger(L,SDLK_s);
	lua_rawset(L,-3);
	lua_pushliteral(L,"s");
	lua_rawseti(L,-3,SDLK_s);
	// SDLK_t
	lua_pushliteral(L,"t");
	lua_pushinteger(L,SDLK_t);
	lua_rawset(L,-3);
	lua_pushliteral(L,"t");
	lua_rawseti(L,-3,SDLK_t);
	// SDLK_u
	lua_pushliteral(L,"u");
	lua_pushinteger(L,SDLK_u);
	lua_rawset(L,-3);
	lua_pushliteral(L,"u");
	lua_rawseti(L,-3,SDLK_u);
	// SDLK_v
	lua_pushliteral(L,"v");
	lua_pushinteger(L,SDLK_v);
	lua_rawset(L,-3);
	lua_pushliteral(L,"v");
	lua_rawseti(L,-3,SDLK_v);
	// SDLK_w
	lua_pushliteral(L,"w");
	lua_pushinteger(L,SDLK_w);
	lua_rawset(L,-3);
	lua_pushliteral(L,"w");
	lua_rawseti(L,-3,SDLK_w);
	// SDLK_x
	lua_pushliteral(L,"x");
	lua_pushinteger(L,SDLK_x);
	lua_rawset(L,-3);
	lua_pushliteral(L,"x");
	lua_rawseti(L,-3,SDLK_x);
	// SDLK_y
	lua_pushliteral(L,"y");
	lua_pushinteger(L,SDLK_y);
	lua_rawset(L,-3);
	lua_pushliteral(L,"y");
	lua_rawseti(L,-3,SDLK_y);
	// SDLK_z
	lua_pushliteral(L,"z");
	lua_pushinteger(L,SDLK_z);
	lua_rawset(L,-3);
	lua_pushliteral(L,"z");
	lua_rawseti(L,-3,SDLK_z);

	// F keys //
	// SDLK_F1
	lua_pushliteral(L,"f1");
	lua_pushinteger(L,SDLK_F1);
	lua_rawset(L,-3);
	lua_pushliteral(L,"f1");
	lua_rawseti(L,-3,SDLK_F1);
	// SDLK_F2
	lua_pushliteral(L,"f2");
	lua_pushinteger(L,SDLK_F2);
	lua_rawset(L,-3);
	lua_pushliteral(L,"f2");
	lua_rawseti(L,-3,SDLK_F2);
	// SDLK_F3
	lua_pushliteral(L,"f3");
	lua_pushinteger(L,SDLK_F3);
	lua_rawset(L,-3);
	lua_pushliteral(L,"f3");
	lua_rawseti(L,-3,SDLK_F3);
	// SDLK_F4
	lua_pushliteral(L,"f4");
	lua_pushinteger(L,SDLK_F4);
	lua_rawset(L,-3);
	lua_pushliteral(L,"f4");
	lua_rawseti(L,-3,SDLK_F4);
	// SDLK_F5
	lua_pushliteral(L,"f5");
	lua_pushinteger(L,SDLK_F5);
	lua_rawset(L,-3);
	lua_pushliteral(L,"f5");
	lua_rawseti(L,-3,SDLK_F5);
	// SDLK_F6
	lua_pushliteral(L,"f6");
	lua_pushinteger(L,SDLK_F6);
	lua_rawset(L,-3);
	lua_pushliteral(L,"f6");
	lua_rawseti(L,-3,SDLK_F6);
	// SDLK_F7
	lua_pushliteral(L,"f7");
	lua_pushinteger(L,SDLK_F7);
	lua_rawset(L,-3);
	lua_pushliteral(L,"f7");
	lua_rawseti(L,-3,SDLK_F7);
	// SDLK_F8
	lua_pushliteral(L,"f8");
	lua_pushinteger(L,SDLK_F8);
	lua_rawset(L,-3);
	lua_pushliteral(L,"f8");
	lua_rawseti(L,-3,SDLK_F8);
	// SDLK_F9
	lua_pushliteral(L,"f9");
	lua_pushinteger(L,SDLK_F9);
	lua_rawset(L,-3);
	lua_pushliteral(L,"f9");
	lua_rawseti(L,-3,SDLK_F9);
	// SDLK_F10
	lua_pushliteral(L,"f10");
	lua_pushinteger(L,SDLK_F10);
	lua_rawset(L,-3);
	lua_pushliteral(L,"f10");
	lua_rawseti(L,-3,SDLK_F10);
	// SDLK_F11
	lua_pushliteral(L,"f11");
	lua_pushinteger(L,SDLK_F11);
	lua_rawset(L,-3);
	lua_pushliteral(L,"f11");
	lua_rawseti(L,-3,SDLK_F11);
	// SDLK_F12
	lua_pushliteral(L,"f12");
	lua_pushinteger(L,SDLK_F12);
	lua_rawset(L,-3);
	lua_pushliteral(L,"f12");
	lua_rawseti(L,-3,SDLK_F12);
	// SDLK_F13
	lua_pushliteral(L,"f13");
	lua_pushinteger(L,SDLK_F13);
	lua_rawset(L,-3);
	lua_pushliteral(L,"f13");
	lua_rawseti(L,-3,SDLK_F13);
	// SDLK_F14
	lua_pushliteral(L,"f14");
	lua_pushinteger(L,SDLK_F14);
	lua_rawset(L,-3);
	lua_pushliteral(L,"f14");
	lua_rawseti(L,-3,SDLK_F14);
	// SDLK_F15
	lua_pushliteral(L,"f15");
	lua_pushinteger(L,SDLK_F15);
	lua_rawset(L,-3);
	lua_pushliteral(L,"f15");
	lua_rawseti(L,-3,SDLK_F15);
	// SDLK_F16
	lua_pushliteral(L,"f16");
	lua_pushinteger(L,SDLK_F16);
	lua_rawset(L,-3);
	lua_pushliteral(L,"f16");
	lua_rawseti(L,-3,SDLK_F16);
	// SDLK_F17
	lua_pushliteral(L,"f17");
	lua_pushinteger(L,SDLK_F17);
	lua_rawset(L,-3);
	lua_pushliteral(L,"f17");
	lua_rawseti(L,-3,SDLK_F17);
	// SDLK_F18
	lua_pushliteral(L,"f18");
	lua_pushinteger(L,SDLK_F18);
	lua_rawset(L,-3);
	lua_pushliteral(L,"f18");
	lua_rawseti(L,-3,SDLK_F18);
	// SDLK_F19
	lua_pushliteral(L,"f19");
	lua_pushinteger(L,SDLK_F19);
	lua_rawset(L,-3);
	lua_pushliteral(L,"f19");
	lua_rawseti(L,-3,SDLK_F19);
	// SDLK_F20
	lua_pushliteral(L,"f20");
	lua_pushinteger(L,SDLK_F20);
	lua_rawset(L,-3);
	lua_pushliteral(L,"f20");
	lua_rawseti(L,-3,SDLK_F20);
	// SDLK_F21
	lua_pushliteral(L,"f21");
	lua_pushinteger(L,SDLK_F21);
	lua_rawset(L,-3);
	lua_pushliteral(L,"f21");
	lua_rawseti(L,-3,SDLK_F21);
	// SDLK_F22
	lua_pushliteral(L,"f22");
	lua_pushinteger(L,SDLK_F22);
	lua_rawset(L,-3);
	lua_pushliteral(L,"f22");
	lua_rawseti(L,-3,SDLK_F22);
	// SDLK_F23
	lua_pushliteral(L,"f23");
	lua_pushinteger(L,SDLK_F23);
	lua_rawset(L,-3);
	lua_pushliteral(L,"f23");
	lua_rawseti(L,-3,SDLK_F23);
	// SDLK_F24
	lua_pushliteral(L,"f24");
	lua_pushinteger(L,SDLK_F24);
	lua_rawset(L,-3);
	lua_pushliteral(L,"f24");
	lua_rawseti(L,-3,SDLK_F24);

	// KP numbers //
	// SDLK_KP_0
	lua_pushliteral(L,"kp0");
	lua_pushinteger(L,SDLK_KP_0);
	lua_rawset(L,-3);
	lua_pushliteral(L,"kp0");
	lua_rawseti(L,-3,SDLK_KP_0);
	// SDLK_KP_1
	lua_pushliteral(L,"kp1");
	lua_pushinteger(L,SDLK_KP_1);
	lua_rawset(L,-3);
	lua_pushliteral(L,"kp1");
	lua_rawseti(L,-3,SDLK_KP_1);
	// SDLK_KP_2
	lua_pushliteral(L,"kp2");
	lua_pushinteger(L,SDLK_KP_2);
	lua_rawset(L,-3);
	lua_pushliteral(L,"kp2");
	lua_rawseti(L,-3,SDLK_KP_2);
	// SDLK_KP_3
	lua_pushliteral(L,"kp3");
	lua_pushinteger(L,SDLK_KP_3);
	lua_rawset(L,-3);
	lua_pushliteral(L,"kp3");
	lua_rawseti(L,-3,SDLK_KP_3);
	// SDLK_KP_4
	lua_pushliteral(L,"kp4");
	lua_pushinteger(L,SDLK_KP_4);
	lua_rawset(L,-3);
	lua_pushliteral(L,"kp4");
	lua_rawseti(L,-3,SDLK_KP_4);
	// SDLK_KP_5
	lua_pushliteral(L,"kp5");
	lua_pushinteger(L,SDLK_KP_5);
	lua_rawset(L,-3);
	lua_pushliteral(L,"kp5");
	lua_rawseti(L,-3,SDLK_KP_5);
	// SDLK_KP_6
	lua_pushliteral(L,"kp6");
	lua_pushinteger(L,SDLK_KP_6);
	lua_rawset(L,-3);
	lua_pushliteral(L,"kp6");
	lua_rawseti(L,-3,SDLK_KP_6);
	// SDLK_KP_7
	lua_pushliteral(L,"kp7");
	lua_pushinteger(L,SDLK_KP_7);
	lua_rawset(L,-3);
	lua_pushliteral(L,"kp7");
	lua_rawseti(L,-3,SDLK_KP_7);
	// SDLK_KP_8
	lua_pushliteral(L,"kp8");
	lua_pushinteger(L,SDLK_KP_8);
	lua_rawset(L,-3);
	lua_pushliteral(L,"kp8");
	lua_rawseti(L,-3,SDLK_KP_8);
	// SDLK_KP_9
	lua_pushliteral(L,"kp9");
	lua_pushinteger(L,SDLK_KP_9);
	lua_rawset(L,-3);
	lua_pushliteral(L,"kp9");
	lua_rawseti(L,-3,SDLK_KP_9);

	// KP symbols //
	// SDLK_KP_COMMA
	lua_pushliteral(L,"kp,");
	lua_pushinteger(L,SDLK_KP_COMMA);
	lua_rawset(L,-3);
	lua_pushliteral(L,"kp,");
	lua_rawseti(L,-3,SDLK_KP_COMMA);
	// SDLK_KP_DECIMAL
	lua_pushliteral(L,"kp.");
	lua_pushinteger(L,SDLK_KP_DECIMAL);
	lua_rawset(L,-3);
	lua_pushliteral(L,"kp.");
	lua_rawseti(L,-3,SDLK_KP_DECIMAL);
	// SDLK_KP_DIVIDE
	lua_pushliteral(L,"kp/");
	lua_pushinteger(L,SDLK_KP_DIVIDE);
	lua_rawset(L,-3);
	lua_pushliteral(L,"kp/");
	lua_rawseti(L,-3,SDLK_KP_DIVIDE);
	// SDLK_KP_ENTER
	lua_pushliteral(L,"kpenter");
	lua_pushinteger(L,SDLK_KP_ENTER);
	lua_rawset(L,-3);
	lua_pushliteral(L,"kpenter");
	lua_rawseti(L,-3,SDLK_KP_ENTER);
	// SDLK_KP_EQUALS
	lua_pushliteral(L,"kp=");
	lua_pushinteger(L,SDLK_KP_EQUALS);
	lua_rawset(L,-3);
	lua_pushliteral(L,"kp=");
	lua_rawseti(L,-3,SDLK_KP_EQUALS);
	// SDLK_KP_MINUS
	lua_pushliteral(L,"kp-");
	lua_pushinteger(L,SDLK_KP_MINUS);
	lua_rawset(L,-3);
	lua_pushliteral(L,"kp-");
	lua_rawseti(L,-3,SDLK_KP_MINUS);
	// SDLK_KP_MULTIPLY
	lua_pushliteral(L,"kp*");
	lua_pushinteger(L,SDLK_KP_MULTIPLY);
	lua_rawset(L,-3);
	lua_pushliteral(L,"kp*");
	lua_rawseti(L,-3,SDLK_KP_MULTIPLY);
	// SDLK_KP_PLUS
	lua_pushliteral(L,"kp+");
	lua_pushinteger(L,SDLK_KP_PLUS);
	lua_rawset(L,-3);
	lua_pushliteral(L,"kp+");
	lua_rawseti(L,-3,SDLK_KP_PLUS);

	// symbol keys //
	// SDLK_SPACE
	lua_pushliteral(L,"space");
	lua_pushinteger(L,SDLK_SPACE);
	lua_rawset(L,-3);
	lua_pushliteral(L,"space");
	lua_rawseti(L,-3,SDLK_SPACE);
	// SDLK_AMPERSAND
	lua_pushliteral(L,"&");
	lua_pushinteger(L,SDLK_AMPERSAND);
	lua_rawset(L,-3);
	lua_pushliteral(L,"&");
	lua_rawseti(L,-3,SDLK_AMPERSAND);
	// SDLK_ASTERISK
	lua_pushliteral(L,"*");
	lua_pushinteger(L,SDLK_ASTERISK);
	lua_rawset(L,-3);
	lua_pushliteral(L,"*");
	lua_rawseti(L,-3,SDLK_ASTERISK);
	// SDLK_AT
	lua_pushliteral(L,"@");
	lua_pushinteger(L,SDLK_AT);
	lua_rawset(L,-3);
	lua_pushliteral(L,"@");
	lua_rawseti(L,-3,SDLK_AT);
	// SDLK_BACKSLASH
	lua_pushliteral(L,"\\");
	lua_pushinteger(L,SDLK_BACKSLASH);
	lua_rawset(L,-3);
	lua_pushliteral(L,"\\");
	lua_rawseti(L,-3,SDLK_BACKSLASH);
	// SDLK_BACKQUOTE
	lua_pushliteral(L,"`");
	lua_pushinteger(L,SDLK_BACKQUOTE);
	lua_rawset(L,-3);
	lua_pushliteral(L,"`");
	lua_rawseti(L,-3,SDLK_BACKQUOTE);
	// SDLK_CARET
	lua_pushliteral(L,"^");
	lua_pushinteger(L,SDLK_CARET);
	lua_rawset(L,-3);
	lua_pushliteral(L,"^");
	lua_rawseti(L,-3,SDLK_CARET);
	// SDLK_COMMA
	lua_pushliteral(L,",");
	lua_pushinteger(L,SDLK_COMMA);
	lua_rawset(L,-3);
	lua_pushliteral(L,",");
	lua_rawseti(L,-3,SDLK_COMMA);
	// SDLK_DOLLAR
	lua_pushliteral(L,"$");
	lua_pushinteger(L,SDLK_DOLLAR);
	lua_rawset(L,-3);
	lua_pushliteral(L,"$");
	lua_rawseti(L,-3,SDLK_DOLLAR);
	// SDLK_EQUALS
	lua_pushliteral(L,"=");
	lua_pushinteger(L,SDLK_EQUALS);
	lua_rawset(L,-3);
	lua_pushliteral(L,"=");
	lua_rawseti(L,-3,SDLK_EQUALS);
	// SDLK_EXCLAIM
	lua_pushliteral(L,"!");
	lua_pushinteger(L,SDLK_EXCLAIM);
	lua_rawset(L,-3);
	lua_pushliteral(L,"!");
	lua_rawseti(L,-3,SDLK_EXCLAIM);
	// SDLK_GREATER
	lua_pushliteral(L,">");
	lua_pushinteger(L,SDLK_GREATER);
	lua_rawset(L,-3);
	lua_pushliteral(L,">");
	lua_rawseti(L,-3,SDLK_GREATER);
	// SDLK_HASH
	lua_pushliteral(L,"#");
	lua_pushinteger(L,SDLK_HASH);
	lua_rawset(L,-3);
	lua_pushliteral(L,"#");
	lua_rawseti(L,-3,SDLK_HASH);
	// SDLK_LEFTBRACKET
	lua_pushliteral(L,"[");
	lua_pushinteger(L,SDLK_LEFTBRACKET);
	lua_rawset(L,-3);
	lua_pushliteral(L,"[");
	lua_rawseti(L,-3,SDLK_LEFTBRACKET);
	// SDLK_LEFTPAREN
	lua_pushliteral(L,"(");
	lua_pushinteger(L,SDLK_LEFTPAREN);
	lua_rawset(L,-3);
	lua_pushliteral(L,"(");
	lua_rawseti(L,-3,SDLK_LEFTPAREN);
	// SDLK_LESS
	lua_pushliteral(L,"<");
	lua_pushinteger(L,SDLK_LESS);
	lua_rawset(L,-3);
	lua_pushliteral(L,"<");
	lua_rawseti(L,-3,SDLK_LESS);
	// SDLK_MINUS
	lua_pushliteral(L,"-");
	lua_pushinteger(L,SDLK_MINUS);
	lua_rawset(L,-3);
	lua_pushliteral(L,"-");
	lua_rawseti(L,-3,SDLK_MINUS);
	// SDLK_PERCENT
	lua_pushliteral(L,"%");
	lua_pushinteger(L,SDLK_PERCENT);
	lua_rawset(L,-3);
	lua_pushliteral(L,"%");
	lua_rawseti(L,-3,SDLK_PERCENT);
	// SDLK_PERIOD
	lua_pushliteral(L,".");
	lua_pushinteger(L,SDLK_PERIOD);
	lua_rawset(L,-3);
	lua_pushliteral(L,".");
	lua_rawseti(L,-3,SDLK_PERIOD);
	// SDLK_PLUS
	lua_pushliteral(L,"+");
	lua_pushinteger(L,SDLK_PLUS);
	lua_rawset(L,-3);
	lua_pushliteral(L,"+");
	lua_rawseti(L,-3,SDLK_PLUS);
	// SDLK_QUESTION
	lua_pushliteral(L,"?");
	lua_pushinteger(L,SDLK_QUESTION);
	lua_rawset(L,-3);
	lua_pushliteral(L,"?");
	lua_rawseti(L,-3,SDLK_QUESTION);
	// SDLK_QUOTE
	lua_pushliteral(L,"'");
	lua_pushinteger(L,SDLK_QUOTE);
	lua_rawset(L,-3);
	lua_pushliteral(L,"'");
	lua_rawseti(L,-3,SDLK_QUOTE);
	// SDLK_QUOTEDBL
	lua_pushliteral(L,"\"");
	lua_pushinteger(L,SDLK_QUOTEDBL);
	lua_rawset(L,-3);
	lua_pushliteral(L,"\"");
	lua_rawseti(L,-3,SDLK_QUOTEDBL);
	// SDLK_RIGHTBRACKET
	lua_pushliteral(L,"]");
	lua_pushinteger(L,SDLK_RIGHTBRACKET);
	lua_rawset(L,-3);
	lua_pushliteral(L,"]");
	lua_rawseti(L,-3,SDLK_RIGHTBRACKET);
	// SDLK_RIGHTPAREN
	lua_pushliteral(L,")");
	lua_pushinteger(L,SDLK_RIGHTPAREN);
	lua_rawset(L,-3);
	lua_pushliteral(L,")");
	lua_rawseti(L,-3,SDLK_RIGHTPAREN);
	// SDLK_SEMICOLON
	lua_pushliteral(L,";");
	lua_pushinteger(L,SDLK_SEMICOLON);
	lua_rawset(L,-3);
	lua_pushliteral(L,";");
	lua_rawseti(L,-3,SDLK_SEMICOLON);
	// SDLK_SLASH
	lua_pushliteral(L,"/");
	lua_pushinteger(L,SDLK_SLASH);
	lua_rawset(L,-3);
	lua_pushliteral(L,"/");
	lua_rawseti(L,-3,SDLK_SLASH);
	// SDLK_UNDERSCORE
	lua_pushliteral(L,"_");
	lua_pushinteger(L,SDLK_UNDERSCORE);
	lua_rawset(L,-3);
	lua_pushliteral(L,"_");
	lua_rawseti(L,-3,SDLK_UNDERSCORE);

	// Nav keys //
	// SDLK_DOWN
	lua_pushliteral(L,"down");
	lua_pushinteger(L,SDLK_DOWN);
	lua_rawset(L,-3);
	lua_pushliteral(L,"down");
	lua_rawseti(L,-3,SDLK_DOWN);
	// SDLK_LEFT
	lua_pushliteral(L,"left");
	lua_pushinteger(L,SDLK_LEFT);
	lua_rawset(L,-3);
	lua_pushliteral(L,"left");
	lua_rawseti(L,-3,SDLK_LEFT);
	// SDLK_RIGHT
	lua_pushliteral(L,"right");
	lua_pushinteger(L,SDLK_RIGHT);
	lua_rawset(L,-3);
	lua_pushliteral(L,"right");
	lua_rawseti(L,-3,SDLK_RIGHT);
	// SDLK_UP
	lua_pushliteral(L,"up");
	lua_pushinteger(L,SDLK_UP);
	lua_rawset(L,-3);
	lua_pushliteral(L,"up");
	lua_rawseti(L,-3,SDLK_UP);
	// SDLK_END
	lua_pushliteral(L,"end");
	lua_pushinteger(L,SDLK_END);
	lua_rawset(L,-3);
	lua_pushliteral(L,"end");
	lua_rawseti(L,-3,SDLK_END);
	// SDLK_HOME
	lua_pushliteral(L,"home");
	lua_pushinteger(L,SDLK_HOME);
	lua_rawset(L,-3);
	lua_pushliteral(L,"home");
	lua_rawseti(L,-3,SDLK_HOME);
	// SDLK_PAGEDOWN
	lua_pushliteral(L,"pagedown");
	lua_pushinteger(L,SDLK_PAGEDOWN);
	lua_rawset(L,-3);
	lua_pushliteral(L,"pagedown");
	lua_rawseti(L,-3,SDLK_PAGEDOWN);
	// SDLK_PAGEUP
	lua_pushliteral(L,"pageup");
	lua_pushinteger(L,SDLK_PAGEUP);
	lua_rawset(L,-3);
	lua_pushliteral(L,"pageup");
	lua_rawseti(L,-3,SDLK_PAGEUP);

	// Editing keys //
	// SDLK_BACKSPACE
	lua_pushliteral(L,"backspace");
	lua_pushinteger(L,SDLK_BACKSPACE);
	lua_rawset(L,-3);
	lua_pushliteral(L,"backspace");
	lua_rawseti(L,-3,SDLK_BACKSPACE);
	// SDLK_CLEAR
	lua_pushliteral(L,"clear");
	lua_pushinteger(L,SDLK_CLEAR);
	lua_rawset(L,-3);
	lua_pushliteral(L,"clear");
	lua_rawseti(L,-3,SDLK_CLEAR);
	// SDLK_DELETE
	lua_pushliteral(L,"delete");
	lua_pushinteger(L,SDLK_DELETE);
	lua_rawset(L,-3);
	lua_pushliteral(L,"delete");
	lua_rawseti(L,-3,SDLK_DELETE);
	// SDLK_INSERT
	lua_pushliteral(L,"insert");
	lua_pushinteger(L,SDLK_INSERT);
	lua_rawset(L,-3);
	lua_pushliteral(L,"insert");
	lua_rawseti(L,-3,SDLK_INSERT);
	// SDLK_RETURN
	lua_pushliteral(L,"return");
	lua_pushinteger(L,SDLK_RETURN);
	lua_rawset(L,-3);
	lua_pushliteral(L,"return");
	lua_rawseti(L,-3,SDLK_RETURN);
	// SDLK_TAB
	lua_pushliteral(L,"tab");
	lua_pushinteger(L,SDLK_TAB);
	lua_rawset(L,-3);
	lua_pushliteral(L,"tab");
	lua_rawseti(L,-3,SDLK_TAB);

	// Modifier keys //
	// SDLK_CAPSLOCK
	lua_pushliteral(L,"capslock");
	lua_pushinteger(L,SDLK_CAPSLOCK);
	lua_rawset(L,-3);
	lua_pushliteral(L,"capslock");
	lua_rawseti(L,-3,SDLK_CAPSLOCK);
	// SDLK_MODE
	lua_pushliteral(L,"mode");
	lua_pushinteger(L,SDLK_MODE);
	lua_rawset(L,-3);
	lua_pushliteral(L,"mode");
	lua_rawseti(L,-3,SDLK_MODE);
	// SDLK_NUMLOCKCLEAR
	lua_pushliteral(L,"numlock");
	lua_pushinteger(L,SDLK_NUMLOCKCLEAR);
	lua_rawset(L,-3);
	lua_pushliteral(L,"numlock");
	lua_rawseti(L,-3,SDLK_NUMLOCKCLEAR);
	// SDLK_SCROLLLOCK
	lua_pushliteral(L,"scrolllock");
	lua_pushinteger(L,SDLK_SCROLLLOCK);
	lua_rawset(L,-3);
	lua_pushliteral(L,"scrolllock");
	lua_rawseti(L,-3,SDLK_SCROLLLOCK);
	// SDLK_LGUI
	lua_pushliteral(L,"lgui");
	lua_pushinteger(L,SDLK_LGUI);
	lua_rawset(L,-3);
	lua_pushliteral(L,"lgui");
	lua_rawseti(L,-3,SDLK_LGUI);
	// SDLK_RGUI
	lua_pushliteral(L,"rgui");
	lua_pushinteger(L,SDLK_RGUI);
	lua_rawset(L,-3);
	lua_pushliteral(L,"rgui");
	lua_rawseti(L,-3,SDLK_RGUI);
	// SDLK_LSHIFT
	lua_pushliteral(L,"lshift");
	lua_pushinteger(L,SDLK_LSHIFT);
	lua_rawset(L,-3);
	lua_pushliteral(L,"lshift");
	lua_rawseti(L,-3,SDLK_LSHIFT);
	// SDLK_RSHIFT
	lua_pushliteral(L,"rshift");
	lua_pushinteger(L,SDLK_RSHIFT);
	lua_rawset(L,-3);
	lua_pushliteral(L,"rshift");
	lua_rawseti(L,-3,SDLK_RSHIFT);
	// SDLK_LALT
	lua_pushliteral(L,"lalt");
	lua_pushinteger(L,SDLK_LALT);
	lua_rawset(L,-3);
	lua_pushliteral(L,"lalt");
	lua_rawseti(L,-3,SDLK_LALT);
	// SDLK_RALT
	lua_pushliteral(L,"ralt");
	lua_pushinteger(L,SDLK_RALT);
	lua_rawset(L,-3);
	lua_pushliteral(L,"ralt");
	lua_rawseti(L,-3,SDLK_RALT);
	// SDLK_LCTRL
	lua_pushliteral(L,"lctrl");
	lua_pushinteger(L,SDLK_LCTRL);
	lua_rawset(L,-3);
	lua_pushliteral(L,"lctrl");
	lua_rawseti(L,-3,SDLK_LCTRL);
	// SDLK_RCTRL
	lua_pushliteral(L,"rctrl");
	lua_pushinteger(L,SDLK_RCTRL);
	lua_rawset(L,-3);
	lua_pushliteral(L,"rctrl");
	lua_rawseti(L,-3,SDLK_RCTRL);

	// misc keys //
	// SDLK_PAUSE
	lua_pushliteral(L,"pause");
	lua_pushinteger(L,SDLK_PAUSE);
	lua_rawset(L,-3);
	lua_pushliteral(L,"pause");
	lua_rawseti(L,-3,SDLK_PAUSE);
	// SDLK_ESCAPE
	lua_pushliteral(L,"escape");
	lua_pushinteger(L,SDLK_ESCAPE);
	lua_rawset(L,-3);
	lua_pushliteral(L,"escape");
	lua_rawseti(L,-3,SDLK_ESCAPE);
	// SDLK_HELP
	lua_pushliteral(L,"help");
	lua_pushinteger(L,SDLK_HELP);
	lua_rawset(L,-3);
	lua_pushliteral(L,"help");
	lua_rawseti(L,-3,SDLK_HELP);
	// SDLK_MENU
	lua_pushliteral(L,"menu");
	lua_pushinteger(L,SDLK_MENU);
	lua_rawset(L,-3);
	lua_pushliteral(L,"menu");
	lua_rawseti(L,-3,SDLK_MENU);
	// SDLK_PRINTSCREEN
	lua_pushliteral(L,"printscreen");
	lua_pushinteger(L,SDLK_PRINTSCREEN);
	lua_rawset(L,-3);
	lua_pushliteral(L,"printscreen");
	lua_rawseti(L,-3,SDLK_PRINTSCREEN);
	// SDLK_SYSREQ
	lua_pushliteral(L,"sysreq");
	lua_pushinteger(L,SDLK_SYSREQ);
	lua_rawset(L,-3);
	lua_pushliteral(L,"sysreq");
	lua_rawseti(L,-3,SDLK_SYSREQ);
	// SDLK_APPLICATION
	lua_pushliteral(L,"application");
	lua_pushinteger(L,SDLK_APPLICATION);
	lua_rawset(L,-3);
	lua_pushliteral(L,"application");
	lua_rawseti(L,-3,SDLK_APPLICATION);
	// SDLK_POWER
	lua_pushliteral(L,"power");
	lua_pushinteger(L,SDLK_POWER);
	lua_rawset(L,-3);
	lua_pushliteral(L,"power");
	lua_rawseti(L,-3,SDLK_POWER);
	// SDLK_CURRENCYUNIT
	lua_pushliteral(L,"currencyunit");
	lua_pushinteger(L,SDLK_CURRENCYUNIT);
	lua_rawset(L,-3);
	lua_pushliteral(L,"currencyunit");
	lua_rawseti(L,-3,SDLK_CURRENCYUNIT);
	// SDLK_UNDO
	lua_pushliteral(L,"undo");
	lua_pushinteger(L,SDLK_UNDO);
	lua_rawset(L,-3);
	lua_pushliteral(L,"undo");
	lua_rawseti(L,-3,SDLK_UNDO);

	lua_setfield(L, LUA_REGISTRYINDEX, LUNA_STR_TO_SDLK_TBL);
	lua_setfield(L, LUA_REGISTRYINDEX, LUNA_SDLK_TO_STR_TBL);
}


static void lh_luna_set_keycode(lua_State *L, SDL_Keysym *ks)
{
	lua_pushstring(L, LUNA_SDLK_TO_STR_TBL); // evt, tblnm
	lua_gettable(L, LUA_REGISTRYINDEX); // evt, tbl
	lua_pushinteger(L, ks->sym); // evt, tbl, int
	lua_gettable(L, -2); // evt, tbl, str
	lua_remove(L, -2); // remove lookup table from stack
	lua_setfield(L,-2,"key"); // set field in event table
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

	// add "window_id" field
	lua_pushinteger(L,e->key.windowID);
	lua_setfield(L,-2,"window_id");

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

	// set keysym info. It gets ugly here, so we're using a helper fn.
	lh_luna_set_keycode(L,&e->key.keysym);
}

static void lh_luna_make_mouse_motion_event(lua_State *L, SDL_Event *e)
{
	// the event table to be returned
	lua_newtable(L);

	// add type field
	lua_pushliteral(L,"mouse_motion");
	lua_setfield(L,-2,"type");

	// add timestamp
	lua_pushinteger(L,e->motion.timestamp);
	lua_setfield(L,-2,"timestamp");

	// add window_id
	lua_pushinteger(L,e->motion.windowID);
	lua_setfield(L,-2,"window_id");

	// add is_touch field
	int istouch = (e->motion.which == SDL_TOUCH_MOUSEID);
	lua_pushboolean(L,istouch);
	lua_setfield(L,-2,"is_touch");

	// set state table fields (event.buttons)
	lua_newtable(L); // table for event.buttons
	int left = (e->motion.state & SDL_BUTTON_LMASK);
	int middle = (e->motion.state & SDL_BUTTON_MMASK);
	int right = (e->motion.state & SDL_BUTTON_RMASK);
	int x1 = (e->motion.state & SDL_BUTTON_X1MASK);
	int x2 = (e->motion.state & SDL_BUTTON_X2MASK);
	lua_pushboolean(L,left);
	lua_setfield(L,-2,"left");
	lua_pushboolean(L,middle);
	lua_setfield(L,-2,"middle");
	lua_pushboolean(L,right);
	lua_setfield(L,-2,"right");
	lua_pushboolean(L,x1);
	lua_setfield(L,-2,"x1");
	lua_pushboolean(L,x2);
	lua_setfield(L,-2,"x2");
	lua_setfield(L,-2,"buttons"); // set event.buttons

	// set x field
	lua_pushinteger(L,e->motion.x);
	lua_setfield(L,-2,"x");

	// set y field
	lua_pushinteger(L, e->motion.y);
	lua_setfield(L,-2,"y");

	// set x_rel field
	lua_pushinteger(L, e->motion.xrel);
	lua_setfield(L,-2,"x_rel");

	// set y_rel field
	lua_pushinteger(L, e->motion.yrel);
	lua_setfield(L,-2,"y_rel");
}

static void lh_luna_make_mouse_button_event(lua_State *L, SDL_Event *e)
{
	// push our table to return
	lua_newtable(L);

	// set type field
	if (e->type == SDL_MOUSEBUTTONDOWN) {
		lua_pushliteral(L,"mouse_down");
	} else {
		lua_pushliteral(L,"mouse_up");
	}
	lua_setfield(L,-2,"type");

	// set timestamp
	lua_pushinteger(L,e->button.timestamp);
	lua_setfield(L,-2,"timestamp");

	// window_id
	lua_pushinteger(L,e->button.windowID);
	lua_setfield(L,-2,"window_id");

	// is_touch
	int istouch = (e->button.which == SDL_TOUCH_MOUSEID);
	lua_pushboolean(L,istouch);
	lua_setfield(L,-2,"is_touch");

	// set button field
	switch (e->button.button) {
		case SDL_BUTTON_LEFT:
			lua_pushliteral(L,"left");
			break;
		case SDL_BUTTON_MIDDLE:
			lua_pushliteral(L,"middle");
			break;
		case SDL_BUTTON_RIGHT:
			lua_pushliteral(L,"right");
			break;
		case SDL_BUTTON_X1:
			lua_pushliteral(L,"x1");
			break;
		case SDL_BUTTON_X2:
			lua_pushliteral(L,"x2");
			break;
	}
	lua_setfield(L,-2,"button");

	// set state field
	if (e->button.state == SDL_PRESSED) {
		lua_pushliteral(L,"pressed");
	} else {
		lua_pushliteral(L,"released");
	}
	lua_setfield(L,-2,"state");

	// x field
	lua_pushinteger(L,e->button.x);
	lua_setfield(L,-2,"x");

	// y field
	lua_pushinteger(L,e->button.y);
	lua_setfield(L,-2,"y");
}

static void lh_luna_make_mouse_wheel_event(lua_State *L, SDL_Event *e)
{
	lua_newtable(L);

	// timestamp
	lua_pushinteger(L, e->wheel.timestamp);
	lua_setfield(L,-2,"timestamp");
	// window_id
	lua_pushinteger(L, e->wheel.windowID);
	lua_setfield(L,-2,"window_id");
	// is_touch
	int istouch = (e->wheel.which == SDL_TOUCH_MOUSEID);
	lua_pushboolean(L,istouch);
	lua_setfield(L,-2,"is_touch");
	// x
	lua_pushinteger(L,e->wheel.x);
	lua_setfield(L,-2,"x");
	// y
	lua_pushinteger(L,e->wheel.y);
	lua_setfield(L,-2,"y");
}

static void lh_luna_make_controller_axis_event(lua_State *L, SDL_Event *e)
{
	// event table
	lua_newtable(L);

	// event type
	lua_pushliteral(L,"controller_axis");
	lua_setfield(L,-2,"type");

	// timestamp
	lua_pushinteger(L,e->caxis.timestamp);
	lua_setfield(L,-2,"timestamp");

	// controller_id
	lua_pushinteger(L,e->caxis.which);
	lua_setfield(L,-2,"controller_id");

	// axis
	switch (e->caxis.axis) {
		case SDL_CONTROLLER_AXIS_LEFTX:
			lua_pushliteral(L,"left_x");
			break;
		case SDL_CONTROLLER_AXIS_LEFTY:
			lua_pushliteral(L,"left_y");
			break;
		case SDL_CONTROLLER_AXIS_RIGHTX:
			lua_pushliteral(L,"right_x");
			break;
		case SDL_CONTROLLER_AXIS_RIGHTY:
			lua_pushliteral(L,"right_y");
			break;
		case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
			lua_pushliteral(L,"left_trigger");
			break;
		case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
			lua_pushliteral(L,"right_trigger");
			break;
	}
	lua_setfield(L,-2,"axis");

	// value:
	lua_pushinteger(L,e->caxis.value);
	lua_setfield(L,-2,"value");
}

static void lh_luna_make_controller_button_event(lua_State *L, SDL_Event *e)
{
	// table
	lua_newtable(L);

	// type
	if (e->cbutton.type == SDL_CONTROLLERBUTTONDOWN) {
		lua_pushliteral(L,"controller_button_down");
	} else {
		lua_pushliteral(L,"controller_button_up");
	}
	lua_setfield(L,-2,"type");

	// timestamp
	lua_pushinteger(L,e->cbutton.timestamp);
	lua_setfield(L,-2,"timestamp");

	// controller_id
	lua_pushinteger(L,e->cbutton.which);
	lua_setfield(L,-2,"controller_id");

	// button
	switch (e->cbutton.button) {
		case SDL_CONTROLLER_BUTTON_A:
			lua_pushliteral(L,"a");
			break;
		case SDL_CONTROLLER_BUTTON_B:
			lua_pushliteral(L,"b");
			break;
		case SDL_CONTROLLER_BUTTON_X:
			lua_pushliteral(L,"x");
			break;
		case SDL_CONTROLLER_BUTTON_Y:
			lua_pushliteral(L,"y");
			break;
		case SDL_CONTROLLER_BUTTON_BACK:
			lua_pushliteral(L,"back");
			break;
		case SDL_CONTROLLER_BUTTON_GUIDE:
			lua_pushliteral(L,"guide");
			break;
		case SDL_CONTROLLER_BUTTON_START:
			lua_pushliteral(L,"start");
			break;
		case SDL_CONTROLLER_BUTTON_LEFTSTICK:
			lua_pushliteral(L,"left_stick");
			break;
		case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
			lua_pushliteral(L,"right_stick");
			break;
		case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
			lua_pushliteral(L,"left_shoulder");
			break;
		case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
			lua_pushliteral(L,"right_shoulder");
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_UP:
			lua_pushliteral(L,"dpad_up");
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
			lua_pushliteral(L,"dpad_down");
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
			lua_pushliteral(L,"dpad_left");
			break;
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
			lua_pushliteral(L,"dpad_right");
			break;
	}
	lua_setfield(L,-2,"button");

	// state
	if (e->cbutton.state == SDL_PRESSED) {
		lua_pushliteral(L,"pressed");
	} else {
		lua_pushliteral(L,"released");
	}
	lua_setfield(L,-2,"state");
}

static void lh_luna_make_controller_device_event(lua_State *L, SDL_Event *e)
{
	lua_newtable(L);
	switch (e->type) {
		case SDL_CONTROLLERDEVICEADDED:
			lua_pushliteral(L,"controller_added");
			break;
		case SDL_CONTROLLERDEVICEREMAPPED:
			lua_pushliteral(L,"controller_remapped");
			break;
		case SDL_CONTROLLERDEVICEREMOVED:
			lua_pushliteral(L,"controller_removed");
			break;
	}
	lua_setfield(L,-2,"type");

	// timestamp
	lua_pushinteger(L,e->cdevice.timestamp);
	lua_setfield(L,-2,"timestamp");

	// controller_id
	lua_pushinteger(L,e->cdevice.which);
	lua_setfield(L,-2,"controller_id");
}

static void lh_luna_make_window_event(lua_State *L, SDL_Event *e)
{
	lua_pushnil(L); // this gets copied over by the event type
	lua_newtable(L);

	// timestamp
	lua_pushinteger(L,e->window.timestamp);
	lua_setfield(L,-2,"timestamp");
	// window_id
	lua_pushinteger(L,e->window.windowID);
	lua_setfield(L,-2,"window_id");
	// type (and data)
	switch (e->window.event) {
		case SDL_WINDOWEVENT_SHOWN:
			lua_pushliteral(L,"window_shown");
			lua_copy(L,-1,-3);
			lua_setfield(L,-2,"type");
			break;
		case SDL_WINDOWEVENT_HIDDEN:
			lua_pushliteral(L,"window_hidden");
			lua_copy(L,-1,-3);
			lua_setfield(L,-2,"type");
			break;
		case SDL_WINDOWEVENT_EXPOSED:
			lua_pushliteral(L,"window_exposed");
			lua_copy(L,-1,-3);
			lua_setfield(L,-2,"type");
			break;
		case SDL_WINDOWEVENT_MOVED:
			lua_pushinteger(L,e->window.data1);
			lua_setfield(L,-2,"x");
			lua_pushinteger(L,e->window.data2);
			lua_setfield(L,-2,"y");
			lua_pushliteral(L,"window_moved");
			lua_copy(L,-1,-3);
			lua_setfield(L,-2,"type");
			break;
		case SDL_WINDOWEVENT_RESIZED:
			lua_pushinteger(L,e->window.data1);
			lua_setfield(L,-2,"w");
			lua_pushinteger(L,e->window.data2);
			lua_setfield(L,-2,"h");
			lua_pushliteral(L,"window_resized");
			lua_copy(L,-1,-3);
			lua_setfield(L,-2,"type");
			break;
		case SDL_WINDOWEVENT_MINIMIZED:
			lua_pushliteral(L,"window_minimized");
			lua_copy(L,-1,-3);
			lua_setfield(L,-2,"type");
			break;
		case SDL_WINDOWEVENT_MAXIMIZED:
			lua_pushliteral(L,"window_maximized");
			lua_copy(L,-1,-3);
			lua_setfield(L,-2,"type");
			break;
		case SDL_WINDOWEVENT_RESTORED:
			lua_pushliteral(L,"window_restored");
			lua_copy(L,-1,-3);
			lua_setfield(L,-2,"type");
			break;
		case SDL_WINDOWEVENT_ENTER:
			lua_pushliteral(L,"window_enter");
			lua_copy(L,-1,-3);
			lua_setfield(L,-2,"type");
			break;
		case SDL_WINDOWEVENT_LEAVE:
			lua_pushliteral(L,"window_leave");
			lua_copy(L,-1,-3);
			lua_setfield(L,-2,"type");
			break;
		case SDL_WINDOWEVENT_FOCUS_GAINED:
			lua_pushliteral(L,"window_focus_gained");
			lua_copy(L,-1,-3);
			lua_setfield(L,-2,"type");
			break;
		case SDL_WINDOWEVENT_FOCUS_LOST:
			lua_pushliteral(L,"window_focus_lost");
			lua_copy(L,-1,-3);
			lua_setfield(L,-2,"type");
			break;
		case SDL_WINDOWEVENT_CLOSE:
			lua_pushliteral(L,"window_close");
			lua_copy(L,-1,-3);
			lua_setfield(L,-2,"type");
			break;
	}
}

// luna.event.poll() -> type: string, event: luna.Event
static int luna_event_poll(lua_State *L)
{
	SDL_Event event;
	SDL_PollEvent(&event);
	// push a table to use as our return value
	switch (event.type) {
		// keyboard events
		case SDL_KEYDOWN:
			lua_pushliteral(L,"key_down");
			lh_luna_make_key_event(L,&event);
			break;
		case SDL_KEYUP:
			lua_pushliteral(L,"key_up");
			lh_luna_make_key_event(L, &event);
			break;
		// mouse events
		case SDL_MOUSEMOTION:
			lua_pushliteral(L,"mouse_motion");
			lh_luna_make_mouse_motion_event(L,&event);
			break;
		case SDL_MOUSEBUTTONDOWN:
			lua_pushliteral(L,"mouse_down");
			lh_luna_make_mouse_button_event(L,&event);
			break;
		case SDL_MOUSEBUTTONUP:
			lua_pushliteral(L,"mouse_up");
			lh_luna_make_mouse_button_event(L,&event);
			break;
		case SDL_MOUSEWHEEL:
			lua_pushliteral(L,"mouse_wheel");
			lh_luna_make_mouse_wheel_event(L,&event);
			break;
		// controller events
		case SDL_CONTROLLERAXISMOTION:
			lua_pushliteral(L,"controller_axis");
			lh_luna_make_controller_axis_event(L,&event);
			break;
		case SDL_CONTROLLERBUTTONDOWN:
			lua_pushliteral(L,"controller_button_down");
			lh_luna_make_controller_button_event(L,&event);
			break;
		case SDL_CONTROLLERBUTTONUP:
			lua_pushliteral(L,"controller_button_up");
			lh_luna_make_controller_button_event(L,&event);
			break;
		case SDL_CONTROLLERDEVICEADDED:
			lua_pushliteral(L,"controller_added");
			lh_luna_make_controller_device_event(L,&event);
			break;
		case SDL_CONTROLLERDEVICEREMOVED:
			lua_pushliteral(L,"controller_removed");
			lh_luna_make_controller_device_event(L,&event);
			break;
		case SDL_CONTROLLERDEVICEREMAPPED:
			lua_pushliteral(L,"controller_remapped");
			lh_luna_make_controller_device_event(L,&event);
			break;
		case SDL_WINDOWEVENT:
			// helper function handles pushing the type too!
			lh_luna_make_window_event(L,&event);
		case SDL_QUIT:
			// this is small enough to handle here
			lua_pushliteral(L,"quit");
			lua_newtable(L);
			lua_pushvalue(L,-2); // push copy of "quit"
			lua_setfield(L,-2,"type");
			lua_pushinteger(L, event.quit.timestamp);
			lua_setfield(L,-2,"timestamp");
			break;
		default:
			break;
	}
	return 2; // returns: type:string, event:table
}

// luna.event module
static const luaL_Reg luna_event_module_fns[] = {
	{"poll", &luna_event_poll},
	{NULL,NULL}
};


// ////////////////////////////
// luna.keyboard.* functions //
// ////////////////////////////

static int lh_luna_init_keymod_table(lua_State *L)
{
	// str->kmod
	lua_newtable(L);
	// kmod->str
	lua_newtable(L);

	// KMOD_NONE
	lua_pushliteral(L,"none");
	lua_pushinteger(L,KMOD_NONE);
	lua_rawset(L,-3);
	lua_pushliteral(L,"none");
	lua_rawseti(L,-3,KMOD_NONE);
	// KMOD_LSHIFT
	lua_pushliteral(L,"lshift");
	lua_pushinteger(L,KMOD_LSHIFT);
	lua_rawset(L,-3);
	lua_pushliteral(L,"lshift");
	lua_rawseti(L,-3,KMOD_LSHIFT);
	// KMOD_RSHIFT
	lua_pushliteral(L,"rshift");
	lua_pushinteger(L,KMOD_RSHIFT);
	lua_rawset(L,-3);
	lua_pushliteral(L,"rshift");
	lua_rawseti(L,-3,KMOD_RSHIFT);
	// KMOD_LCTRL
	lua_pushliteral(L,"lctrl");
	lua_pushinteger(L,KMOD_LCTRL);
	lua_rawset(L,-3);
	lua_pushliteral(L,"lctrl");
	lua_rawseti(L,-3,KMOD_LCTRL);
	// KMOD_RCTRL
	lua_pushliteral(L,"rctrl");
	lua_pushinteger(L,KMOD_RCTRL);
	lua_rawset(L,-3);
	lua_pushliteral(L,"rctrl");
	lua_rawseti(L,-3,KMOD_RCTRL);
	// KMOD_LALT
	lua_pushliteral(L,"lalt");
	lua_pushinteger(L,KMOD_LALT);
	lua_rawset(L,-3);
	lua_pushliteral(L,"lalt");
	lua_rawseti(L,-3,KMOD_LALT);
	// KMOD_RALT
	lua_pushliteral(L,"ralt");
	lua_pushinteger(L,KMOD_RALT);
	lua_rawset(L,-3);
	lua_pushliteral(L,"ralt");
	lua_rawseti(L,-3,KMOD_RALT);
	// KMOD_LGUI
	lua_pushliteral(L,"lgui");
	lua_pushinteger(L,KMOD_LGUI);
	lua_rawset(L,-3);
	lua_pushliteral(L,"lgui");
	lua_rawseti(L,-3,KMOD_LGUI);
	// KMOD_RGUI
	lua_pushliteral(L,"rgui");
	lua_pushinteger(L,KMOD_RGUI);
	lua_rawset(L,-3);
	lua_pushliteral(L,"rgui");
	lua_rawseti(L,-3,KMOD_RGUI);
	// KMOD_NUM
	lua_pushliteral(L,"num");
	lua_pushinteger(L,KMOD_NUM);
	lua_rawset(L,-3);
	lua_pushliteral(L,"num");
	lua_rawseti(L,-3,KMOD_NUM);
	// KMOD_CAPS
	lua_pushliteral(L,"caps");
	lua_pushinteger(L,KMOD_CAPS);
	lua_rawset(L,-3);
	lua_pushliteral(L,"caps");
	lua_rawseti(L,-3,KMOD_CAPS);
	// KMOD_MODE
	lua_pushliteral(L,"mode");
	lua_pushinteger(L,KMOD_MODE);
	lua_rawset(L,-3);
	lua_pushliteral(L,"mode");
	lua_rawseti(L,-3,KMOD_MODE);
	// KMOD_CTRL
	lua_pushliteral(L,"ctrl");
	lua_pushinteger(L,KMOD_CTRL);
	lua_rawset(L,-3);
	lua_pushliteral(L,"ctrl");
	lua_rawseti(L,-3,KMOD_CTRL);
	// KMOD_SHIFT
	lua_pushliteral(L,"shift");
	lua_pushinteger(L,KMOD_SHIFT);
	lua_rawset(L,-3);
	lua_pushliteral(L,"shift");
	lua_rawseti(L,-3,KMOD_SHIFT);
	// KMOD_ALT
	lua_pushliteral(L,"alt");
	lua_pushinteger(L,KMOD_ALT);
	lua_rawset(L,-3);
	lua_pushliteral(L,"alt");
	lua_rawseti(L,-3,KMOD_ALT);
	// KMOD_GUI
	lua_pushliteral(L,"gui");
	lua_pushinteger(L,KMOD_GUI);
	lua_rawset(L,-3);
	lua_pushliteral(L,"gui");
	lua_rawseti(L,-3,KMOD_GUI);

	lua_setfield(L, LUA_REGISTRYINDEX, LUNA_STR_TO_KEYMOD_TBL);
	lua_setfield(L, LUA_REGISTRYINDEX, LUNA_KEYMOD_TO_STR_TBL);
	return 0;
}

// luna.keyboard.key_down(key:key_string) -> boolean
static int luna_keyboard_key_down(lua_State *L)
{
	const char *keystr = luaL_checkstring(L, 1);

	lua_pushstring(L, LUNA_STR_TO_SDLK_TBL); // key, tblnm
	lua_gettable(L, LUA_REGISTRYINDEX); // key, tbl
	lua_pushstring(L, keystr); // key, tbl, key
	lua_gettable(L, -2); // key, tbl, sdlk
	// get the sdlk constant
	int sdlk = lua_tointeger(L, -1);
	int sc = SDL_GetScancodeFromKey(sdlk);
	lua_pushboolean(L, KEYBOARD_STATE[sc]);
	return 1;
}

// luna.keyboard.mod_down(mod:mod_string) -> boolean
static int luna_keyboard_mod_down(lua_State *L)
{
	const char *modstr = luaL_checkstring(L, 1);
	lua_pushstring(L, LUNA_STR_TO_KEYMOD_TBL);
	lua_gettable(L, LUA_REGISTRYINDEX);
	lua_pushstring(L, modstr);
	lua_gettable(L, -2);
	int mod = lua_tointeger(L,-1);
	int ret = mod & SDL_GetModState();
	lua_pushboolean(L, ret);
	return 1;
}

static luaL_Reg luna_keyboard_module_fns[] = {
	{"key_down", &luna_keyboard_key_down},
	{"mod_down", &luna_keyboard_mod_down},
	{NULL,NULL}
};

// //////////////////////////////////
// luna.window.* functions/methods //
// //////////////////////////////////

// luna.Window.new(w:int,h:int,fullscreen:boolean) -> luna.Window
static int luna_c_window_new(lua_State *L)
{
	int w = luaL_checkinteger(L,1);
	int h = luaL_checkinteger(L,2);
	luaL_checktype(L,3,LUA_TBOOLEAN);
	int fs = lua_toboolean(L,3);
	luna_Window *win = lua_newuserdata(L, sizeof(*win));

	int win_flags = SDL_WINDOW_HIDDEN;
	if (fs) {
		win_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}
	int err = SDL_CreateWindowAndRenderer(w, h,
			win_flags, &win->window, &win->renderer);

	if (err) {
		lua_pushfstring(L,"ERROR: luna.window.new failed: %s\n", SDL_GetError);
		lua_error(L);
	}

	win->closed = 0;
	luaL_setmetatable(L,LUNA_WINDOW_MT);
	return 1; // return our window
}

// luna.Window:show()
static int luna_m_window_show(lua_State *L)
{
	luna_Window *win = luaL_checkudata(L,1,LUNA_WINDOW_MT);
	SDL_ShowWindow(win->window);
	return 0;
}

static int luna_m_window_hide(lua_State *L)
{
	luna_Window *win = luaL_checkudata(L,1,LUNA_WINDOW_MT);
	SDL_HideWindow(win->window);
	return 0;
}

// luna.Window:close()
static int luna_m_window_close(lua_State *L)
{
	luna_Window *win = luaL_checkudata(L,1,LUNA_WINDOW_MT);
	if (!win->closed) {
		SDL_DestroyRenderer(win->renderer);
		SDL_DestroyWindow(win->window);
		win->closed = 1;
	}
	return 0;
}

//  luna.Window:draw(tex: luna.Texture, x:int, y:int)
//  	stages drawing; need to paint to actually display
static int luna_m_window_draw(lua_State *L)
{
	luna_Window *win = luaL_checkudata(L,1,LUNA_WINDOW_MT);
	luna_Texture *tex = luaL_checkudata(L,2,LUNA_TEXTURE_MT);
	int x = luaL_checkinteger(L,3);
	int y = luaL_checkinteger(L,4);

	SDL_Rect dst = (SDL_Rect) {.x = x, .y = y, .w = tex->raw_w, .h = tex->raw_h};

	if (tex->is_framed) {
		dst.w = tex->frame.w;
		dst.h = tex->frame.h;
		SDL_RenderCopy(win->renderer, tex->texture, &tex->frame, &dst);
	} else {
		dst.w = tex->raw_w;
		dst.h = tex->raw_h;
		SDL_RenderCopy(win->renderer, tex->texture, NULL, &dst);
	}
	return 0;
}

// luna.Window:paint()
static int luna_m_window_paint(lua_State *L)
{
	luna_Window *win = luaL_checkudata(L,1,LUNA_WINDOW_MT);
	SDL_RenderPresent(win->renderer);
	return 0;
}

// luna.Window:clear()
static int luna_m_window_clear(lua_State *L)
{
	luna_Window *win = luaL_checkudata(L,1,LUNA_WINDOW_MT);
	SDL_RenderClear(win->renderer);
	return 0;
}

// luna.Window.__gc() -- called on garbage collection
static int luna_m_window_gc(lua_State *L)
{
	// all we need to do is close it
	luna_m_window_close(L);
	return 0;
}

static const luaL_Reg luna_window_module_fns[] = {
	{"new", &luna_c_window_new},
	{NULL,NULL}
};

static const luaL_Reg luna_m_window_metatable[] = {
	{"close", &luna_m_window_close},
	{"draw", &luna_m_window_draw},
	{"paint", &luna_m_window_paint},
	{"clear", &luna_m_window_clear},
	{"show", &luna_m_window_show},
	{"hide", &luna_m_window_hide},
	{"__gc", &luna_m_window_gc},
	{NULL,NULL}
};

// ///////////////
// luna.Texture //
// ///////////////

// luna.Texture.new(window:luna.Window,filename:string) -> luna.Texture
static int luna_c_texture_new(lua_State *L)
{
	// TODO maybe add colorkey support?
	luna_Window *win = luaL_checkudata(L,1,LUNA_WINDOW_MT);
	const char *fname = luaL_checkstring(L,2);

	luna_Texture *tex = lua_newuserdata(L, sizeof(*tex));
	// get our surface from the filename
	SDL_Surface *surf = IMG_Load(fname);
	if (!surf) {
		lua_pushfstring(L,"ERROR luna.Texture.new IMG_Load: %s\n",
				IMG_GetError());
		lua_error(L);
	}
	tex->raw_w = surf->w;
	tex->raw_h = surf->h;
	// start out with no framing
	tex->is_framed = 0;
	tex->frame = (SDL_Rect) {};
	// create our texture
	tex->texture = SDL_CreateTextureFromSurface(win->renderer,surf);
	if (!tex->texture) {
		lua_pushfstring(L,
				"ERROR luna.Texture.new: SDL_CreateTextureFromSurface: %s\n",
				SDL_GetError());
		lua_error(L);
	}
	// free our temporary surface
	SDL_FreeSurface(surf);


	luaL_setmetatable(L,LUNA_TEXTURE_MT);
	return 1;
}

// luna.Texture:raw_w() -> int
static int luna_m_texture_raw_w(lua_State *L)
{
	luna_Texture *t = luaL_checkudata(L,1,LUNA_TEXTURE_MT);
	lua_pushinteger(L,t->raw_w);
	return 1;
}

// luna.Texture:raw_h() -> int
static int luna_m_texture_raw_h(lua_State *L)
{
	luna_Texture *t = luaL_checkudata(L,1,LUNA_TEXTURE_MT);
	lua_pushinteger(L,t->raw_h);
	return 1;
}

// luna.Texture:frame() -> table{x:int, y:int, w:int, h:int}
static int luna_m_texture_frame(lua_State *L)
{
	luna_Texture *t = luaL_checkudata(L,1,LUNA_TEXTURE_MT);
	if (t->is_framed) {
		lua_newtable(L);
		lua_pushinteger(L,t->frame.x);
		lua_setfield(L,-2,"x");
		lua_pushinteger(L,t->frame.y);
		lua_setfield(L,-2,"y");
		lua_pushinteger(L,t->frame.w);
		lua_setfield(L,-2,"w");
		lua_pushinteger(L,t->frame.h);
		lua_setfield(L,-2,"h");
	} else {
		lua_pushnil(L);
	}
	return 1;
}

// luna.Texture:set_frame(dims:table{x:int, y:int, w:int, h:int})
// OR
// luna.Texture:set_frame(x:int, y:int, w:int, h:int)
static int luna_m_texture_set_frame(lua_State *L)
{
	luna_Texture *t = luaL_checkudata(L,1,LUNA_TEXTURE_MT);
	// see if we got a table or 4 ints; 2 args = table, 5 args = ints
	int nargs = lua_gettop(L);

	// table version
	if (nargs == 2) {
		luaL_checktype(L,2,LUA_TTABLE);

		lua_getfield(L,2,"x");
		t->frame.x = lua_tointeger(L,-1);
		lua_pop(L,1);

		lua_getfield(L,2,"y");
		t->frame.y = lua_tointeger(L,-1);
		lua_pop(L,1);

		lua_getfield(L,2,"w");
		t->frame.w = lua_tointeger(L,-1);
		lua_pop(L,1);

		lua_getfield(L,2,"h");
		t->frame.h = lua_tointeger(L,-1);
		lua_pop(L,1);
	} else {
		t->frame.x = luaL_checkinteger(L,2);
		t->frame.y = luaL_checkinteger(L,3);
		t->frame.w = luaL_checkinteger(L,4);
		t->frame.h = luaL_checkinteger(L,5);
	}
	t->is_framed = 1;
	return 0;
}
// luna.Texture:is_framed() -> boolean
static int luna_m_texture_is_framed(lua_State *L)
{
	luna_Texture *t = luaL_checkudata(L,1,LUNA_TEXTURE_MT);
	lua_pushboolean(L,t->is_framed);
	return 1;
}

// luna.Texture:unframe()
static int luna_m_texture_unframe(lua_State *L)
{
	luna_Texture *t = luaL_checkudata(L,1,LUNA_TEXTURE_MT);
	t->is_framed = 0;
	return 0;
}

// luna.Texture:__gc() (garbage collection)
static int luna_m_texture_gc(lua_State *L)
{
	luna_Texture *tex = luaL_checkudata(L,1,LUNA_TEXTURE_MT);
	SDL_DestroyTexture(tex->texture);
	return 0;
}

static const luaL_Reg luna_texture_module_fns[] = {
	{"new", &luna_c_texture_new},
	{NULL,NULL}
};

static const luaL_Reg luna_m_texture_metatable[] =  {
	{"raw_w", &luna_m_texture_raw_w},
	{"raw_h", &luna_m_texture_raw_h},
	{"frame", &luna_m_texture_frame},
	{"set_frame", &luna_m_texture_set_frame},
	{"is_framed", &luna_m_texture_is_framed},
	{"unframe", &luna_m_texture_unframe},
	{"__gc", &luna_m_texture_gc},
	{NULL,NULL}
};

// ///////////////////////
// luna.music functions //
// ///////////////////////

// luna.music.new(filename:string) -> luna.Music
static int luna_c_music_new(lua_State *L)
{
	const char *filename = luaL_checkstring(L,1);

	luna_Music *m = lua_newuserdata(L,sizeof(*m));
	m->music = Mix_LoadMUS(filename);
	if (!m->music) {
		lua_pushfstring(L,"luna.Music.new: Mix_LoadMUS: %s\n", Mix_GetError());
		lua_error(L);
	}

	luaL_setmetatable(L,LUNA_MUSIC_MT);
	return 1;
}

// luna.music.pause()
static int luna_music_pause(lua_State *L)
{
	Mix_PauseMusic();
	return 0;
}

// luna.music.resume()
static int luna_music_resume(lua_State *L)
{
	Mix_ResumeMusic();
	return 0;
}

// luna.music.rewind()
static int luna_music_rewind(lua_State *L)
{
	Mix_RewindMusic();
	return 0;
}

// luna.music.set_volume(volume:int)
static int luna_music_set_volume(lua_State *L)
{
	int volume = luaL_checkinteger(L,1);
	Mix_VolumeMusic(volume);
	return 0;
}

// luna.music.volume() -> int
static int luna_music_volume(lua_State *L)
{
	int volume = Mix_VolumeMusic(-1);
	lua_pushinteger(L,volume);
	return 1;
}

// luna.music.halt()
static int luna_music_halt(lua_State *L)
{
	Mix_HaltMusic();
	return 0;
}

// luna.music.fade_out(ms:int)
static int luna_music_fade_out(lua_State *L)
{
	int ms = luaL_checkinteger(L,1);
	Mix_FadeOutMusic(ms);
	return 0;
}

// luna.music.set_position(seconds:number)
static int luna_music_set_position(lua_State *L)
{
	double seconds = luaL_checknumber(L,1);
	Mix_RewindMusic();
	Mix_SetMusicPosition(seconds);
	return 0;
}

// luna.music.is_paused() -> boolean
static int luna_music_is_paused(lua_State *L)
{
	lua_pushboolean(L,Mix_PausedMusic());
	return 1;
}
// luna.music.is_playing() -> boolean
static int luna_music_is_playing(lua_State *L)
{
	lua_pushboolean(L,Mix_PlayingMusic());
	return 1;
}
// luna.music.is_fading() -> fading:boolean, dir:string['in' | 'out']
static int luna_music_is_fading(lua_State *L)
{
	switch (Mix_FadingMusic()) {
		case MIX_NO_FADING:
			lua_pushboolean(L,0);
			lua_pushnil(L);
			break;
		case MIX_FADING_IN:
			lua_pushboolean(L,1);
			lua_pushliteral(L,"in");
			break;
		case MIX_FADING_OUT:
			lua_pushboolean(L,1);
			lua_pushliteral(L,"out");
			break;
	}
	return 2;
}

static luaL_Reg luna_music_module_fns[] = {
	{"new", &luna_c_music_new},
	{"pause", &luna_music_pause},
	{"resume", &luna_music_resume},
	{"rewind", &luna_music_rewind},
	{"set_volume", &luna_music_set_volume},
	{"volume", &luna_music_volume},
	{"halt", &luna_music_halt},
	{"fade_out", &luna_music_fade_out},
	{"set_position", &luna_music_set_position},
	{"is_playing", &luna_music_is_playing},
	{"is_paused", &luna_music_is_paused},
	{"is_fading", &luna_music_is_fading},
	{NULL,NULL}
};

// /////////////////////
// luna.Music methods //
// /////////////////////

// luna.Music:play([loops:int = -1]) -- loop forever if no argument!
static int luna_m_music_play(lua_State *L)
{
	luna_Music *m = luaL_checkudata(L,1,LUNA_MUSIC_MT);
	int loops = luaL_optinteger(L,2,-1); // loop forever if no argument
	Mix_PlayMusic(m->music, loops);
	return 0;
}

// luna.Music:play_from(start_pos:number, [loops:int = -1])
static int luna_m_play_from(lua_State *L)
{
	luna_Music *m = luaL_checkudata(L,1,LUNA_MUSIC_MT);
	double pos = luaL_checknumber(L,2);
	int loops = luaL_optinteger(L,3,-1);
	Mix_FadeInMusicPos(m->music,loops,0,pos);
	return 0;
}

// luna.Music:fade_in(ms:int, [loops:int = -1])
static int luna_m_music_fade_in(lua_State *L)
{
	luna_Music *m = luaL_checkudata(L,1,LUNA_MUSIC_MT);
	int ms = luaL_checkinteger(L,2);
	int loops = luaL_optinteger(L,3,-1);
	Mix_FadeInMusic(m->music, loops, ms);
	return 0;
}

// luna.Music:fade_in_from(pos:number, ms:int, [loops:int = -1])
static int luna_m_music_fade_in_from(lua_State *L)
{
	luna_Music *m = luaL_checkudata(L,1,LUNA_MUSIC_MT);
	double pos = luaL_checknumber(L,2);
	int ms = luaL_checkinteger(L,3);
	int loops = luaL_optinteger(L,4,-1);
	Mix_FadeInMusicPos(m->music, loops, ms, pos);
	return 0;
}

// luna.Music.__gc() -- garbage collection
static int luna_m_music_gc(lua_State *L)
{
	luna_Music *m = luaL_checkudata(L,1,LUNA_MUSIC_MT);
	Mix_FreeMusic(m->music);
	return 0;
}


static luaL_Reg luna_m_music_metatable[] = {
	{"play", &luna_m_music_play},
	{"play_from", &luna_m_play_from},
	{"fade_in", &luna_m_music_fade_in},
	{"fade_in_from", &luna_m_music_fade_in_from},
	{"__gc", &luna_m_music_gc},
	{NULL,NULL}
};

// ///////////////////////
// luna.sound functions //
// ///////////////////////

// luna.sound.new(filename:string) -> luna.Sound
static int luna_sound_new(lua_State *L)
{
	const char *fname = luaL_checkstring(L,1);
	luna_Sound *s = lua_newuserdata(L, sizeof(*s));

	s->chunk = Mix_LoadWAV(fname);
	s->channel = -1; // we want to play on the first unreserved channel

	luaL_setmetatable(L, LUNA_SOUND_MT);
	return 1;
}

// luna.sound.pause_all()
static int luna_sound_pause_all(lua_State *L)
{
	Mix_Pause(-1);
	return 0;
}

// luna.sound.resume_all()
static int luna_sound_resume_all(lua_State *L)
{
	Mix_Resume(-1);
	return 0;
}

// luna.sound.halt_all()
static int luna_sound_halt_all(lua_State *L)
{
	Mix_HaltChannel(-1);
	return 0;
}

// luna.sound.halt_timed_all(ms: int)
static int luna_sound_halt_timed_all(lua_State *L)
{
	int ms = luaL_checkinteger(L,1);
	Mix_ExpireChannel(-1, ms);
	return 0;
}
// luna.sound.fade_out_all(ms: int)
static int luna_sound_fade_out_all(lua_State *L)
{
	int ms = luaL_checkinteger(L,1);
	Mix_FadeOutChannel(-1, ms);
	return 0;
}

// luna.sound.set_volume_all(volume: int)
static int luna_sound_set_volume_all(lua_State *L)
{
	int vol = luaL_checkinteger(L,1);
	Mix_Volume(-1, vol);
	return 0;
}
// luna.sound.number_playing() -> int
static int luna_sound_number_playing(lua_State *L)
{
	int playing = Mix_Playing(-1);
	lua_pushinteger(L,playing);
	return 1;
}
// luna.sound.number_paused() -> int
static int luna_sound_number_paused(lua_State *L)
{
	int paused = Mix_Paused(-1);
	lua_pushinteger(L,paused);
	return 1;
}

// Module def
static luaL_Reg luna_sound_module_fns[] = {
	{"new", &luna_sound_new},
	{"pause_all", &luna_sound_pause_all},
	{"resume_all", &luna_sound_resume_all},
	{"halt_all", &luna_sound_halt_all},
	{"halt_timed_all", &luna_sound_halt_timed_all},
	{"fade_out_all", &luna_sound_fade_out_all},
	{"set_volume_all", &luna_sound_set_volume_all},
	{"number_playing", &luna_sound_number_playing},
	{"number_paused", &luna_sound_number_paused},
	{NULL,NULL}
};

// /////////////////////
// luna.Sound methods //
// /////////////////////

// luna.Sound:play([repeats:int = 0])) -- SPECIAL: -1 repeats = forever
static int luna_m_sound_play(lua_State *L)
{
	luna_Sound *s = luaL_checkudata(L, 1, LUNA_SOUND_MT);
	// default is no repeats
	int loops = luaL_optinteger(L, 2, 0);
	// we can just use s->channel here because it's initially set to -1
	// so we automatically play on an unreserved channel. Then we store the
	// channel we actually used in our userdata so Sound:pause() etc. works.
	int chan = Mix_PlayChannel(s->channel, s->chunk, loops);
	s->channel = chan;
	return 0;
}

// luna.Sound:play_timed(ms:int, [repeats:int = 0]) -- -1 repeats = forever
static int luna_m_sound_play_timed(lua_State *L)
{
	// same implementation as luna_m_sound_play
	luna_Sound *s = luaL_checkudata(L,1,LUNA_SOUND_MT);
	int ms = luaL_checkinteger(L,2);
	int loops = luaL_optinteger(L,3,0);
	int chan = Mix_PlayChannelTimed(s->channel, s->chunk, loops, ms);
	s->channel = chan;
	return 0;
}

// luna.Sound:fade_in(fade_ms:int, [repeats:int = 0]) -- -1 loops forever
static int luna_m_sound_fade_in(lua_State *L)
{
	luna_Sound *s = luaL_checkudata(L,1,LUNA_SOUND_MT);
	int ms = luaL_checkinteger(L,2);
	int loops = luaL_optinteger(L,3,0);
	int chan = Mix_FadeInChannel(s->channel, s->chunk, loops, ms);
	s->channel = chan;
	return 0;
}

// luna.Sound:fade_in_timed(f_ms:int, ms:int, [rpts:int=0]) -- -1 rpts=forever
static int luna_m_sound_fade_in_timed(lua_State *L)
{
	luna_Sound *s = luaL_checkudata(L,1,LUNA_SOUND_MT);
	int fade_ms = luaL_checkinteger(L,2);
	int ms = luaL_checkinteger(L,3);
	int loops = luaL_optinteger(L,4,0);
	int chan = Mix_FadeInChannelTimed(s->channel, s->chunk, loops, fade_ms, ms);
	s->channel = chan;
	return 0;
}
// luna.Sound:pause()
static int luna_m_sound_pause(lua_State *L)
{
	luna_Sound *s = luaL_checkudata(L,1,LUNA_SOUND_MT);
	// don't do anything if we never even started playing (channel = -1)
	if (s->channel != -1) {
		Mix_Pause(s->channel);
	}
	return 0;
}

// luna.Sound:resume()
static int luna_m_sound_resume(lua_State *L)
{
	luna_Sound *s = luaL_checkudata(L,1,LUNA_SOUND_MT);
	// don't do anyting if we never started playing (channel = -1)
	if (s->channel != -1) {
		Mix_Resume(s->channel);
	}
	return 0;
}

// luna.Sound:volume() -> int
static int luna_m_sound_volume(lua_State *L)
{
	luna_Sound *s = luaL_checkudata(L,1,LUNA_SOUND_MT);
	// return 0 if we never started playing (channel = -1)
	if (s->channel != -1) {
		int vol = Mix_Volume(s->channel, -1);
		lua_pushinteger(L,vol);
	} else {
		lua_pushinteger(L,0);
	}
	return 1;
}

// luna.Sound:set_volume(vol: int)
static int luna_m_sound_set_volume(lua_State *L)
{
	luna_Sound *s = luaL_checkudata(L,1,LUNA_SOUND_MT);
	int vol = luaL_checkinteger(L,2);
	// do nothing if we didn't play yet
	if (s->channel != -1) {
		Mix_Volume(s->channel, vol);
	}
	return 0;
}
// luna.Sound:halt()
static int luna_m_sound_halt(lua_State *L)
{
	luna_Sound *s = luaL_checkudata(L,1,LUNA_SOUND_MT);
	if (s->channel != -1) {
		Mix_HaltChannel(s->channel);
	}
	return 0;
}
// luna.Sound:halt_timed(ms: int)
static int luna_m_sound_halt_timed(lua_State *L)
{
	luna_Sound *s = luaL_checkudata(L,1,LUNA_SOUND_MT);
	int ms = luaL_checkinteger(L,2);
	if (s->channel != -1) {
		Mix_ExpireChannel(s->channel, ms);
	}
	return 0;
}
// luna.Sound:fade_out(ms: int)
static int luna_m_sound_fade_out(lua_State *L)
{
	luna_Sound *s = luaL_checkudata(L,1,LUNA_SOUND_MT);
	int ms = luaL_checkinteger(L,2);
	if (s->channel != -1) {
		Mix_FadeOutChannel(s->channel, ms);
	}
	return 0;
}
// luna.Sound:is_playing() -> boolean
static int luna_m_sound_is_playing(lua_State *L)
{
	luna_Sound *s = luaL_checkudata(L,1,LUNA_SOUND_MT);
	if (s->channel != -1) {
		int playing = Mix_Playing(s->channel);
		lua_pushboolean(L, playing);
	} else {
		lua_pushboolean(L, 0);
	}
	return 1;
}
// luna.Sound:is_paused() -> boolean
static int luna_m_sound_is_paused(lua_State *L)
{
	luna_Sound *s = luaL_checkudata(L,1,LUNA_SOUND_MT);
	if (s->channel != -1) {
		int paused = Mix_Paused(s->channel);
		lua_pushboolean(L, paused);
	} else {
		lua_pushboolean(L, 0);
	}
	return 1;
}

// luna.Sound:is_fading() -> is_fading:boolean, dir: 'in' | 'out'
static int luna_m_sound_is_fading(lua_State *L)
{
	luna_Sound *s = luaL_checkudata(L,1,LUNA_SOUND_MT);
	if (s->channel != -1) {
		switch (Mix_FadingChannel(s->channel)) {
			case MIX_NO_FADING:
				lua_pushboolean(L,0);
				lua_pushnil(L);
				break;
			case MIX_FADING_OUT:
				lua_pushboolean(L,1);
				lua_pushliteral(L,"out");
				break;
			case MIX_FADING_IN:
				lua_pushboolean(L,1);
				lua_pushliteral(L,"in");
				break;
		}
	} else {
		lua_pushboolean(L,0);
		lua_pushnil(L);
	}
	return 2;
}

// luna.Sound:__gc() -- garbage collection
static int luna_m_sound_gc(lua_State *L)
{
	luna_Sound *s = luaL_checkudata(L,1,LUNA_SOUND_MT);
	// make sure we stop playing before we free
	if (s->channel != -1) {
		Mix_HaltChannel(s->channel);
	}
	Mix_FreeChunk(s->chunk);
	return 0;
}

// metatable def
static luaL_Reg luna_m_sound_metatable[] = {
	{"play", &luna_m_sound_play},
	{"play_timed", &luna_m_sound_play_timed},
	{"fade_in", &luna_m_sound_fade_in},
	{"fade_in_timed", &luna_m_sound_fade_in_timed},
	{"pause", &luna_m_sound_pause},
	{"resume", &luna_m_sound_resume},
	{"volume", &luna_m_sound_volume},
	{"set_volume", &luna_m_sound_set_volume},
	{"halt", &luna_m_sound_halt},
	{"halt_timed", &luna_m_sound_halt_timed},
	{"fade_out", &luna_m_sound_fade_out},
	{"is_playing", &luna_m_sound_is_playing},
	{"is_paused", &luna_m_sound_is_paused},
	{"is_fading", &luna_m_sound_is_fading},
	{"__gc", &luna_m_sound_gc},
	{NULL,NULL}
};

// ////////////////////////
// Library opening code! //
// ////////////////////////
int luaopen_luna(lua_State *L)
{
	// init keymap tables
	lh_luna_init_keymaps(L);
	lh_luna_init_keymod_table(L);
	KEYBOARD_STATE = SDL_GetKeyboardState(NULL);
	// set window metatable
	luaL_newmetatable(L,LUNA_WINDOW_MT);
	lua_pushliteral(L,"__index");
	lua_pushvalue(L,-2);
	lua_rawset(L,-3); // metatable.__index = metatable
	luaL_setfuncs(L, luna_m_window_metatable, 0);
	lua_pop(L,1);

	// set texture metatable
	luaL_newmetatable(L,LUNA_TEXTURE_MT);
	lua_pushliteral(L,"__index");
	lua_pushvalue(L,-2);
	lua_rawset(L,-3); // metatable.__index = metatable
	luaL_setfuncs(L, luna_m_texture_metatable, 0);
	lua_pop(L,1);

	// set music metatable
	luaL_newmetatable(L, LUNA_MUSIC_MT);
	lua_pushliteral(L,"__index");
	lua_pushvalue(L,-2);
	lua_rawset(L,-3); // metatable.__index = metatable
	luaL_setfuncs(L, luna_m_music_metatable, 0);
	lua_pop(L,1);

	// set sound metatable
	luaL_newmetatable(L, LUNA_SOUND_MT);
	lua_pushliteral(L,"__index");
	lua_pushvalue(L,-2);
	lua_rawset(L,-3); // metatable.__index = metatable
	luaL_setfuncs(L, luna_m_sound_metatable, 0);
	lua_pop(L,1);

	// add luna module
	luaL_newlib(L, luna_module_fns);
	// add luna.event module
	luaL_newlib(L, luna_event_module_fns);
	lua_setfield(L, -2, "event");
	// add luna.window module functions
	luaL_newlib(L, luna_window_module_fns);
	lua_setfield(L, -2, "window");
	// add luna.texture module functions
	luaL_newlib(L, luna_texture_module_fns);
	lua_setfield(L, -2, "texture");
	// add luna.music module functions
	luaL_newlib(L, luna_music_module_fns);
	lua_setfield(L, -2, "music");
	// add luna.sound module functions
	luaL_newlib(L, luna_sound_module_fns);
	lua_setfield(L, -2, "sound");
	// add luna.keyboard module functions
	luaL_newlib(L, luna_keyboard_module_fns);
	lua_setfield(L, -2, "keyboard");

	return 1; // return our library
}
