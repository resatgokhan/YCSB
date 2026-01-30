#!/usr/bin/env python3
"""
Parse YCSB output and plot throughput vs threads for each workload
for both splitlsm and unifiedlsm.

Usage:
    python plot_ycsb.py ycsb_output.txt

If no file is given, it defaults to 'ycsb_output.txt'.
"""

import argparse
import re
from pathlib import Path

import pandas as pd
import matplotlib.pyplot as plt


def parse_ycsb_log(text: str) -> pd.DataFrame:
    """
    Parse YCSB log text into a DataFrame with columns:
    [engine, workload, threads, throughput_ktps]
    """
    rows = []
    for line in text.splitlines():
        line = line.strip()
        if not line:
            continue
        if line.startswith("#") or line.startswith("./"):
            # Skip comments and shell errors
            continue

        parts = re.split(r"\s+", line)
        # Expect: engine  path  threads  throughput
        if len(parts) != 4:
            continue

        engine, path, threads, throughput = parts
        workload = Path(path).name.replace(".spec", "")

        try:
            threads_i = int(threads)
            throughput_f = float(throughput)
        except ValueError:
            continue

        rows.append(
            {
                "engine": engine,
                "workload": workload,
                "threads": threads_i,
                "throughput_ktps": throughput_f,
            }
        )

    if not rows:
        raise ValueError("No valid YCSB result lines found in the input.")

    return pd.DataFrame(rows)


def summarize(df: pd.DataFrame) -> pd.DataFrame:
    """
    Aggregate multiple runs: mean/std throughput per (workload, engine, threads).
    """
    summary = (
        df.groupby(["workload", "engine", "threads"])
        .agg(
            mean_throughput=("throughput_ktps", "mean"),
            std_throughput=("throughput_ktps", "std"),
        )
        .reset_index()
        .sort_values(["workload", "threads", "engine"])
    )
    return summary


def plot_workloads(summary: pd.DataFrame, out_dir: Path) -> None:
    """
    For each workload, create a plot:
        threads (x) vs mean_throughput (y), one line per engine.
    Saves PNGs into out_dir.
    """
    out_dir.mkdir(parents=True, exist_ok=True)

    workload_names = {
        "workload_delete": "Delete-only",
        "workload_get": "Get-only",
        "workload_mixed_balanced": "Mixed balanced",
        "workload_mixed_read_heavy": "Mixed read-heavy",
        "workload_mixed_write_delete": "Mixed write+delete",
        "workload_put": "Put-only",
    }

    for workload in sorted(summary["workload"].unique()):
        sub = summary[summary["workload"] == workload]
        if sub.empty:
            continue

        pretty_name = workload_names.get(workload, workload)

        plt.figure(figsize=(10, 5))
        for engine in sub["engine"].unique():
            s = sub[sub["engine"] == engine]
            s = s.sort_values("threads")
            plt.plot(
                s["threads"],
                s["mean_throughput"],
                marker="o",
                label=engine,
            )

        plt.xlabel("Threads")
        plt.ylabel("Throughput (KTPS)")
        plt.title(f"YCSB throughput vs threads – {pretty_name}")
        plt.grid(True)
        plt.legend()
        plt.tight_layout()

        out_path = out_dir / f"{workload}_throughput_vs_threads.png"
        plt.savefig(out_path, dpi=150)
        plt.close()
        print(f"Saved plot: {out_path}")


def main():
    parser = argparse.ArgumentParser(
        description="Create YCSB throughput plots from benchmark output."
    )
    parser.add_argument(
        "logfile",
        nargs="?",
        default="ycsb_output.txt",
        help="Path to YCSB output log (default: ycsb_output.txt)",
    )
    parser.add_argument(
        "--out-dir",
        default="plots",
        help="Directory to store generated PNG plots (default: plots)",
    )
    parser.add_argument(
        "--summary-csv",
        default="ycsb_summary.csv",
        help="Path to write aggregated summary CSV (default: ycsb_summary.csv)",
    )
    args = parser.parse_args()

    log_path = Path(args.logfile)
    if not log_path.is_file():
        raise SystemExit(f"Log file not found: {log_path}")

    text = log_path.read_text()
    df = parse_ycsb_log(text)
    summary = summarize(df)

    # Save summary CSV (mean/std per workload/engine/thread)
    summary.to_csv(args.summary_csv, index=False)
    print(f"Saved summary CSV: {args.summary_csv}")

    # Make plots
    plot_workloads(summary, Path(args.out_dir))


if __name__ == "__main__":
    main()
