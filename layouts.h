/**
 * @file layouts.h
 */

#pragma once

enum {ZOOM, GRID, HSTACK, VSTACK, END_LAYOUT};

void change_layout(const Arg *arg);
void next_layout(const Arg *arg);
void previous_layout(const Arg *arg);
void last_layout(const Arg *arg);
void stack(void);
void grid(void);
void zoom(void);
static void arrange_windows(void);
