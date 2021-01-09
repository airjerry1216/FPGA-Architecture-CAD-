import sys
from sys import exit
import random
from decimal import Decimal
class primary_input:
	def __init__(self):
		self.id = ""
		self.x = 0
		self.y = 0
class primary_output:
	def __init__(self):
		self.id = ""
		self.x = 0
		self.y = 0
class Instance:
	def __init__(self):
		self.master = ""
		self.id = ""
		self.num_input = 0
		self.input_signal = []
		self.output_signal = ""
		self.x = 0
		self.y = 0
class Net:
	def __init__(self):
		self.id = ""
		self.source = ""
		self.sinks = []
	def print_Net(self):
		print("id "+ self.id +", source is " + self.source, end = "")
		print(" and sinks are ", end = "")
		for each_sink in self.sinks:
			print(each_sink, end = " ")
		print()

net_list = {}
PI = {}
PO = {}
infomation_file = sys.argv[1]
netlist_file = sys.argv[2]
placement_file = sys.argv[3]
CLB_R = 0
CLB_C = 0
num_PI = 0
num_PO = 0
num_LUT = 0
num_FF = 0
inst_list = {}
def parse_info():
	with open(infomation_file,"r") as f:
		line_list = f.read().splitlines()
		for current_line in range(0, len(line_list)):
			temp = list(filter(("").__ne__, line_list[current_line].split(" ")))
			if temp[0] == "CLB_Dim":
				global CLB_C
				CLB_C = int(temp[1])
				global CLB_R
				CLB_R = int(temp[2])
			elif temp[0] == "Num_PI":
				global num_PI
				num_PI = int(temp[1])
				for i in range(0, num_PI):
					current_line = current_line + 1
					pi = primary_input()
					temp = list(filter(("").__ne__, line_list[current_line].split(" ")))
					pi.id = temp[0]
					pi.x = float(temp[1])
					pi.y = float(temp[2])
					PI[temp[0]] = pi
			elif temp[0] == "Num_PO":
				global num_PO
				num_PO = int(temp[1])
				for i in range(0, num_PO):
					current_line = current_line + 1
					po = primary_output()
					temp = list(filter(("").__ne__, line_list[current_line].split(" ")))
					po.id = temp[0]
					po.x = float(temp[1])
					po.y = float(temp[2])
					PO[temp[0]] = po
			elif temp[0] == "Num_Inst":
				global num_LUT
				num_LUT = int(temp[1])
				global num_FF
				num_FF = int(temp[2])
				for i in range(0, num_LUT):
					current_line = current_line + 1
					temp = list(filter(("").__ne__, line_list[current_line].split(" ")))
					lut = Instance()
					lut.id = temp[0]
					inst_list[lut.id] = lut
				for i in range(0, num_FF):
					current_line = current_line + 1
					temp = list(filter(("").__ne__, line_list[current_line].split(" ")))
					FF = Instance()
					FF.id = temp[0]
					inst_list[FF.id] = FF



def parse_placement():
	with open(placement_file,"r") as f:
		inst_count = 0
		line_list = f.read().splitlines()
		for current_line in range(0, len(line_list)):
			temp = list(filter(("").__ne__, line_list[current_line].split(" ")))
			if temp[0] in inst_list:
				inst_count = inst_count + 1
				inst_list[temp[0]].x = float(temp[1])
				inst_list[temp[0]].y = float(temp[2])
				if Decimal(temp[1]) % Decimal('1.0') != 0 or Decimal(temp[2]) % Decimal('1.0') != 0 :
					print("There is at least one instance which doesn't align on CLB locations")
					exit(0) 
				if inst_list[temp[0]].x < 1 or inst_list[temp[0]].x > CLB_C or\
				 	inst_list[temp[0]].y < 1 or inst_list[temp[0]].y > CLB_R:
				 	print("There is at least one instance which doesn't align on CLB locations")
				 	exit(0) 
				
				if temp[0][0] == "L" and inst_count > num_LUT:
					print("Error placement")
				if temp[0][0] == "F" and inst_count <= num_LUT:
					print("Error placement")
				
			else:
				print("There is at least one non-existing instance ")
				#print("There is at least one non-existing instance", + temp[0]+", placed on FPGA")
				exit(0)
		if inst_count != num_LUT + num_FF:
			#print(inst_count, num_LUT + num_FF)
			print("# your placed instance mismatches with sum of # LUTs and # FFs")
			#print("You didn't place all instances on FPGA")
			exit(0)

