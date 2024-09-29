// SPDX-License-Identifier: GPL-2.0-only
#define _POSIX_C_SOURCE 200809L
/*
 * Based on gdk-pixbuf/io-xpm.c
 * https://gitlab.gnome.org/GNOME/gtk/-/....
 *      blob/4623a485c7d2eeffe3ed5d89d1b3359cc8e9f495/gdk-pixbuf/io-xpm.c
 * Copyright (C) 1999 Mark Crichton
 * Copyright (C) 1999 The Free Software Foundation
 *
 * Authors: Mark Crichton <crichton@gimp.org>
 *          Federico Mena-Quintero <federico@gimp.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 *
 * Modified by:
 * Copyright (C) 2016 Ovidiu M
 * Copyright (C) 2016-2024 Johan Malm
 *
 * ...because unless we need Gtk for something else it seems like a sledge
 * hammer to crack a nut. If linking with Gtk it's better to base the
 * implementation on the following:
 *
 *      GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
 *      gdk_cairo_set_source_pixbuf(cr, pixbuf, 0, 0);
 */
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <errno.h>
#include "buffer.h"
#include "common/macros.h"
#include "common/mem.h"
#include "common/string-helpers.h"
#include "img/img-xpm.h"
#include "labwc.h"

/* Output from gtk/gdk-pixbuf/gen-color-table.pl (commit f0175e1f) */
static const char color_names[] =
	"AliceBlue\0"
	"AntiqueWhite\0"
	"AntiqueWhite1\0"
	"AntiqueWhite2\0"
	"AntiqueWhite3\0"
	"AntiqueWhite4\0"
	"aqua\0"
	"aquamarine\0"
	"aquamarine1\0"
	"aquamarine2\0"
	"aquamarine3\0"
	"aquamarine4\0"
	"azure\0"
	"azure1\0"
	"azure2\0"
	"azure3\0"
	"azure4\0"
	"beige\0"
	"bisque\0"
	"bisque1\0"
	"bisque2\0"
	"bisque3\0"
	"bisque4\0"
	"black\0"
	"BlanchedAlmond\0"
	"blue\0"
	"blue1\0"
	"blue2\0"
	"blue3\0"
	"blue4\0"
	"BlueViolet\0"
	"brown\0"
	"brown1\0"
	"brown2\0"
	"brown3\0"
	"brown4\0"
	"burlywood\0"
	"burlywood1\0"
	"burlywood2\0"
	"burlywood3\0"
	"burlywood4\0"
	"CadetBlue\0"
	"CadetBlue1\0"
	"CadetBlue2\0"
	"CadetBlue3\0"
	"CadetBlue4\0"
	"chartreuse\0"
	"chartreuse1\0"
	"chartreuse2\0"
	"chartreuse3\0"
	"chartreuse4\0"
	"chocolate\0"
	"chocolate1\0"
	"chocolate2\0"
	"chocolate3\0"
	"chocolate4\0"
	"coral\0"
	"coral1\0"
	"coral2\0"
	"coral3\0"
	"coral4\0"
	"CornflowerBlue\0"
	"cornsilk\0"
	"cornsilk1\0"
	"cornsilk2\0"
	"cornsilk3\0"
	"cornsilk4\0"
	"crimson\0"
	"cyan\0"
	"cyan1\0"
	"cyan2\0"
	"cyan3\0"
	"cyan4\0"
	"DarkBlue\0"
	"DarkCyan\0"
	"DarkGoldenrod\0"
	"DarkGoldenrod1\0"
	"DarkGoldenrod2\0"
	"DarkGoldenrod3\0"
	"DarkGoldenrod4\0"
	"DarkGray\0"
	"DarkGreen\0"
	"DarkGrey\0"
	"DarkKhaki\0"
	"DarkMagenta\0"
	"DarkOliveGreen\0"
	"DarkOliveGreen1\0"
	"DarkOliveGreen2\0"
	"DarkOliveGreen3\0"
	"DarkOliveGreen4\0"
	"DarkOrange\0"
	"DarkOrange1\0"
	"DarkOrange2\0"
	"DarkOrange3\0"
	"DarkOrange4\0"
	"DarkOrchid\0"
	"DarkOrchid1\0"
	"DarkOrchid2\0"
	"DarkOrchid3\0"
	"DarkOrchid4\0"
	"DarkRed\0"
	"DarkSalmon\0"
	"DarkSeaGreen\0"
	"DarkSeaGreen1\0"
	"DarkSeaGreen2\0"
	"DarkSeaGreen3\0"
	"DarkSeaGreen4\0"
	"DarkSlateBlue\0"
	"DarkSlateGray\0"
	"DarkSlateGray1\0"
	"DarkSlateGray2\0"
	"DarkSlateGray3\0"
	"DarkSlateGray4\0"
	"DarkSlateGrey\0"
	"DarkTurquoise\0"
	"DarkViolet\0"
	"DeepPink\0"
	"DeepPink1\0"
	"DeepPink2\0"
	"DeepPink3\0"
	"DeepPink4\0"
	"DeepSkyBlue\0"
	"DeepSkyBlue1\0"
	"DeepSkyBlue2\0"
	"DeepSkyBlue3\0"
	"DeepSkyBlue4\0"
	"DimGray\0"
	"DimGrey\0"
	"DodgerBlue\0"
	"DodgerBlue1\0"
	"DodgerBlue2\0"
	"DodgerBlue3\0"
	"DodgerBlue4\0"
	"firebrick\0"
	"firebrick1\0"
	"firebrick2\0"
	"firebrick3\0"
	"firebrick4\0"
	"FloralWhite\0"
	"ForestGreen\0"
	"fuchsia\0"
	"gainsboro\0"
	"GhostWhite\0"
	"gold\0"
	"gold1\0"
	"gold2\0"
	"gold3\0"
	"gold4\0"
	"goldenrod\0"
	"goldenrod1\0"
	"goldenrod2\0"
	"goldenrod3\0"
	"goldenrod4\0"
	"gray\0"
	"gray0\0"
	"gray1\0"
	"gray10\0"
	"gray100\0"
	"gray11\0"
	"gray12\0"
	"gray13\0"
	"gray14\0"
	"gray15\0"
	"gray16\0"
	"gray17\0"
	"gray18\0"
	"gray19\0"
	"gray2\0"
	"gray20\0"
	"gray21\0"
	"gray22\0"
	"gray23\0"
	"gray24\0"
	"gray25\0"
	"gray26\0"
	"gray27\0"
	"gray28\0"
	"gray29\0"
	"gray3\0"
	"gray30\0"
	"gray31\0"
	"gray32\0"
	"gray33\0"
	"gray34\0"
	"gray35\0"
	"gray36\0"
	"gray37\0"
	"gray38\0"
	"gray39\0"
	"gray4\0"
	"gray40\0"
	"gray41\0"
	"gray42\0"
	"gray43\0"
	"gray44\0"
	"gray45\0"
	"gray46\0"
	"gray47\0"
	"gray48\0"
	"gray49\0"
	"gray5\0"
	"gray50\0"
	"gray51\0"
	"gray52\0"
	"gray53\0"
	"gray54\0"
	"gray55\0"
	"gray56\0"
	"gray57\0"
	"gray58\0"
	"gray59\0"
	"gray6\0"
	"gray60\0"
	"gray61\0"
	"gray62\0"
	"gray63\0"
	"gray64\0"
	"gray65\0"
	"gray66\0"
	"gray67\0"
	"gray68\0"
	"gray69\0"
	"gray7\0"
	"gray70\0"
	"gray71\0"
	"gray72\0"
	"gray73\0"
	"gray74\0"
	"gray75\0"
	"gray76\0"
	"gray77\0"
	"gray78\0"
	"gray79\0"
	"gray8\0"
	"gray80\0"
	"gray81\0"
	"gray82\0"
	"gray83\0"
	"gray84\0"
	"gray85\0"
	"gray86\0"
	"gray87\0"
	"gray88\0"
	"gray89\0"
	"gray9\0"
	"gray90\0"
	"gray91\0"
	"gray92\0"
	"gray93\0"
	"gray94\0"
	"gray95\0"
	"gray96\0"
	"gray97\0"
	"gray98\0"
	"gray99\0"
	"green\0"
	"green1\0"
	"green2\0"
	"green3\0"
	"green4\0"
	"GreenYellow\0"
	"grey\0"
	"grey0\0"
	"grey1\0"
	"grey10\0"
	"grey100\0"
	"grey11\0"
	"grey12\0"
	"grey13\0"
	"grey14\0"
	"grey15\0"
	"grey16\0"
	"grey17\0"
	"grey18\0"
	"grey19\0"
	"grey2\0"
	"grey20\0"
	"grey21\0"
	"grey22\0"
	"grey23\0"
	"grey24\0"
	"grey25\0"
	"grey26\0"
	"grey27\0"
	"grey28\0"
	"grey29\0"
	"grey3\0"
	"grey30\0"
	"grey31\0"
	"grey32\0"
	"grey33\0"
	"grey34\0"
	"grey35\0"
	"grey36\0"
	"grey37\0"
	"grey38\0"
	"grey39\0"
	"grey4\0"
	"grey40\0"
	"grey41\0"
	"grey42\0"
	"grey43\0"
	"grey44\0"
	"grey45\0"
	"grey46\0"
	"grey47\0"
	"grey48\0"
	"grey49\0"
	"grey5\0"
	"grey50\0"
	"grey51\0"
	"grey52\0"
	"grey53\0"
	"grey54\0"
	"grey55\0"
	"grey56\0"
	"grey57\0"
	"grey58\0"
	"grey59\0"
	"grey6\0"
	"grey60\0"
	"grey61\0"
	"grey62\0"
	"grey63\0"
	"grey64\0"
	"grey65\0"
	"grey66\0"
	"grey67\0"
	"grey68\0"
	"grey69\0"
	"grey7\0"
	"grey70\0"
	"grey71\0"
	"grey72\0"
	"grey73\0"
	"grey74\0"
	"grey75\0"
	"grey76\0"
	"grey77\0"
	"grey78\0"
	"grey79\0"
	"grey8\0"
	"grey80\0"
	"grey81\0"
	"grey82\0"
	"grey83\0"
	"grey84\0"
	"grey85\0"
	"grey86\0"
	"grey87\0"
	"grey88\0"
	"grey89\0"
	"grey9\0"
	"grey90\0"
	"grey91\0"
	"grey92\0"
	"grey93\0"
	"grey94\0"
	"grey95\0"
	"grey96\0"
	"grey97\0"
	"grey98\0"
	"grey99\0"
	"honeydew\0"
	"honeydew1\0"
	"honeydew2\0"
	"honeydew3\0"
	"honeydew4\0"
	"HotPink\0"
	"HotPink1\0"
	"HotPink2\0"
	"HotPink3\0"
	"HotPink4\0"
	"IndianRed\0"
	"IndianRed1\0"
	"IndianRed2\0"
	"IndianRed3\0"
	"IndianRed4\0"
	"indigo\0"
	"ivory\0"
	"ivory1\0"
	"ivory2\0"
	"ivory3\0"
	"ivory4\0"
	"khaki\0"
	"khaki1\0"
	"khaki2\0"
	"khaki3\0"
	"khaki4\0"
	"lavender\0"
	"LavenderBlush\0"
	"LavenderBlush1\0"
	"LavenderBlush2\0"
	"LavenderBlush3\0"
	"LavenderBlush4\0"
	"LawnGreen\0"
	"LemonChiffon\0"
	"LemonChiffon1\0"
	"LemonChiffon2\0"
	"LemonChiffon3\0"
	"LemonChiffon4\0"
	"LightBlue\0"
	"LightBlue1\0"
	"LightBlue2\0"
	"LightBlue3\0"
	"LightBlue4\0"
	"LightCoral\0"
	"LightCyan\0"
	"LightCyan1\0"
	"LightCyan2\0"
	"LightCyan3\0"
	"LightCyan4\0"
	"LightGoldenrod\0"
	"LightGoldenrod1\0"
	"LightGoldenrod2\0"
	"LightGoldenrod3\0"
	"LightGoldenrod4\0"
	"LightGoldenrodYellow\0"
	"LightGray\0"
	"LightGreen\0"
	"LightGrey\0"
	"LightPink\0"
	"LightPink1\0"
	"LightPink2\0"
	"LightPink3\0"
	"LightPink4\0"
	"LightSalmon\0"
	"LightSalmon1\0"
	"LightSalmon2\0"
	"LightSalmon3\0"
	"LightSalmon4\0"
	"LightSeaGreen\0"
	"LightSkyBlue\0"
	"LightSkyBlue1\0"
	"LightSkyBlue2\0"
	"LightSkyBlue3\0"
	"LightSkyBlue4\0"
	"LightSlateBlue\0"
	"LightSlateGray\0"
	"LightSlateGrey\0"
	"LightSteelBlue\0"
	"LightSteelBlue1\0"
	"LightSteelBlue2\0"
	"LightSteelBlue3\0"
	"LightSteelBlue4\0"
	"LightYellow\0"
	"LightYellow1\0"
	"LightYellow2\0"
	"LightYellow3\0"
	"LightYellow4\0"
	"lime\0"
	"LimeGreen\0"
	"linen\0"
	"magenta\0"
	"magenta1\0"
	"magenta2\0"
	"magenta3\0"
	"magenta4\0"
	"maroon\0"
	"maroon1\0"
	"maroon2\0"
	"maroon3\0"
	"maroon4\0"
	"MediumAquamarine\0"
	"MediumBlue\0"
	"MediumOrchid\0"
	"MediumOrchid1\0"
	"MediumOrchid2\0"
	"MediumOrchid3\0"
	"MediumOrchid4\0"
	"MediumPurple\0"
	"MediumPurple1\0"
	"MediumPurple2\0"
	"MediumPurple3\0"
	"MediumPurple4\0"
	"MediumSeaGreen\0"
	"MediumSlateBlue\0"
	"MediumSpringGreen\0"
	"MediumTurquoise\0"
	"MediumVioletRed\0"
	"MidnightBlue\0"
	"MintCream\0"
	"MistyRose\0"
	"MistyRose1\0"
	"MistyRose2\0"
	"MistyRose3\0"
	"MistyRose4\0"
	"moccasin\0"
	"NavajoWhite\0"
	"NavajoWhite1\0"
	"NavajoWhite2\0"
	"NavajoWhite3\0"
	"NavajoWhite4\0"
	"navy\0"
	"NavyBlue\0"
	"OldLace\0"
	"olive\0"
	"OliveDrab\0"
	"OliveDrab1\0"
	"OliveDrab2\0"
	"OliveDrab3\0"
	"OliveDrab4\0"
	"orange\0"
	"orange1\0"
	"orange2\0"
	"orange3\0"
	"orange4\0"
	"OrangeRed\0"
	"OrangeRed1\0"
	"OrangeRed2\0"
	"OrangeRed3\0"
	"OrangeRed4\0"
	"orchid\0"
	"orchid1\0"
	"orchid2\0"
	"orchid3\0"
	"orchid4\0"
	"PaleGoldenrod\0"
	"PaleGreen\0"
	"PaleGreen1\0"
	"PaleGreen2\0"
	"PaleGreen3\0"
	"PaleGreen4\0"
	"PaleTurquoise\0"
	"PaleTurquoise1\0"
	"PaleTurquoise2\0"
	"PaleTurquoise3\0"
	"PaleTurquoise4\0"
	"PaleVioletRed\0"
	"PaleVioletRed1\0"
	"PaleVioletRed2\0"
	"PaleVioletRed3\0"
	"PaleVioletRed4\0"
	"PapayaWhip\0"
	"PeachPuff\0"
	"PeachPuff1\0"
	"PeachPuff2\0"
	"PeachPuff3\0"
	"PeachPuff4\0"
	"peru\0"
	"pink\0"
	"pink1\0"
	"pink2\0"
	"pink3\0"
	"pink4\0"
	"plum\0"
	"plum1\0"
	"plum2\0"
	"plum3\0"
	"plum4\0"
	"PowderBlue\0"
	"purple\0"
	"purple1\0"
	"purple2\0"
	"purple3\0"
	"purple4\0"
	"red\0"
	"red1\0"
	"red2\0"
	"red3\0"
	"red4\0"
	"RosyBrown\0"
	"RosyBrown1\0"
	"RosyBrown2\0"
	"RosyBrown3\0"
	"RosyBrown4\0"
	"RoyalBlue\0"
	"RoyalBlue1\0"
	"RoyalBlue2\0"
	"RoyalBlue3\0"
	"RoyalBlue4\0"
	"SaddleBrown\0"
	"salmon\0"
	"salmon1\0"
	"salmon2\0"
	"salmon3\0"
	"salmon4\0"
	"SandyBrown\0"
	"SeaGreen\0"
	"SeaGreen1\0"
	"SeaGreen2\0"
	"SeaGreen3\0"
	"SeaGreen4\0"
	"seashell\0"
	"seashell1\0"
	"seashell2\0"
	"seashell3\0"
	"seashell4\0"
	"sienna\0"
	"sienna1\0"
	"sienna2\0"
	"sienna3\0"
	"sienna4\0"
	"silver\0"
	"SkyBlue\0"
	"SkyBlue1\0"
	"SkyBlue2\0"
	"SkyBlue3\0"
	"SkyBlue4\0"
	"SlateBlue\0"
	"SlateBlue1\0"
	"SlateBlue2\0"
	"SlateBlue3\0"
	"SlateBlue4\0"
	"SlateGray\0"
	"SlateGray1\0"
	"SlateGray2\0"
	"SlateGray3\0"
	"SlateGray4\0"
	"SlateGrey\0"
	"snow\0"
	"snow1\0"
	"snow2\0"
	"snow3\0"
	"snow4\0"
	"SpringGreen\0"
	"SpringGreen1\0"
	"SpringGreen2\0"
	"SpringGreen3\0"
	"SpringGreen4\0"
	"SteelBlue\0"
	"SteelBlue1\0"
	"SteelBlue2\0"
	"SteelBlue3\0"
	"SteelBlue4\0"
	"tan\0"
	"tan1\0"
	"tan2\0"
	"tan3\0"
	"tan4\0"
	"teal\0"
	"thistle\0"
	"thistle1\0"
	"thistle2\0"
	"thistle3\0"
	"thistle4\0"
	"tomato\0"
	"tomato1\0"
	"tomato2\0"
	"tomato3\0"
	"tomato4\0"
	"turquoise\0"
	"turquoise1\0"
	"turquoise2\0"
	"turquoise3\0"
	"turquoise4\0"
	"violet\0"
	"VioletRed\0"
	"VioletRed1\0"
	"VioletRed2\0"
	"VioletRed3\0"
	"VioletRed4\0"
	"wheat\0"
	"wheat1\0"
	"wheat2\0"
	"wheat3\0"
	"wheat4\0"
	"white\0"
	"WhiteSmoke\0"
	"yellow\0"
	"yellow1\0"
	"yellow2\0"
	"yellow3\0"
	"yellow4\0"
	"YellowGreen\0";

