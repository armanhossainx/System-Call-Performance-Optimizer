import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path


INPUT_FILE = Path(
    "results/stats/baseline_vs_optimized.csv"
)

OUTPUT_FILE = Path(
    "report/figures/baseline_vs_optimized.png"
)


def main():
    df = pd.read_csv(INPUT_FILE)

    methods = df["method"].tolist()
    means = df["mean_ms"].tolist()
    stddevs = df["std_ms"].tolist()

    speedup = df.loc[
        df["method"] == "mmap()",
        "speedup_percent"
    ].iloc[0]

    fig, ax = plt.subplots(figsize=(12, 8), dpi=300)

    bars = ax.bar(
        methods,
        means,
        yerr=stddevs,
        capsize=6,
        width=0.55,
    )

    ax.set_title(
        "Baseline vs. Optimized File Read Performance",
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
        "Mean Wall-Clock Time (ms)",
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

    #adding measured values above bars
    for bar, mean, std in zip(bars, means, stddevs):
        ax.text(
            bar.get_x() + bar.get_width() / 2,
            mean + std + 0.08,
            f"{mean:.3f} ms",
            ha="center",
            va="bottom",
            fontsize=12,
            fontweight="bold",
        )

    #annotating measured improvement
    ax.text(
        0.5,
        0.94,
        f"mmap() mean time: {speedup:.2f}% lower",
        transform=ax.transAxes,
        ha="center",
        va="top",
        fontsize=14,
        fontweight="bold",
    )

    #space for labels and annotations
    upper_limit = max(
        mean + std
        for mean, std in zip(means, stddevs)
    )

    ax.set_ylim(
        0,
        upper_limit * 1.35,
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

    print("Optimization comparison figure created:")
    print(OUTPUT_FILE)


if __name__ == "__main__":
    main()
