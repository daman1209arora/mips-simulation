#!/usr/bin/env python3

import os
import subprocess
import re
import time

BIN_LOCATIONS = ["../bin/proc_sim1", "../bin/proc_sim2", "../bin/proc_sim3"]
#BIN_LOCATIONS = ["../bin/proc_sim2"]
RF_SIZE = 32
MEM_SIZE = 10000
TEST_CASE_GEN_LOC = "TestGenerator"
TEST_CASE_LOC = "test_case"



def gen_test_case(folder):
    src = folder + "/src"

    #Generating the test_case using the TestGenerator in obj folder
    cmd = "java {} {} {}".format(TEST_CASE_GEN_LOC, src, TEST_CASE_LOC)
    prog = subprocess.Popen(cmd, shell = True)
    prog.wait()



def extract(output):
    rep = output.replace("\n", " ")
    
    match = re.search(r"Register file: ([\w -\.]*) Memory: ([\w -\.]*)", rep)
    if match == None:
        return [], []
    
    rf_str = match.group(1).split()
    mem_str = match.group(2).split()

    rf = list(map(int, rf_str))
    mem = list(map(int, mem_str))
    return rf, mem



def result(output):
    rep = output.replace("\n", " ")
    
    match = re.search(r"Cycles: ([0-9 ]*) Instructions: ([0-9 ]*) Register file: ([\w -\.]*) Memory: ([\w -\.]*)", rep)
    if match == None:
        return "", "", [], []
    
    cycles = match.group(1)
    instr = match.group(2)
    rf_str = match.group(3).split()
    mem_str = match.group(4).split()

    rf = list(map(int, rf_str))
    mem = list(map(int, mem_str))
    return rf, mem, cycles, instr



def check_test_case(folder, bin):
    try:
        if os.path.exists(folder):
            gen_test_case(folder)
        else:
            print("Couldn't find test case.", end = "\t\t\t\t")
            return {"verdict": "error"}
    except: 
        return {"verdict": "error"}
    
    mem = folder + "/mem"
    res = folder + "/res"
    cmd = "{} {} {}".format(bin, TEST_CASE_LOC, mem)
    time_sec = 0

    try:
        start_time = time.time()
        prog = subprocess.Popen(cmd, stdout = subprocess.PIPE ,shell = True)
        out, err = prog.communicate()
        end_time = time.time()
        time_sec = end_time - start_time
    except:
        return {"verdict": "error"}

    res_file = open(res)
    rf, mem, cycles, instr = result(out.decode("utf-8"))
    rf_res, mem_res = extract(res_file.read())
    res_file.close()
    verdict = {
        "verdict" : "Success" if ((rf == rf_res) and (mem == mem_res)) else "Failure",
        "cycles" : cycles,
        "instr" : instr,
        "time" : time_sec
    }
    return verdict


if __name__ == "__main__":

    test_cases_basic = ["basic/branch", "basic/bne_branch", "basic/haz1", "basic/haz2", "basic/haz3", "basic/haz4", "basic/immediate", 
                "basic/jump", "basic/load_store", "basic/Rtype"]

    test_cases_hard = ["hard/array_sum"]
    #test_cases_hard = ["hard/array_sum", "hard/sel_sort"]

    test_cases = test_cases_basic + test_cases_hard

    for test_case in test_cases:
        for bin in BIN_LOCATIONS:    

            print("Test: {}, Binary: {}".format(test_case, bin), end = "\t")
            result_test_case = check_test_case(test_case, bin)
            print(str(result_test_case["verdict"]) + "...")
            if result_test_case["verdict"] == "error":
                print("Something went wrong.\n")
            else:
                print("Clock cycles: {}, Instructions: {}, Time: {:.3f}s".format(result_test_case["cycles"], result_test_case["instr"], result_test_case["time"]))
        print()
