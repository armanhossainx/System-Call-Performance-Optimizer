import pandas as pd
from pathlib import Path

INPUT_FILE = Path("results/raw_timing/file_read_baseline.csv")
OUTPUT_FILE = Path("results/stats/timing_summary.csv")


def main():
    if not INPUT_FILE.exists():
        print(f"Error: input file not found: {INPUT_FILE}")
        return

    df = pd.read_csv(INPUT_FILE)

    summary = (
        df.groupby("method")["wall_time_ms"]
        .agg(
            count="count",
            mean_ms="mean",
            median_ms="median",
            min_ms="min",
            max_ms="max",
            std_ms="std",
        )
        .reset_index()
    )

    summary["buffer_kb"] = (
        summary["method"]
        .str.extract(r"(\d+)KB")[0]
        .astype(int)
    )

    summary = summary.sort_values("buffer_kb")

    OUTPUT_FILE.parent.mkdir(parents=True, exist_ok=True)
    summary.to_csv(OUTPUT_FILE, index=False)

    print("Timing analysis complete.")
    print(f"Input:  {INPUT_FILE}")
    print(f"Output: {OUTPUT_FILE}")
    print()
    print(summary.to_string(index=False))


if __name__ == "__main__":
    main()
