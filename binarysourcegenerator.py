#!/usr/bin/env python3

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


from sys import argv

blah, inputBinaryFile, includePath, variableName, outputName = argv

output = []
with open(inputBinaryFile, "rb") as f:
    byte = f.read(1)
    while byte:
        output.append("0x" + byte.hex())
        byte = f.read(1)

with open(outputName, "w") as f:
    f.write("//This file was generated at build time.\n")
    f.write("#include \"%s\"\n" % includePath)
    f.write("const unsigned char %s[] = {\n" % variableName)
    
    length = len(output)
    i = 1;
    for char in output:
        f.write(char)
        if i != length:
            f.write(",")
            
        if i and not i % 12:
            f.write("\n")
        elif i != length:
            f.write(" ")
            
        i += 1
    
    f.write("};\n")
    f.write("const int %sSize = %d;\n" % (variableName, len(output)))
