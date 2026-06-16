# Probe Wasm Import Report

- Module: `build/stl-probe.wasm`
- Generated: `2026-06-16T09:36:28Z`
- Import count: `0`

## Import Section

No imports.

## Full `wasm-objdump -x`

```

stl-probe.wasm:	file format wasm 0x1

Section Details:

Type[7]:
 - type[0] () -> i32
 - type[1] (i32, i32) -> i32
 - type[2] (i32) -> i32
 - type[3] (i32) -> nil
 - type[4] (i32, i32, i32, i32) -> i32
 - type[5] (i32, i32) -> nil
 - type[6] (i32, i32, i32) -> nil
Function[23]:
 - func[0] sig=0 <stl_probe>
 - func[1] sig=1
 - func[2] sig=1
 - func[3] sig=2
 - func[4] sig=3
 - func[5] sig=3
 - func[6] sig=1
 - func[7] sig=1
 - func[8] sig=4
 - func[9] sig=5
 - func[10] sig=2
 - func[11] sig=1
 - func[12] sig=5
 - func[13] sig=2
 - func[14] sig=2 <malloc>
 - func[15] sig=3 <free>
 - func[16] sig=2
 - func[17] sig=5
 - func[18] sig=1
 - func[19] sig=6
 - func[20] sig=6
 - func[21] sig=2
 - func[22] sig=5
Table[1]:
 - table[0] type=funcref initial=1 max=1
Memory[1]:
 - memory[0] pages: initial=2
Global[1]:
 - global[0] i32 mutable=1 - init i32=65536
Export[4]:
 - memory[0] -> "memory"
 - func[0] <stl_probe> -> "stl_probe"
 - func[14] <malloc> -> "malloc"
 - func[15] <free> -> "free"
Code[23]:
 - func[0] size=177 <stl_probe>
 - func[1] size=22
 - func[2] size=138
 - func[3] size=52
 - func[4] size=45
 - func[5] size=32
 - func[6] size=137
 - func[7] size=61
 - func[8] size=91
 - func[9] size=82
 - func[10] size=59
 - func[11] size=28
 - func[12] size=54
 - func[13] size=39
 - func[14] size=46 <malloc>
 - func[15] size=2 <free>
 - func[16] size=46
 - func[17] size=2
 - func[18] size=120
 - func[19] size=42
 - func[20] size=42
 - func[21] size=67
 - func[22] size=55
Data[1]:
 - segment[0] memory=0 size=22 - init i32=65536
  - 0010000: 696e 7420 6d61 696e 2829 7b72 6574 7572  int main(){retur
  - 0010010: 6e20 313b 7d00                           n 1;}.
```
