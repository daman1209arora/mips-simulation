#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

#define ll long long
using namespace std;


enum Operation {ADD, SUB, AND, OR, SLT, SLL, SRL, NOOP};

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
    return matches(bin, 0, 6, "000100");
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

class IFID {
public: 
    ll PC = 0;
    ll instruction = 0;

};

class IDEX {
public:
    ll PC = 0;
    ll instruction = 0;
    ll r1 = 0, r2 = 0; // values read from the register file.
};

class EXMEM {
public: 
    ll instruction = 0;
    ll writeData = 0;
    ll branchPC = 0;
    ll PC = 0;
    bool branch = false;
    ll aluResult = 0;
    ll writeMemoryAddress = 0;
    ll loadMemoryAddress = 0;

    bool toBranch() {
        // returns whether a branch has to be done or not
        return isBranch(instruction) && branch;
    }

    bool toWrite() {
        // returns whether data is to be written or not
        return isStore(instruction);
    }
};

class MEMWB {
public: 
    ll PC = 0;
    ll instruction = 0;     // stores the type of instruction
    ll writeData = 0;   // stores the data that is to be written
    ll writeRFAddress = 0;
    bool toWrite() {
        // interpolates whether the data has to be written or not
        return isRType(instruction) || isLoad(instruction) || isLUI(instruction);
    }
};

class PC {
public:
    ll counter = 0;
};

class RegisterFile {
public: 
    RegisterFile() {
        rf = vector<ll>(32, 0);
    }
    vector<ll> rf;
};
 
class InstructionMemory {
public: 
    /*
        Constructor reads instructions from file and writes them to the memory vector.
        Instructions are in the form of longs which will be decoded using an Instruction
        class.
    */
    InstructionMemory(string file) {
        imem = vector<ll>(4096, 0);
        ifstream myfile (file);
        int i = 0;
        if (myfile.is_open()) {
            string line;
            while (getline(myfile, line)){
                imem[i] = stoll(line);
                i++;
            }
            myfile.close();
        }
    }
    vector<ll> imem;
};

class Memory {
public:
    Memory(string file) {
        memory = vector<ll>(100000, 0); // change to 1e6 later
        ifstream myfile (file);
        if (myfile.is_open()) {
            string line;
            while (getline(myfile, line)){
                size_t index = line.find("-");
                size_t start = 0;
                ll pos = stoll(line.substr(start, index));
                ll val = stoll(line.substr(index + 1, string::npos));
                memory[pos] = val;
            }
            myfile.close();
        }
    }

    vector<ll> memory; 
};

void writeLogs(int numCycles, int numInstr, vector<ll> &rf, vector<ll> &memory) {
    cout << "Cycles: " << numCycles << endl;
    cout << "Instructions: " << numInstr << endl;
    cout << endl << "Register file: " << endl;
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 8; j++) 
            cout << rf[8 * i + j] << " ";
        cout<<endl;
    }

    cout << endl << "Memory: " << endl;
    for(int i = 0; i < 20; i++) {
        for(int j = 0; j < 5000; j++) 
            cout << memory[5000 * i + j] << " ";
        cout<<endl;
    }
    cout << endl;
}

