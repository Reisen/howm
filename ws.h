/**
 * @file ws.h
 */

#pragma once

#include "client.h"

/**
 * @brief Represents a workspace, which stores clients.
 *
 * Clients are stored as a linked list. Changing to a different workspace will
 * cause different clients to be rendered on the screen.
 */
typedef struct {
	int layout; /**< The current layout of the WS, as defined in the
		      layout enum. */
	Client *head;  /**< The start of the linked list. */
	Client *prev_foc; /**< The last focused client. This is seperate to
			    the linked list structure. */
	Client *current;  /**< The client that is currently in focus. */
} Workspace;

static void kill_ws(const int ws);
static void focus_next_ws(const Arg *arg);
static void focus_prev_ws(const Arg *arg);
static void focus_last_ws(const Arg *arg);
static void change_ws(const Arg *arg);
static void save_ws(int i);
static void select_ws(int i);
static int prev_ws(int ws);
static int next_ws(int ws);
static int correct_ws(int ws);
static void move_ws_down(int ws);
static void move_ws_up(int ws);
static void move_ws(int s_ws, int d_ws);
