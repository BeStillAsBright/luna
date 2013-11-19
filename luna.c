#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>

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
	int w;
	int h;
} luna_Texture;

typedef struct luna_Sound {
	Mix_Chunk *chunk;
	int channel;
} luna_Sound;

typedef struct luna_Music {
	Mix_Music *music;
} luna_Music;


// ///////////////////
// luna.* functions //
// ///////////////////

// luna.init()
static int l_luna_init(lua_State *L)
{
	SDL_SetMainReady(); // needed because we're using SDL_MAIN_HANDLED
	SDL_Init(SDL_INIT_EVERYTHING); // just init everything (even if unused)
	int flags = IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF;
	int initted = IMG_Init(flags);
	if ((initted&flags) != flags) {
		lua_pushfstring(L,"SDL_image init failed: %s\n",IMG_GetError());
		lua_error(L);
	}

	int mixflags = MIX_INIT_OGG | MIX_INIT_MP3;
	int mixinitted = Mix_Init(mixflags);
	if ((mixinitted&mixflags) != mixflags) {
		lua_pushfstring(L, "luna.init-Mix_Init: %s\n", Mix_GetError());
		lua_error(L);
	}

	int merr = Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT,2,1024);
	if (merr) {
		lua_pushfstring(L,"luna.init-Mix_OpenAudio: %s\n",Mix_GetError());
		lua_error(L);
	}

	return 0;
}

// luna.quit()
static int l_luna_quit(lua_State *L)
{
	Mix_CloseAudio();
	Mix_Quit();
	IMG_Quit();
	SDL_Quit();
	return 0;
}

// luna.delay(ms:int)
static int l_luna_delay(lua_State *L)
{
	int ms = luaL_checkinteger(L,1);
	SDL_Delay(ms);
	return 0;
}

static const luaL_Reg l_luna_module_fns[] = {
	{"init", &l_luna_init},
	{"quit", &l_luna_quit},
	{"delay", &l_luna_delay},
	{NULL,NULL}
};

// //////////////////
// luna.event code //
// //////////////////

