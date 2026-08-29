<div align="center">

# ⚙️ System Call Performance Optimizer ![Visitors](https://views-counter.vercel.app/badge?pageId=https%3A%2F%2Fgithub%2Ecom%2Farmanhossainx%2FSystem-Call-Performance-Optimizer&leftColor=808080&rightColor=3469e5&type=total&label=Visitors&style=none)

### Measuring Linux file I/O. Optimizing the access path. Proving the result.

<br>

**`read()` → `mmap()`**

<br>

[![Language](https://img.shields.io/badge/Language-C-00599C?style=for-the-badge&logo=c&logoColor=white)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Compiler](https://img.shields.io/badge/GCC-15.2-5C2D91?style=for-the-badge&logo=gnu&logoColor=white)](https://gcc.gnu.org/)
[![Python](https://img.shields.io/badge/Python-3.14-3776AB?style=for-the-badge&logo=python&logoColor=white)](https://www.python.org/)
[![Platform](https://img.shields.io/badge/Platform-Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)](https://www.linux.org/)
[![Tracing](https://img.shields.io/badge/Tracing-strace-222222?style=for-the-badge)](https://strace.io/)

<br><br>

<p>
<a href="#demo-video"><img src="https://img.shields.io/badge/🎬Demo_Video-1a1a2e?style=for-the-badge"></a>
<a href="#quick-start"><img src="https://img.shields.io/badge/⚡Quick_Start-1a1a2e?style=for-the-badge"></a>
<a href="#methodology"><img src="https://img.shields.io/badge/🔬Methodology-1a1a2e?style=for-the-badge"></a>
<a href="#results"><img src="https://img.shields.io/badge/📊Results-1a1a2e?style=for-the-badge"></a>
</p>

<p>
<a href="#implementation"><img src="https://img.shields.io/badge/⚙️Implementation-1a1a2e?style=for-the-badge"></a>
<a href="#project-structure"><img src="https://img.shields.io/badge/📁Structure-1a1a2e?style=for-the-badge"></a>
</p>

</div>

---

<a id="demo-video"></a>
## 🎬 Demo Video

<div align="center">

<a href="https://www.youtube.com/watch?v=lZCsafPlsx8">
  <img src="https://img.youtube.com/vi/lZCsafPlsx8/maxresdefault.jpg"
       alt="System Call Performance Optimizer Demo"
       width="800">
</a>
<br>

*A short walkthrough of the benchmark methodology, implementation, and results.*
</div>

---

<a id="methodology"></a>
## 🔬 Methodology

```text
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                                    ┃
┃  WORKLOAD                                          ┃
┃  └─ 10 MiB binary file                             ┃
┃                                                    ┃
┃  BASELINE                                          ┃
┃  └─ read() with six buffer sizes                   ┃
┃                                                    ┃
┃  MEASUREMENT                                       ┃
┃  └─ 3 warm-up + 20 recorded iterations             ┃
┃                                                    ┃
┃  OPTIMIZATION                                      ┃
┃  └─ mmap()-based file access                       ┃
┃                                                    ┃
┃  EVALUATION                                        ┃
┃  └─ Timing + CPU time + page faults + strace       ┃
┃                                                    ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
```

### Experimental Flow

```text
10 MiB workload
      │
      ▼
┌───────────────┐
│ read()        │
│ baseline      │
└───────┬───────┘
        │
        ▼
  1 KB → 1 MB
  buffer study
        │
        ▼
   1 MB baseline
        │
        ▼
┌───────────────┐
│ mmap()        │
│ optimization  │
└───────┬───────┘
        │
        ▼
 Performance + syscall
       analysis
```

<a id="results"></a>
## 📊 Results

The optimization produced a measurable improvement.

| Metric | read() 1 MB | mmap() | Improvement |
|---|---|---|---|
| Mean execution time | 3.062 ms | 2.301 ms | 24.85% lower |
| Median execution time | 2.904 ms | 2.255 ms | 22.33% lower |
| Standard deviation | 0.497 ms | 0.187 ms | 62.36% lower |
| Explicit read() calls | 12 | 2 | 83.33% fewer |
| Measurement runs | 20 | 20 | Same sample count |

> **Key result:** Under the conditions of this experiment, `mmap()` reduced mean wall-clock execution time by **24.85%** compared with the 1 MB `read()` baseline.

<div align="center">

<table>
<tr>
<td width="50%" align="center">

**01 / Baseline Performance**
<br>
<img src="report/figures/buffer_vs_time.png" alt="Buffer size versus execution time" width="420">
<br>
<sub>Increasing buffer size reduced execution time; 1 MB was selected as baseline.</sub>

</td>
<td width="50%" align="center">

**02 / System-Call Behavior**
<br>
<img src="report/figures/buffer_vs_syscalls.png" alt="Buffer size versus read system calls" width="420">
<br>
<sub>Larger buffers required fewer explicit read() calls (12 at 1 MB).</sub>

</td>
</tr>
<tr>
<td width="50%" align="center">

**03 / Baseline vs. Optimized**
<br>
<img src="report/figures/baseline_vs_optimized.png" alt="Baseline versus optimized performance" width="420">
<br>
<sub>mmap() cut mean execution time from 3.062 ms to 2.301 ms.</sub>

</td>
<td width="50%" align="center">

**04 / System-Call Comparison**
<br>
<img src="report/figures/syscall_comparison.png" alt="System-call comparison" width="420">
<br>
<sub>read() calls dropped from 12 to 2 — an 83.33% reduction.</sub>

</td>
</tr>
</table>

</div>

<br>

<div align="center">

```text
┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
┃                                                    ┃
┃              FINAL OBSERVATION                     ┃
┃                                                    ┃
┃           3.062 ms  →  2.301 ms                    ┃
┃                                                    ┃
┃                  ↓ 24.85%                          ┃
┃                                                    ┃
┃         mean execution time reduction              ┃
┃                                                    ┃
┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
```

</div>

<a id="implementation"></a>
## ⚙️ Implementation

### Baseline — `read()`

The baseline implementation processes the 10 MiB test file through repeated `read()` system calls using a configurable user-space buffer.

```text
File
 │
 ▼
┌──────────────┐
│    read()    │
└──────┬───────┘
       ▼
    Buffer
       │
       ▼
   Process data
       │
       └──────→ read() → Buffer → ...
```

Six buffer sizes were evaluated:

**1 KB → 4 KB → 16 KB → 64 KB → 256 KB → 1 MB**

The 1 MB configuration was selected as the final baseline.

### Optimized — `mmap()`

The optimized implementation maps the complete test file into the process's address space.

```text
File
 │
 ▼
┌──────────────┐
│    mmap()    │
└──────┬───────┘
       ▼
Memory-mapped region
       │
       ▼
Access file contents
       │
       ▼
┌──────────────┐
│   munmap()   │
└──────────────┘
```

The mapped region is accessed across the complete 10 MiB workload before the mapping is released.

<a id="quick-start"></a>
## ⚡ Quick Start

### Build

```bash
gcc -Wall -Wextra -O2 -std=c99 -D_GNU_SOURCE \
    src/file_read_baseline.c src/benchmark_utils.c \
    -o file_read_baseline -lm

gcc -Wall -Wextra -O2 -std=c99 -D_GNU_SOURCE \
    src/file_read_optimized.c src/benchmark_utils.c \
    -o file_read_optimized -lm
```

### Run

```bash
./file_read_baseline
./file_read_optimized
```

### Analyze

```bash
python3 scripts/compare_baseline_optimized.py
python3 scripts/compare_syscalls.py
```

<a id="project-structure"></a>
## 📁 Project Structure

```text
system-call-optimizer/
├── src/          # C implementations and benchmark utilities
├── scripts/      # Analysis and visualization
├── test_files/   # 10 MiB benchmark workload
├── results/      # Experimental data and statistics
├── report/       # Generated performance figures
└── README.md
```

`src` builds it. `scripts` measures it. `results` records it. `report` presents it.
