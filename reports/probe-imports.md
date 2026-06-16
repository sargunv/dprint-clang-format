# Probe Wasm Import Report

- Module: `build/probe.wasm`
- Generated: `2026-06-16T09:36:19Z`
- Import count: `0`

## Import Section

No imports.

## Full `wasm-objdump -x`

```

probe.wasm:	file format wasm 0x1

Section Details:

Type[2]:
 - type[0] () -> i32
 - type[1] (i32, i32, i32, i32) -> i32
Function[2]:
 - func[0] sig=0 <probe_version>
 - func[1] sig=1 <format_probe>
Memory[1]:
 - memory[0] pages: initial=1
Global[1]:
 - global[0] i32 mutable=1 - init i32=65536
Export[3]:
 - memory[0] -> "memory"
 - func[0] <probe_version> -> "probe_version"
 - func[1] <format_probe> -> "format_probe"
Code[2]:
 - func[0] size=4 <probe_version>
 - func[1] size=98 <format_probe>
```
