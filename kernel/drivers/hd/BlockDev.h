#pragma once
#include "driver/buffer/Request.h"
#define NR_REQUEST	32
/* harddisk */
#define DEVICE_NAME "harddisk"
#define DEVICE_INTR do_hd
#define DEVICE_TIMEOUT hd_timeout
#define DEVICE_REQUEST do_hd_request
#define DEVICE_NR(device) (MINOR(device)/5)
#define DEVICE_ON(device)
#define DEVICE_OFF(device)

#define CURRENT (request->GetCurrentReq())
#define CURRENT_DEV (CURRENT->dev)

#ifdef DEVICE_TIMEOUT
#define SET_INTR(x) (DEVICE_INTR = (x),DEVICE_TIMEOUT = 200)
#else
#define SET_INTR(x) (DEVICE_INTR = (x))
#endif

#ifdef DEVICE_TIMEOUT
#define CLEAR_DEVICE_TIMEOUT DEVICE_TIMEOUT = 0;
#else
#define CLEAR_DEVICE_TIMEOUT
#endif

#ifdef DEVICE_INTR
#define CLEAR_DEVICE_INTR DEVICE_INTR = 0;
#else
#define CLEAR_DEVICE_INTR
#endif

#define INIT_REQUEST \
repeat: \
	if (!CURRENT) {\
		CLEAR_DEVICE_INTR \
		CLEAR_DEVICE_TIMEOUT \
		return; \
	} \
	if (CURRENT->bh) { \
		if (!CURRENT->bh->b_lock) \
			panic(DEVICE_NAME ": block not locked"); \
	}

