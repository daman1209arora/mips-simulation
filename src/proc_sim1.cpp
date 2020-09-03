#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "instruction.h"

#define ll long long
using namespace std;

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
    ll instruction = 0;
    ll writeData = 0;   // stores the data that is to be written into the register file
    ll writeRFAddress = 0;
    bool toWrite() {
        // interpolates whether the data has to be written or not
        return isRType(instruction) || isLoad(instruction) || isLUI(instruction);
    }
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
    InstructionMemory(string file) {
        /*
            Constructor reads instructions from file and writes them to the memory vector.
            Instructions are in the form of longs which will be decoded using an Instruction
            class.
        */
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

        memory = vector<ll>(100000, 0);
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

    /*  
        In the logs, we mention number of cycles required, total instruction
        executed, contents of the register files and the contents of the
        memory file. 
    */

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

    bool stop = false; // stop execution when instruction in all pipeline registers are noops.
    ll jumpOffset = 0;
    int jumpRegIndex = 0;
    ll numCycles = 0, numStalls = 0, numInstr = 0;

    bool hazard = false;    // flag to indicate whether a hazard is present b/w instructions
    bool branchStall = false;   // flag to indicate a stall if a branch instruction is seen
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
    */

    while(!stop) {
        if(clk == 0) {

            /*
                When the clock is not asserted, all the writes are done. 
                There are only two writes to be made that is, one in the
                memory and one in the register file. 
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

            hazard = hazardExists(ifid.instruction, exmem.instruction) || 
                    hazardExists(ifid.instruction, idex.instruction);

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

                if(isBEQ(idex.instruction) && idex.r1 == idex.r2) 
                    exmem.branch = true;
                else if(isBNE(idex.instruction) && idex.r1 != idex.r2) 
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

            if(!hazard) {
                idex.instruction = ifid.instruction;
                idex.PC = ifid.PC;
                idex.r1 = RF.rf[getRS(ifid.instruction)];
                idex.r2 = RF.rf[getRT(ifid.instruction)];    
            }
            else {
                //insert bubble 
                
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