struct xpm_color_entry {
	uint16_t name_offset;
	unsigned char red;
	unsigned char green;
	unsigned char blue;
};

static const struct xpm_color_entry xcolors[] = {
	{ 0, 240, 248, 255 },
	{ 10, 250, 235, 215 },
	{ 23, 255, 239, 219 },
	{ 37, 238, 223, 204 },
	{ 51, 205, 192, 176 },
	{ 65, 139, 131, 120 },
	{ 79, 0, 255, 255 },
	{ 84, 127, 255, 212 },
	{ 95, 127, 255, 212 },
	{ 107, 118, 238, 198 },
	{ 119, 102, 205, 170 },
	{ 131, 69, 139, 116 },
	{ 143, 240, 255, 255 },
	{ 149, 240, 255, 255 },
	{ 156, 224, 238, 238 },
	{ 163, 193, 205, 205 },
	{ 170, 131, 139, 139 },
	{ 177, 245, 245, 220 },
	{ 183, 255, 228, 196 },
	{ 190, 255, 228, 196 },
	{ 198, 238, 213, 183 },
	{ 206, 205, 183, 158 },
	{ 214, 139, 125, 107 },
	{ 222, 0, 0, 0 },
	{ 228, 255, 235, 205 },
	{ 243, 0, 0, 255 },
	{ 248, 0, 0, 255 },
	{ 254, 0, 0, 238 },
	{ 260, 0, 0, 205 },
	{ 266, 0, 0, 139 },
	{ 272, 138, 43, 226 },
	{ 283, 165, 42, 42 },
	{ 289, 255, 64, 64 },
	{ 296, 238, 59, 59 },
	{ 303, 205, 51, 51 },
	{ 310, 139, 35, 35 },
	{ 317, 222, 184, 135 },
	{ 327, 255, 211, 155 },
	{ 338, 238, 197, 145 },
	{ 349, 205, 170, 125 },
	{ 360, 139, 115, 85 },
	{ 371, 95, 158, 160 },
	{ 381, 152, 245, 255 },
	{ 392, 142, 229, 238 },
	{ 403, 122, 197, 205 },
	{ 414, 83, 134, 139 },
	{ 425, 127, 255, 0 },
	{ 436, 127, 255, 0 },
	{ 448, 118, 238, 0 },
	{ 460, 102, 205, 0 },
	{ 472, 69, 139, 0 },
	{ 484, 210, 105, 30 },
	{ 494, 255, 127, 36 },
	{ 505, 238, 118, 33 },
	{ 516, 205, 102, 29 },
	{ 527, 139, 69, 19 },
	{ 538, 255, 127, 80 },
	{ 544, 255, 114, 86 },
	{ 551, 238, 106, 80 },
	{ 558, 205, 91, 69 },
	{ 565, 139, 62, 47 },
	{ 572, 100, 149, 237 },
	{ 587, 255, 248, 220 },
	{ 596, 255, 248, 220 },
	{ 606, 238, 232, 205 },
	{ 616, 205, 200, 177 },
	{ 626, 139, 136, 120 },
	{ 636, 220, 20, 60 },
	{ 644, 0, 255, 255 },
	{ 649, 0, 255, 255 },
	{ 655, 0, 238, 238 },
	{ 661, 0, 205, 205 },
	{ 667, 0, 139, 139 },
	{ 673, 0, 0, 139 },
	{ 682, 0, 139, 139 },
	{ 691, 184, 134, 11 },
	{ 705, 255, 185, 15 },
	{ 720, 238, 173, 14 },
	{ 735, 205, 149, 12 },
	{ 750, 139, 101, 8 },
	{ 765, 169, 169, 169 },
	{ 774, 0, 100, 0 },
	{ 784, 169, 169, 169 },
	{ 793, 189, 183, 107 },
	{ 803, 139, 0, 139 },
	{ 815, 85, 107, 47 },
	{ 830, 202, 255, 112 },
	{ 846, 188, 238, 104 },
	{ 862, 162, 205, 90 },
	{ 878, 110, 139, 61 },
	{ 894, 255, 140, 0 },
	{ 905, 255, 127, 0 },
	{ 917, 238, 118, 0 },
	{ 929, 205, 102, 0 },
	{ 941, 139, 69, 0 },
	{ 953, 153, 50, 204 },
	{ 964, 191, 62, 255 },
	{ 976, 178, 58, 238 },
	{ 988, 154, 50, 205 },
	{ 1000, 104, 34, 139 },
	{ 1012, 139, 0, 0 },
	{ 1020, 233, 150, 122 },
	{ 1031, 143, 188, 143 },
	{ 1044, 193, 255, 193 },
	{ 1058, 180, 238, 180 },
	{ 1072, 155, 205, 155 },
	{ 1086, 105, 139, 105 },
	{ 1100, 72, 61, 139 },
	{ 1114, 47, 79, 79 },
	{ 1128, 151, 255, 255 },
	{ 1143, 141, 238, 238 },
	{ 1158, 121, 205, 205 },
	{ 1173, 82, 139, 139 },
	{ 1188, 47, 79, 79 },
	{ 1202, 0, 206, 209 },
	{ 1216, 148, 0, 211 },
	{ 1227, 255, 20, 147 },
	{ 1236, 255, 20, 147 },
	{ 1246, 238, 18, 137 },
	{ 1256, 205, 16, 118 },
	{ 1266, 139, 10, 80 },
	{ 1276, 0, 191, 255 },
	{ 1288, 0, 191, 255 },
	{ 1301, 0, 178, 238 },
	{ 1314, 0, 154, 205 },
	{ 1327, 0, 104, 139 },
	{ 1340, 105, 105, 105 },
	{ 1348, 105, 105, 105 },
	{ 1356, 30, 144, 255 },
	{ 1367, 30, 144, 255 },
	{ 1379, 28, 134, 238 },
	{ 1391, 24, 116, 205 },
	{ 1403, 16, 78, 139 },
	{ 1415, 178, 34, 34 },
	{ 1425, 255, 48, 48 },
	{ 1436, 238, 44, 44 },
	{ 1447, 205, 38, 38 },
	{ 1458, 139, 26, 26 },
	{ 1469, 255, 250, 240 },
	{ 1481, 34, 139, 34 },
	{ 1493, 255, 0, 255 },
	{ 1501, 220, 220, 220 },
	{ 1511, 248, 248, 255 },
	{ 1522, 255, 215, 0 },
	{ 1527, 255, 215, 0 },
	{ 1533, 238, 201, 0 },
	{ 1539, 205, 173, 0 },
	{ 1545, 139, 117, 0 },
	{ 1551, 218, 165, 32 },
	{ 1561, 255, 193, 37 },
	{ 1572, 238, 180, 34 },
	{ 1583, 205, 155, 29 },
	{ 1594, 139, 105, 20 },
	{ 1605, 128, 128, 128 },
	{ 1610, 0, 0, 0 },
	{ 1616, 3, 3, 3 },
	{ 1622, 26, 26, 26 },
	{ 1629, 255, 255, 255 },
	{ 1637, 28, 28, 28 },
	{ 1644, 31, 31, 31 },
	{ 1651, 33, 33, 33 },
	{ 1658, 36, 36, 36 },
	{ 1665, 38, 38, 38 },
	{ 1672, 41, 41, 41 },
	{ 1679, 43, 43, 43 },
	{ 1686, 46, 46, 46 },
	{ 1693, 48, 48, 48 },
	{ 1700, 5, 5, 5 },
	{ 1706, 51, 51, 51 },
	{ 1713, 54, 54, 54 },
	{ 1720, 56, 56, 56 },
	{ 1727, 59, 59, 59 },
	{ 1734, 61, 61, 61 },
	{ 1741, 64, 64, 64 },
	{ 1748, 66, 66, 66 },
	{ 1755, 69, 69, 69 },
	{ 1762, 71, 71, 71 },
	{ 1769, 74, 74, 74 },
	{ 1776, 8, 8, 8 },
	{ 1782, 77, 77, 77 },
	{ 1789, 79, 79, 79 },
	{ 1796, 82, 82, 82 },
	{ 1803, 84, 84, 84 },
	{ 1810, 87, 87, 87 },
	{ 1817, 89, 89, 89 },
	{ 1824, 92, 92, 92 },
	{ 1831, 94, 94, 94 },
	{ 1838, 97, 97, 97 },
	{ 1845, 99, 99, 99 },
	{ 1852, 10, 10, 10 },
	{ 1858, 102, 102, 102 },
	{ 1865, 105, 105, 105 },
	{ 1872, 107, 107, 107 },
	{ 1879, 110, 110, 110 },
	{ 1886, 112, 112, 112 },
	{ 1893, 115, 115, 115 },
	{ 1900, 117, 117, 117 },
	{ 1907, 120, 120, 120 },
	{ 1914, 122, 122, 122 },
	{ 1921, 125, 125, 125 },
	{ 1928, 13, 13, 13 },
	{ 1934, 127, 127, 127 },
	{ 1941, 130, 130, 130 },
	{ 1948, 133, 133, 133 },
	{ 1955, 135, 135, 135 },
	{ 1962, 138, 138, 138 },
	{ 1969, 140, 140, 140 },
	{ 1976, 143, 143, 143 },
	{ 1983, 145, 145, 145 },
	{ 1990, 148, 148, 148 },
	{ 1997, 150, 150, 150 },
	{ 2004, 15, 15, 15 },
	{ 2010, 153, 153, 153 },
	{ 2017, 156, 156, 156 },
	{ 2024, 158, 158, 158 },
	{ 2031, 161, 161, 161 },
	{ 2038, 163, 163, 163 },
	{ 2045, 166, 166, 166 },
	{ 2052, 168, 168, 168 },
	{ 2059, 171, 171, 171 },
	{ 2066, 173, 173, 173 },
	{ 2073, 176, 176, 176 },
	{ 2080, 18, 18, 18 },
	{ 2086, 179, 179, 179 },
	{ 2093, 181, 181, 181 },
	{ 2100, 184, 184, 184 },
	{ 2107, 186, 186, 186 },
	{ 2114, 189, 189, 189 },
	{ 2121, 191, 191, 191 },
	{ 2128, 194, 194, 194 },
	{ 2135, 196, 196, 196 },
	{ 2142, 199, 199, 199 },
	{ 2149, 201, 201, 201 },
	{ 2156, 20, 20, 20 },
	{ 2162, 204, 204, 204 },
	{ 2169, 207, 207, 207 },
	{ 2176, 209, 209, 209 },
	{ 2183, 212, 212, 212 },
	{ 2190, 214, 214, 214 },
	{ 2197, 217, 217, 217 },
	{ 2204, 219, 219, 219 },
	{ 2211, 222, 222, 222 },
	{ 2218, 224, 224, 224 },
	{ 2225, 227, 227, 227 },
	{ 2232, 23, 23, 23 },
	{ 2238, 229, 229, 229 },
	{ 2245, 232, 232, 232 },
	{ 2252, 235, 235, 235 },
	{ 2259, 237, 237, 237 },
	{ 2266, 240, 240, 240 },
	{ 2273, 242, 242, 242 },
	{ 2280, 245, 245, 245 },
	{ 2287, 247, 247, 247 },
	{ 2294, 250, 250, 250 },
	{ 2301, 252, 252, 252 },
	{ 2308, 0, 128, 0 },
	{ 2314, 0, 255, 0 },
	{ 2321, 0, 238, 0 },
	{ 2328, 0, 205, 0 },
	{ 2335, 0, 139, 0 },
	{ 2342, 173, 255, 47 },
	{ 2354, 128, 128, 128 },
	{ 2359, 0, 0, 0 },
	{ 2365, 3, 3, 3 },
	{ 2371, 26, 26, 26 },
	{ 2378, 255, 255, 255 },
	{ 2386, 28, 28, 28 },
	{ 2393, 31, 31, 31 },
	{ 2400, 33, 33, 33 },
	{ 2407, 36, 36, 36 },
	{ 2414, 38, 38, 38 },
	{ 2421, 41, 41, 41 },
	{ 2428, 43, 43, 43 },
	{ 2435, 46, 46, 46 },
	{ 2442, 48, 48, 48 },
	{ 2449, 5, 5, 5 },
	{ 2455, 51, 51, 51 },
	{ 2462, 54, 54, 54 },
	{ 2469, 56, 56, 56 },
	{ 2476, 59, 59, 59 },
	{ 2483, 61, 61, 61 },
	{ 2490, 64, 64, 64 },
	{ 2497, 66, 66, 66 },
	{ 2504, 69, 69, 69 },
	{ 2511, 71, 71, 71 },
	{ 2518, 74, 74, 74 },
	{ 2525, 8, 8, 8 },
	{ 2531, 77, 77, 77 },
	{ 2538, 79, 79, 79 },
	{ 2545, 82, 82, 82 },
	{ 2552, 84, 84, 84 },
	{ 2559, 87, 87, 87 },
	{ 2566, 89, 89, 89 },
	{ 2573, 92, 92, 92 },
	{ 2580, 94, 94, 94 },
	{ 2587, 97, 97, 97 },
	{ 2594, 99, 99, 99 },
	{ 2601, 10, 10, 10 },
	{ 2607, 102, 102, 102 },
	{ 2614, 105, 105, 105 },
	{ 2621, 107, 107, 107 },
	{ 2628, 110, 110, 110 },
	{ 2635, 112, 112, 112 },
	{ 2642, 115, 115, 115 },
	{ 2649, 117, 117, 117 },
	{ 2656, 120, 120, 120 },
	{ 2663, 122, 122, 122 },
	{ 2670, 125, 125, 125 },
	{ 2677, 13, 13, 13 },
	{ 2683, 127, 127, 127 },
	{ 2690, 130, 130, 130 },
	{ 2697, 133, 133, 133 },
	{ 2704, 135, 135, 135 },
	{ 2711, 138, 138, 138 },
	{ 2718, 140, 140, 140 },
	{ 2725, 143, 143, 143 },
	{ 2732, 145, 145, 145 },
	{ 2739, 148, 148, 148 },
	{ 2746, 150, 150, 150 },
	{ 2753, 15, 15, 15 },
	{ 2759, 153, 153, 153 },
	{ 2766, 156, 156, 156 },
	{ 2773, 158, 158, 158 },
	{ 2780, 161, 161, 161 },
	{ 2787, 163, 163, 163 },
	{ 2794, 166, 166, 166 },
	{ 2801, 168, 168, 168 },
	{ 2808, 171, 171, 171 },
	{ 2815, 173, 173, 173 },
	{ 2822, 176, 176, 176 },
	{ 2829, 18, 18, 18 },
	{ 2835, 179, 179, 179 },
	{ 2842, 181, 181, 181 },
	{ 2849, 184, 184, 184 },
	{ 2856, 186, 186, 186 },
	{ 2863, 189, 189, 189 },
	{ 2870, 191, 191, 191 },
	{ 2877, 194, 194, 194 },
	{ 2884, 196, 196, 196 },
	{ 2891, 199, 199, 199 },
	{ 2898, 201, 201, 201 },
	{ 2905, 20, 20, 20 },
	{ 2911, 204, 204, 204 },
	{ 2918, 207, 207, 207 },
	{ 2925, 209, 209, 209 },
	{ 2932, 212, 212, 212 },
	{ 2939, 214, 214, 214 },
	{ 2946, 217, 217, 217 },
	{ 2953, 219, 219, 219 },
	{ 2960, 222, 222, 222 },
	{ 2967, 224, 224, 224 },
	{ 2974, 227, 227, 227 },
	{ 2981, 23, 23, 23 },
	{ 2987, 229, 229, 229 },
	{ 2994, 232, 232, 232 },
	{ 3001, 235, 235, 235 },
	{ 3008, 237, 237, 237 },
	{ 3015, 240, 240, 240 },
	{ 3022, 242, 242, 242 },
	{ 3029, 245, 245, 245 },
	{ 3036, 247, 247, 247 },
	{ 3043, 250, 250, 250 },
	{ 3050, 252, 252, 252 },
	{ 3057, 240, 255, 240 },
	{ 3066, 240, 255, 240 },
	{ 3076, 224, 238, 224 },
	{ 3086, 193, 205, 193 },
	{ 3096, 131, 139, 131 },
	{ 3106, 255, 105, 180 },
	{ 3114, 255, 110, 180 },
	{ 3123, 238, 106, 167 },
	{ 3132, 205, 96, 144 },
	{ 3141, 139, 58, 98 },
	{ 3150, 205, 92, 92 },
	{ 3160, 255, 106, 106 },
	{ 3171, 238, 99, 99 },
	{ 3182, 205, 85, 85 },
	{ 3193, 139, 58, 58 },
	{ 3204, 75, 0, 130 },
	{ 3211, 255, 255, 240 },
	{ 3217, 255, 255, 240 },
	{ 3224, 238, 238, 224 },
	{ 3231, 205, 205, 193 },
	{ 3238, 139, 139, 131 },
	{ 3245, 240, 230, 140 },
	{ 3251, 255, 246, 143 },
	{ 3258, 238, 230, 133 },
	{ 3265, 205, 198, 115 },
	{ 3272, 139, 134, 78 },
	{ 3279, 230, 230, 250 },
	{ 3288, 255, 240, 245 },
	{ 3302, 255, 240, 245 },
	{ 3317, 238, 224, 229 },
	{ 3332, 205, 193, 197 },
	{ 3347, 139, 131, 134 },
	{ 3362, 124, 252, 0 },
	{ 3372, 255, 250, 205 },
	{ 3385, 255, 250, 205 },
	{ 3399, 238, 233, 191 },
	{ 3413, 205, 201, 165 },
	{ 3427, 139, 137, 112 },
	{ 3441, 173, 216, 230 },
	{ 3451, 191, 239, 255 },
	{ 3462, 178, 223, 238 },
	{ 3473, 154, 192, 205 },
	{ 3484, 104, 131, 139 },
	{ 3495, 240, 128, 128 },
	{ 3506, 224, 255, 255 },
	{ 3516, 224, 255, 255 },
	{ 3527, 209, 238, 238 },
	{ 3538, 180, 205, 205 },
	{ 3549, 122, 139, 139 },
	{ 3560, 238, 221, 130 },
	{ 3575, 255, 236, 139 },
	{ 3591, 238, 220, 130 },
	{ 3607, 205, 190, 112 },
	{ 3623, 139, 129, 76 },
	{ 3639, 250, 250, 210 },
	{ 3660, 211, 211, 211 },
	{ 3670, 144, 238, 144 },
	{ 3681, 211, 211, 211 },
	{ 3691, 255, 182, 193 },
	{ 3701, 255, 174, 185 },
	{ 3712, 238, 162, 173 },
	{ 3723, 205, 140, 149 },
	{ 3734, 139, 95, 101 },
	{ 3745, 255, 160, 122 },
	{ 3757, 255, 160, 122 },
	{ 3770, 238, 149, 114 },
	{ 3783, 205, 129, 98 },
	{ 3796, 139, 87, 66 },
	{ 3809, 32, 178, 170 },
	{ 3823, 135, 206, 250 },
	{ 3836, 176, 226, 255 },
	{ 3850, 164, 211, 238 },
	{ 3864, 141, 182, 205 },
	{ 3878, 96, 123, 139 },
	{ 3892, 132, 112, 255 },
	{ 3907, 119, 136, 153 },
	{ 3922, 119, 136, 153 },
	{ 3937, 176, 196, 222 },
	{ 3952, 202, 225, 255 },
	{ 3968, 188, 210, 238 },
	{ 3984, 162, 181, 205 },
	{ 4000, 110, 123, 139 },
	{ 4016, 255, 255, 224 },
	{ 4028, 255, 255, 224 },
	{ 4041, 238, 238, 209 },
	{ 4054, 205, 205, 180 },
	{ 4067, 139, 139, 122 },
	{ 4080, 0, 255, 0 },
	{ 4085, 50, 205, 50 },
	{ 4095, 250, 240, 230 },
	{ 4101, 255, 0, 255 },
	{ 4109, 255, 0, 255 },
	{ 4118, 238, 0, 238 },
	{ 4127, 205, 0, 205 },
	{ 4136, 139, 0, 139 },
	{ 4145, 128, 0, 0 },
	{ 4152, 255, 52, 179 },
	{ 4160, 238, 48, 167 },
	{ 4168, 205, 41, 144 },
	{ 4176, 139, 28, 98 },
	{ 4184, 102, 205, 170 },
	{ 4201, 0, 0, 205 },
	{ 4212, 186, 85, 211 },
	{ 4225, 224, 102, 255 },
	{ 4239, 209, 95, 238 },
	{ 4253, 180, 82, 205 },
	{ 4267, 122, 55, 139 },
	{ 4281, 147, 112, 219 },
	{ 4294, 171, 130, 255 },
	{ 4308, 159, 121, 238 },
	{ 4322, 137, 104, 205 },
	{ 4336, 93, 71, 139 },
	{ 4350, 60, 179, 113 },
	{ 4365, 123, 104, 238 },
	{ 4381, 0, 250, 154 },
	{ 4399, 72, 209, 204 },
	{ 4415, 199, 21, 133 },
	{ 4431, 25, 25, 112 },
	{ 4444, 245, 255, 250 },
	{ 4454, 255, 228, 225 },
	{ 4464, 255, 228, 225 },
	{ 4475, 238, 213, 210 },
	{ 4486, 205, 183, 181 },
	{ 4497, 139, 125, 123 },
	{ 4508, 255, 228, 181 },
	{ 4517, 255, 222, 173 },
	{ 4529, 255, 222, 173 },
	{ 4542, 238, 207, 161 },
	{ 4555, 205, 179, 139 },
	{ 4568, 139, 121, 94 },
	{ 4581, 0, 0, 128 },
	{ 4586, 0, 0, 128 },
	{ 4595, 253, 245, 230 },
	{ 4603, 128, 128, 0 },
	{ 4609, 107, 142, 35 },
	{ 4619, 192, 255, 62 },
	{ 4630, 179, 238, 58 },
	{ 4641, 154, 205, 50 },
	{ 4652, 105, 139, 34 },
	{ 4663, 255, 165, 0 },
	{ 4670, 255, 165, 0 },
	{ 4678, 238, 154, 0 },
	{ 4686, 205, 133, 0 },
	{ 4694, 139, 90, 0 },
	{ 4702, 255, 69, 0 },
	{ 4712, 255, 69, 0 },
	{ 4723, 238, 64, 0 },
	{ 4734, 205, 55, 0 },
	{ 4745, 139, 37, 0 },
	{ 4756, 218, 112, 214 },
	{ 4763, 255, 131, 250 },
	{ 4771, 238, 122, 233 },
	{ 4779, 205, 105, 201 },
	{ 4787, 139, 71, 137 },
	{ 4795, 238, 232, 170 },
	{ 4809, 152, 251, 152 },
	{ 4819, 154, 255, 154 },
	{ 4830, 144, 238, 144 },
	{ 4841, 124, 205, 124 },
	{ 4852, 84, 139, 84 },
	{ 4863, 175, 238, 238 },
	{ 4877, 187, 255, 255 },
	{ 4892, 174, 238, 238 },
	{ 4907, 150, 205, 205 },
	{ 4922, 102, 139, 139 },
	{ 4937, 219, 112, 147 },
	{ 4951, 255, 130, 171 },
	{ 4966, 238, 121, 159 },
	{ 4981, 205, 104, 137 },
	{ 4996, 139, 71, 93 },
	{ 5011, 255, 239, 213 },
	{ 5022, 255, 218, 185 },
	{ 5032, 255, 218, 185 },
	{ 5043, 238, 203, 173 },
	{ 5054, 205, 175, 149 },
	{ 5065, 139, 119, 101 },
	{ 5076, 205, 133, 63 },
	{ 5081, 255, 192, 203 },
	{ 5086, 255, 181, 197 },
	{ 5092, 238, 169, 184 },
	{ 5098, 205, 145, 158 },
	{ 5104, 139, 99, 108 },
	{ 5110, 221, 160, 221 },
	{ 5115, 255, 187, 255 },
	{ 5121, 238, 174, 238 },
	{ 5127, 205, 150, 205 },
	{ 5133, 139, 102, 139 },
	{ 5139, 176, 224, 230 },
	{ 5150, 128, 0, 128 },
	{ 5157, 155, 48, 255 },
	{ 5165, 145, 44, 238 },
	{ 5173, 125, 38, 205 },
	{ 5181, 85, 26, 139 },
	{ 5189, 255, 0, 0 },
	{ 5193, 255, 0, 0 },
	{ 5198, 238, 0, 0 },
	{ 5203, 205, 0, 0 },
	{ 5208, 139, 0, 0 },
	{ 5213, 188, 143, 143 },
	{ 5223, 255, 193, 193 },
	{ 5234, 238, 180, 180 },
	{ 5245, 205, 155, 155 },
	{ 5256, 139, 105, 105 },
	{ 5267, 65, 105, 225 },
	{ 5277, 72, 118, 255 },
	{ 5288, 67, 110, 238 },
	{ 5299, 58, 95, 205 },
	{ 5310, 39, 64, 139 },
	{ 5321, 139, 69, 19 },
	{ 5333, 250, 128, 114 },
	{ 5340, 255, 140, 105 },
	{ 5348, 238, 130, 98 },
	{ 5356, 205, 112, 84 },
	{ 5364, 139, 76, 57 },
	{ 5372, 244, 164, 96 },
	{ 5383, 46, 139, 87 },
	{ 5392, 84, 255, 159 },
	{ 5402, 78, 238, 148 },
	{ 5412, 67, 205, 128 },
	{ 5422, 46, 139, 87 },
	{ 5432, 255, 245, 238 },
	{ 5441, 255, 245, 238 },
	{ 5451, 238, 229, 222 },
	{ 5461, 205, 197, 191 },
	{ 5471, 139, 134, 130 },
	{ 5481, 160, 82, 45 },
	{ 5488, 255, 130, 71 },
	{ 5496, 238, 121, 66 },
	{ 5504, 205, 104, 57 },
	{ 5512, 139, 71, 38 },
	{ 5520, 192, 192, 192 },
	{ 5527, 135, 206, 235 },
	{ 5535, 135, 206, 255 },
	{ 5544, 126, 192, 238 },
	{ 5553, 108, 166, 205 },
	{ 5562, 74, 112, 139 },
	{ 5571, 106, 90, 205 },
	{ 5581, 131, 111, 255 },
	{ 5592, 122, 103, 238 },
	{ 5603, 105, 89, 205 },
	{ 5614, 71, 60, 139 },
	{ 5625, 112, 128, 144 },
	{ 5635, 198, 226, 255 },
	{ 5646, 185, 211, 238 },
	{ 5657, 159, 182, 205 },
	{ 5668, 108, 123, 139 },
	{ 5679, 112, 128, 144 },
	{ 5689, 255, 250, 250 },
	{ 5694, 255, 250, 250 },
	{ 5700, 238, 233, 233 },
	{ 5706, 205, 201, 201 },
	{ 5712, 139, 137, 137 },
	{ 5718, 0, 255, 127 },
	{ 5730, 0, 255, 127 },
	{ 5743, 0, 238, 118 },
	{ 5756, 0, 205, 102 },
	{ 5769, 0, 139, 69 },
	{ 5782, 70, 130, 180 },
	{ 5792, 99, 184, 255 },
	{ 5803, 92, 172, 238 },
	{ 5814, 79, 148, 205 },
	{ 5825, 54, 100, 139 },
	{ 5836, 210, 180, 140 },
	{ 5840, 255, 165, 79 },
	{ 5845, 238, 154, 73 },
	{ 5850, 205, 133, 63 },
	{ 5855, 139, 90, 43 },
	{ 5860, 0, 128, 128 },
	{ 5865, 216, 191, 216 },
	{ 5873, 255, 225, 255 },
	{ 5882, 238, 210, 238 },
	{ 5891, 205, 181, 205 },
	{ 5900, 139, 123, 139 },
	{ 5909, 255, 99, 71 },
	{ 5916, 255, 99, 71 },
	{ 5924, 238, 92, 66 },
	{ 5932, 205, 79, 57 },
	{ 5940, 139, 54, 38 },
	{ 5948, 64, 224, 208 },
	{ 5958, 0, 245, 255 },
	{ 5969, 0, 229, 238 },
	{ 5980, 0, 197, 205 },
	{ 5991, 0, 134, 139 },
	{ 6002, 238, 130, 238 },
	{ 6009, 208, 32, 144 },
	{ 6019, 255, 62, 150 },
	{ 6030, 238, 58, 140 },
	{ 6041, 205, 50, 120 },
	{ 6052, 139, 34, 82 },
	{ 6063, 245, 222, 179 },
	{ 6069, 255, 231, 186 },
	{ 6076, 238, 216, 174 },
	{ 6083, 205, 186, 150 },
	{ 6090, 139, 126, 102 },
	{ 6097, 255, 255, 255 },
	{ 6103, 245, 245, 245 },
	{ 6114, 255, 255, 0 },
	{ 6121, 255, 255, 0 },
	{ 6129, 238, 238, 0 },
	{ 6137, 205, 205, 0 },
	{ 6145, 139, 139, 0 },
	{ 6153, 154, 205, 50 }
};

