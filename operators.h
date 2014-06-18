/**
 * @file operators.h
 */

#pragma once

/**
 * @brief Represents an operator.
 *
 * Operators perform an action upon one or more targets (identified by
 * motions).
 */
typedef struct {
	int mod; /**< The mask of the modifiers pressed. */
	xcb_keysym_t sym; /**< The keysym of the pressed key. */
	void (*func)(const int unsigned type, const int cnt); /**< The function to be
						       called when the key is pressed. */
} Operator;

enum {CLIENT, WORKSPACE};

void op_kill(const int type, int cnt);
void op_move_up(const int type, int cnt);
void op_move_down(const int type, int cnt);
void move_ws_or_client(const int type, int cnt, bool up);
