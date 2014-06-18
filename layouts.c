/**
 * @file layouts.c
 */

#include "config.h"
#include "layouts.h"
#include "utils.h"

/**
 * @brief Change the layout of the current workspace.
 *
 * @param arg A numerical value (arg->i) representing the layout that should be
 * used.
 */
void change_layout(const Arg *arg)
{
	if (arg->i == cur_layout || arg->i >= END_LAYOUT || arg->i < 0)
		return;
	prev_layout = cur_layout;
	cur_layout = arg->i;
	arrange_windows();
	update_focused_client(current);
	DEBUGP("Changed layout to %d\n", cur_layout);
}

/**
 * @brief Change to the next layout.
 *
 * @param arg Unused.
 */
void next_layout(const Arg *arg)
{
	const Arg a = {.i = (cur_layout + 1) % END_LAYOUT};
	change_layout(&a);
}

/**
 * @brief Change to the previous layout.
 *
 * @param arg Unused.
 */
void previous_layout(const Arg *arg)
{
	const Arg a = {.i =  cur_layout < 1 ? END_LAYOUT - 1 : cur_layout - 1};
	change_layout(&a);
}

/**
 * @brief Change to the last used layout.
 *
 * @param arg Unused.
 */
void last_layout(const Arg *arg)
{
	const Arg a = {.i = prev_layout};
	change_layout(&a);
}

/**
 * @brief Arrange the windows in a stack, whether that be horizontal or
 * vertical is decided by the current_layout.
 */
void stack(void)
{
	Client *c = NULL;
	bool vert = (cur_layout == VSTACK);
	int span = vert ? screen_height - BAR_HEIGHT :  screen_width;
	int i, n, client_x = 0;
	int client_y = BAR_BOTTOM ? 0 : BAR_HEIGHT;

	n = get_non_tff_count();
	/* TODO: Need to take into account when this has remainders. */
	/* TODO: Fix gaps between windows. */
	int client_span = (span / n) - (2 * BORDER_PX);
	DEBUG("STACK")

	if (vert) {
		move_resize(head->win, true, 0, client_y,
			screen_width - (2 * BORDER_PX), client_span);
		client_y += (BORDER_PX + (span / n));
	} else {
		move_resize(head->win, true, client_x,  BAR_BOTTOM ? 0 : BAR_HEIGHT,
			client_span, screen_height - (2 * BORDER_PX) - BAR_HEIGHT);
		client_x += (BORDER_PX + (span / n));
	}

	if (!head->next)
		return;

	for (c = head->next, i = 0; i < n - 1; c = c->next, i++) {
		if (vert) {
			move_resize(c->win, true, GAP, client_y,
					screen_width - (2 * BORDER_PX),
					client_span - BORDER_PX);
			client_y += (BORDER_PX + client_span);
		} else {
			move_resize(c->win, true, client_x, BAR_BOTTOM ? 0 : BAR_HEIGHT,
					client_span - BORDER_PX,
					screen_height - (2 * BORDER_PX) - BAR_HEIGHT);
			client_x += (BORDER_PX + client_span);
		}
	}
}

/**
 * @brief Arrange the windows into a grid layout.
 */
void grid(void)
{
	DEBUG("GRID");
}

/**
 * @brief Have one window at a time taking up the entire screen.
 */
void zoom(void)
{
	DEBUG("ZOOM");
	Client *c;
	for (c = head; c; c = c->next)
		if (!FFT(c))
			move_resize(c->win, ZOOM_GAP ? true : false,
					0, BAR_BOTTOM ? 0 : BAR_HEIGHT,
					screen_width, screen_height - BAR_HEIGHT);
}