def read_net():
	with open(netlist_file,"r") as f:
		line_list = f.read().splitlines()
		for current_line in range(1, len(line_list)):
			temp = list(filter(("").__ne__, line_list[current_line].split(" ")))
			net = Net()
			net.id = temp[0]
			net.source = temp[1]
			for s in range(2, len(temp)):
				net.sinks.append(temp[s])
			net_list[net.id] = net
		total_HPWL = 0
		for each_key in net_list.keys():
			max_X = -9999.0
			max_Y = -9999.0
			min_X = 9999.0
			min_Y = 9999.0
			#print(each_key, net_list[each_key].source)
			if net_list[each_key].source in PI.keys():
				if PI[net_list[each_key].source].x > max_X:
					max_X = PI[net_list[each_key].source].x
				if PI[net_list[each_key].source].x < min_X:
					min_X = PI[net_list[each_key].source].x
				if PI[net_list[each_key].source].y > max_Y:
					max_Y = PI[net_list[each_key].source].y
				if PI[net_list[each_key].source].y < min_Y:
					min_Y = PI[net_list[each_key].source].y
			elif net_list[each_key].source in PO.keys():
				if PO[net_list[each_key].source].x > max_X:
					max_X = PO[net_list[each_key].source].x
				if PO[net_list[each_key].source].x < min_X:
					min_X = PO[net_list[each_key].source].x
				if PO[net_list[each_key].source].y > max_Y:
					max_Y = PO[net_list[each_key].source].y
				if PO[net_list[each_key].source].y < min_Y:
					min_Y = PO[net_list[each_key].source].y
			elif net_list[each_key].source in inst_list.keys():
				if inst_list[net_list[each_key].source].x > max_X:
					max_X = inst_list[net_list[each_key].source].x
				if inst_list[net_list[each_key].source].x < min_X:
					min_X = inst_list[net_list[each_key].source].x
				if inst_list[net_list[each_key].source].y > max_Y:
					max_Y = inst_list[net_list[each_key].source].y
				if inst_list[net_list[each_key].source].y < min_Y:
					min_Y = inst_list[net_list[each_key].source].y
			#print("    Sinks are ", end = "")
			for each_sink in  net_list[each_key].sinks:
				#print(each_sink, end = " ")
				if each_sink in PI.keys():
					if PI[each_sink].x > max_X:
						max_X = PI[each_sink].x
					if PI[each_sink].x < min_X:
						min_X = PI[each_sink].x
					if PI[each_sink].y > max_Y:
						max_Y = PI[each_sink].y
					if PI[each_sink].y < min_Y:
						min_Y = PI[each_sink].y
				elif each_sink in PO.keys():
					if PO[each_sink].x > max_X:
						max_X = PO[each_sink].x
					if PO[each_sink].x < min_X:
						min_X = PO[each_sink].x
					if PO[each_sink].y > max_Y:
						max_Y = PO[each_sink].y
					if PO[each_sink].y < min_Y:
						min_Y = PO[each_sink].y
				elif each_sink in inst_list.keys():
					if inst_list[each_sink].x > max_X:
						max_X = inst_list[each_sink].x
					if inst_list[each_sink].x < min_X:
						min_X = inst_list[each_sink].x
					if inst_list[each_sink].y > max_Y:
						max_Y = inst_list[each_sink].y
					if inst_list[each_sink].y < min_Y:
						min_Y = inst_list[each_sink].y
			#print()
			net_HPWL = (max_X - min_X) + (max_Y - min_Y)
			total_HPWL = total_HPWL + net_HPWL
			#print("HPWL is " + str(net_HPWL))
		print(placement_file+ ", Total HPWL is " + str(total_HPWL))

		
def LUT_legality():
	used_LUT_table = {}
	LUT_coordinate_list = []
	for x in range(1, CLB_C+1):
		for y in range(1, CLB_R+1):
			LUT_coordinate_list.append((x,y))
	for each_coordinate in LUT_coordinate_list:
		used_LUT_table[each_coordinate] = 0
	for index in range(0, num_LUT):
		each_key = list(inst_list.keys())[index]
		
		used_coordinate = (inst_list[each_key].x, inst_list[each_key].y)
		used_LUT_table[used_coordinate] = used_LUT_table[used_coordinate] + 1
		if used_LUT_table[used_coordinate] >= 3:
			#print("Error placement result")
			print("LUTs exceed the limit")
			exit(0)
		
def FF_legality():
	used_FF_table = {}
	FF_coordinate_list = []
	for x in range(1, CLB_C+1):
		for y in range(1, CLB_R+1):
			FF_coordinate_list.append((x,y))
	for each_coordinate in FF_coordinate_list:
		used_FF_table[each_coordinate] = 0
	for index in range(num_LUT, num_LUT + num_FF):
		each_key = list(inst_list.keys())[index]
		used_coordinate = (inst_list[each_key].x, inst_list[each_key].y)
		used_FF_table[used_coordinate] = used_FF_table[used_coordinate] + 1
		if used_FF_table[used_coordinate] >= 3:
			print("FFs exceed the limit")
			exit(0)
		
parse_info()
parse_placement()
LUT_legality()
FF_legality()
read_net()




