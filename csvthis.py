#!/usr/bin/env python3

import sys

filename = sys.argv[1]

try:
    with open(filename, "r") as file:
        contents = file.read()
        groups = [g for g in contents.split("Running") if g]
        for group in groups:
            output = ""
            lines = group.partition("/")[2].split("\n")
            ws = lines[0].split(" ")
            output += f"{ws[0]}, {ws[-1]}, {sys.argv[1].split('/')[0]}, "
            t = lines[2]
            g = lines[3]

            output += f"{t.split(':')[1].strip().split(' ')[0]}, "
            output += f"{g.split(':')[1].strip().split(' ')[0]}"

            print(output)
except Exception as e:
    # print(e, filename)
    pass
