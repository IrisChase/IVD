#!/bin/bash

# Copyright 2020 Iris Chase

# Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

shopt -s nullglob

compiler="./ivdserializingcompiler"

filesTested=0
filesPassed=0
filesFailed=0
filesSkipped=0

function printRed()
{
	printf "\e[97m\e[41m$1"
}

function printGreen()
{
	printf "\e[92m$1"
}


function runForDirectory()
{
	echo -e "\e[93mIn \"$1\":"
	allIVDfiles=$PWD/$1/*.ivd

	serialPath=$PWD/$1/"ivdserial"
	mkdir -p $serialPath

	for f in $allIVDfiles
	do
		fileName=$(basename $f)
		serialFile="$serialPath/$fileName"serial

		if test -f "$serialFile"
		then
			 theDiff=$(diff $serialFile <($compiler $f))
			 ((filesTested++))

			 if [ -z "$theDiff" ]
			 then
				((filesPassed++))
				printGreen "PASS"
			 else
				((filesFailed++))
				printRed "FAIL"
			 fi

		echo -e ": $fileName\e[0m"

		else
			echo -e "\e[33mSerial file not found for: $fileName"
			((filesSkipped++))
		fi


	done
}

runForDirectory "valid"
runForDirectory "errors"

printf "\e[0m\n"
echo -e "\e[33m$filesSkipped SKIPPED"
echo -e "\e[35m$filesTested TESTED"


if [[ $filesPassed -ne 0  ]]
then
	printGreen "$(($filesPassed * 100 / $filesTested))%% ($filesPassed) PASSED\e[0m\n"
fi

if [[ $filesFailed -ne 0 ]]
then
	printRed "$(($filesFailed * 100 / $filesTested))%% ($filesFailed) FAILED\e[0m\n"
fi
