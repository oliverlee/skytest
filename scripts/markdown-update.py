#!/usr/bin/env python3
 # -*- coding: utf-8 -*-

import argparse
import os
from pathlib import Path
import sys

from markdown_embed_code import get_code_emb

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="embed code into a markdown file.")
    parser.add_argument("filename")
    args = parser.parse_args()

    os.chdir(os.environ.get("BUILD_WORKSPACE_DIRECTORY"))

    with open(Path(args.filename), "r") as f:
        indoc = f.read()

    outdoc = get_code_emb()(indoc)

    sys.stdout.buffer.write(outdoc.encode())

