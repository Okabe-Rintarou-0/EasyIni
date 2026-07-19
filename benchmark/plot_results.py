# /// script
# requires-python = ">=3.12"
# dependencies = [
#     "matplotlib",
#     "pandas",
#     "numpy",
#     "seaborn",
# ]
# ///

"""Benchmark comparison chart — Reflection vs SimpleIni."""

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
from pathlib import Path


def main():
    csv_path = Path("benchmark/results.csv")
    if not csv_path.exists():
        print("No results.csv found. Run benchmark first:")
        return

    df = pd.read_csv(csv_path)
    time_col = "real_time"
    df["time_us"] = df[time_col] / 1000.0

    ref = df[df["name"] == "BM_ReflectionParse"].iloc[0]["time_us"]
    si_total = df[df["name"] == "BM_SimpleIniLoadAndBind"].iloc[0]["time_us"]
    si_bind = df[df["name"] == "BM_SimpleIniBindOnly"].iloc[0]["time_us"]
    si_load = si_total - si_bind

    # --- Style Setup ---
    sns.set_theme(font_scale=1.0, style="whitegrid", font="DejaVu Sans")

    fig, ax = plt.subplots(figsize=(7, 6.5), dpi=150)

    # DIVERGING_COLORBLIND: blue for Reflection, red for SimpleIni
    blue, red = "#4575b4", "#d73027"
    grey = "#b3b3b3"

    x = [0, 1]
    w = 0.45

    # Stacked bar: SimpleIni LoadFile (lighter) + Bind (darker)
    ax.bar(1, si_load, w, color=red, alpha=0.45, zorder=3, label="SimpleIni LoadFile")
    ax.bar(1, si_bind, w, bottom=si_load, color=red, zorder=3, label="SimpleIni Bind")
    # Reflection bar
    ax.bar(0, ref, w, color=blue, zorder=3, label="Reflection (auto)")

    # Value labels
    for xi, val, color in [(0, ref, blue), (1, si_total, red)]:
        ax.text(
            xi, val + 280,
            f"{val:,.0f} µs",
            ha="center", va="bottom", weight="semibold", size=13, color="dimgrey",
        )

    # Bind / LoadFile annotation on stacked bar
    ax.text(1, si_load / 2, f"LoadFile\n{si_load:,.0f} µs",
            ha="center", va="center", color="dimgrey", weight="semibold", size=9)
    ax.text(1, si_load + si_bind / 2, f"Bind\n{si_bind:,.0f} µs",
            ha="center", va="center", color="white", weight="semibold", size=9)

    # Insight: speedup
    speedup = si_total / ref
    ax.text(
        0.98, 0.97,
        f"Reflection is {speedup:.1f}× faster\n"
        f"and requires 0 lines of\n"
        f"manual binding code",
        transform=ax.transAxes, ha="right", va="top",
        fontsize=9, color="dimgrey", style="italic",
        linespacing=1.4,
    )

    # Highlight bar with accent colour
    ax.patches[0].set_edgecolor(blue)
    ax.patches[0].set_linewidth(0.5)

    ax.set_title("INI Parser Performance  —  Reflection vs SimpleIni",
                 loc="left", fontsize=13, pad=8, weight="semibold")
    ax.set_ylabel("Time per iteration (µs, lower is better)", fontsize=11, color="dimgrey")
    ax.set_xlim(-0.6, 1.6)
    ax.set_ylim(0, si_total * 1.28)
    ax.set_xticks([0, 1])
    ax.set_xticklabels(["Reflection\n(compile-time)", "SimpleIni\n(runtime)"],
                       fontsize=10, color="dimgrey")

    sns.despine(left=True, bottom=True)
    ax.grid(axis="y", alpha=0.4, linewidth=0.6)
    ax.tick_params(axis="both", which="both", length=0, labelcolor="dimgrey")
    ax.legend(
        frameon=True, facecolor="white", framealpha=0.85, edgecolor="lightgrey",
        labelcolor="dimgrey", loc="upper left", fontsize=9,
    )

    # --- Save ---
    Path("./figures").mkdir(exist_ok=True)
    plt.savefig("./figures/bench_comparison.pdf", dpi=150, bbox_inches="tight")
    plt.savefig("./figures/bench_comparison.png", dpi=150, bbox_inches="tight")
    print("Saved figures/bench_comparison.pdf + .png")


if __name__ == "__main__":
    main()
