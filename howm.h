#ifndef HOWM_H
#define HOWM_H

#include <stdbool.h>
#include <xcb/xcb.h>
#include <X11/keysym.h>
#include <xcb/xcb_ewmh.h>
#include <X11/X.h>

/** Calculates a mask that can be applied to a window in order to reconfigure a
 * window. */
#define MOVE_RESIZE_MASK (XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | \
			  XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT)
/** Ensures that the number lock doesn't intefere with checking the equality
 * of two modifier masks.*/
#define CLEANMASK(mask) (mask & ~(numlockmask | XCB_MOD_MASK_LOCK))
/** Wraps up the comparison of modifier masks into a neat package. */
#define EQUALMODS(mask, omask) (CLEANMASK(mask) == CLEANMASK(omask))
/** Calculates the length of an array. */
#define LENGTH(x) (unsigned int)(sizeof(x) / sizeof(*x))
/** Checks to see if a client is floating, fullscreen or transient. */
#define FFT(c) (c->is_transient || c->is_floating || c->is_fullscreen)
/** Supresses the unused variable compiler warnings. */
#define UNUSED(x) (void)(x)

/** The remove action for a WM_STATE request. */
#define _NET_WM_STATE_REMOVE 0
/** The add action for a WM_STATE request. */
#define _NET_WM_STATE_ADD 1
/** The toggle action for a WM_STATE request. */
#define _NET_WM_STATE_TOGGLE 2

/**
 * @brief Represents an argument.
 *
 * Used to hold data that is sent as a parameter to a function when called as a
 * result of a keypress.
 */
typedef union {
	const char * const * const cmd; /**< Represents a command that will be called by a shell.  */
	int i; /**< Usually used for specifying workspaces or clients. */
} Arg;

/**
 * @brief Represents a key.
 *
 * Holds information relative to a key, such as keysym and the mode during
 * which the keypress can be seen as valid.
 */
typedef struct {
	int mod; /**< The mask of the modifiers pressed. */
	unsigned int mode; /**< The mode within which this keypress is valid. */
	xcb_keysym_t sym;  /**< The keysym of the pressed key. */
	void (*func)(const Arg *); /**< The function to be called when this key is pressed. */
	const Arg arg; /**< The argument passed to the above function. */
} Key;

/**
 * @brief Represents a rule that is applied to a client upon it starting.
 */
typedef struct {
	const char *class; /**<	The class or name of the client. */
	int ws; /**<  The workspace that the client should be spawned
				on (0 means current workspace). */
	bool follow; /**< If the client is spawned on another ws, shall we follow? */
	bool is_floating; /**< Spawn the client in a floating state? */
	bool is_fullscreen; /**< Spawn the client in a fullscreen state? */
} Rule;

/**
 * @brief Represents an operator.
 *
 * Operators perform an action upon one or more targets (identified by
 * motions).
 */
typedef struct {
	int mod; /**< The mask of the modifiers pressed. */
	xcb_keysym_t sym; /**< The keysym of the pressed key. */
	unsigned int mode; /**< The mode within which this keypress is valid. */
	void (*func)(const unsigned int type, const int cnt); /**< The function to be
								 * called when the key is pressed. */
} Operator;

/**
 * @brief Represents a motion.
 *
 * A motion can be used to target an operation at something specific- such as a
 * client or workspace.
 *
 * For example:
 *
 * q4c (Kill, 4, Clients).
 */
typedef struct {
	int mod; /**< The mask of the modifiers pressed. */
	xcb_keysym_t sym; /**< The keysym of the pressed key. */
	unsigned int type; /**< Represents whether the motion is for clients, WS etc. */
} Motion;

/**
 * @brief Represents a button.
 *
 * Allows the mapping of a button to a function, as is done with the Key struct
 * for keys.
 */
typedef struct {
	int mod; /**< The mask of the modifiers pressed.  */
	short int button; /**< The button that was pressed. */
	void (*func)(const Arg *); /**< The function to be called when the
					* button is pressed. */
	const Arg arg; /**< The argument passed to the above function. */
} Button;

/**
 * @brief Represents a client that is being handled by howm.
 *
 * All the attributes that are needed by howm for a client are stored here.
 */
typedef struct Client {
	struct Client *next; /**< Clients are stored in a linked list-
					* this represents the client after this one. */
	bool is_fullscreen; /**< Is the client fullscreen? */
	bool is_floating; /**< Is the client floating? */
	bool is_transient; /**< Is the client transient?
					* Defined at: http://standards.freedesktop.org/wm-spec/wm-spec-latest.html*/
	bool is_urgent; /**< This is set by a client that wants focus for some reason. */
	xcb_window_t win; /**< The window that this client represents. */
	uint16_t x; /**< The x coordinate of the client. */
	uint16_t y; /**< The y coordinate of the client. */
	uint16_t w; /**< The width of the client. */
	uint16_t h; /**< The height of the client. */
	uint16_t gap; /**< The size of the useless gap between this client and
			the others. */
} Client;

