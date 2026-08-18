import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path

INPUT_FILE = Path("results/matrices/read_performance_matrix.csv")
FIGURE_DIR = Path("report/figures")

BUFFER_SIZES = [1, 4, 16, 64, 256, 1024]
BUFFER_LABELS = ["1","4","16","64","256","1024"]


def main():
    if not INPUT_FILE.exists():
        print(f"Error: input file not found: {INPUT_FILE}")
        return

    data = pd.read_csv(INPUT_FILE)
    FIGURE_DIR.mkdir(parents=True, exist_ok=True)

    # mean read time
    plt.figure(figsize=(9,6))
    plt.plot(
        data["buffer_kb"],
        data["mean_ms"],
        marker="o",
        linewidth=2,
        markersize=7,
    )

    plt.xscale("log", base=2)
    plt.xticks(BUFFER_SIZES, BUFFER_LABELS)
    plt.xlabel("Buffer size (KB)")
    plt.ylabel("Mean read time (ms)")
    plt.title("Mean File Read Time vs. Buffer Size")
    plt.grid(True, alpha=0.3)
    plt.tight_layout()

    plt.savefig(
        FIGURE_DIR / "buffer_vs_time.png",
        dpi=300,
        bbox_inches="tight",
    )
    plt.close()

    # read() system call count
    plt.figure(figsize=(9,6))
    plt.plot(
        data["buffer_kb"],
        data["read_calls"],
        marker="o",
        linewidth=2,
        markersize=7,
    )

    plt.xscale("log", base=2)
    plt.xticks(BUFFER_SIZES, BUFFER_LABELS)
    plt.xlabel("Buffer size (KB)")
    plt.ylabel("Observed read() system calls")
    plt.title("Observed read() System Calls vs. Buffer Size")
    plt.grid(True, alpha=0.3)
    plt.tight_layout()

    plt.savefig(
        FIGURE_DIR/"buffer_vs_syscalls.png",
        dpi=300,
        bbox_inches="tight",
    )
    plt.close()

    print("Presentation-ready figures created:")
    print(FIGURE_DIR/"buffer_vs_time.png")
    print(FIGURE_DIR/"buffer_vs_syscalls.png")


if __name__ == "__main__":
    main()
