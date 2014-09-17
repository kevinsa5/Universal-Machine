#!/usr/bin/python

# see http://www.boundvariable.org/um-spec.txt

import argparse
import sys

parser = argparse.ArgumentParser()
parser.add_argument("input")
args = parser.parse_args()

reg = [0 for i in range(1,9)]
arrays = {0:[]}
# program counter
pc = 0
serial = 1
print "loading"
with open(args.input, 'rb') as f:
	lis = []
	while True:
		platter = f.read(4)
		if not platter: break
		if len(platter) != 4:
			print "Read a non 32-bit platter!"
			sys.exit()
		platter =  ((ord(platter[0]) << 24)+
					(ord(platter[1]) << 16)+
					(ord(platter[2]) << 8) +
					(ord(platter[3]))
					)
		lis.append(platter)
arrays[0] = lis

print "executing"
while True:
	try:
		platter = arrays[0][pc]
		pc += 1
	except IndexError as e:
		print "out of bounds"
		sys.exit()
	'''
                              A     C
                              |     |
                              vvv   vvv                    
      .--------------------------------.
      |VUTSRQPONMLKJIHGFEDCBA9876543210|
      `--------------------------------'
       ^^^^                      ^^^
       |                         |
       operator number           B
	'''
	opcode = (platter >> 28)
	A = (platter >> 6) & 0b111
	B = (platter >> 3) & 0b111
	C = platter & 0b111
	
	if not True:
		print "platter:{4}\topcode:{0}\tA:{1}\tB:{2}\tC:{3}\tpc:{5}".format(opcode, A,B,C,hex(platter),pc-1)
		print reg
		print arrays.keys()
		raw_input()
	
	if opcode == 0:
		if reg[C] != 0:
			reg[A] = reg[B]
	elif opcode == 1:
		reg[A] = arrays[reg[B]][reg[C]]
	elif opcode == 2:
		if reg[B] > len(arrays[reg[A]]):
			print B
			print reg[B]
			print arrays[reg[A]]
		arrays[reg[A]][reg[B]] = reg[C]
	elif opcode == 3:
		reg[A] = (reg[B] + reg[C]) % 2**32
	elif opcode == 4:
		reg[A] = (reg[B] * reg[C]) % 2**32
	elif opcode == 5:
		if reg[C] == 0:
			print "can't divide by zero!"
			sys.exit()
		reg[A] = (reg[B] / reg[C])
	elif opcode == 6:
		s = ""
		sb = bin(reg[B])[2:]
		sc = bin(reg[C])[2:]
		sb = '0'*(32-len(sb)) + sb
		sc = '0'*(32-len(sc)) + sc
		
		for b,c in zip(sb, sc) :
			s = s + ("1" if b == "0" or c == "0" else "0")
		reg[A] = int(s,2)
	elif opcode == 7:
		break
	elif opcode == 8:
		arrays[serial] = [0 for i in range(reg[C])]
		reg[B] = serial
		serial += 1
	elif opcode == 9:
		if reg[C] == 0:
			print "can't remove the 0 array!"
			sys.exit()
		arrays.pop(reg[C])
	elif opcode == 10:
		if 0 <= reg[C] <= 255:
			sys.stdout.write(chr(reg[C]))
		else:
			print "outside printable range: %s" % reg[C]
			sys.exit()
	elif opcode == 11:
		try:
			x = raw_input('input:')
			if 0 <= ord(x) <= 255:
				reg[C] = ord(x)
		except KeyboardInterrupt:
			reg[C] = 0xFFFFFFFF
	elif opcode == 12:
		if reg[B] not in arrays.keys():
			print "no such array: %s" % reg[B]
			sys.exit()
		if reg[B] != 0:
			arrays[0] = list(arrays[reg[B]])
		pc = reg[C]
	elif opcode == 13:
		A = (platter >> 25) & 0b111
		val = platter & ((1<<25) - 1)
		reg[A] = val
	else:
		print "invalid opcode: %s" % opcode
		sys.exit()
	
