/**
 * @file client.c
 */

#include "client.h"
#include "utils.h"

/**
 * @brief Moves the current client down.
 *
 * @param arg Unused.
 */
void move_current_down(const Arg *arg)
{
	move_down(current);
}

/**
 * @brief Moves the current client up.
 *
 * @param arg Unused.
 */
void move_current_up(const Arg *arg)
{
	move_up(current);
}

/**
 * @brief Kills the current client.
 */
void kill_client(void)
{
	if (!current)
		return;
	/* TODO: Kill the window in a nicer way and get it to consistently die. */
	xcb_kill_client(dpy, current->win);
	DEBUG("Killing Client");
	remove_client(current);
}

/**
 * @brief Move a client down in its client list.
 *
 * @param c The client to be moved.
 */
void move_down(Client *c)
{
	if (!c)
		return;
	Client *prev = prev_client(c);
	Client *n = (c->next) ? c->next : head;
	if (!prev)
		return;
	if (head == c)
		head = n;
	else
		prev->next = c->next;
	c->next = (c->next) ? n->next : n;
	if (n->next == c->next)
		n->next = c;
	else
		head = c;
	arrange_windows();
}

/**
 * @brief Move a client up in its client list.
 *
 * @param c The client to be moved down.
 */
void move_up(Client *c)
{
	if (!c)
		return;
	Client *p = prev_client(c);
	Client *pp = NULL;
	if (!p)
		return;
	if (p->next)
		for (pp = head; pp && pp->next != p; pp = pp->next)
			;
	if (pp)
		pp->next = c;
	else
		head = (head == c) ? c->next : c;
	p->next = (c->next == head) ? c : c->next;
	c->next = (c->next == head) ? NULL : p;
	arrange_windows();
}

/**
 * @brief Find the next client.
 *
 * Note: This function wraps around the end of the list of clients. If c is the
 * last item in the list of clients, then the head of the list is returned.
 *
 * @param c The client which needs to have its next found.
 *
 * @return The next client, if c is the last client in the list then this will
 * be head. If c is NULL or there is only one client in the client list, NULL
 * will be returned.
 */
Client *next_client(Client *c)
{
	if (!c || !head->next)
		return NULL;
	if (c->next)
		return c->next;
	return head;
}


/**
 * @brief brief Move focus onto the client next in the client list.
 *
 * @param arg The argument passed from the config file. Note: The argument goes
 * unused.
 */
void focus_next_client(const Arg *arg)
{
	if (!current || !head->next)
		return;
	DEBUG("focus_next");
	update_focused_client(current->next ? current->next : head);
}

/**
 * @brief brief Move focus onto the client previous in the client list.
 *
 * @param arg The argument passed from the config file. Note: The argument goes
 * unused.
 */
void focus_prev_client(const Arg *arg)
{
	if (!current || !head->next)
		return;
	DEBUG("focus_prev");
	prev_foc = current;
	update_focused_client(prev_client(prev_foc));
}

/**
 * @brief Sets c to the active window and gives it input focus. Sorts out
 * border colours as well.
 *
 * @param c The client that is currently in focus.
 */
void update_focused_client(Client *c)
{
	if (!head) {
		prev_foc = current = NULL;
		xcb_delete_property(dpy, screen->root, net_atoms[NET_ACTIVE_WINDOW]);
		return;
	} else if (c == prev_foc) {
		current = (prev_foc ? prev_foc : head);
		prev_foc = prev_client(current);
	} else if (c != current) {
		prev_foc = current;
		current = c;
	}

	DEBUG("UPDATING");
	unsigned int all = 0, fullscreen = 0, float_trans = 0;
	for (c = head; c; c = c->next, ++all) {
		if (FFT(c))
			fullscreen++;
			if (!c->is_fullscreen)
				float_trans++;
	}
	xcb_window_t windows[all];
	windows[(current->is_floating || current->is_transient) ? 0 : fullscreen] = current->win;
	c = head;
	for (fullscreen += FFT(current) ? 1 : 0; c; c = c->next) {
		set_border_width(c->win, (c->is_fullscreen ||
					!head->next) ? 0 : BORDER_PX);
		xcb_change_window_attributes(dpy, c->win, XCB_CW_BORDER_PIXEL,
			(c == current ? &border_focus : &border_unfocus));
		if (c != current)
			windows[c->is_fullscreen ? --fullscreen : FFT(c) ?
				--float_trans : --all] = c->win;
	}

	for (float_trans = 0; float_trans <= all; ++float_trans)
		elevate_window(windows[all - float_trans]);

	xcb_change_property(dpy, XCB_PROP_MODE_REPLACE, screen->root,
			net_atoms[NET_ACTIVE_WINDOW], XCB_ATOM_WINDOW, 32, 1, &current->win);
	xcb_set_input_focus(dpy, XCB_INPUT_FOCUS_POINTER_ROOT, current->win,
			XCB_CURRENT_TIME);
	arrange_windows();
}

