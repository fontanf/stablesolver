import argparse
import gdown
import os
import shutil
import pathlib
import time
import sys
import py7zr


parser = argparse.ArgumentParser(description='')
parser.add_argument(
        "-d", "--data",
        type=str,
        nargs='*',
        help='')
args = parser.parse_args()

def download(id):
    for _ in range(3):
        try:
            gdown.download(id=id, output="data.7z")
        except:
            time.sleep(10)
            continue
        return
    sys.exit(1)

if args.data is None:
    download("10v7bSBSKRn7Lpl6wpXP83n4QUko3wa1E")
    with py7zr.SevenZipFile("data.7z", mode="r") as z:
        z.extractall(path="data")
    pathlib.Path("data.7z").unlink()
