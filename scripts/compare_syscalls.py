import pandas as pd
from pathlib import Path


OUTPUT_FILE = Path(
    "results/stats/syscall_comparison.csv"
)


def main():
    data = [
        {
            "method": "read() 1 MB",
            "read_calls": 12,
            "mmap_calls": 0,
            "munmap_calls": 0,
            "total_traced_calls": 12,
        },
        {
            "method": "mmap()",
            "read_calls": 2,
            "mmap_calls": 35,
            "munmap_calls": 24,
            "total_traced_calls": 61,
        },
    ]

    df = pd.DataFrame(data)

    OUTPUT_FILE.parent.mkdir(
        parents=True,
        exist_ok=True,
    )

    df.to_csv(
        OUTPUT_FILE,
        index=False,
    )

    print("Syscall comparison created.")
    print(f"Output: {OUTPUT_FILE}")
    print()
    print(df.to_string(index=False))


if __name__ == "__main__":
    main()
