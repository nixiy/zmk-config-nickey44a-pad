# Scroll inertia behavior host test

This test includes the production `src/behavior_scroll_inertia.c` directly.
Zephyr primitives, the scheduler, device readiness and the driver API are mocked.
No separate copy of the request-processing implementation is tested.

From the repository root, using a host C compiler:

```sh
cc -std=c11 -Wall -Wextra -Werror -Wno-unused-const-variable \
  -Itests/scroll-inertia/include tests/scroll-inertia/test.c -o /tmp/inertia-test
/tmp/inertia-test
```

On Windows `zig cc` can replace `cc`; run the resulting `.exe`.
The compiler and temporary binaries are not firmware dependencies.

Coverage: driver calls deferred until work execution; press/release; repeated
presses and capacity rejection; get/set busy retries; preserved retry deadline
when new presses arrive; get/set retry exhaustion; non-busy failures; not-ready
devices; new presses injected during get/set; reading the driver's actual state.

This deterministic test exercises request accounting and error handling. It does
not execute Zephyr's real concurrent scheduler, radio transport, sensor, or HID.
Those require firmware builds and hardware validation described in
`docs/runtime-scroll-inertia.md`.
