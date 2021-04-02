/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2019 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Mutex implementation.
 */
#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <omvhaldef.h>

#define MUTEX_TID_IDE (1<<0)
#define MUTEX_TID_OMV (1<<1)

typedef omvhal_mutex_t mutex_t;

void mutex_init(mutex_t *mutex);
void mutex_lock(mutex_t *mutex, uint32_t tid);
int mutex_try_lock(mutex_t *mutex, uint32_t tid);
void mutex_unlock(mutex_t *mutex, uint32_t tid);
#endif /* __MUTEX_H__ */
