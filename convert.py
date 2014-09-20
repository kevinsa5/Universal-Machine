#!/usr/bin/python

funcs = ["" for i in range(0,14)]

with open("uvm.c","r") as input, open("uvm-prof.c","w") as output:
	for i in range(0,14):
		output.write("void f{0}();\n".format(i))
	line = input.readline()
	while line != "":
		if 'case' not in line:
			output.write(line)
			line = input.readline()
		else:
			output.write(line)
			num = int(line.split(":")[0].split(" ")[-1])
			output.write("\t"*(line.count("\t",0,line.index("case"))+1))
			output.write("f"+str(num)+"(); break;\n")
			line = input.readline()
			while "case" not in line and 'default' not in line and line != "":
				if 'break' not in line: funcs[num] += line
				line = input.readline()
	for i in range(len(funcs)):
		output.write("void f{}()".format(i) + "{\n")
		output.write(funcs[i])
		output.write("}\n");