enum buf_op { op_header, op_cmap, op_body };

struct xpm_color {
	char *color_string;
	uint16_t red;
	uint16_t green;
	uint16_t blue;
	int transparent;
};

struct file_handle {
	FILE *infile;
	char *buffer;
	uint32_t buffer_size;
};

/* The following 2 routines (parse_color, find_color) come from Tk, via the Win32
 * port of GDK. The licensing terms on these (longer than the functions) is:
 *
 * This software is copyrighted by the Regents of the University of
 * California, Sun Microsystems, Inc., and other parties.  The following
 * terms apply to all files associated with the software unless explicitly
 * disclaimed in individual files.
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 *
 * IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY
 * DERIVATIVES THEREOF, EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE
 * IS PROVIDED ON AN "AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE
 * NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
 * MODIFICATIONS.
 *
 * GOVERNMENT USE: If you are acquiring this software on behalf of the
 * U.S. government, the Government shall have only "Restricted Rights"
 * in the software and related documentation as defined in the Federal
 * Acquisition Regulations (FARs) in Clause 52.227.19 (c) (2).  If you
 * are acquiring the software on behalf of the Department of Defense, the
 * software shall be classified as "Commercial Computer Software" and the
 * Government shall have only "Restricted Rights" as defined in Clause
 * 252.227-7013 (c) (1) of DFARs.  Notwithstanding the foregoing, the
 * authors grant the U.S. Government and others acting in its behalf
 * permission to use and distribute the software in accordance with the
 * terms specified in this license.
 */

