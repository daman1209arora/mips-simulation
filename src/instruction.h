#ifndef INSTRUCTION_HEADER
#define INSTRUCTION_HEADER

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#define ll long long
using namespace std;

enum Operation {ADD, SUB, AND, OR, SLT, SLL, SRL, NOOP};

vector<int> toBinary(ll instruction);
bool matches(vector<int>& bin, int start, int end, string s);
bool isRType(ll instruction);
bool isLoad(ll instruction);
bool isStore(ll instruction);
bool isBranch(ll instruction);
bool isBEQ(ll instruction);
bool isBNE(ll instruction);
bool isJump(ll instruction);
bool isJAL(ll instruction);
bool isJR(ll instruction);
bool isNoop(ll instruction);
bool isLUI(ll instruction);
bool isSLL(ll instruction);
bool isSRL(ll instruction);
bool writes(ll instruction, int reg);
bool reads(ll instruction, int reg);
Operation getOperation(ll instruction);
ll toDecimal(vector<int> instruction, int start, int end);
ll getWriteOffset(ll instruction);
ll getBranchOffset(ll instruction);
ll getJumpOffset(ll instruction);
ll getRS(ll instruction);
ll getRT(ll instruction);
ll getRD(ll instruction);
vector<ll> getReadReg(ll instruction);
ll getWriteReg(ll instruction);
ll hazardExists(ll i1, ll i2);

#endif