/**
 * @brief Represents a workspace, which stores clients.
 *
 * Clients are stored as a linked list. Changing to a different workspace will
 * cause different clients to be rendered on the screen.
 */
typedef struct {
	int layout; /**< The current layout of the WS, as defined in the
				* layout enum. */
	int client_cnt; /**< The amount of clients on this workspace. */
	uint16_t gap; /**< The size of the useless gap between windows for this workspace. */
	float master_ratio; /**< The ratio of the size of the master window
				 compared to the screen's size. */
	uint16_t bar_height; /**< The height of the space left for a bar. Stored
			      here so it can be toggled per ws. */
	Client *head; /**< The start of the linked list. */
	Client *prev_foc; /**< The last focused client. This is seperate to
				* the linked list structure. */
	Client *current; /**< The client that is currently in focus. */
} Workspace;

/**
 * @brief Represents the last command (and its arguments) or the last
 * combination of operator, count and motion (ocm).
 */
struct replay_state {
	void (*last_op)(const unsigned int type, int cnt); /**< The last operator to be called. */
	void (*last_cmd)(const Arg *arg); /**< The last command to be called. */
	const Arg *last_arg; /**< The last argument, passed to the last command. */
	unsigned int last_type; /**< The value determine by the last motion
				(workspace, client etc).*/
	int last_cnt; /**< The last count passed to the last operator function. */
};

/**
 * @brief Represents a stack. This stack is going to hold linked lists of
 * clients. An example of the stack is below:
 *
 * TOP
 * ==========
 * c1->c2->c3->NULL
 * ==========
 * c1->NULL
 * ==========
 * c1->c2->c3->NULL
 * ==========
 * BOTTOM
 *
 */
struct stack {
	int size; /**< The amount of items in the stack. */
	Client **contents; /**< The contents is an array of linked lists. Storage
			is malloced later as we don't know the size yet.*/
};


/* Operators */
void op_kill(const unsigned int type, int cnt);
void op_move_up(const unsigned int type, int cnt);
void op_move_down(const unsigned int type, int cnt);
void op_focus_down(const unsigned int type, int cnt);
void op_focus_up(const unsigned int type, int cnt);
void op_shrink_gaps(const unsigned int type, int cnt);
void op_grow_gaps(const unsigned int type, int cnt);
void op_cut(const unsigned int type, int cnt);