/*
 *----------------------------------------------------------------------
 *
 * find_color --
 *
 *	This routine finds the color entry that corresponds to the
 *	specified color.
 *
 * Results:
 *	Returns non-zero on success.  The RGB values of the XColor
 *	will be initialized to the proper values on success.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int compare_xcolor_entries(const void *a, const void *b)
{
	return strcasecmp((const char *)a, color_names
		+ ((const struct xpm_color_entry *)b)->name_offset);
}

static int find_color(const char *name, struct xpm_color *color_ptr)
{
	struct xpm_color_entry *found;

	found = bsearch(name, xcolors, ARRAY_SIZE(xcolors),
		sizeof(struct xpm_color_entry), compare_xcolor_entries);
	if (!found) {
		return 0;
	}

	color_ptr->red = (found->red * 65535) / 255;
	color_ptr->green = (found->green * 65535) / 255;
	color_ptr->blue = (found->blue * 65535) / 255;

	return 1;
}

/*
 *----------------------------------------------------------------------
 *
 * parse_color --
 *
 *	Partial implementation of X color name parsing interface.
 *
 * Results:
 *	Returns 1 on success.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int parse_color(const char *spec, struct xpm_color *color_ptr)
{
	if (spec[0] == '#') {
		int i;
		unsigned int red, green, blue;

		i = strlen(spec + 1);
		if (i % 3) {
			return 0;
		}
		i /= 3;

		if (i == 4) {
			if (sscanf(spec + 1, "%4x%4x%4x", &red, &green, &blue) != 3) {
				return 0;
			}
			color_ptr->red = red;
			color_ptr->green = green;
			color_ptr->blue = blue;
		} else if (i == 1) {
			if (sscanf(spec + 1, "%1x%1x%1x", &red, &green, &blue) != 3) {
				return 0;
			}
			color_ptr->red = (red * 65535) / 15;
			color_ptr->green = (green * 65535) / 15;
			color_ptr->blue = (blue * 65535) / 15;
		} else if (i == 2) {
			if (sscanf(spec + 1, "%2x%2x%2x", &red, &green, &blue) != 3) {
				return 0;
			}
			color_ptr->red = (red * 65535) / 255;
			color_ptr->green = (green * 65535) / 255;
			color_ptr->blue = (blue * 65535) / 255;
		} else /* if (i == 3) */ {
			if (sscanf(spec + 1, "%3x%3x%3x", &red, &green, &blue) != 3) {
				return 0;
			}
			color_ptr->red = (red * 65535) / 4095;
			color_ptr->green = (green * 65535) / 4095;
			color_ptr->blue = (blue * 65535) / 4095;
		}
	} else {
		if (!find_color(spec, color_ptr)) {
			return 0;
		}
	}
	return 1;
}

