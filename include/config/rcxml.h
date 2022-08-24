/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef __LABWC_RCXML_H
#define __LABWC_RCXML_H

#include <stdbool.h>
#include <stdio.h>
#include <wayland-server-core.h>

#include "common/buf.h"
#include "config/libinput.h"
#include "theme.h"

struct rcxml {
	char *config_dir;

	/* core */
	bool xdg_shell_server_side_deco;
	int gap;
	bool adaptive_sync;

	/* focus */
	bool focus_follow_mouse;
	bool raise_on_focus;

	/* theme */
	char *theme_name;
	int corner_radius;
	char *font_name_activewindow;
	char *font_name_menuitem;
	char *font_name_osd;
	int font_size_activewindow;
	int font_size_menuitem;
	int font_size_osd;
	/* Pointer to current theme */
	struct theme *theme;

	/* keyboard */
	int repeat_rate;
	int repeat_delay;
	struct wl_list keybinds;   /* struct keybind.link */

	/* mouse */
	long doubleclick_time;     /* in ms */
	struct wl_list mousebinds; /* struct mousebind.link */

	/* libinput */
	struct wl_list libinput_categories;

	/* resistance */
	int screen_edge_strength;

	/* window snapping */
	int snap_edge_range;
	bool snap_top_maximize;

	/* cycle view (alt+tab) */
	bool cycle_preview_contents;
	bool cycle_preview_outlines;

	struct {
		int popuptime;
		struct wl_list workspaces;  /* struct workspace.link */
	} workspace_config;
};

extern struct rcxml rc;

void rcxml_parse_xml(struct buf *b);
void rcxml_read(const char *filename);
void rcxml_finish(void);

#endif /* __LABWC_RCXML_H */
