import argparse
import gdown
import os
import shutil
import pathlib


parser = argparse.ArgumentParser(description='')
parser.add_argument(
        "-d", "--data",
        type=str,
        nargs='*',
        help='')
args = parser.parse_args()

if args.data is None:
    gdown.download(id="1OUJqeX9IMlRczq4Ycta-lppcnGgFJd2E", output="data.7z")
    os.system("7z x data.7z")
    pathlib.Path("data.7z").unlink()
    shutil.copytree("stable", "data", dirs_exist_ok=True)
