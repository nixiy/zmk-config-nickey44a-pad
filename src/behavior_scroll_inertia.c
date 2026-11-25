/* SPDX-License-Identifier: MIT */
#define DT_DRV_COMPAT zmk_behavior_scroll_inertia

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <drivers/behavior.h>

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

#if IS_ENABLED(CONFIG_ZMK_SPLIT) && !IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
#error "scroll-inertia must be enabled only on the central with the IQS9151"
#endif

#include <errno.h>
#include <drivers/input/iqs9151.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

/* Reuse this one behavior on any number of keys. One worker serializes all
 * requests for the pad, including presses originating on the peripheral. */
BUILD_ASSERT(DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT) == 1,
             "Use a single scroll-inertia behavior instance for this keyboard");
BUILD_ASSERT(DT_NODE_HAS_COMPAT(DT_INST_PHANDLE(0, device), azoteq_iqs9151),
             "scroll-inertia device must be an IQS9151");
BUILD_ASSERT(CONFIG_INPUT_INIT_PRIORITY > CONFIG_INPUT_IQS9151_INIT_PRIORITY,
             "scroll-inertia must initialize after the IQS9151");

#define MAX_PENDING 32
#define MAX_BUSY_RETRIES 100
#define RETRY_MS 10

struct scroll_inertia_data {
    struct k_work_delayable work;
    struct k_spinlock lock;
    unsigned int pending; /* Includes the request currently being attempted. */
    unsigned int busy_retries; /* Owned only by the work handler. */
};

static struct scroll_inertia_data data;
static const struct device *const pad = DEVICE_DT_GET(DT_INST_PHANDLE(0, device));

static void schedule_request(k_timeout_t delay) {
    int ret = k_work_schedule(&data.work, delay);
    if (ret < 0) {
        k_spinlock_key_t key = k_spin_lock(&data.lock);
        unsigned int failed = data.pending;
        data.pending = 0;
        k_spin_unlock(&data.lock, key);
        LOG_ERR("Scroll inertia scheduling failed: %d; discarded %u requests", ret, failed);
    }
}

static void process_request(struct k_work *work) {
    ARG_UNUSED(work);
    bool enabled;
    int ret = -ENODEV;
    if (device_is_ready(pad)) {
        ret = iqs9151_scroll_inertia_get_enabled(pad, &enabled);
        if (ret == 0) {
            ret = iqs9151_scroll_inertia_set_enabled(pad, !enabled);
        }
    }

    /* Both API calls guarantee no state change on failure. Re-read on retry;
     * a successful set is completed exactly once and never retried. */
    if (ret == -EBUSY && data.busy_retries < MAX_BUSY_RETRIES) {
        data.busy_retries++;
        schedule_request(K_MSEC(RETRY_MS));
        return;
    }
    if (ret != 0) {
        LOG_ERR("Scroll inertia toggle failed: %d after %u busy retries; discarded one request",
                ret, data.busy_retries);
    }
    data.busy_retries = 0;

    k_spinlock_key_t key = k_spin_lock(&data.lock);
    data.pending--;
    bool more = data.pending != 0;
    k_spin_unlock(&data.lock, key);
    if (more) {
        schedule_request(K_NO_WAIT);
    }
}

static int pressed(struct zmk_behavior_binding *binding,
                   struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    ARG_UNUSED(event);
    if (!device_is_ready(pad)) {
        LOG_ERR("Scroll inertia device is not ready");
        return -ENODEV;
    }

    k_spinlock_key_t key = k_spin_lock(&data.lock);
    if (data.pending == MAX_PENDING) {
        k_spin_unlock(&data.lock, key);
        LOG_ERR("Scroll inertia queue full; rejected press");
        return -ENOSPC;
    }
    bool start = data.pending++ == 0;
    k_spin_unlock(&data.lock, key);
    if (start) {
        schedule_request(K_NO_WAIT);
    }
    return ZMK_BEHAVIOR_OPAQUE;
}

static int released(struct zmk_behavior_binding *binding,
                    struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    ARG_UNUSED(event);
    return ZMK_BEHAVIOR_OPAQUE;
}

static int init(const struct device *dev) {
    ARG_UNUSED(dev);
    k_work_init_delayable(&data.work, process_request);
    /* The behavior priority is later than the pad's initialization. */
    return 0;
}

static const struct behavior_driver_api api = {
    .binding_pressed = pressed,
    .binding_released = released,
    .locality = BEHAVIOR_LOCALITY_CENTRAL,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .get_parameter_metadata = zmk_behavior_get_empty_param_metadata,
#endif
};

BEHAVIOR_DT_INST_DEFINE(0, init, NULL, &data, NULL, POST_KERNEL,
                        CONFIG_INPUT_INIT_PRIORITY, &api);

#endif /* DT_HAS_COMPAT_STATUS_OKAY */
