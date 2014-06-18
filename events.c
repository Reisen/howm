/**
 * @file events.c
 */

#include "events.h"
#include "client.h"
#include "config.h"
#include "utils.h"

/**
 * @brief The event that occurs when the mouse pointer enters a window.
 *
 * @param ev The enter event.
 */
void enter_event(xcb_generic_event_t *ev)
{
	xcb_enter_notify_event_t *ee = (xcb_enter_notify_event_t *) ev;
	DEBUG("enter_event");
	if (FOCUS_MOUSE)
		focus_window(ee->event);
}

/**
 * @brief The handler for destroy events.
 *
 * Used when a window sends a destroy event, signalling that it wants to be
 * unmapped. The client that the window belongs to is then removed from the
 * client list for its repective workspace.
 *
 * @param ev The destroy event.
 */
void destroy_event(xcb_generic_event_t *ev)
{
	DEBUG("DESTROY");
	xcb_destroy_notify_event_t *de = (xcb_destroy_notify_event_t *) ev;
	Client *c = find_client_by_win(de->window);
	if (c)
		remove_client(c);
}

/**
 * @brief Process a button press.
 *
 * @param ev The button press event.
 */
void button_press_event(xcb_generic_event_t *ev)
{
	/* FIXME: be->event doesn't seem to match with any windows managed by howm.*/
	xcb_button_press_event_t *be = (xcb_button_press_event_t *) ev;
	DEBUG("button_press_event");
	if (FOCUS_MOUSE_CLICK && be->detail == XCB_BUTTON_INDEX_1)
		focus_window(be->event);
}

/**
 * @brief Process a key press.
 *
 * This function implements an FSA that determines which command to run, as
 * well as with what targets and how many times.
 *
 * An keyboard input of the form qc (Assuming the correct mod keys have been
 * pressed) will lead to one client being killed- howm assumes no count means
 * perform the operation once. This is the behaviour that vim uses.
 *
 * Only counts as high as 9 are acceptable- I feel that any higher would just
 * be pointless.
 *
 * @param ev A keypress event.
 */
void key_press_event(xcb_generic_event_t *ev)
{
	unsigned int i = 0;
	xcb_keysym_t keysym;
	xcb_key_press_event_t *ke = (xcb_key_press_event_t *)ev;
	DEBUGP("[+] Keypress code:%d mod:%d\n", ke->detail, ke->state);
	keysym = keycode_to_keysym(ke->detail);
	switch (cur_state) {
	case OPERATOR_STATE:
		for (i = 0; i < LENGTH(operators); i++) {
			if (keysym == operators[i].sym && EQUALMODS(operators[i].mod, ke->state)) {
				operator_func = operators[i].func;
				cur_state = COUNT_STATE;
				break;
			}
		}
		break;
	case COUNT_STATE:
		if (EQUALMODS(count_mod, ke->state) && XK_1 <= keysym &&
				keysym <= XK_9) {
			/* Get a value between 1 and 9 inclusive.  */
			cur_cnt = keysym - XK_0;
			cur_state = MOTION_STATE;
			break;
		}
	case MOTION_STATE:
		for (i = 0; i < LENGTH(motions); i++) {
			if (keysym == motions[i].sym && EQUALMODS(motions[i].mod, ke->state)) {
				operator_func(motions[i].type, cur_cnt);
				cur_state = OPERATOR_STATE;
				/* Reset so that qc is equivalent to q1c. */
				cur_cnt = 1;
			}
		}
	}
	for (i = 0; i < LENGTH(keys); i++)
		if (keysym == keys[i].sym && EQUALMODS(keys[i].mod, ke->state)
				&& keys[i].func && keys[i].mode == cur_mode)
			keys[i].func(&keys[i].arg);
}

/**
 * @brief Handles mapping requests.
 *
 * When an X window wishes to be displayed, it send a mapping request. This
 * function processes that mapping request and inserts the new client (created
 * from the map requesting window) into the list of clients for the current
 * workspace.
 *
 * @param ev A mapping request event.
 */
void map_request_event(xcb_generic_event_t *ev)
{
	xcb_window_t transient = 0;
	xcb_get_window_attributes_reply_t *wa;
	xcb_map_request_event_t *me = (xcb_map_request_event_t *)ev;
	Client *c;

	wa = xcb_get_window_attributes_reply(dpy, xcb_get_window_attributes(dpy, me->window), NULL);
	if (!wa || wa->override_redirect || find_client_by_win(me->window)) {
		free(wa);
		return;
	}
	free(wa);
	DEBUG("Mapping request");
	/* Rule stuff needs to be here. */
	c = client_from_window(me->window);

	/* Assume that transient windows MUST float. */
	xcb_icccm_get_wm_transient_for_reply(dpy, xcb_icccm_get_wm_transient_for(dpy, me->window), &transient, NULL);
	c->is_floating = c->is_transient = transient ? true : false;
	arrange_windows();
	xcb_map_window(dpy, c->win);
	update_focused_client(c);
}

