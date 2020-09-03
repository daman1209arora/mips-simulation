#!/usr/bin/env python3

import random

def write_lines(n):
    filename = "test_sum_array"
    str = ""
    i = 0
    with open(filename) as file:
        for i in range(n):
            str += "{}-{}\n".format(i, random.randint(0, 10))
            i += 1
            if i == n: 
                break

    print(str)

write_lines(5000)
