import java.io.*;
import java.lang.Math;
import java.util.HashMap;

public class TestGenerator{
    public static String toBinary(long k, int digits){
        char[] s = new char[digits];
        long n = k;
        if(k < 0)
          n = (long)(Math.pow(2, digits) + k);
        for(int i = 0; i < digits; i++)
            s[i] = '0';
        int idx = 0;
        while(n != 0){
            if(n % 2 == 0)
                s[digits - idx - 1] = '0';
            else
                s[digits - idx - 1] = '1';
            n /= 2;
            idx++;
        }
        return new String(s);
    }

    public static long toDecimal(String instr) {
        long num = 0;
        long pow = 1;
        int len = instr.length();
        for(int i = len - 1; i >= 0; i--) {
            if(instr.charAt(i) == '1')
                num = num + pow;
            pow = pow << 1; 
        }
        return num;
    }

    public static void main(String[] args)throws IOException{
        BufferedReader br = new BufferedReader(new FileReader(new File(args[0])));
        BufferedWriter bw = new BufferedWriter(new FileWriter(new File(args[1])));
        String s;
        String[] prog = new String[4096];
        HashMap<String, Integer> map = new HashMap<String, Integer>();
        String[] arr = {  "$zero", "$at", "$v0", "$v1", 
                            "$a0", "$a1", "$a2", "$a3",
                            "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7",
                            "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
                            "$t8", "$t9", "$k0", "$k1",
                            "$gp", "$sp", "$fp", "$ra"
                        };
        for(int i = 0; i < 32; i++)
            map.put(arr[i], i);
        int i = 0;


        while((s = br.readLine()) != null){
            String[] tokens = s.split(" ");
            String instr = null;
            if(tokens[0].compareTo("add") == 0){
                instr = "000000";
                instr += toBinary(map.get(tokens[2]), 5);
                instr += toBinary(map.get(tokens[3]), 5);
                instr += toBinary(map.get(tokens[1]), 5);
                instr += "00000";
                instr += "100000";
            }
            else if(tokens[0].compareTo("sub") == 0){
                instr = "000000";
                instr += toBinary(map.get(tokens[2]), 5);
                instr += toBinary(map.get(tokens[3]), 5);
                instr += toBinary(map.get(tokens[1]), 5);
                instr += "00000";
                instr += "100010";
            }
            else if(tokens[0].compareTo("and") == 0){
                instr = "000000";
                instr += toBinary(map.get(tokens[2]), 5);
                instr += toBinary(map.get(tokens[3]), 5);
                instr += toBinary(map.get(tokens[1]), 5);
                instr += "00000";
                instr += "100100";
            }
            else if(tokens[0].compareTo("or") == 0){
                instr = "000000";
                instr += toBinary(map.get(tokens[2]), 5);
                instr += toBinary(map.get(tokens[3]), 5);
                instr += toBinary(map.get(tokens[1]), 5);
                instr += "00000";
                instr += "100101";
            }
            else if(tokens[0].compareTo("slt") == 0){
                instr = "000000";
                instr += toBinary(map.get(tokens[2]), 5);
                instr += toBinary(map.get(tokens[3]), 5);
                instr += toBinary(map.get(tokens[1]), 5);
                instr += "00000";
                instr += "101010";
            }
            else if(tokens[0].compareTo("sll") == 0){
                int amt = Integer.parseInt(tokens[3]);
                instr = "000000";
                instr += "00000";
                instr += toBinary(map.get(tokens[2]), 5);
                instr += toBinary(map.get(tokens[1]), 5);
                instr += toBinary(amt, 5);
                instr += "000000";
            }
            else if(tokens[0].compareTo("srl") == 0){
                int amt = Integer.parseInt(tokens[3]);
                instr = "000000";
                instr += "00000";
                instr += toBinary(map.get(tokens[2]), 5);
                instr += toBinary(map.get(tokens[1]), 5);
                instr += toBinary(amt, 5);
                instr += "000010";
            }
            else if(tokens[0].compareTo("sw") == 0){
                String[] addr = tokens[2].split("\\(");
                int offset = Integer.parseInt(addr[0]);
                instr = "101011";
                instr += toBinary(map.get(addr[1].substring(0, addr[1].length() - 1)), 5);
                instr += toBinary(map.get(tokens[1]), 5);
                instr += toBinary(offset, 16);
            }
            else if(tokens[0].compareTo("lw") == 0){
                String[] addr = tokens[2].split("\\(");
                int offset = Integer.parseInt(addr[0]);
                instr = "100011";
                instr += toBinary(map.get(addr[1].substring(0, addr[1].length() - 1)), 5);
                instr += toBinary(map.get(tokens[1]), 5);
                instr += toBinary(offset, 16);
            }
            else if(tokens[0].compareTo("j") == 0){
                int offset = Integer.parseInt(tokens[1]);
                instr = "000010";
                instr += toBinary(offset, 26);
            }
            else if(tokens[0].compareTo("beq") == 0){
                int offset = Integer.parseInt(tokens[3]);
                instr = "000100";
                instr += toBinary(map.get(tokens[1]), 5);
                instr += toBinary(map.get(tokens[2]), 5);
                instr += toBinary(offset, 16);
            }
            else if(tokens[0].compareTo("bne") == 0){
                int offset = Integer.parseInt(tokens[3]);
                instr = "000101";
                instr += toBinary(map.get(tokens[1]), 5);
                instr += toBinary(map.get(tokens[2]), 5);
                instr += toBinary(offset, 16);
            }
            else if(tokens[0].compareTo("blez") == 0){
                int offset = Integer.parseInt(tokens[3]);
                instr = "000110";
                instr += toBinary(map.get(tokens[1]), 5);
                instr += toBinary(map.get(tokens[2]), 5);
                instr += toBinary(offset, 16);
            }
            else if(tokens[0].compareTo("bgtz") == 0){
                int offset = Integer.parseInt(tokens[3]);
                instr = "000111";
                instr += toBinary(map.get(tokens[1]), 5);
                instr += toBinary(map.get(tokens[2]), 5);
                instr += toBinary(offset, 16);
            }
            else if(tokens[0].compareTo("jal") == 0){
                int offset = Integer.parseInt(tokens[1]);
                instr = "000011";
                instr += toBinary(offset, 26);
            }
            else if(tokens[0].compareTo("jr") == 0){
                instr = "000000";
                instr += toBinary(map.get(tokens[1]), 5);
                instr += "000000000000000001000";
            }
            else if(tokens[0].compareTo("lui") == 0){
                int val = Integer.parseInt(tokens[2]);
                instr = "00111100000";
                instr += toBinary(map.get(tokens[1]), 5);
                instr += toBinary(val, 16);
            }
            else if(tokens[0].compareTo("ori") == 0){
                int val = Integer.parseInt(tokens[3]);
                instr = "001101";
                instr += toBinary(map.get(tokens[2]), 5);
                instr += toBinary(map.get(tokens[1]), 5);
                instr += toBinary(val, 16);
            }
            else{
                System.out.println("Invalid program\t");
                System.out.println(tokens[0]);
            }
            prog[i] = instr;
            i++;
        }
        int j = 0;
        while(prog[j]!=null){
            bw.write(toDecimal(prog[j]) + "\n");
            j++;
        }

        br.close();
        bw.close();
    }
}