/**
 * @brief Find the client before the given client.
 *
 * @param c The client which needs to have its previous found.
 *
 * @return The previous client, so long as the given client isn't NULL and
 * there is more than one client. Else, NULL.
 */
Client *prev_client(Client *c)
{
	if (!c || !head->next)
		return NULL;
	Client *p;
	for (p = head; p->next && p->next != c; p = p->next)
		;
	return p;
}

/**
 * @brief Convert a window into a client.
 *
 * @param w A valid xcb window.
 *
 * @return A client that has already been inserted into the linked list of
 * clients.
 */
Client *client_from_window(xcb_window_t w)
{
	Client *c = (Client *)calloc(1, sizeof(Client));
	Client *t = prev_client(head); /* Get the last element. */

	if (!c)
		err(EXIT_FAILURE, "Can't allocate memory for client.");
	if (!head)
		head = c;
	else if (t)
		t->next = c;
	else
		head->next = c;
	c->win = w;
	unsigned int vals[1] = {XCB_EVENT_MASK_PROPERTY_CHANGE |
		(FOCUS_MOUSE ? XCB_EVENT_MASK_ENTER_WINDOW : 0)};
	xcb_change_window_attributes(dpy, c->win, XCB_CW_EVENT_MASK, vals);
	return c;
}

/**
 * @brief Remove a client from its workspace client list.
 *
 * @param c The client to be removed.
 */
void remove_client(Client *c)
{
	Client **temp = NULL;
	int w = 1, cw = cur_ws;
	bool found;
	for (found = false; w < WORKSPACES && !found; w++)
		for (temp = &head, select_ws(cw); temp &&
				!(found = *temp == c); temp = &(*temp)->next)
			;
	*temp = c->next;
	if (c == prev_foc)
		prev_foc = prev_client(c);
	if (c == head)
		head = NULL;
	if (c == current || !head->next)
		update_focused_client(prev_foc);
	free(c);
	c = NULL;
	if (cw == w)
		arrange_windows();
	else
		select_ws(cw);
}

/**
 * @brief Search workspaces for a window, returning the client that it belongs
 * to.
 *
 * During searching, the current workspace is changed so that all workspaces
 * can be searched. Upon finding the client, the original workspace is
 * restored.
 *
 * @param win A valid XCB window that is used when searching all clients across
 * all desktops.
 *
 * @return The found client.
 */
Client *find_client_by_win(xcb_window_t win)
{
	bool found;
	int w = 1, cw = cur_ws;
	Client *c = NULL;
	for (found = false; w < WORKSPACES && !found; ++w)
		for (select_ws(w), c = head; c && !(found = (win == c->win)); c = c->next)
			;
	if (cw != w)
		select_ws(cw);
	return c;
}

/**
 * @brief Moves a client from one workspace to another.
 *
 * @param c The client to be moved.
 * @param ws The ws that the client should be moved to.
 */
void client_to_ws(Client *c, const int ws)
{
	/* Performed for the current workspace. */
	if (!c || ws == cur_ws)
		return;
	Client *last;
	Client *prev = prev_client(c);
	int cw = cur_ws;
	Arg arg = {.i = ws};
	/* Target workspace. */
	change_ws(&arg);
	last = prev_client(head);
	if (!head)
		head = c;
	else if (last)
		last->next = c;
	else
		head->next = c;

	arg.i = cw;
	/* Current workspace. */
	change_ws(&arg);
	if (c == head || !prev)
		head = next_client(c);
	else
		prev->next = next_client(c);
	c->next = NULL;
	xcb_unmap_window(dpy, c->win);
	update_focused_client(prev_foc);
	if (FOLLOW_MOVE) {
		arg.i = ws;
		change_ws(&arg);
	} else {
		arrange_windows();
	}
}

/**
 * @brief Moves the current client to the workspace passed in through arg.
 *
 * @param arg arg->i is the target workspace.
 */
void current_to_ws(const Arg *arg)
{
	client_to_ws(current, arg->i);
}

