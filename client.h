/**
 * @file client.h
 */

#pragma once

/**
 * @brief Represents a client that is being handled by howm.
 *
 * All the attributes that are needed by howm for a client are stored here.
 */
typedef struct Client {
	struct Client *next; /**< Clients are stored in a linked list-
			       this represents the client after this one. */
	bool is_fullscreen; /**< Is the client fullscreen? */
	bool is_floating;  /**< Is the client floating? */
	bool is_transient; /**< Is the client transient?
			     Defined at: http://standards.freedesktop.org/wm-spec/wm-spec-latest.html*/
	xcb_window_t win; /**< The window that this client represents. */
} Client;

static void move_current_down(const Arg *arg);
static void move_current_up(const Arg *arg);
static void kill_client(void);
static void move_down(Client *c);
static void move_up(Client *c);
static Client *next_client(Client *c);
static void focus_next_client(const Arg *arg);
static void focus_prev_client(const Arg *arg);
static void update_focused_client(Client *c);
static Client *prev_client(Client *c);
static Client *client_from_window(xcb_window_t w);
static void remove_client(Client *c);
static Client *find_client_by_win(xcb_window_t w);
static void client_to_ws(Client *c, const int ws);
static void current_to_ws(const Arg *arg);
