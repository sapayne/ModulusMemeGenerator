import os
from typing import List
import sys

validYes = {"yes" : True,
			"y" : True,
			"true" : True,
			"1" : True,
			"no" : False,
			"n" : False,
			"false" : False,
			"0" : False}

def makeModulusMeme(file: str, startingNum: int, endingNum: int, beginningText: str, sentenceArray: List[str], endingText: str):
	
	useBeginningText = False
	endingNum += 1
	
	if os.path.exists(file):
		startOver: str = input('Do you wish to remake the file? (y/n)\n').lower()

		while startOver not in validYes:
			print("invalid answer to the question: ")
			startOver = input('Do you wish to remake the file? (y/n)\n').lower()

		if validYes[startOver]:
			open(file, "w").close()
			useBeginningText = True
		else:
			# good lord, what sin have I created
			with open(file, "r") as openFile:

				contents = openFile.readlines()

				for line in contents[-4:-3]:
					for word in line.split():
						digit = ""

						for ch in word:
							if ch.isdigit():
								digit += ch
						
						if len(digit) > 0:
							startingNum = max(startingNum, int(digit) + 1)

				openFile.close()	

			openFile = open(file, "w")
			openFile.writelines(contents[:-2])
			openFile.close()
	else:
		open(file, "x")
		useBeginningText = True

	with open(file, "a") as openFile:

		fileContentToAdd = ""

		if useBeginningText:
			#print("adding beginning text.")
			fileContentToAdd += beginningText + "\n"

		line: str

		if startingNum != endingNum:
			for i in range(startingNum, endingNum):
				if i % len(sentenceArray):
					fileContentToAdd += '\telif num == ' + str(i) + ':\n\t\tprint("odd")\n'
				else:
					fileContentToAdd += '\telif num == ' + str(i) + ':\n\t\tprint("even")\n'


		fileContentToAdd += endingText
		openFile.write(fileContentToAdd)
		openFile.close()


numOfCommands = len(sys.argv)

if numOfCommands == 2:
	makeModulusMeme('meme.py', 2, int(sys.argv[1]), 'def isEvenOrOdd(num: int):\n\tif num == 1:\n\t\tprint("odd")', ['\telif num == *:\n\t\tprint("even")\n','\telif num == *:\n\t\tprint("odd")\n'], 
				'\telse:\n\t\tprint("God there has to be an easier way.")')
else:
	makeModulusMeme('meme.py', 2, 25, 'def isEvenOrOdd(num: int):\n\tif num == 1:\n\t\tprint("odd")', ['\telif num == *:\n\t\tprint("even")\n','\telif num == *:\n\t\tprint("odd")\n'], 
				'\telse:\n\t\tprint("God there has to be an easier way.")')