/* Clients */
void teleport_client(const Arg *arg);
void change_client_gaps(Client *c, int size);
void change_gaps(const unsigned int type, int cnt, int size);
void move_current_down(const Arg *arg);
void move_current_up(const Arg *arg);
void kill_client(const int ws, bool arrange);
void move_down(Client *c);
void move_up(Client *c);
Client *next_client(Client *c);
void focus_next_client(const Arg *arg);
void focus_prev_client(const Arg *arg);
void update_focused_client(Client *c);
Client *prev_client(Client *c, int ws);
Client *create_client(xcb_window_t w);
void remove_client(Client *c);
Client *find_client_by_win(xcb_window_t w);
void client_to_ws(Client *c, const int ws, bool follow);
void current_to_ws(const Arg *arg);
void draw_clients(void);
void change_client_geom(Client *c, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void toggle_float(const Arg *arg);
void resize_float_width(const Arg *arg);
void resize_float_height(const Arg *arg);
void move_float_y(const Arg *arg);
void move_float_x(const Arg *arg);
void make_master(const Arg *arg);
void grab_buttons(Client *c);
void set_fullscreen(Client *c, bool fscr);
void set_urgent(Client *c, bool urg);
void toggle_fullscreen(const Arg *arg);
void focus_urgent(const Arg *arg);
void send_to_scratchpad(const Arg *arg);
void get_from_scratchpad(const Arg *arg);

/* Workspaces */
void kill_ws(const int ws);
void toggle_bar(const Arg *arg);
void resize_master(const Arg *arg);
void focus_next_ws(const Arg *arg);
void focus_prev_ws(const Arg *arg);
void focus_last_ws(const Arg *arg);
void change_ws(const Arg *arg);
int correct_ws(int ws);

/* Layouts */
void change_layout(const Arg *arg);
void next_layout(const Arg *arg);
void previous_layout(const Arg *arg);
void last_layout(const Arg *arg);
void stack(void);
void grid(void);
void zoom(void);
void arrange_windows(void);

/* Modes */
void change_mode(const Arg *arg);

/* Stack */
void stack_push(struct stack *s, Client *c);
Client *stack_pop(struct stack *s);
void stack_init(struct stack *s);
void stack_free(struct stack *s);

/* Events */
void enter_event(xcb_generic_event_t *ev);
void destroy_event(xcb_generic_event_t *ev);
void button_press_event(xcb_generic_event_t *ev);
void key_press_event(xcb_generic_event_t *ev);
void map_event(xcb_generic_event_t *ev);
void configure_event(xcb_generic_event_t *ev);
void unmap_event(xcb_generic_event_t *ev);
void client_message_event(xcb_generic_event_t *ev);

/* XCB */
void grab_keys(void);
xcb_keycode_t *keysym_to_keycode(xcb_keysym_t sym);
void grab_keycode(xcb_keycode_t *keycode, const int mod);
void elevate_window(xcb_window_t win);
void move_resize(xcb_window_t win, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void set_border_width(xcb_window_t win, uint16_t w);
void get_atoms(char **names, xcb_atom_t *atoms);
void check_other_wm(void);
xcb_keysym_t keycode_to_keysym(xcb_keycode_t keycode);
void ewmh_process_wm_state(Client *c, xcb_atom_t a, int action);

/* Misc */
void apply_rules(Client *c);
void howm_info(void);
void save_last_ocm(void (*op) (const unsigned int, int), const unsigned int type, int cnt);
void save_last_cmd(void (*cmd)(const Arg *), const Arg *arg);
void replay(const Arg *arg);
void paste(const Arg *arg);
int get_non_tff_count(void);
Client *get_first_non_tff(void);
uint32_t get_colour(char *colour);
void spawn(const Arg *arg);
void setup(void);
void move_client(int cnt, bool up);
void focus_window(xcb_window_t win);
void quit_howm(const Arg *arg);
void restart_howm(const Arg *arg);
void cleanup(void);
void delete_win(xcb_window_t win);
void setup_ewmh(void);

enum layouts { ZOOM, GRID, HSTACK, VSTACK, END_LAYOUT };
enum states { OPERATOR_STATE, COUNT_STATE, MOTION_STATE, END_STATE };
enum modes { NORMAL, FOCUS, FLOATING, END_MODES };
enum motions { CLIENT, WORKSPACE };
enum net_atom_enum { NET_WM_STATE_FULLSCREEN, NET_SUPPORTED, NET_WM_STATE,
	NET_ACTIVE_WINDOW };
enum wm_atom_enum { WM_DELETE_WINDOW, WM_PROTOCOLS };
enum teleport_locations { TOP_LEFT, TOP_CENTER, TOP_RIGHT, CENTER, BOTTOM_LEFT, BOTTOM_CENTER, BOTTOM_RIGHT };

/* Handlers */
void(*handler[XCB_NO_OPERATION]) (xcb_generic_event_t *) = {
	[XCB_BUTTON_PRESS] = button_press_event,
	[XCB_KEY_PRESS] = key_press_event,
	[XCB_MAP_REQUEST] = map_event,
	[XCB_DESTROY_NOTIFY] = destroy_event,
	[XCB_ENTER_NOTIFY] = enter_event,
	[XCB_CONFIGURE_NOTIFY] = configure_event,
	[XCB_UNMAP_NOTIFY] = unmap_event,
	[XCB_CLIENT_MESSAGE] = client_message_event
};

void(*layout_handler[]) (void) = {
	[GRID] = grid,
	[ZOOM] = zoom,
	[HSTACK] = stack,
	[VSTACK] = stack
};

#include "config.h"

void (*operator_func)(const unsigned int type, int cnt);

Client *scratchpad;
struct stack del_reg;
xcb_connection_t *dpy;
char *WM_ATOM_NAMES[2];
xcb_atom_t wm_atoms[LENGTH(WM_ATOM_NAMES)];
xcb_screen_t *screen;
xcb_ewmh_connection_t *ewmh;
int numlockmask, retval, last_ws, prev_layout, cw = DEFAULT_WORKSPACE;
uint32_t border_focus, border_unfocus, border_prev_focus, border_urgent;
unsigned int cur_mode, cur_state = OPERATOR_STATE, cur_cnt = 1;
uint16_t screen_height, screen_width;
bool restart, running;

struct replay_state rep_state;

/* Add comments so that splint ignores this as it doesn't support variadic
 * macros.
 */
/*@ignore@*/
#ifdef DEBUG_ENABLE
/** Output debugging information using puts. */
#       define DEBUG(x) puts(x)
/** Output debugging information using printf to allow for formatting. */
#	define DEBUGP(M, ...) fprintf(stderr, "[DBG] %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#       define DEBUG(x) do {} while (0)
#       define DEBUGP(x, ...) do {} while (0)
#endif

#define LOG_DEBUG 1
#define LOG_INFO 2
#define LOG_WARN 3
#define LOG_ERR 4
#define LOG_NONE 5

#if LOG_LEVEL == LOG_DEBUG
#define log_debug(M, ...) fprintf(stderr, "[DEBUG] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define log_debug(x, ...) do {} while (0)
#endif


#if LOG_LEVEL <= LOG_INFO
#define log_info(M, ...) fprintf(stderr, "[INFO] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define log_info(x, ...) do {} while (0)
#endif

#if LOG_LEVEL <= LOG_WARN
#define log_warn(M, ...) fprintf(stderr, "[WARN] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define log_warn(x, ...) do {} while (0)
#endif

#if LOG_LEVEL <= LOG_ERR
#define log_err(M, ...) fprintf(stderr, "[ERROR] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define log_err(x, ...) do {} while (0)
#endif

/*@end@*/
#endif
