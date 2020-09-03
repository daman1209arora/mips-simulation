#include <string>
#include <vector>
#include <algorithm>
#include "instruction.h"
#define ll long long

using namespace std;

// Utility functions:
vector<int> toBinary(ll instruction) {
    vector<int> bin(32, 0);
    for(int i = 31; i >= 0; i--) {
        bin[i] = instruction % 2;
        instruction /= 2;
    }
    return bin;
}

bool matches(vector<int>& bin, int start, int end, string s){
    for(int i = start; i < end; i++) {
        if(bin[i] == 1 && s[i - start] != '1')
            return false;
        else if(bin[i] == 0 && s[i - start] != '0')
            return false;
    }
    return true;
}

bool isRType(ll instruction) {
    vector<int> bin = toBinary(instruction);
    // jr is not considered to be an R type instruction here
    return (matches(bin, 0, 6, "000000") && 
        !(matches(bin, 26, 32, "001000") || 
        matches(bin, 0, 32, "00000000000000000000000000000000")));
}

bool isLoad(ll instruction) {
    vector<int> bin = toBinary(instruction);
    return matches(bin, 0, 6, "100011");
}

bool isStore(ll instruction) {
    vector<int> bin = toBinary(instruction);
    return matches(bin, 0, 6, "101011");
}

bool isBranch(ll instruction) {
    vector<int> bin = toBinary(instruction);
    return matches(bin, 0, 6, "000100") || matches(bin, 0, 6, "000101");
}


bool isBEQ(ll instruction) {
    vector<int> bin = toBinary(instruction);
    return matches(bin, 0, 6, "000100");
}

bool isBNE(ll instruction) {
    vector<int> bin = toBinary(instruction);
    return matches(bin, 0, 6, "000101");
}

bool isJump(ll instruction) {
    vector<int> bin = toBinary(instruction);
    return matches(bin, 0, 6, "000010");
}

bool isJAL(ll instruction) {
    vector<int> bin = toBinary(instruction);
    return matches(bin, 0, 6, "000011");
}

bool isJR(ll instruction) {
    vector<int> bin = toBinary(instruction);
    return matches(bin, 0, 6, "000000") && matches(bin, 26, 32, "001000");
}

bool isNoop(ll instruction) {
    vector<int> bin = toBinary(instruction);
    return matches(bin, 0, 32, "00000000000000000000000000000000");
}

bool isLUI(ll instruction) {
    vector<int> bin = toBinary(instruction);
    return matches(bin, 0, 6, "001111");
}

bool isSLL(ll instruction) {
    vector<int> bin = toBinary(instruction);
    return matches(bin, 0, 6, "000000") && matches(bin, 26, 32, "000000");
}

bool isSRL(ll instruction) {
    vector<int> bin = toBinary(instruction);
    return matches(bin, 0, 6, "000000") && matches(bin, 26, 32, "000010");
}

Operation getOperation(ll instruction) {
    // It is assumed already that the instruction is an R-type instruction
    vector<int> bin = toBinary(instruction);
    if(matches(bin, 26, 32, "100000"))
        return ADD;
    else if(matches(bin, 26, 32, "100010"))
        return SUB;
    else if(matches(bin, 26, 32, "100101"))
        return OR;
    else if(matches(bin, 26, 32, "101010"))
        return SLT;
    else if(matches(bin, 26, 32, "100100"))
        return AND;
    else if(matches(bin, 0, 32, "00000000000000000000000000000000"))
        return NOOP;
    else if(matches(bin, 26, 32, "000000"))
        return SLL;
    else if(matches(bin, 26, 32, "000010"))
        return SRL;
}

ll toDecimal(vector<int> instruction, int start, int end) {
    ll num = 0;
    ll pow = 1;
    for(int i = end - 1; i >= start; i--) {
        if(instruction[i] == 1)
            num += pow;
        pow = pow << 1;
    }
    return num;
}

ll getWriteOffset(ll instruction) {
    vector<int> bin = toBinary(instruction);
    return toDecimal(bin, 16, 32);
}

ll getBranchOffset(ll instruction) {
    vector<int> bin = toBinary(instruction);
    return toDecimal(bin, 16, 32);
}

ll getJumpOffset(ll instruction) {
    vector<int> bin = toBinary(instruction);
    return toDecimal(bin, 6, 32);
}

ll getRS(ll instruction) {
    vector<int> bin = toBinary(instruction);
    return toDecimal(bin, 6, 11);
}

ll getRT(ll instruction) {
    vector<int> bin = toBinary(instruction);
    return toDecimal(bin, 11, 16);
}

ll getRD(ll instruction) {
    vector<int> bin = toBinary(instruction);
    return toDecimal(bin, 16, 21);
}

vector<ll> getReadReg(ll instruction) {
    // assert: instruction is not NOOP
    vector<int> bin = toBinary(instruction);
    if(isRType(instruction) || isStore(instruction) || isBranch(instruction)) {
        // separate case required for sll. 
        if(isSLL(instruction) || isSRL(instruction)) {
            ll rt = toDecimal(bin, 11, 16);
            return vector<ll>({rt});
        }
        ll rs = toDecimal(bin, 6, 11);
        ll rt = toDecimal(bin, 11, 16);
        return vector<ll>({rs, rt});
    }
    else if(isLoad(instruction) || isJR(instruction)) {
        ll base = toDecimal(bin, 6, 11);
        return vector<ll>({base});
    }
    else if(isJump(instruction) || isJAL(instruction) || isLUI(instruction)) {
        return vector<ll>();
    }
    return vector<ll>();
}

ll getWriteReg(ll instruction) {
    // assert: instruction is not NOOP
    vector<int> bin = toBinary(instruction);
    if(isRType(instruction)) {
        ll rd = toDecimal(bin, 16, 21);
        return rd;
    }
    else if(isLoad(instruction) || isLUI(instruction)) {
        ll rt = toDecimal(bin, 11, 16);
        return rt;
    }
    else
        return -1;
}

ll hazardExists(ll i1, ll i2) {
    // i1 is executed after i2
    if(isNoop(i1) || isNoop(i2))
        return false;
    else {
        vector<ll> readReg1 = getReadReg(i1);
        ll writeReg2 = getWriteReg(i2);
        for(int i = 0; i < readReg1.size(); i++) {
            if(readReg1[i] == writeReg2)
                return true;
        }
        return false;
    }
}

bool writes(ll instruction, int reg) {
    ll wr = getWriteReg(instruction);
    return wr == reg;
}

bool reads(ll instruction, int reg) {
    vector<ll> rd = getReadReg(instruction);
    return find(rd.begin(), rd.end(), reg) != rd.end();
}