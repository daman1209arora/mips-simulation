all: 
	javac -d tests/ util/TestGenerator.java
	cp tests/TestGenerator.class bin/TestGenerator.class
	chmod +x tests/checker.py
	g++ -c -I./src/ src/instruction.cpp -o obj/instruction.o
	g++ -c -I./src/ src/proc_sim1.cpp -o obj/proc_sim1.o
	g++ -o bin/proc_sim1 obj/instruction.o obj/proc_sim1.o
	g++ -c -I./src/ src/proc_sim2.cpp -o obj/proc_sim2.o
	g++ -o bin/proc_sim2 obj/instruction.o obj/proc_sim2.o
	g++ -c -I./src/ src/proc_sim3.cpp -o obj/proc_sim3.o
	g++ -o bin/proc_sim3 obj/instruction.o obj/proc_sim3.o

clean:  
	rm obj/*
	rm bin/*
	rm tests/*.class

test:
	cd tests && $(MAKE)