// 
static void lh_luna_set_keycode(lua_State *L, SDL_Keysym *ks)
{
	// push the keycode string
	switch (ks->sym) {
		// number keys
		case SDLK_0:
			lua_pushliteral(L,"0");
			break;
		case SDLK_1:
			lua_pushliteral(L,"1");
			break;
		case SDLK_2:
			lua_pushliteral(L,"2");
			break;
		case SDLK_3:
			lua_pushliteral(L,"3");
			break;
		case SDLK_4:
			lua_pushliteral(L,"4");
			break;
		case SDLK_5:
			lua_pushliteral(L,"5");
			break;
		case SDLK_6:
			lua_pushliteral(L,"6");
			break;
		case SDLK_7:
			lua_pushliteral(L,"7");
			break;
		case SDLK_8:
			lua_pushliteral(L,"8");
			break;
		case SDLK_9:
			lua_pushliteral(L,"9");
			break;
		// letter keys
		case SDLK_a:
			lua_pushliteral(L,"a");
			break;
		case SDLK_b:
			lua_pushliteral(L,"b");
			break;
		case SDLK_c:
			lua_pushliteral(L,"c");
			break;
		case SDLK_d:
			lua_pushliteral(L,"d");
			break;
		case SDLK_e:
			lua_pushliteral(L,"e");
			break;
		case SDLK_f:
			lua_pushliteral(L,"f");
			break;
		case SDLK_g:
			lua_pushliteral(L,"g");
			break;
		case SDLK_h:
			lua_pushliteral(L,"h");
			break;
		case SDLK_i:
			lua_pushliteral(L,"i");
			break;
		case SDLK_j:
			lua_pushliteral(L,"j");
			break;
		case SDLK_k:
			lua_pushliteral(L,"k");
			break;
		case SDLK_l:
			lua_pushliteral(L,"l");
			break;
		case SDLK_m:
			lua_pushliteral(L,"m");
			break;
		case SDLK_n:
			lua_pushliteral(L,"n");
			break;
		case SDLK_o:
			lua_pushliteral(L,"o");
			break;
		case SDLK_p:
			lua_pushliteral(L,"p");
			break;
		case SDLK_q:
			lua_pushliteral(L,"q");
			break;
		case SDLK_r:
			lua_pushliteral(L,"r");
			break;
		case SDLK_s:
			lua_pushliteral(L,"s");
			break;
		case SDLK_t:
			lua_pushliteral(L,"t");
			break;
		case SDLK_u:
			lua_pushliteral(L,"u");
			break;
		case SDLK_v:
			lua_pushliteral(L,"v");
			break;
		case SDLK_w:
			lua_pushliteral(L,"w");
			break;
		case SDLK_x:
			lua_pushliteral(L,"x");
			break;
		case SDLK_y:
			lua_pushliteral(L,"y");
			break;
		case SDLK_z:
			lua_pushliteral(L,"z");
			break;
		// symbol keys
		case SDLK_SPACE:
			lua_pushliteral(L,"space");
			break;
		case SDLK_AMPERSAND:
			lua_pushliteral(L,"&");
			break;
		case SDLK_ASTERISK:
			lua_pushliteral(L,"*");
			break;
		case SDLK_AT:
			lua_pushliteral(L,"@");
			break;
		case SDLK_BACKSLASH:
			lua_pushliteral(L,"\\");
			break;
		case SDLK_BACKQUOTE:
			lua_pushliteral(L,"`");
			break;
		case SDLK_CARET:
			lua_pushliteral(L,"^");
			break;
		case SDLK_COMMA:
			lua_pushliteral(L,",");
			break;
		case SDLK_DOLLAR:
			lua_pushliteral(L,"$");
			break;
		case SDLK_EQUALS:
			lua_pushliteral(L,"=");
			break;
		case SDLK_EXCLAIM:
			lua_pushliteral(L,"!"); break;
		case SDLK_GREATER:
			lua_pushliteral(L,">");
			break;
		case SDLK_HASH:
			lua_pushliteral(L,"#");
			break;
		case SDLK_LEFTBRACKET:
			lua_pushliteral(L,"[");
			break;
		case SDLK_LEFTPAREN:
			lua_pushliteral(L,"(");
			break;
		case SDLK_LESS:
			lua_pushliteral(L,"<");
			break;
		case SDLK_MINUS:
			lua_pushliteral(L,"-");
			break;
		case SDLK_PERCENT:
			lua_pushliteral(L,"%");
			break;
		case SDLK_PERIOD:
			lua_pushliteral(L,".");
			break;
		case SDLK_PLUS:
			lua_pushliteral(L,"+");
			break;
		case SDLK_QUESTION:
			lua_pushliteral(L,"?");
			break;
		case SDLK_QUOTE:
			lua_pushliteral(L,"'");
			break;
		case SDLK_QUOTEDBL:
			lua_pushliteral(L,"\"");
			break;
		case SDLK_RIGHTBRACKET:
			lua_pushliteral(L,"]");
			break;
		case SDLK_RIGHTPAREN:
			lua_pushliteral(L,")");
			break;
		case SDLK_SEMICOLON:
			lua_pushliteral(L,";");
			break;
		case SDLK_SLASH:
			lua_pushliteral(L,"/");
			break;
		case SDLK_UNDERSCORE:
			lua_pushliteral(L,"_");
			break;
		// F keys
		case SDLK_F1:
			lua_pushliteral(L,"f1");
			break;
		case SDLK_F2:
			lua_pushliteral(L,"f2");
			break;
		case SDLK_F3:
			lua_pushliteral(L,"f3");
			break;
		case SDLK_F4:
			lua_pushliteral(L,"f4");
			break;
		case SDLK_F5:
			lua_pushliteral(L,"f5");
			break;
		case SDLK_F6:
			lua_pushliteral(L,"f6");
			break;
		case SDLK_F7:
			lua_pushliteral(L,"f7");
			break;
		case SDLK_F8:
			lua_pushliteral(L,"f8");
			break;
		case SDLK_F9:
			lua_pushliteral(L,"f9");
			break;
		case SDLK_F10:
			lua_pushliteral(L,"f10");
			break;
		case SDLK_F11:
			lua_pushliteral(L,"f11");
			break;
		case SDLK_F12:
			lua_pushliteral(L,"f12");
			break;
		case SDLK_F13:
			lua_pushliteral(L,"f13");
			break;
		case SDLK_F14:
			lua_pushliteral(L,"f14");
			break;
		case SDLK_F15:
			lua_pushliteral(L,"f15");
			break;
		case SDLK_F16:
			lua_pushliteral(L,"f16");
			break;
		case SDLK_F17:
			lua_pushliteral(L,"f17");
			break;
		case SDLK_F18:
			lua_pushliteral(L,"f18");
			break;
		case SDLK_F19:
			lua_pushliteral(L,"f19");
			break;
		case SDLK_F20:
			lua_pushliteral(L,"f20");
			break;
		case SDLK_F21:
			lua_pushliteral(L,"f21");
			break;
		case SDLK_F22:
			lua_pushliteral(L,"f22");
			break;
		case SDLK_F23:
			lua_pushliteral(L,"f23");
			break;
		case SDLK_F24:
			lua_pushliteral(L,"f24");
			break;
		// keypad
		case SDLK_KP_0:
			lua_pushliteral(L,"kp0");
			break;
		case SDLK_KP_1:
			lua_pushliteral(L,"kp1");
			break;
		case SDLK_KP_2:
			lua_pushliteral(L,"kp2");
			break;
		case SDLK_KP_3:
			lua_pushliteral(L,"kp3");
			break;
		case SDLK_KP_4:
			lua_pushliteral(L,"kp4");
			break;
		case SDLK_KP_5:
			lua_pushliteral(L,"kp5");
			break;
		case SDLK_KP_6:
			lua_pushliteral(L,"kp6");
			break;
		case SDLK_KP_7:
			lua_pushliteral(L,"kp7");
			break;
		case SDLK_KP_8:
			lua_pushliteral(L,"kp8");
			break;
		case SDLK_KP_9:
			lua_pushliteral(L,"kp9");
			break;
		case SDLK_KP_COMMA:
			lua_pushliteral(L,"kp,");
			break;
		case SDLK_KP_DECIMAL:
			lua_pushliteral(L,"kp.");
			break;
		case SDLK_KP_DIVIDE:
			lua_pushliteral(L,"kp/");
			break;
		case SDLK_KP_ENTER:
			lua_pushliteral(L,"kpenter");
			break;
		case SDLK_KP_EQUALS:
			lua_pushliteral(L,"kp=");
			break;
		case SDLK_KP_MINUS:
			lua_pushliteral(L,"kp-");
			break;
		case SDLK_KP_MULTIPLY:
			lua_pushliteral(L,"kp*");
			break;
		case SDLK_KP_PLUS:
			lua_pushliteral(L,"kp+");
			break;
		// navigation keys
		case SDLK_DOWN:
			lua_pushliteral(L,"down");
			break;
		case SDLK_LEFT:
			lua_pushliteral(L,"left");
			break;
		case SDLK_RIGHT:
			lua_pushliteral(L,"right");
			break;
		case SDLK_UP:
			lua_pushliteral(L,"up");
			break;
		case SDLK_END:
			lua_pushliteral(L,"end");
			break;
		case SDLK_HOME:
			lua_pushliteral(L,"home");
			break;
		case SDLK_PAGEDOWN:
			lua_pushliteral(L,"pagedown");
			break;
		case SDLK_PAGEUP:
			lua_pushliteral(L,"pageup");
			break;
		// editing keys
		case SDLK_BACKSPACE:
			lua_pushliteral(L,"backspace");
			break;
		case SDLK_CLEAR:
			lua_pushliteral(L,"clear");
			break;
		case SDLK_DELETE:
			lua_pushliteral(L,"delete");
			break;
		case SDLK_INSERT:
			lua_pushliteral(L,"insert");
			break;
		case SDLK_RETURN:
			lua_pushliteral(L,"return");
			break;
		case SDLK_TAB:
			lua_pushliteral(L,"tab");
			break;
		// modifier keys
		case SDLK_CAPSLOCK:
			lua_pushliteral(L,"capslock");
			break;
		case SDLK_MODE:
			lua_pushliteral(L,"mode");
			break;
		case SDLK_NUMLOCKCLEAR:
			lua_pushliteral(L,"numlock");
			break;
		case SDLK_SCROLLLOCK:
			lua_pushliteral(L,"scrolllock");
			break;
		case SDLK_LGUI:
			lua_pushliteral(L,"lgui");
			break;
		case SDLK_RGUI:
			lua_pushliteral(L,"rgui");
			break;
		case SDLK_LSHIFT:
			lua_pushliteral(L,"lshift");
			break;
		case SDLK_RSHIFT:
			lua_pushliteral(L,"rshift");
			break;
		case SDLK_LALT:
			lua_pushliteral(L,"lalt");
			break;
		case SDLK_RALT:
			lua_pushliteral(L,"ralt");
			break;
		case SDLK_LCTRL:
			lua_pushliteral(L,"lctrl");
			break;
		case SDLK_RCTRL:
			lua_pushliteral(L,"rctrl");
			break;
		// miscellaneous keys
		case SDLK_PAUSE:
			lua_pushliteral(L,"pause");
			break;
		case SDLK_ESCAPE:
			lua_pushliteral(L,"escape");
			break;
		case SDLK_HELP:
			lua_pushliteral(L,"help");
			break;
		case SDLK_MENU:
			lua_pushliteral(L,"menu");
			break;
		case SDLK_PRINTSCREEN:
			lua_pushliteral(L,"printscreen");
			break;
		case SDLK_SYSREQ:
			lua_pushliteral(L,"sysreq");
			break;
		case SDLK_APPLICATION:
			lua_pushliteral(L,"application");
			break;
		case SDLK_POWER:
			lua_pushliteral(L,"power");
			break;
		case SDLK_CURRENCYUNIT:
			lua_pushliteral(L,"currencyunit");
			break;
		case SDLK_UNDO:
			lua_pushliteral(L,"undo");
			break;
	}
	lua_setfield(L,-2,"sym");
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
static int l_luna_event_poll(lua_State *L)
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
static const luaL_Reg l_luna_event_module_fns[] = {
	{"poll", &l_luna_event_poll},
	{NULL,NULL}
};

// //////////////////////////////////
// luna.window.* functions/methods //
// //////////////////////////////////

// luna.Window.new(w:int,h:int,fullscreen:boolean) -> luna.Window
static int c_luna_window_new(lua_State *L)
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
static int m_luna_window_show(lua_State *L)
{
	luna_Window *win = luaL_checkudata(L,1,LUNA_WINDOW_MT);
	SDL_ShowWindow(win->window);
	return 0;
}

static int m_luna_window_hide(lua_State *L)
{
	luna_Window *win = luaL_checkudata(L,1,LUNA_WINDOW_MT);
	SDL_HideWindow(win->window);
	return 0;
}

// luna.Window:close()
static int m_luna_window_close(lua_State *L)
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
static int m_luna_window_draw(lua_State *L) 
{
	luna_Window *win = luaL_checkudata(L,1,LUNA_WINDOW_MT);
	luna_Texture *tex = luaL_checkudata(L,2,LUNA_TEXTURE_MT);
	int x = luaL_checkinteger(L,3);
	int y = luaL_checkinteger(L,4);
	SDL_Rect dst = (SDL_Rect) {.x = x, .y = y, .w = tex->w, .h = tex->h};
	
	SDL_RenderCopy(win->renderer, tex->texture, NULL, &dst);
	return 0;
}

// luna.Window:paint()
static int m_luna_window_paint(lua_State *L)
{
	luna_Window *win = luaL_checkudata(L,1,LUNA_WINDOW_MT);
	SDL_RenderPresent(win->renderer);
	return 0;
}

// luna.Window:clear()
static int m_luna_window_clear(lua_State *L)
{
	luna_Window *win = luaL_checkudata(L,1,LUNA_WINDOW_MT);
	SDL_RenderClear(win->renderer);
	return 0;
}

// luna.Window.__gc() -- called on garbage collection
static int m_luna_window_gc(lua_State *L)
{
	// all we need to do is close it
	m_luna_window_close(L);
	return 0;
}

static const luaL_Reg l_luna_window_module_fns[] = {
	{"new", &c_luna_window_new},
	{NULL,NULL}
};

static const luaL_Reg m_luna_window_metatable[] = {
	{"close", &m_luna_window_close},
	{"draw", &m_luna_window_draw},
	{"paint", &m_luna_window_paint},
	{"clear", &m_luna_window_clear},
	{"show", &m_luna_window_show},
	{"hide", &m_luna_window_hide},
	{"__gc", &m_luna_window_gc},
	{NULL,NULL}
};

// ///////////////
// luna.Texture //
// ///////////////

// luna.Texture.new(window:luna.Window,filename:string) -> luna.Texture
static int c_luna_texture_new(lua_State *L)
{
	// TODO maybe add colorkey support?
	luna_Window *win = luaL_checkudata(L,1,LUNA_WINDOW_MT);
	char *fname = luaL_checkstring(L,2);

	luna_Texture *tex = lua_newuserdata(L, sizeof(*tex));
	// get our surface from the filename
	SDL_Surface *surf = IMG_Load(fname);
	if (!surf) {
		lua_pushfstring(L,"ERROR luna.Texture.new IMG_Load: %s\n",
				IMG_GetError());
		lua_error(L);
	}
	tex->w = surf->w;
	tex->h = surf->h;
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

// luna.Texture:__gc() (garbage collection)
static int m_luna_texture_gc(lua_State *L)
{
	luna_Texture *tex = luaL_checkudata(L,1,LUNA_TEXTURE_MT);
	SDL_DestroyTexture(tex->texture);
	return 0;
}

static const luaL_Reg l_luna_texture_module_fns[] = {
	{"new", &c_luna_texture_new},
	{NULL,NULL}
};

static const luaL_Reg m_luna_texture_metatable[] =  {
	{"__gc", &m_luna_texture_gc},
	{NULL,NULL}
};

// ///////////////////////
// luna.music functions //
// ///////////////////////

// luna.music.new(filename:string) -> luna.Music
static int c_luna_music_new(lua_State *L)
{
	char *filename = luaL_checkstring(L,1);

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
static int l_luna_music_pause(lua_State *L)
{
	Mix_PauseMusic();
	return 0;
}

// luna.music.resume()
static int l_luna_music_resume(lua_State *L)
{
	Mix_ResumeMusic();
	return 0;
}

// luna.music.rewind()
static int l_luna_music_rewind(lua_State *L)
{
	Mix_RewindMusic();
	return 0;
}

// luna.music.set_volume(volume:int)
static int l_luna_music_set_volume(lua_State *L)
{
	int volume = luaL_checkinteger(L,1);
	Mix_VolumeMusic(volume);
	return 0;
}

// luna.music.volume() -> int
static int l_luna_music_volume(lua_State *L)
{
	int volume = Mix_VolumeMusic(-1);
	lua_pushinteger(L,volume);
	return 1;
}

// luna.music.halt()
static int l_luna_music_halt(lua_State *L)
{
	Mix_HaltMusic();
	return 0;
}

// luna.music.fade_out(ms:int)
static int l_luna_music_fade_out(lua_State *L)
{
	int ms = luaL_checkinteger(L,1);
	Mix_FadeOutMusic(ms);
	return 0;
}

// luna.music.set_position(seconds:number)
static int l_luna_music_set_position(lua_State *L)
{
	double seconds = luaL_checknumber(L,1);
	Mix_RewindMusic();
	Mix_SetMusicPosition(seconds);
	return 0;
}

// luna.music.is_paused() -> boolean
static int l_luna_music_is_paused(lua_State *L)
{
	lua_pushboolean(L,Mix_PausedMusic());
	return 1;
}
// luna.music.is_playing() -> boolean
static int l_luna_music_is_playing(lua_State *L)
{
	lua_pushboolean(L,Mix_PlayingMusic());
	return 1;
}
// luna.music.is_fading() -> fading:boolean, dir:string['in' | 'out']
static int l_luna_music_is_fading(lua_State *L)
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
			break
	}
	return 2;
}

static luaL_Reg l_luna_music_module_fns[] = {
	{"new", &c_luna_music_new},
	{"pause", &l_luna_music_pause},
	{"resume", &l_luna_music_resume},
	{"rewind", &l_luna_music_rewind},
	{"set_volume", &l_luna_music_set_volume},
	{"volume", &l_luna_music_volume},
	{"halt", &l_luna_music_halt},
	{"fade_out", &l_luna_music_fade_out},
	{"set_position", &l_luna_music_set_position},
	{"is_playing", &l_luna_music_is_playing},
	{"is_paused", &l_luna_music_is_paused},
	{"is_fading", &l_luna_music_is_fading},
	{NULL,NULL}
};

// /////////////////////
// luna.Music methods //
// /////////////////////

// luna.Music:play([loops:int = -1]) -- loop forever if no argument!
static int m_luna_music_play(lua_State *L)
{
	luna_Music *m = luaL_checkudata(L,1,LUNA_MUSIC_MT);
	int loops = luaL_optint(L,2,-1); // loop forever if no argument
	Mix_PlayMusic(m->music, loops);
	return 0;
}

// luna.Music:play_from(start_pos:number, [loops:int = -1])
static int m_luna_play_from(lua_State *L)
{
	luna_Music *m = luaL_checkudata(L,1,LUNA_MUSIC_MT);
	double pos = luaL_checknumber(L,2);
	int loops = luaL_optint(L,3,-1);
	Mix_FadeInMusicPos(m->music,loops,0,pos);
	return 0;
}

// luna.Music:fade_in(ms:int, [loops:int = -1])
static int m_luna_music_fade_in(lua_State *L)
{
	luna_Music *m = luaL_checkudata(L,1,LUNA_MUSIC_MT);
	int ms = luaL_checkinteger(L,2);
	int loops = luaL_optint(L,3,-1);
	Mix_FadeInMusic(m->music, loops, ms);
	return 0;
}

// luna.Music:fade_in_from(pos:number, ms:int, [loops:int = -1])
static int m_luna_music_fade_in_from(lua_State *L)
{
	luna_Music *m = luaL_checkudata(L,1,LUNA_MUSIC_MT);
	double pos = luaL_checknumber(L,2);
	int ms = luaL_checkinteger(L,3);
	int loops = luaL_optint(L,4,-1);
	Mix_FadeInMusicPos(m->music, loops, ms, pos);
	return 0;
}

// luna.Music.__gc() -- garbage collection
static int m_luna_music_gc(lua_State *L)
{
	luna_Music *m = luaL_checkudata(L,1,LUNA_MUSIC_MT);
	Mix_FreeMusic(m->music);
	return 0;
}


static luaL_Reg m_luna_music_metatable[] = {
	{"play", &m_luna_music_play},
	{"play_from", &m_luna_play_from},
	{"fade_in", &m_luna_music_fade_in},
	{"fade_in_from", &m_luna_music_fade_in_from},
	{"__gc", &m_luna_music_gc},
	{NULL,NULL}
};

// ///////////////////////
// luna.sound functions //
// ///////////////////////

// luna.sound.new(filename:string) -> luna.Sound
static int l_luna_sound_new(lua_State *L)
{
	char *fname = luaL_checkstring(L,1);
	luna_Sound *s = lua_newuserdata(L, sizeof(*s));

	s->chunk = Mix_LoadWAV(fname);
	s->channel = -1; // we want to play on the first unreserved channel

	luaL_setmetatable(L, LUNA_SOUND_MT);
	return 1;
}

// TODO:
// luna.sound.pause_all()
static int l_luna_sound_pause_all(lua_State *L)
{
	Mix_Pause(-1);
	return 0;
}

// luna.sound.resume_all()
static int l_luna_sound_resume_all(lua_State *L)
{
	Mix_Resume(-1);
	return 0;
}

// luna.sound.halt_all()
static int l_luna_sound_halt_all(lua_State *L)
{
	Mix_HaltChannel(-1);
	return 0;
}

// luna.sound.halt_timed_all(ms: int)
static int l_luna_sound_halt_timed_all(lua_State *L)
{
	int ms = luaL_checkinteger(L,1);
	Mix_ExpireChannel(-1, ms);
	return 0;
}
// luna.sound.fade_out_all(ms: int)
static int l_luna_sound_fade_out_all(lua_State *L)
{
	int ms = luaL_checkinteger(L,1);
	Mix_FadeOutChannel(-1, ms);
	return 0;
}

// luna.sound.set_volume_all(volume: int)
static int l_luna_sound_set_volume_all(lua_State *L)
{
	int vol = luaL_checkinteger(L,1);
	Mix_Volume(-1, vol);
}
// luna.sound.number_playing() -> int
static int l_luna_sound_number_playing(lua_State *L)
{
	int playing = Mix_Playing(-1);
	lua_pushinteger(L,playing);
	return 1;
}
// luna.sound.number_paused() -> int
static int l_luna_sound_number_paused(lua_State *L)
{
	int paused = Mix_Paused(-1);
	lua_pushinteger(L,paused);
	return 1;

// Module def
static luaL_Reg l_luna_sound_module_fns[] = {
	{"new", &l_luna_sound_new},
	{NULL,NULL}
};

// /////////////////////
// luna.Sound methods //
// /////////////////////

// luna.Sound:play([repeats:int = 0])) -- SPECIAL: -1 repeats = forever
static int m_luna_sound_play(lua_State *L)
{
	luna_Sound *s = luaL_checkudata(L, 1, LUNA_SOUND_MT);
	// default is no repeats
	int loops = luaL_optint(L, 2, 0); 
	// we can just use s->channel here because it's initially set to -1
	// so we automatically play on an unreserved channel. Then we store the
	// channel we actually used in our userdata so Sound:pause() etc. works.
	int chan = Mix_PlayChannel(s->channel, s->chunk, loops);
	s->channel = chan;
	return 0;
}

// luna.Sound:play_timed(ms:int, [repeats:int = 0]) -- -1 repeats = forever
static int m_luna_sound_play_timed(lua_State *L)
{
	// same implementation as m_luna_sound_play
	luna_Sound *s = luaL_checkudata(L,1,LUNA_SOUND_MT);
	int ms = luaL_checkinteger(L,2);
	int loops = luaL_optint(L,3,0);
	int chan = Mix_PlayChannelTimed(s->channel, s->chunk, loops, ms);
	s->channel = chan;
	return 0;
}

// TODO:
// luna.Sound:fade_in(fade_ms:int, [repeats:int = 0]) -- -1 loops forever
// luna.Sound:fade_in_timed(f_ms:int, ms:int, [rpts:int=0]) -- -1 rpts=forever
// luna.Sound:pause()
// luna.Sound:volume() -> int
// luna.Sound:set_volume(vol: int)
// luna.Sound:halt()
// luna.Sound:halt_timed(ms: int)
// luna.Sound:fade_out(ms: int)
// luna.Sound:is_playing() -> boolean
// luna.Sound:is_paused() -> boolean
// luna.Sound:is_fading() -> is_fading:boolean, dir: 'in' | 'out'

// metatable def
static luaL_Reg m_luna_sound_metatable[] = {
	{"play", &m_luna_sound_play},
	{"play_timed", &m_luna_sound_play_timed},
	{NULL,NULL}
};

// ////////////////////////
// Library opening code! //
// ////////////////////////
int luaopen_luna(lua_State *L)
{
	// set window metatable
	luaL_newmetatable(L,LUNA_WINDOW_MT);
	lua_pushliteral(L,"__index");
	lua_pushvalue(L,-2);
	lua_rawset(L,-3); // metatable.__index = metatable
	luaL_setfuncs(L, m_luna_window_metatable, 0);
	lua_pop(L,1);

	// set texture metatable
	luaL_newmetatable(L,LUNA_TEXTURE_MT);
	lua_pushliteral(L,"__index");
	lua_pushvalue(L,-2);
	lua_rawset(L,-3); // metatable.__index = metatable
	luaL_setfuncs(L, m_luna_texture_metatable, 0);
	lua_pop(L,1);

	// set music metatable
	luaL_newmetatable(L, LUNA_MUSIC_MT);
	lua_pushliteral(L,"__index");
	lua_pushvalue(L,-2);
	lua_rawset(L,-3); // metatable.__index = metatable
	luaL_setfuncs(L, m_luna_music_metatable, 0);
	lua_pop(L,1);

	// add luna module
	luaL_newlib(L, l_luna_module_fns);
	// add luna.event module
	luaL_newlib(L, l_luna_event_module_fns);
	lua_setfield(L, -2, "event");
	// add luna.window module functions
	luaL_newlib(L, l_luna_window_module_fns);
	lua_setfield(L, -2, "window");
	// add luna.texture module functions
	luaL_newlib(L, l_luna_texture_module_fns);
	lua_setfield(L, -2, "texture");
	// add luna.music module functions
	luaL_newlib(L, l_luna_music_module_fns);
	lua_setfield(L, -2, "music");

	return 1; // return our library
}

