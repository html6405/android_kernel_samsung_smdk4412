/* Compat shim for ported willow (General Mobile E-Tab 4) drivers.
 * Android early suspend was removed from this kernel; these drivers keep
 * compiling and simply never get early-suspend callbacks. */
#ifndef _LINUX_EARLYSUSPEND_H
#define _LINUX_EARLYSUSPEND_H

#include <linux/list.h>

enum {
	EARLY_SUSPEND_LEVEL_BLANK_SCREEN = 50,
	EARLY_SUSPEND_LEVEL_STOP_DRAWING = 100,
	EARLY_SUSPEND_LEVEL_DISABLE_FB = 150,
};

struct early_suspend {
	struct list_head link;
	int level;
	void (*suspend)(struct early_suspend *h);
	void (*resume)(struct early_suspend *h);
};

static inline void register_early_suspend(struct early_suspend *handler) { }
static inline void unregister_early_suspend(struct early_suspend *handler) { }

#endif
