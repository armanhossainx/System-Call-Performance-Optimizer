import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path

INPUT_FILE = Path("results/matrices/read_performance_matrix.csv")
FIGURE_DIR = Path("report/figures")


def main():
    if not INPUT_FILE.exists():
        print(f"Error: input file not found: {INPUT_FILE}")
        return

    data = pd.read_csv(INPUT_FILE)
    FIGURE_DIR.mkdir(parents=True, exist_ok=True)

    #figure 1: buffer size vs. mean read time
    plt.figure()
    plt.plot(data["buffer_kb"], data["mean_ms"], marker="o")
    plt.xscale("log", base=2)
    plt.xlabel("Buffer size (KB)")
    plt.ylabel("Mean read time (ms)")
    plt.title("File Read Performance vs. Buffer Size")
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(FIGURE_DIR / "buffer_vs_time.png", dpi=200)
    plt.close()

    #figure 2: buffer size vs. read syscall count
    plt.figure()
    plt.plot(data["buffer_kb"], data["read_calls"], marker="o")
    plt.xscale("log", base=2)
    plt.xlabel("Buffer size (KB)")
    plt.ylabel("read() system calls")
    plt.title("read() System Calls vs. Buffer Size")
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(FIGURE_DIR / "buffer_vs_syscalls.png", dpi=200)
    plt.close()

    print("Figures created:")
    print(FIGURE_DIR / "buffer_vs_time.png")
    print(FIGURE_DIR / "buffer_vs_syscalls.png")


if __name__ == "__main__":
    main()