static int xpm_seek_string(FILE *infile, const char *str)
{
	char instr[1024];

	while (!feof(infile)) {
		if (fscanf(infile, "%1023s", instr) < 0) {
			return 0;
		}
		if (strcmp(instr, str) == 0) {
			return 1;
		}
	}

	return 0;
}

static int xpm_seek_char(FILE *infile, char c)
{
	int b, oldb;

	while ((b = getc(infile)) != EOF) {
		if (c != b && b == '/') {
			b = getc(infile);
			if (b == EOF) {
				return 0;
			} else if (b == '*') {
				/* we have a comment */
				b = -1;
				do {
					oldb = b;
					b = getc(infile);
					if (b == EOF) {
						return 0;
					}
				} while (!(oldb == '*' && b == '/'));
			}
		} else if (c == b) {
			return 1;
		}
	}

	return 0;
}

static int xpm_read_string(FILE *infile, char **buffer, uint32_t *buffer_size)
{
	int c;
	uint32_t cnt = 0, bufsiz, ret = 0;
	char *buf;

	buf = *buffer;
	bufsiz = *buffer_size;
	if (!buf) {
		bufsiz = 10 * sizeof(char);
		buf = (char *)calloc(bufsiz, sizeof(char));
	}

	do {
		c = getc(infile);
	} while (c != EOF && c != '"');

	if (c != '"') {
		goto out;
	}

	while ((c = getc(infile)) != EOF) {
		if (cnt == bufsiz) {
			uint32_t new_size = bufsiz * 2;

			if (new_size > bufsiz) {
				bufsiz = new_size;
			} else {
				goto out;
			}

			buf = (char *)xrealloc(buf, bufsiz);
			buf[bufsiz - 1] = '\0';
		}

		if (c != '"') {
			buf[cnt++] = c;
		} else {
			buf[cnt] = 0;
			ret = 1;
			break;
		}
	}

out:
	buf[bufsiz - 1] = '\0'; /* ensure null termination for errors */
	*buffer = buf;
	*buffer_size = bufsiz;
	return ret;
}

