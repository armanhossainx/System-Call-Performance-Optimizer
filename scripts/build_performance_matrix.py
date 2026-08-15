import pandas as pd
from pathlib import Path

TIMING_FILE = Path("results/stats/timing_summary.csv")
SYSCALL_FILE = Path("results/stats/syscall_summary.csv")
OUTPUT_FILE = Path("results/matrices/read_performance_matrix.csv")


def main():
    if not TIMING_FILE.exists():
        print(f"Error: missing {TIMING_FILE}")
        return

    if not SYSCALL_FILE.exists():
        print(f"Error: missing {SYSCALL_FILE}")
        return

    timing = pd.read_csv(TIMING_FILE)
    syscalls = pd.read_csv(SYSCALL_FILE)

    matrix = timing.merge(
        syscalls,
        on="buffer_kb",
        how="inner"
    )

    columns = [
        "buffer_kb",
        "count",
        "mean_ms",
        "median_ms",
        "min_ms",
        "max_ms",
        "std_ms",
        "read_calls",
    ]

    matrix = matrix[columns]
    matrix = matrix.sort_values("buffer_kb")

    OUTPUT_FILE.parent.mkdir(parents=True, exist_ok=True)
    matrix.to_csv(OUTPUT_FILE, index=False)

    print("Performance matrix created.")
    print(f"Output: {OUTPUT_FILE}")
    print()
    print(matrix.to_string(index=False))


if __name__ == "__main__":
    main()