int main(int argc, char* argv[]) {
    RegisterFile RF;
    InstructionMemory IMEM(argv[1]);
    Memory MEM(argv[2]);
    IFID ifid;
    IDEX idex;
    EXMEM exmem;
    MEMWB memwb;

    ll PC = 0;
    int clk = 0;
    bool stop = false; // stop execution when instruction in IFID and MEMWB are noops.
    ll jumpOffset = 0;
    int jumpRegIndex = 0;
    ll numCycles = 0, numStalls = 0, numInstr = 0;

    bool hazard = false;
    bool branchStall = false;
    bool jumpPosition = false;
    bool jumpReg = false;

    /*
        There are three types of situations that need to be handled:
        1)  A data hazard. A data hazard can occur when the read register for
            the IFID register and the write register for the IDEX or EXMEM 
            register are the same.
        2)  A jump instruction requires a gap of one instruction. A bubble
            will be inserted after each jump(j, jr, jal)
        3)  For a branch instruction, we wait till the instruction has reached
            the end of the IDEX stage. After that, we update PC and then 
            resumption of execution takes place. 

    -------------------------------------------------------------------------------


        While dealing with forwarding, there are two types of forwarding.
        Let the instructions be I1, I2, I3 where I3 is fed first into
        the pipeline. 
        1) I3 is an R-type instruction:
            a)  If there is a conflict with I2, then we can forward from the EXMEM
                register without any stalls
            b)  If there is a conflict with I1, then we can forward from the MEMWB
                register without any stalls
            c)  If there is a conflict in both I1 and I2 we can forward in both
                the cases without any stalls necessary

        2) I3 is a load instruction: 
            a)  If I2 is conflicting with I3, then we will have to stall for one
                clock cycle and perform forwarding in the MEMWB stage. 
            b)  If I1 is conflicting with I3, then we do not need to stall and we
                can forward in the MEMWB stage. 
            c)  Note that both the conflicts can not occur simultaneously. 


        How we deal with the process of forwarding. 
        Check for conflicts before updation of signals to their new values in the 
        EXMEM and MEMWB stages. 
    */

    while(!stop) {
        if(clk == 0) {
            /*
                When the clock is not asserted, all the writes are done. 
                There are only two writes to be made that is in the
                memory and the register file. 
            */

            if(memwb.toWrite()) {
                int position = memwb.writeRFAddress;
                RF.rf[position] = memwb.writeData;
            }

            if(exmem.toWrite()) {
                int position = exmem.writeMemoryAddress ;
                MEM.memory[position / 4] = exmem.writeData;
            }
            clk = 1;
        }
        else {
            /*
                When the clock is asserted, the following things have to be done:
                1) Updating all the intermediate register(IFID, IDEX...)
                2) Updating the Program counter

                The idea of continuing the execution of statements after the
                branch is problematic. That is because of the possibility of
                a jump just after the branch. The right way of solving this 
                problem is to stop execution until the branch condition
                is truely known. 

                Algorithm: 
                1)  If a branch instruction appears at the IFID register, 
                    stop PC updation
                2)  When the instruction reaches EXMEM stage, it is possible 
                    to know whether a branch is to be taken or not. Normal 
                    process can resume from the EXMEM stage.

                Two bubbles have to be inserted in the IFID register, then 
                control will be resumed after one cycle. 
            */

            branchStall = (isBranch(ifid.instruction) || isBranch(idex.instruction));

            ll PCTaken = exmem.branchPC;

            hazard = (isLUI(idex.instruction) || isLoad(idex.instruction)) && hazardExists(ifid.instruction, idex.instruction);

            jumpOffset = 4 * getJumpOffset(ifid.instruction);
            jumpRegIndex = toDecimal(toBinary(ifid.instruction), 6, 11);

            
            // Updating MEMWB

            memwb.instruction = exmem.instruction;
            if(isRType(exmem.instruction)) {
                memwb.writeRFAddress = getRD(exmem.instruction);
                memwb.writeData = exmem.aluResult;
            }
            else if(isLoad(exmem.instruction)) {
                memwb.writeRFAddress = getRT(exmem.instruction);
                memwb.writeData = MEM.memory[exmem.loadMemoryAddress / 4];
            }
            else if(isLUI(exmem.instruction)) {
                memwb.writeRFAddress = getRT(exmem.instruction);
                memwb.writeData = toDecimal(toBinary(exmem.instruction), 16, 32) << 16;
            }
            memwb.PC = exmem.PC;

            /*
                The writeData in MEMWB depends on the previous instruction.
                It can come from the Memory if the instruction is a load
                and from the ALU if it is an R-type instruction. 

                Also, we must copy the instruction from EXMEM register.
            */

            // Updating EXMEM


            exmem.instruction = idex.instruction;
            exmem.PC = idex.PC;

            if(isRType(idex.instruction)) {
                Operation op = getOperation(idex.instruction);
                int offset = toDecimal(toBinary(idex.instruction), 21, 26);
                if(op == ADD) 
                    exmem.aluResult = idex.r1 + idex.r2;
                else if(op == SUB)
                    exmem.aluResult = idex.r1 - idex.r2;
                else if(op == AND)
                    exmem.aluResult = idex.r1 & idex.r2;
                else if(op == OR)
                    exmem.aluResult = idex.r1 | idex.r2;
                else if(op == SLT)
                    exmem.aluResult = idex.r1 < idex.r2 ? 1 : 0;
                else if(op == SLL)
                    exmem.aluResult = idex.r2 << offset;
                else if(op == SRL)
                    exmem.aluResult = idex.r2 >> offset;
            }
            else if(isLoad(idex.instruction)) {
                exmem.loadMemoryAddress = idex.r1 + getWriteOffset(idex.instruction);
            }
            else if(isStore(idex.instruction)) {
                exmem.writeMemoryAddress = idex.r1 + getWriteOffset(idex.instruction);
                exmem.writeData = idex.r2;
            }
            else if(isBranch(idex.instruction)) {
                if(idex.r1 == idex.r2)
                    exmem.branch = true;
                else
                    exmem.branch = false;
                exmem.branchPC = idex.PC + 4 + getBranchOffset(idex.instruction) * 4;

            }
            
            /*
                In EXMEM, the things that have to be updated are the ALU results,
                newly calculated PC, instruction, writeData for the memwb stage, 
            */


            // Updating IDEX


            /*
                Logic for forwarding is pretty straight-forward. 
                While updating the EXMEM register, just check if the value
                to be used for calculating aluResult or memory address is being 
                written in by the instructions in exmem registers o
            */

            if(!hazard) {
                idex.instruction = ifid.instruction;
                idex.PC = ifid.PC;
                int rs = getRS(ifid.instruction);
                int rt = getRT(ifid.instruction);

                if(writes(exmem.instruction, rs) && reads(idex.instruction, rs)) 
                    idex.r1 = exmem.aluResult;
                else if(writes(memwb.instruction, rs) && reads(idex.instruction, rs))
                    idex.r1 = memwb.writeData;
                else 
                    idex.r1 = RF.rf[rs];

                if(writes(exmem.instruction, rt) && reads(idex.instruction, rt)) 
                    idex.r2 = exmem.aluResult;
                else if(writes(memwb.instruction, rt) && reads(idex.instruction, rt))
                    idex.r2 = memwb.writeData;
                else
                    idex.r2 = RF.rf[rt];
                    
            }
            else {
                idex.instruction = 0;
                idex.PC = 0;
                idex.r1 = 0;
                idex.r2 = 0;
            }
            // else if hazard2, contents of the register remain same

            /*
                In IDEX, only PC, instruction and the values from the register
                file are to be read.
            */

            // Updating IFID
            jumpPosition = isJump(ifid.instruction) || isJAL(ifid.instruction);
            jumpReg = isJR(ifid.instruction);

            if(!branchStall) {
                if(!hazard) {
                    if(isJump(ifid.instruction)) {
                        ifid.PC = 0;
                        ifid.instruction = 0;
                    }
                    else if(isJAL(ifid.instruction)) {
                        ifid.PC = 0;
                        ifid.instruction = 0;
                        RF.rf[31] = PC;
                    }
                    else if(isJR(ifid.instruction)) {
                        vector<int> bin = toBinary(ifid.instruction);
                        ifid.PC = 0;
                        ifid.instruction = 0;
                    }
                    else {
                        ifid.PC = PC;
                        ifid.instruction = IMEM.imem[PC / 4];
                    }
                }
                // else remains the same as before.
            }
            else {
                if(!hazard) {
                    ifid.PC = 0;
                    ifid.instruction = 0;
                }
            }


            // Updating the PC
            if(branchStall && isBranch(exmem.instruction)) {
                /* 
                    Note that exmem.instruction has been executed in this cycle. 
                    It is actually the value of IFID in the last cycle. 
                */    
                if(exmem.branch)
                    PC = exmem.branchPC;
                branchStall = false;
            }
            else if(!branchStall){
                if(!hazard) {
                    if(jumpPosition) {
                        numStalls++;
                        PC = jumpOffset;
                    }
                    else if(jumpReg) {
                        numStalls++;
                        PC = RF.rf[jumpRegIndex];
                    }
                    else {
                        PC = PC + 4;    
                    }
                }
                // else PC remains the same.
            }

            /*
                It is important to remember that the offset of branch
                is added to PC + 4 not directly to PC.
            */ 

            stop = isNoop(ifid.instruction) && isNoop(memwb.instruction) 
                    && isNoop(idex.instruction) && isNoop(exmem.instruction);
            clk = 0;
            numCycles++;
            numInstr += !(isNoop(memwb.instruction));
        }
    }
    writeLogs(numCycles, numInstr, RF.rf, MEM.memory);
}