import os
import sys
import pathlib


def run_command(command):
    print(command)
    status = os.system(command)
    if status != 0:
        sys.exit(1)
    print()


main = os.path.join(
        "install",
        "bin",
        "stablesolver_stable_mps_writer")


if __name__ == "__main__":

    pathlist = pathlib.Path("data/dimacs1992/").glob('*.clq')
    instance_format = "dimacs1992"
    # pathlist = pathlib.Path("data/dimacs2010//").glob('**/*.graph')
    # instance_format = "dimacs2010"
    for instance_path in pathlist:

        output_path_1 = os.path.join(
                "mps",
                str(instance_path) + "_1.mps")
        output_path_2 = os.path.join(
                "mps",
                str(instance_path) + "_2.mps")
        if not os.path.exists(os.path.dirname(output_path_1)):
            os.makedirs(os.path.dirname(output_path_1))

        command = (
                f"{main}"
                f"  --input \"{instance_path}\""
                f" --format {instance_format}"
                " --model 1"
                f"  --solver highs"
                f"  --output \"{output_path_1}\"")
        run_command(command)
        # command = (
        #         f"{main}"
        #         f"  --input \"{instance_path}\""
        #         f" --format {instance_format}"
        #         " --model 3"
        #         f"  --solver highs"
        #         f"  --output \"{output_path_2}\"")
        # run_command(command)
