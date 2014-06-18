/**
 * @file events.h
 */

#pragma once

#include <xcb/xcb.h>

static void enter_event(xcb_generic_event_t *ev);
static void destroy_event(xcb_generic_event_t *ev);
static void button_press_event(xcb_generic_event_t *ev);
static void key_press_event(xcb_generic_event_t *ev);
static void map_request_event(xcb_generic_event_t *ev);
