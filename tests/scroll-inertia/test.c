/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include <test_shim.h>
#include "../../src/behavior_scroll_inertia.c"

struct device test_pad;
unsigned int error_logs;
static bool state, scheduled, in_worker, press_during_get, press_during_set;
static int deadline, get_error, set_error;
static unsigned int get_calls, set_calls, successes;

int k_work_schedule(struct k_work_delayable *work, k_timeout_t delay) {
    assert(work == &data.work);
    if (scheduled) { return 0; }
    scheduled = true;
    deadline = delay;
    return 1;
}

static void press(void) {
    struct zmk_behavior_binding_event event = {0};
    assert(pressed(NULL, event) == 0);
}

int iqs9151_scroll_inertia_get_enabled(const struct device *dev, bool *enabled) {
    assert(dev == &test_pad && in_worker);
    assert(!data.lock.held);
    get_calls++;
    if (press_during_get) { press_during_get = false; press(); }
    if (get_error) { return get_error; } /* Deliberately leave output untouched. */
    *enabled = state;
    return 0;
}

int iqs9151_scroll_inertia_set_enabled(const struct device *dev, bool enabled) {
    assert(dev == &test_pad && in_worker);
    assert(!data.lock.held);
    set_calls++;
    if (press_during_set) { press_during_set = false; press(); }
    if (set_error) { return set_error; }
    assert(enabled != state);
    state = enabled;
    successes++;
    return 0;
}

static void reset(void) {
    memset(&data, 0, sizeof(data));
    state = test_pad.ready = true;
    scheduled = in_worker = press_during_get = press_during_set = false;
    get_error = set_error = deadline = 0;
    error_logs = get_calls = set_calls = successes = 0;
    init(NULL);
}

static void step(void) {
    assert(scheduled && data.pending > 0);
    scheduled = false;
    in_worker = true;
    data.work.handler(&data.work.work);
    in_worker = false;
}

static void drain(void) {
    unsigned int budget = 10000;
    while (scheduled) { assert(budget--); step(); }
    assert(data.pending == 0);
}

int main(void) {
    struct zmk_behavior_binding_event event = {0};
    reset();
    assert(state);
    press();
    assert(get_calls == 0); /* No driver API on the calling/input thread. */
    released(NULL, event);
    drain();
    assert(!state && successes == 1);
    released(NULL, event);
    assert(!scheduled);
    press(); drain();
    assert(state && successes == 2);

    reset();
    for (unsigned int i = 0; i < MAX_PENDING; i++) { press(); }
    assert(pressed(NULL, event) == -ENOSPC);
    drain();
    assert(state && successes == MAX_PENDING && error_logs == 1);

    reset();
    press();
    get_error = -EBUSY;
    step();
    assert(scheduled && deadline == RETRY_MS && set_calls == 0 && data.pending == 1);
    press(); /* New press must not overwrite the existing retry deadline. */
    assert(deadline == RETRY_MS && data.pending == 2);
    get_error = 0;
    set_error = -EBUSY;
    step();
    assert(state && successes == 0 && data.pending == 2);
    set_error = 0;
    drain();
    assert(state && successes == 2 && get_calls == 4 && set_calls == 3);

    reset();
    press(); press();
    get_error = -EBUSY;
    for (unsigned int i = 0; i <= MAX_BUSY_RETRIES; i++) { step(); }
    assert(get_calls == MAX_BUSY_RETRIES + 1 && data.pending == 1 && error_logs == 1);
    assert(deadline == 0); /* Failed head removed; following request can run. */
    get_error = 0;
    drain();
    assert(!state && successes == 1);

    reset();
    press();
    set_error = -EBUSY;
    for (unsigned int i = 0; i <= MAX_BUSY_RETRIES; i++) { step(); }
    assert(state && successes == 0 && !scheduled && error_logs == 1);

    reset();
    press(); press();
    get_error = -EINVAL;
    step();
    assert(set_calls == 0 && data.pending == 1 && error_logs == 1);
    get_error = 0;
    set_error = -EIO;
    drain();
    assert(state && successes == 0 && error_logs == 2);

    reset();
    test_pad.ready = false;
    assert(pressed(NULL, event) == -ENODEV && !scheduled);
    test_pad.ready = true;
    press();
    test_pad.ready = false;
    drain();
    assert(get_calls == 0 && error_logs == 2);

    reset();
    press();
    press_during_get = press_during_set = true;
    drain();
    assert(!state && successes == 3); /* Arrivals while the head is in flight. */

    reset();
    state = false; /* Driver state, not behavior cache, is authoritative. */
    press(); drain();
    assert(state && successes == 1);
    assert(api.locality == BEHAVIOR_LOCALITY_CENTRAL);
    puts("PASS: press/release, burst/overflow, busy get/set, retry limits, errors, arrivals, driver state");
    return 0;
}
