import argparse
import sys
import os

parser = argparse.ArgumentParser(description='')
parser.add_argument('directory')
parser.add_argument(
        "-t", "--tests",
        type=str,
        nargs='*',
        help='')

args = parser.parse_args()


stable_main = os.path.join(
        "install",
        "bin",
        "stablesolver_stable")
data_dir = os.environ['STABLE_DATA']


greedy_data = [
        (os.path.join("dimacs1992", "brock200_1.clq"), "dimacs1992"),
        (os.path.join("dimacs1992", "C125.9.clq"), "dimacs1992"),
        (os.path.join("dimacs1992", "c-fat200-1.clq"), "dimacs1992"),
        (os.path.join("dimacs1992", "gen200_p0.9_44.clq"), "dimacs1992"),
        (os.path.join("dimacs1992", "hamming6-2.clq"), "dimacs1992"),
        (os.path.join("dimacs1992", "johnson8-2-4.clq"), "dimacs1992"),
        (os.path.join("dimacs1992", "keller4.clq"), "dimacs1992"),
        (os.path.join("dimacs1992", "MANN_a9.clq"), "dimacs1992"),
        (os.path.join("dimacs1992", "p_hat300-1.clq"), "dimacs1992"),
        (os.path.join("dimacs1992", "san200_0.7_1.clq"), "dimacs1992"),
        (os.path.join("dimacs1992", "sanr200_0.7.clq"), "dimacs1992")]


if args.tests is None or "stable-greedy-gwmin" in args.tests:
    print("Stable, greedy GWMIN")
    print("--------------------")
    print()

    for instance, instance_format in greedy_data:
        instance_path = os.path.join(
                data_dir,
                instance)
        json_output_path = os.path.join(
                args.directory,
                "stable",
                "greedy_gwmin",
                instance + ".json")
        if not os.path.exists(os.path.dirname(json_output_path)):
            os.makedirs(os.path.dirname(json_output_path))
        command = (
                stable_main
                + "  --verbosity-level 1"
                + "  --input \"" + instance_path + "\""
                + "  --format \"" + instance_format + "\""
                + "  --algorithm greedy-gwmin"
                + "  --output \"" + json_output_path + "\"")
        print(command)
        status = os.system(command)
        if status != 0:
            sys.exit(1)
        print()
    print()
    print()


if args.tests is None or "stable-greedy-gwmax" in args.tests:
    print("Stable, greedy GWMAX")
    print("--------------------")
    print()

    for instance, instance_format in greedy_data:
        instance_path = os.path.join(
                data_dir,
                instance)
        json_output_path = os.path.join(
                args.directory,
                "stable",
                "greedy_gwmax",
                instance + ".json")
        if not os.path.exists(os.path.dirname(json_output_path)):
            os.makedirs(os.path.dirname(json_output_path))
        command = (
                stable_main
                + "  --verbosity-level 1"
                + "  --input \"" + instance_path + "\""
                + "  --format \"" + instance_format + "\""
                + "  --algorithm greedy-gwmax"
                + "  --output \"" + json_output_path + "\"")
        print(command)
        status = os.system(command)
        if status != 0:
            sys.exit(1)
        print()
    print()
    print()


if args.tests is None or "stable-greedy-gwmin2" in args.tests:
    print("Stable, greedy GWMIN2")
    print("---------------------")
    print()

    for instance, instance_format in greedy_data:
        instance_path = os.path.join(
                data_dir,
                instance)
        json_output_path = os.path.join(
                args.directory,
                "stable",
                "greedy_gwmin2",
                instance + ".json")
        if not os.path.exists(os.path.dirname(json_output_path)):
            os.makedirs(os.path.dirname(json_output_path))
        command = (
                stable_main
                + "  --verbosity-level 1"
                + "  --input \"" + instance_path + "\""
                + "  --format \"" + instance_format + "\""
                + "  --algorithm greedy-gwmin2"
                + "  --output \"" + json_output_path + "\"")
        print(command)
        status = os.system(command)
        if status != 0:
            sys.exit(1)
        print()
    print()
    print()


clique_main = os.path.join(
        "install",
        "bin",
        "stablesolver_clique")


if args.tests is None or "clique-greedy-gwmin" in args.tests:
    print("Clique, greedy GWMIN")
    print("--------------------")
    print()

    for instance, instance_format in greedy_data:
        instance_path = os.path.join(
                data_dir,
                instance)
        json_output_path = os.path.join(
                args.directory,
                "clique",
                "greedy_gwmin",
                instance + ".json")
        if not os.path.exists(os.path.dirname(json_output_path)):
            os.makedirs(os.path.dirname(json_output_path))
        command = (
                clique_main
                + "  --verbosity-level 1"
                + "  --input \"" + instance_path + "\""
                + "  --format \"" + instance_format + "\""
                + "  --algorithm greedy-gwmin"
                + "  --output \"" + json_output_path + "\"")
        print(command)
        status = os.system(command)
        if status != 0:
            sys.exit(1)
        print()
    print()
    print()
