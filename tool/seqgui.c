#include "seqgui.h"
#include "atto/gl.h"

static struct {
	GuiViewport vp;
} g;

void guiSetViewport(GuiViewport v) {
	g.vp = v;
	glScalef(1.f / v.w, 1.f / v.h, 1);
	glTranslatef(-v.w, -v.h, 0);
}

void guiRect(int ax, int ay, int bx, int by, GuiColor c) {
	glColor4ub(c.r, c.g, c.b, c.a);
	glRecti(ax, ay, bx, by);
}

void guiLine(int ax, int ay, int bx, int by, GuiColor c) {
	glColor4ub(c.r, c.g, c.b, c.a);
	glBegin(GL_LINES);
	glVertex2i(ax, ay);
	glVertex2i(bx, by);
	glEnd();
}

void guiPaintAutomation(const Automation *a, float now_sec, GuiTransform transform) {
	(void)transform;

	const int tick_width = 10;
	const int note_height = 20;
	const float y_offset = 10;
	const float y_scale = 50;

	for (int i = 0; i < SCORE_PATTERNS; ++i) {
		const GuiColor color = {
			(unsigned char)(127 * (1 + (i/2)%2)),
			(unsigned char)(127 * (i%2)),
			0,
			255
		};
		const Pattern *p = a->patterns + i;
		int note_num = -1, note_start = -1;
		for (int j = 0; j < PATTERN_TICKS; ++j) {
			const Note *n = p->notes + j;
			if (n->event) {
				if (n->off && note_start >= 0) {
					guiRect(
						note_start * tick_width, note_num * note_height,
						j * tick_width, (note_num + 1) * note_height,
						color);
					note_start = -1;
					note_num = -1;
				}
				if (n->num != note_num) {
					/* FIXME draw if note_start >= 0 */
					note_start = j;
					note_num = n->num;
				}
			}
		}
	} // for all patterns

	for (int i = 0; i < SCORE_ENVELOPES; ++i) {
		GuiColor color = {
			(unsigned char)(127 * (1 + (i/2)%2)),
			(unsigned char)(127 * (i%2)),
			0,
			255
		};
		const Envelope *e = a->env + i;
		int xp = -1;
		for (int j = 0; j < MAX_ENVELOPE_POINTS; ++j) {
			const Point *p = e->points + j;
			if (p->time < 0)
				break;

			const int point_size = 10;
			const int x = p->time * tick_width;

			for (int k = 0; k < MAX_POINT_VALUES; ++k) {
				const int y = (p->v[k] + y_offset) * y_scale;
				color.b = 255 * k / MAX_POINT_VALUES;
				guiRect(x - point_size, y - point_size,
					x + point_size, y + point_size, color);
				if (j > 0) {
					const int yp = (p[-1].v[k] + y_offset) * y_scale;
					guiLine(xp, yp, x, y, color);
				}
			}
			xp = x;
		}
	}

	const int cursor_x = tick_width * now_sec / a->seconds_per_tick;
	const GuiColor cursor_color = {255, 255, 255, 255};
	guiLine(cursor_x, 0, cursor_x, g.vp.h * 2, cursor_color);
}

#if 0

static void sg_paintEnvelope(const Envelope *e, viewport) {
	rect(env aabb, alpha = .1);
	for (all points) {
		line(prev_point, point);
		rect(point, 10);
	}
}

static void sg_paintPattern(const Pattern *p, viewport) {
	rect(p aabb, alpha = .1);
	for (all notes) {
		calc note rect
		rect(note_rect);
	}
}

void seqguiPaint(const Automation *a, viewport) {
	for (all mapped envelopes) {
		calc env rect
		draw envelope in viewport
	}
	for (all mapped patterns) {
		calc pattern rect
		draw pattern
	}
}

static struct {
	struct {
		int env, keypoint, submask;
		int pat, note;
	} selection;
} g;

void seqguiPointer(Automation *a, event, viewport) {
	if (selection)
		move selection by event dx dy

	for (all mapped envelopes) {
		calc env rect
		if (event in rect) {
			for (all env keypoint) {
				event in keypoint rect
			}
		}
	}
	for (all mapped patterns) {
		calc pattern rect
		if (event in rect) {
		}
	}
}
#endif
