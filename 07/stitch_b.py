"""
Stiches together the binary for part 7a.
"""

import numpy as np
from itertools import permutations
from subprocess import Popen, PIPE
from p_tqdm import p_umap


def check_input_number(input: int):
    cmd = ["./7b", "input.txt", input]
    proc = Popen(cmd, stdout=PIPE, stderr=PIPE)
    output, _ = proc.communicate()

    return int(output), input


if __name__ == "__main__":
    from time import time

    #inputs = permutations("01234")
    inputs = permutations("56789")
    inputs = ["".join(x) for x in inputs]

    initial_t = time()
    output = p_umap(check_input_number, inputs)
    max_thrust = max(output, key=lambda x: x[0])
    final_t = time()

    print(f"Took t = {final_t - initial_t} seconds")
    print(f"Max thrust: {max_thrust[0]}, for seq: {max_thrust[1]}")