/*
 * Unlike the standard C library isspace() function, this only recognizes
 * standard ASCII white-space and ignores the locale, returning FALSE for all
 * non-ASCII characters.  Also, unlike the standard library function, this takes
 * a char, not an int, so don't call it on EOF, but no need to cast to unsigned
 * char before passing a possibly non-ASCII character in.  Similar to
 * g_ascii_isspace.
 */
static int xpm_isspace(char c)
{
	return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v';
}

static char *xpm_extract_color(const char *buffer)
{
	const char *p = &buffer[0];
	int new_key = 0;
	int key = 0;
	int current_key = 1;
	int space = 128;
	char word[129], color[129], current_color[129];
	char *r;

	word[0] = '\0';
	color[0] = '\0';
	current_color[0] = '\0';
	while (1) {
		/* skip whitespace */
		for (; *p != '\0' && xpm_isspace(*p); p++) {
			; /* nothing to do */
		}
		/* copy word */
		for (r = word; *p != '\0' && !xpm_isspace(*p) && r - word < (int)sizeof(word) - 1;
				p++, r++) {
			*r = *p;
		}
		*r = '\0';
		if (*word == '\0') {
			if (color[0] == '\0') {
				/* incomplete colormap entry */
				return NULL;
			}
			/* end of entry, still store the last color */
			new_key = 1;
		} else if (key > 0 && color[0] == '\0') {
			/* next word must be a color name part */
			new_key = 0;
		} else {
			if (strcmp(word, "c") == 0) {
				new_key = 5;
			} else if (strcmp(word, "g") == 0) {
				new_key = 4;
			} else if (strcmp(word, "g4") == 0) {
				new_key = 3;
			} else if (strcmp(word, "m") == 0) {
				new_key = 2;
			} else if (strcmp(word, "s") == 0) {
				new_key = 1;
			} else {
				new_key = 0;
			}
		}
		if (new_key == 0) { /* word is a color name part */
			if (key == 0) {
				/* key expected */
				return NULL;
			}
			/* accumulate color name */
			if (color[0] != '\0') {
				strncat(color, " ", space);
				space -= MIN(space, 1);
			}
			strncat(color, word, space);
			space -= MIN(space, (int)strlen(word));
		} else { /* word is a key */
			if (key > current_key) {
				current_key = key;
				strcpy(current_color, color);
			}
			space = 128;
			color[0] = '\0';
			key = new_key;
			if (*p == '\0') {
				break;
			}
		}
	}
	if (current_key > 1) {
		return xstrdup(current_color);
	} else {
		return NULL;
	}
}

