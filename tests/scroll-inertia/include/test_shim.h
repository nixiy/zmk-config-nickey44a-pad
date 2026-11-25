/* Minimal host substitutes; the test includes the production behavior C file. */
#pragma once
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <errno.h>

#define DT_HAS_COMPAT_STATUS_OKAY(x) 1
#define DT_NUM_INST_STATUS_OKAY(x) 1
#define DT_INST_PHANDLE(i, p) 0
#define DT_NODE_HAS_COMPAT(n, c) 1
#define IS_ENABLED(x) x
#define CONFIG_ZMK_SPLIT 1
#define CONFIG_ZMK_SPLIT_ROLE_CENTRAL 1
#define CONFIG_ZMK_BEHAVIOR_METADATA 0
#define CONFIG_INPUT_INIT_PRIORITY 90
#define CONFIG_INPUT_IQS9151_INIT_PRIORITY 80
#define BUILD_ASSERT(c, msg) _Static_assert(c, msg)
#define ARG_UNUSED(x) (void)(x)
#define LOG_MODULE_DECLARE(...)
extern unsigned int error_logs;
static inline void test_log(const char *fmt, ...) { (void)fmt; error_logs++; }
#define LOG_ERR(...) test_log(__VA_ARGS__)

struct device { bool ready; };
extern struct device test_pad;
#define DEVICE_DT_GET(n) (&test_pad)
static inline bool device_is_ready(const struct device *dev) { return dev->ready; }

typedef int k_timeout_t;
#define K_NO_WAIT 0
#define K_MSEC(ms) (ms)
struct k_work { int unused; };
struct k_work_delayable {
    struct k_work work;
    void (*handler)(struct k_work *);
};
struct k_spinlock { bool held; };
typedef int k_spinlock_key_t;
static inline k_spinlock_key_t k_spin_lock(struct k_spinlock *lock) {
    assert(!lock->held);
    lock->held = true;
    return 0;
}
static inline void k_spin_unlock(struct k_spinlock *lock, k_spinlock_key_t key) {
    (void)key;
    assert(lock->held);
    lock->held = false;
}
static inline void k_work_init_delayable(struct k_work_delayable *work,
                                         void (*handler)(struct k_work *)) {
    work->handler = handler;
}
int k_work_schedule(struct k_work_delayable *work, k_timeout_t delay);

struct zmk_behavior_binding { int unused; };
struct zmk_behavior_binding_event { int source; };
#define ZMK_BEHAVIOR_OPAQUE 0
#define BEHAVIOR_LOCALITY_CENTRAL 0
struct behavior_driver_api {
    int (*binding_pressed)(struct zmk_behavior_binding *, struct zmk_behavior_binding_event);
    int (*binding_released)(struct zmk_behavior_binding *, struct zmk_behavior_binding_event);
    int locality;
};
#define BEHAVIOR_DT_INST_DEFINE(...)
int iqs9151_scroll_inertia_get_enabled(const struct device *, bool *);
int iqs9151_scroll_inertia_set_enabled(const struct device *, bool);
