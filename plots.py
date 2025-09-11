#!/usr/bin/env python3

import pandas as pd
import numpy as np
from matplotlib import pyplot as plt

df = pd.read_csv("benchmark_results.csv")

groups = {
    "mmul": ["mmul", "mmul_wall", "clblast_mmul", "openacc_mmul"],
    "times_table": ["times_table", "clblast_times_tables", "openacc_times_table"],
    "transpose": ["transpose", "clblast_transpose", "openacc_transpose"],
    "asum": ["asum", "clblast_asum", "openacc_asum"],
    "scan": ["scan", "openacc_scan"],
    "l2norm": ["l2norm"],
}

devices = ["a40", "v100", "l40s", "a100", "h100"]

linestyles = ["-", "--", "-.", ":"]
markers = ["o", "s", "D", "^"]

alignments = [
    {"ha": "left", "va": "top"},
    {"ha": "right", "va": "top"},
    {"ha": "left", "va": "bottom"},
    {"ha": "right", "va": "bottom"},
]


def lines(group, device):
    for i, bench in enumerate(groups[group]):
        d = df[(df["benchmark"] == bench) & (df["device"] == device)]

        line_color = plt.rcParams["axes.prop_cycle"].by_key()["color"][
            i % len(linestyles)
        ]

        plt.plot(
            d["size"],
            d["runtime"],
            label=bench,
            linestyle=linestyles[i % len(linestyles)],
            marker=markers[i % len(markers)],
            color=line_color,
        )

        align = alignments[i % len(alignments)]

        threshold = 0.1 * d["runtime"].max()

        for x, y in zip(d["size"], d["runtime"]):
            if y >= threshold:
                plt.text(
                    x,
                    y,
                    f"{int(y)} ms",
                    fontsize=8,
                    verticalalignment=align["va"],
                    horizontalalignment=align["ha"],
                    color=line_color,
                    transform=plt.gca().transData,
                    clip_on=False,
                )

    plt.xlabel("size")
    plt.xscale("log", base=2)
    plt.ylabel("runtime (ms)")
    plt.title(f"{group} (fp32) NVIDIA {device}")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(f"plots/{group}_{device}.png", dpi=600, bbox_inches="tight")
    plt.close()


def bars(group):
    benchmarks = groups[group]
    df_group = df[df["benchmark"].isin(benchmarks)]

    bar_width = 0.1
    devices_present = [
        device for device in devices if not df_group[df_group["device"] == device].empty
    ]

    x = np.arange(len(benchmarks))

    for i, device in enumerate(devices_present):
        max_gflops = []
        for bench in benchmarks:
            d = df_group[
                (df_group["benchmark"] == bench) & (df_group["device"] == device)
            ]
            max_gflop = d["gflops"].max() if not d.empty else 0
            max_gflops.append(max_gflop)

        bars = plt.bar(x + i * bar_width, max_gflops, width=bar_width, label=device)

        for bar, gflop in zip(bars, max_gflops):
            tflops = gflop / 1000
            x_text = bar.get_x() + bar.get_width() / 2
            y_text = bar.get_height() * 0.02

            plt.text(
                x_text,
                y_text,
                f"{tflops:.2f} TFLOPS" if tflops != 0.0 else "",
                ha="center",
                va="bottom",
                fontsize=8,
                rotation=90,
                color="black",
                weight="bold",
            )

    plt.xticks(
        x + bar_width * (len(devices_present) - 1) / 2,
        benchmarks,
        rotation=45,
        ha="right",
    )
    plt.ylabel("GFLOPS")
    plt.title(f"{group} (fp32)")
    plt.legend(title="Device")
    plt.grid(axis="y")
    plt.tight_layout()
    plt.savefig(f"plots/{group}_bars.png", dpi=600, bbox_inches="tight")
    plt.close()


for group in groups:
    lines(group, "h100")
    bars(group)
