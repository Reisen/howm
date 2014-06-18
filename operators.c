/**
 * @file operators.c
 */

#include <stdbool.h>
#include "operators.h"
#include "client.h"
#include "ws.h"

/**
 * @brief An operator that kills an arbitrary amount of clients or workspaces.
 *
 * @param type Whether to kill workspaces or clients.
 * @param cnt How many "things" to kill.
 */
void op_kill(const int type, int cnt)
{
	if (type == WORKSPACE) {
		DEBUGP("Killing %d workspaces.\n", cnt);
		while (cnt > 0) {
			kill_ws((cur_ws + cnt) % WORKSPACES);
			cnt--;
		}

	} else if (type == CLIENT) {
		DEBUGP("Killing %d clients.\n", cnt);
		while (cnt > 0) {
			kill_client();
			cnt--;
		}
	}
}

/**
 * @brief Move workspace/s or client/s down.
 *
 * @param type Whether to move a client or workspace.
 * @param cnt How many "things" to move.
 */
void op_move_down(const int type, int cnt)
{
	move_ws_or_client(type, cnt, false);
}

/**
 * @brief Move workspace/s or client/s up.
 *
 * @param type Whether to move a client or workspace.
 * @param cnt How many "things" to move.
 */

void op_move_up(const int type, int cnt)
{
	move_ws_or_client(type, cnt, true);
}

/**
 * @brief Moves workspace or a client either upwards or down.
 *
 * Moves a single client/workspace or multiple clients/workspaces either up or
 * down. The op_move_* functions server as simple wrappers to this.
 *
 * @param type Whether to move a client or workspace.
 * @param cnt How many "things" to move.
 * @param up Whether to move the "things" up or down. True is up.
 */

void move_ws_or_client(const int type, int cnt, bool up)
{
	if (type == WORKSPACE) {
		if (up)
			for (; cnt > 0; cnt--)
				move_ws_up(correct_ws(cur_ws + cnt - 1));
		else
			for (int i = 0; i < cnt; i++)
				move_ws_down(correct_ws(cur_ws + i));

	} else if (type == CLIENT) {
		if (up) {
			if (current == head)
				return;
			Client *c = prev_client(current);
			/* TODO optimise this by inserting the client only once
			 * and in the correct location.*/
			for (; cnt > 0; move_down(c), cnt--);
		} else {
			if (current == prev_client(head))
				return;
			int cntcopy = cnt;
			Client *c;
			for (c = current; cntcopy > 0; c = next_client(c), cntcopy--);
			for (; cnt > 0; move_up(c), cnt--);
		}
	}
}