/* (almost) direct copy from gdkpixmap.c... loads an XPM from a file */

static const char *file_buffer(enum buf_op op, void *handle)
{
	struct file_handle *h = (struct file_handle *)handle;

	if (op == op_header) {
		if (xpm_seek_string(h->infile, "XPM") != 1) {
			goto out;
		}
		if (xpm_seek_char(h->infile, '{') != 1) {
			goto out;
		}
	}
	if (op == op_cmap || op == op_header) {
		xpm_seek_char(h->infile, '"');
		fseek(h->infile, -1, SEEK_CUR);
	}
	if (xpm_read_string(h->infile, &h->buffer, &h->buffer_size)) {
		return h->buffer;
	}
out:
	return NULL;
}

static struct xpm_color *lookup_color(struct xpm_color *colors, int n_colors, const char *name)
{
	int i;

	for (i = 0; i < n_colors; i++) {
		struct xpm_color *color = &colors[i];

		if (strcmp(name, color->color_string) == 0) {
			return color;
		}
	}
	return NULL;
}

/* This function does all the work. */
static uint32_t *pixbuf_create_from_xpm(const char *(*get_buf)(enum buf_op op, void *handle),
		void *handle, int *width, int *height)
{
	int w, h, n_col, cpp, x_hot, y_hot, items;
	int cnt, xcnt, ycnt, wbytes, n;
	int is_trans = 0;
	const char *buffer;
	char *name_buf;
	char pixel_str[32];
	struct xpm_color *colors, *color, *fallbackcolor;
	uint32_t *data = NULL;

	fallbackcolor = NULL;

	buffer = (*get_buf)(op_header, handle);
	if (!buffer) {
		return NULL;
	}
	items = sscanf(buffer, "%d %d %d %d %d %d", &w, &h, &n_col, &cpp, &x_hot, &y_hot);

	if (items != 4 && items != 6) {
		return NULL;
	}
	if (w <= 0) {
		return NULL;
	}
	if (h <= 0) {
		return NULL;
	}
	if (cpp <= 0 || cpp >= 32) {
		return NULL;
	}
	if (n_col <= 0 || n_col >= INT_MAX / (cpp + 1)
			|| n_col >= INT_MAX / (int)sizeof(struct xpm_color)) {
		return NULL;
	}

	name_buf = (char *)calloc(n_col, cpp + 1);
	if (!name_buf) {
		return NULL;
	}
	colors = (struct xpm_color *)calloc(n_col, sizeof(struct xpm_color));
	if (!colors) {
		free(name_buf);
		return NULL;
	}

	for (cnt = 0; cnt < n_col; cnt++) {
		char *color_name;

		buffer = (*get_buf)(op_cmap, handle);
		if (!buffer) {
			free(colors);
			free(name_buf);
			return NULL;
		}

		color = &colors[cnt];
		color->color_string = &name_buf[cnt * (cpp + 1)];
		strncpy(color->color_string, buffer, cpp);
		color->color_string[cpp] = 0;
		buffer += strlen(color->color_string);
		color->transparent = 0;

		color_name = xpm_extract_color(buffer);

		if (!color_name || (strcasecmp(color_name, "None") == 0) ||
				(parse_color(color_name, color) == 0)) {
			color->transparent = 1;
			color->red = 0;
			color->green = 0;
			color->blue = 0;
			is_trans = 1;
		}

		free(color_name);

		if (cnt == 0) {
			fallbackcolor = color;
		}
	}

	data = (uint32_t *)calloc(w * h, sizeof(uint32_t));
	if (!data) {
		free(colors);
		free(name_buf);
		return NULL;
	}

	wbytes = w * cpp;

	for (ycnt = 0; ycnt < h; ycnt++) {
		buffer = (*get_buf)(op_body, handle);
		if (!buffer || ((int)strlen(buffer) < wbytes)) {
			continue;
		}

		for (n = 0, xcnt = 0; n < wbytes; n += cpp, xcnt++) {
			uint32_t a, r, g, b;

			strncpy(pixel_str, &buffer[n], cpp);
			pixel_str[cpp] = 0;

			color = lookup_color(colors, n_col, pixel_str);

			/* Bad XPM...punt */
			if (!color) {
				color = fallbackcolor;
			}

			a = 0xFF;
			if (is_trans && color->transparent) {
				a = 0;
			}
			r = color->red >> 8;
			g = color->green >> 8;
			b = color->blue >> 8;
			data[ycnt * w + xcnt] = (a << 24) | (r << 16) | (g << 8) | b;
		}
	}

	free(colors);
	free(name_buf);

	*width = w;
	*height = h;
	return data;
}

