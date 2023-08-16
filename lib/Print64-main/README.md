# Print64

*Library for printing 64-bit integers on Particle devices*

The standard C library on Particle devices (at least as of Device OS 3.0) does not support 64-bit sprintf formatting such as `%llu` or `%llx` or Microsoft extensions like `%I64u`. This includes not only `sprintf()`, and its variations like `snprintf()`, but also things that use it, like `Log.info()`.

## API

### toString (uint64_t)

Convert an unsigned 64-bit integer to a string

```cpp
String toString(uint64_t value, unsigned char base = 10);
```

- `value` The value to convert (uint64_t)
- `base` The number base. Acceptable values are 2, 8, 10, and 16. Default is 10 (decimal).
- Returns A String object containing an ASCII representation of the value.


### toString (int64_t)

Convert an signed 64-bit integer to a string (ASCII decimal signed integer)

```cpp
String toString(int64_t value);
```

- `value` The value to convert (int64_t)
- Returns A String object containing an ASCII representation of the value (decimal)
 

## Example

```
#include "Particle.h"

#include "Print64.h"

SYSTEM_THREAD(ENABLED);
SerialLogHandler logHandler;

void setup() {

}

void loop() {
    Log.info("millis=%s", toString(System.millis()).c_str());
    delay(1000);
}
```

The `System.millis()` function returns a `uint64_t`. It works like `millis()` but since it's 64-bit, it doesn't wrap around to 0 after 49 days so it's easier to do comparisons with it.

Note that you must add a `.c_str()` to the result of `toString()` when passing it as variable arguments to `sprintf()`, `Log.info()`, etc.!

