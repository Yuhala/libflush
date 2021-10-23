# libflush

Cache flushing library which provides wrapper functions for priming and flushing cache on x86 and arm-v8 architectures.

It implements the cacheline flush instruction on the respective architectures and the fence instruction to serialize loads and stores.

The library provides a useful testbed for launching Cache Side-Channel attacks.


## Building

### x86-64

#### Building libflush

1. For docker / podman
    ```console
    docker|podman build -t sc-x86 -f Containerfile.x86 .
    ```

2. For nix
   ```console
   nix-shell contrib/shell.nix
   meson build -Dplatform=x86
   meson test -C build
   ```

### arm-v8

#### Building libflush

1. For docker / podman
    ```console
    docker|podman build -t sc-arm -f Containerfile.arm .
    ```

2. For nix
    ```console
    nix-shell contrib/shell.nix
    meson build -Dplatform=arm
    ```

## Unit tests

To run unit tests, do
```console
meson test -C build
```

## Timing and Measurements

libflush provides 3 API's for timing and measurements

1. Monotonic clock (Default)

2. Perf

    To use perf, build with timer=perf
    ```console
    meson build -Dplatform=<arch> -Dtimer=perf
    ```

3. Thread Counter

    To use thread counter, build with timer=thread-counter
    ```console
    meson build -Dplatform=<arch> -Dtimer=thread-counter
    ```

## Examples

To build examples and customise, use the following options

```console
meson build -dplatform=<arch> -dtimer=<timer> -dexamples=true
```

This builds libflush with example implementations for creating a session, evict-time and prime-probe.
To run the samples,

```console
./build/sample-session
./build/evict-time
./build/prime-probe
```

## References

[1] [ARMageddon: Cache Attacks on Mobile Devices - Lipp, Gruss, Spreitzer, Maurice, Mangard](https://www.usenix.org/conference/usenixsecurity16/technical-sessions/presentation/lipp)
