#ifndef __KEY_H__
#define __KEY_H__

#include "gpio.h"

typedef void (* key_function)(void);

struct single_key_param {
    uint8 key_level;
    uint8 gpio_id;
    uint8 gpio_func;
    uint32 gpio_name;
    os_timer_t key_long;
    os_timer_t key_short;
    key_function short_press;
    key_function long_press;
    uint32 shortDelay;
    uint32 longDelay;
};

struct keys_param {
    uint8 key_num;
    struct single_key_param **single_key;
};

struct single_key_param *key_init_single(uint8 gpio_id, uint32 gpio_name, uint8 gpio_func, key_function long_press, key_function short_press, uint32 shortDelay, uint32 longDelay);
void key_init(struct keys_param *key);

#endif
