#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>

// we need this since we're not defining a main() function.
// if we didn't #define this, SDL would try to #define main SDL_main
// and put parachute code in main and also die.
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

// Metatable defines


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
		case SDK_4:
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
		case SDLK_APOSTROPHE:
			lua_pushliteral(L,"'");
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
			lua_pushliteral(L,"!");
			break;
		case SDLK_GRAVE:
			lua_pushliteral(L,"`");
			break;
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
		case SDL_KP_MULTIPLY:
			lua_pushliteral(L,"kp*");
			break;
		case SDLK_KP_PLUS:
			lua_pushliteral(L,"kp+");
			break;
		// navigation keys
		case SDL_DOWN:
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
		case SDL_DELETE:
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
	lh_luna_set_keycode(L,e->key->keysym);
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

static int l_luna_event_poll(lua_State *L)
{
	SDL_Event event;
	SDL_PollEvent(&event);
	// push a table to use as our return value
	switch (event.type) {
		case SDL_KEYDOWN:
			lua_pushliteral(L,"key_down");
			lh_luna_make_key_event(L,&event);
			break;
		case SDL_KEYUP:
			lua_pushliteral(L,"key_up");
			lh_luna_make_key_event(L, &event);
			break;
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
		default:
			break;
	}
	return 2; // return type, event_table
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

