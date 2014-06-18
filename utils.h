/**
 * @file utils.h
 */

#pragma once

#include "config.h"

/** Calculates the length of an array. */
#define LENGTH(x) (unsigned int)(sizeof(x) / sizeof(*x))
/** Checks to see if a client is floating, fullscreen or transient. */
#define FFT(client) (c->is_transient || c->is_floating || c->is_fullscreen)

/* Add comments so that splint ignores this as it doesn't support variadic
 * macros.
 */
/*@ignore@*/
#ifdef DEBUG_ENABLE
 /** Output debugging information using puts. */
#	define DEBUG(x) puts(x);
 /** Output debugging information using printf to allow for formatting. */
#	define DEBUGP(x, ...) printf(x, ##__VA_ARGS__);
#else
#	define DEBUG(x) do {} while (0)
#	define DEBUGP(x, ...) do {} while (0)
#endif

/*@end@*/

/**
 * @brief Represents an argument.
 *
 * Used to hold data that is sent as a parameter to a function when called as a
 * result of a keypress.
 */
typedef union {
	const char **cmd; /**< Represents a command that will be called by a shell.  */
	float f; /**< Commonly used for scaling operations. */
	int i; /**< Usually used for specifying workspaces or clients. */
} Arg;

