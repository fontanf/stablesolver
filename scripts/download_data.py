import argparse
import gdown
import os
import shutil
import pathlib
import time
import sys


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
    download("1OUJqeX9IMlRczq4Ycta-lppcnGgFJd2E")
    os.system("7z x data.7z")
    pathlib.Path("data.7z").unlink()
    shutil.copytree("stable", "data", dirs_exist_ok=True)