void
img_xpm_load(const char *filename, struct lab_data_buffer **buffer)
{
	if (*buffer) {
		wlr_buffer_drop(&(*buffer)->base);
		*buffer = NULL;
	}
	if (string_null_or_empty(filename)) {
		return;
	}

	struct file_handle handle;
	int w, h;
	uint32_t *data;
	cairo_surface_t *surface;
	unsigned char *surface_data;

	memset(&h, 0, sizeof(h));
	handle.infile = fopen(filename, "r");
	if (!handle.infile) {
		fprintf(stderr, "Failed to load XPM file: %s\n", filename);
		return;
	}
	handle.buffer_size = 4096;
	handle.buffer = calloc(4096, 1);
	data = pixbuf_create_from_xpm(file_buffer, &handle, &w, &h);
	free(handle.buffer);
	fclose(handle.infile);
	if (!data) {
		fprintf(stderr, "Failed to load XPM file: %s\n", filename);
		return;
	}

	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
	if (!surface) {
		fprintf(stderr, "Failed to load XPM file: %s\n", filename);
		free(data);
		return;
	}
	surface_data = cairo_image_surface_get_data(surface);
	cairo_surface_flush(surface);
	memcpy(surface_data, data, w * h * 4);
	free(data);
	cairo_surface_mark_dirty(surface);

	*buffer = buffer_create_cairo((int)w, (int)h, 1.0, true);
	cairo_t *cairo = (*buffer)->cairo;
	cairo_set_source_surface(cairo, surface, 0, 0);
	cairo_paint_with_alpha(cairo, 1.0);
}
