#include "seqgui.h"
#include "atto/gl.h"
#include "atto/app.h"

static struct {
	GuiRect vp;
} g;

void guiSetViewport(GuiRect v) {
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

static const int tick_width = 10;
static const int note_height = 20;
static const float y_offset = 10;
static const float y_scale = 50;

void guiPaintAutomation(const Automation *a, float now_sec, GuiTransform transform) {
	(void)transform;

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

enum Mode {
	Mode_Idle,
	Mode_SelectNotes,
	Mode_SelectKeypoints,
	Mode_DragKeypoints,
	Mode_DragNotes,
	Mode_ResizeNote
};

#define MAX_SELECTION 128

static struct {
	enum Mode mode;
	struct {
		int pattern;
		int envelope;
		int selected[MAX_SELECTION];
	} selection;
} edit;

void guiEventPointer(Automation *a, GuiEventPointer ptr) {
	(void)(a);
	if (!ptr.button_mask) {
		edit.mode = Mode_Idle;
		return;
	}

	switch (edit.mode) {
	case Mode_Idle:
		if (ptr.xor_button_mask == AB_Left && ptr.button_mask == AB_Left) {
			// TODO
			// 1. find object under; -> (envelope, key) || (pattern, note)
			// 2. if not env|pattern then idle and return
			// 3. if env|pat but not key|note, then start new selection under env|pat and return
			// 4. if key|note not in selection, then replace selection with key|note
			// 5. if key|note in selection start drag selection
		}
		break;
	default:
		break;
	}

	if (ptr.xor_button_mask == 0 && ptr.button_mask == AB_Left) {
		// if mode == drag, drag
	}
}
