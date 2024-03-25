/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef LABWC_OVERLAY_H
#define LABWC_OVERLAY_H

#include <wlr/util/box.h>
#include "common/graphic-helpers.h"
#include "regions.h"
#include "view.h"

struct overlay {
	struct wlr_scene_tree *tree;
	union {
		struct wlr_scene_rect *rect;
		struct multi_rect *pixman_rect;
	};

	/* Represents currently shown or delayed overlay */
	struct {
		/* Region overlay */
		struct region *region;

		/* Snap-to-edge overlay */
		enum view_edge edge;
		struct output *output;
	} active;

	/* For delayed snap-to-edge overlay */
	struct wl_event_source *timer;
};

/* Calls overlay_hide() internally if there's no overlay to show */
void overlay_update(struct seat *seat);
/* This function must be called when server->grabbed_view is destroyed */
void overlay_hide(struct seat *seat);

#endif
