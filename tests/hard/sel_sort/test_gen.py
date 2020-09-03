#!/usr/bin/env python3

import random

def write_lines(n):
    str = ""
    for i in range(n):
        str += "{}-{}\n".format(i, random.randint(0, 100))
        i += 1
        if i == n: 
            break
    print(str)

write_lines(500)
