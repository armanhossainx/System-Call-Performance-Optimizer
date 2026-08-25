import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path


INPUT_FILE = Path(
    "results/stats/syscall_comparison.csv"
)

OUTPUT_FILE = Path(
    "report/figures/syscall_comparison.png"
)


def main():
    df = pd.read_csv(INPUT_FILE)

    methods = df["method"].tolist()
    read_calls = df["read_calls"].tolist()

    baseline_reads = read_calls[0]
    optimized_reads = read_calls[1]

    reduction_percent = (
        (baseline_reads - optimized_reads)
        / baseline_reads
        * 100.0
    )

    fig, ax = plt.subplots(
        figsize=(12, 8),
        dpi=300,
    )

    bars = ax.bar(
        methods,
        read_calls,
        width=0.55,
    )

    ax.set_title(
        "Explicit read() System Calls",
        fontsize=20,
        fontweight="bold",
        pad=18,
    )

    ax.set_xlabel(
        "File Reading Method",
        fontsize=14,
        labelpad=10,
    )

    ax.set_ylabel(
        "Number of read() System Calls",
        fontsize=14,
        labelpad=10,
    )

    ax.grid(
        axis="y",
        linestyle="--",
        alpha=0.35,
    )

    ax.set_axisbelow(True)

    ax.tick_params(
        axis="both",
        labelsize=12,
    )

    for bar, value in zip(bars, read_calls):
        ax.text(
            bar.get_x() + bar.get_width() / 2,
            value + 0.3,
            f"{value:,}",
            ha="center",
            va="bottom",
            fontsize=13,
            fontweight="bold",
        )

    ax.text(
        0.5,
        0.94,
        f"mmap() reduced explicit read() calls by "
        f"{reduction_percent:.1f}%",
        transform=ax.transAxes,
        ha="center",
        va="top",
        fontsize=14,
        fontweight="bold",
    )

    ax.set_ylim(
        0,
        max(read_calls) * 1.25,
    )

    fig.tight_layout()

    OUTPUT_FILE.parent.mkdir(
        parents=True,
        exist_ok=True,
    )

    fig.savefig(
        OUTPUT_FILE,
        dpi=300,
        bbox_inches="tight",
    )

    plt.close(fig)

    print("Syscall comparison figure created:")
    print(OUTPUT_FILE)
    print(
        f"Explicit read() reduction: "
        f"{reduction_percent:.2f}%"
    )


if __name__ == "__main__":
    main()
