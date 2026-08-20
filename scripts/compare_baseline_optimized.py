import pandas as pd
from pathlib import Path


BASELINE_FILE = Path(
    "results/raw_timing/file_read_baseline.csv"
)

OPTIMIZED_FILE = Path(
    "results/raw_timing/file_read_optimized.csv"
)

OUTPUT_FILE = Path(
    "results/stats/baseline_vs_optimized.csv"
)


def calculate_summary(data):
    timing = data["wall_time_ms"]

    return {
        "count": len(timing),
        "mean_ms": timing.mean(),
        "median_ms": timing.median(),
        "min_ms": timing.min(),
        "max_ms": timing.max(),
        "std_ms": timing.std(),
    }


def main():
    if not BASELINE_FILE.exists():
        print(f"Error: baseline file not found: {BASELINE_FILE}")
        return

    if not OPTIMIZED_FILE.exists():
        print(f"Error: optimized file not found: {OPTIMIZED_FILE}")
        return

    baseline = pd.read_csv(BASELINE_FILE)
    optimized = pd.read_csv(OPTIMIZED_FILE)

    #using largest baseline buffer as primary control
    baseline = baseline[
        baseline["method"] == "read() 1024KB buffer"
    ].copy()

    optimized = optimized[
        optimized["method"] == "mmap()"
    ].copy()

    if baseline.empty:
        print("Error: no 1024 KB baseline measurements found.")
        return

    if optimized.empty:
        print("Error: no mmap() measurements found.")
        return

    baseline_summary = calculate_summary(baseline)
    optimized_summary = calculate_summary(optimized)

    baseline_mean = baseline_summary["mean_ms"]
    optimized_mean = optimized_summary["mean_ms"]

    mean_difference = baseline_mean - optimized_mean

    speedup_percent = (
        mean_difference / baseline_mean
    ) * 100.0

    comparison = pd.DataFrame([
        {
            "method": "read() 1 MB",
            **baseline_summary,
        },
        {
            "method": "mmap()",
            **optimized_summary,
        },
    ])

    comparison["mean_difference_from_baseline_ms"] = [
        0.0,
        mean_difference,
    ]

    comparison["speedup_percent"] = [
        0.0,
        speedup_percent,
    ]

    OUTPUT_FILE.parent.mkdir(
        parents=True,
        exist_ok=True,
    )

    comparison.to_csv(
        OUTPUT_FILE,
        index=False,
    )

    print("Baseline vs. optimized comparison complete.")
    print(f"Baseline:  {BASELINE_FILE}")
    print(f"Optimized: {OPTIMIZED_FILE}")
    print(f"Output:    {OUTPUT_FILE}")
    print()
    print(comparison.to_string(index=False))

    print()
    if speedup_percent > 0:
        print(
            f"mmap() mean time is "
            f"{speedup_percent:.2f}% lower than the "
            f"1 MB read() baseline."
        )
    elif speedup_percent < 0:
        print(
            f"mmap() mean time is "
            f"{abs(speedup_percent):.2f}% higher than the "
            f"1 MB read() baseline."
        )
    else:
        print(
            "mmap() and the 1 MB read() baseline "
            "have the same mean time."
        )


if __name__ == "__main__":
    main()
