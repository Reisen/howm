/**
 * @file ws.c
 */

#include "ws.h"
#include "howm.h"

/**
 * @brief Kills the given workspace.
 *
 * @param ws The workspace to be killed.
 */
void kill_ws(const int ws)
{
	Arg arg = {.i = ws};
	change_ws(&arg);
	while (head)
		kill_client();
}

/**
 * @brief Focus the previous workspace.
 *
 * @param arg Unused.
 */
void focus_prev_ws(const Arg *arg)
{
	const Arg a = {.i = cur_ws < 2 ? WORKSPACES :
				cur_ws - 1};
	change_ws(&a);
}

/**
 * @brief Focus the last focused workspace.
 *
 * @param arg Unused.
 */
void focus_last_ws(const Arg *arg)
{
	const Arg a = {.i = last_ws};
	change_ws(&a);
}

/**
 * @brief Focus the next workspace.
 *
 * @param arg Unused.
 */
void focus_next_ws(const Arg *arg)
{
	const Arg a = {.i = (cur_ws + 1) % WORKSPACES};
	change_ws(&a);
}

/**
 * @brief Change to a different workspace.
 *
 * @param arg arg->i indicates which workspace howm should change to.
 */
void change_ws(const Arg *arg)
{
	if (arg->i > WORKSPACES || arg->i <= 0 || arg->i == cur_ws)
		return;
	last_ws = cur_ws;
	select_ws(arg->i);
	for (Client *c = head; c; c = c->next)
		xcb_map_window(dpy, c->win);
	select_ws(last_ws);
	for (Client *c = head; c; c = c->next)
		xcb_unmap_window(dpy, c->win);
	select_ws(arg->i);
	arrange_windows();
	update_focused_client(current);
	howm_info();
}

/**
 * @brief Saves the information about a current workspace.
 *
 * @param i The index of the workspace to be saved. Note: Workspaces begin at
 * index 1.
 */
void save_ws(int i)
{
	if (i < 1 || i > WORKSPACES)
		return;
	workspaces[i].layout = cur_layout;
	workspaces[i].current = current;
	workspaces[i].head = head;
	workspaces[i].prev_foc = prev_foc;
}

/**
 * @brief Reloads the information about a workspace and sets it as the current
 * workspace.
 *
 * @param i The index of the workspace to be reloaded and set as current. Note:
 * Workspaces begin at index 1.
 */
void select_ws(int i)
{
	save_ws(cur_ws);
	cur_layout = workspaces[i].layout;
	current = workspaces[i].current;
	head = workspaces[i].head;
	prev_foc = workspaces[i].prev_foc;
	cur_ws = i;
}

/**
 * @brief Focus the previous workspace.
 *
 * @param arg Unused.
 */
void focus_prev_ws(const Arg *arg)
{
	const Arg a = {.i = cur_ws < 2 ? WORKSPACES :
				cur_ws - 1};
	change_ws(&a);
}

/**
 * @brief Focus the next workspace.
 *
 * @param arg Unused.
 */
void focus_next_ws(const Arg *arg)
{
	const Arg a = {.i = (cur_ws + 1) % WORKSPACES};
	change_ws(&a);
}

/**
 * @brief Correctly wrap a workspace number.
 *
 * This prevents workspace numbers from being greater than WORKSPACES or less
 * than 1.
 *
 * @param ws The value that needs to be corrected.
 *
 * @return A corrected workspace number.
 */
int correct_ws(int ws)
{
	if (ws > WORKSPACES)
		return ws - WORKSPACES;
	else if (ws < 1)
		return ws + WORKSPACES;
	else
		return ws;
}

/**
 * @brief Move the entirety of one workspace to another.
 *
 * Takes every client from one workspace and places them, in original order,
 * onto the end of the destination workspace's client list.
 *
 * @param s_ws The source workspace that clients should be moved from.
 * @param d_ws The target workspace that clients should be moved to.
 */
void move_ws(int s_ws, int d_ws)
{
		/* Source workspace. */
		Arg arg = {.i = s_ws};
		change_ws(&arg);
		while (head)
			/* The destination workspace. */
			client_to_ws(head, d_ws);
		change_ws(&arg);
}

/**
 * @brief Move the entirety of the current workspace to the next workspace
 * down.
 *
 * @param ws The workspace to be moved.
 */
void move_ws_down(int ws)
{
	move_ws(ws, correct_ws(prev_ws(ws)));
}

/**
 * @brief Move the entirety of the current workspace to the next workspace
 * up.
 *
 * @param ws The workspace to be moved.
 */
void move_ws_up(int ws)
{
	move_ws(ws, correct_ws(next_ws(ws)));
}


