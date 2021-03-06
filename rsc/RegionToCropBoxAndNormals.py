#!/usr/bin/python

'''
Copyright (c) 2016, Aumann Florian, Borella Jocelyn, Heller Florian, Meissner Pascal, Schleicher Ralf, Stoeckle Patrick, Stroh Daniel, Trautmann Jeremias, Walter Milena, Wittenbeck Valerij
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

4. The use is explicitly not permitted to any application which deliberately try to kill or do harm to any living creature.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
'''

import csv
import sys, getopt

class Cropbox:

    def __init__(self):
       self.Region = []
       self.Normals = []

    def __repr__(self):
      return  "Region: " + str(self.Region) + "\nNormals: " + str(self.Normals)

def get_max_point(point):
    return [(point[3]-point[0]),(point[4]-point[1]),(point[5]-point[2])]

def get_translation(point):
    return [point[0],point[1],point[2]]

def main():

    Cropboxes = []
    i = 0

    inputfile = ''
    outputfile = ''
    try:
        opts, args = getopt.getopt(sys.argv[1:],"hi:o:",["ifile=","ofile="])
    except getopt.error:
        print 'RegionToCropBox.py -i <inputfile>.csv -o <outputfile>.xml'
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print 'RegionToCropBox.py -i <inputfile>.csv -o <outputfile>.xml'
            sys.exit()
        elif opt in ("-i", "--ifile"):
            inputfile = arg
        elif opt in ("-o", "--ofile"):
            outputfile = arg
    print 'Input file is "', inputfile
    print 'Output file is "', outputfile

    with open(inputfile, 'rb') as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',', quotechar='|')
        cropbox = Cropbox()
        isFirst = True
        isNormalsRead = False
        for row in spamreader:
            if len(row) != 0  and str(row[0]).startswith('#'):
                continue
            if len(row) == 0 and isFirst is True:
                continue
            isFirst = False
            if len(row) == 0:
                isNormalsRead = False
                Cropboxes.append(cropbox)
                cropbox = Cropbox()
                continue

            if isNormalsRead is False:
                region = []
                for elt in row:
                    region.append(float(elt))
                cropbox.Region = region
                isNormalsRead = True
            else:
                normal = []
                for elt in row:
                    normal.append(float(elt))
                cropbox.Normals.append(normal)

    returnString = '<CropBoxList name="Map">'
    for c in Cropboxes:
        returnString += '<CropBox name="Region_'+str(i)+'">'
        returnString += '<min_pt x="0" y="0" z="0"/>'
        max_point = get_max_point(c.Region)
        if max_point[0] < 0.0 or max_point[1] < 0.0 or max_point[2] < 0.0:
            print "ABORTING: Max values must be positive for PCL crop box filtering."
            return
        returnString += '<max_pt x="'+str(max_point[0])+'" y="'+str(max_point[1])+'" z="'+str(max_point[2])+'"/>'
        returnString += '<rotation x="0" y="0" z="0"/>'
        translation = get_translation(c.Region)
        returnString += ' <translation x="'+str(translation[0])+'" y="'+str(translation[1])+'" z="'+str(translation[2])+'"/>'
        j = 0
        for n in c.Normals:
            returnString += ' <normal_' + str(j) + ' x="'+str(n[0])+'" y="'+str(n[1])+'" z="'+str(n[2])+'"/>'
            j += 1
        returnString += '</CropBox>'
        i += 1

    returnString += '</CropBoxList>'

    f = open(outputfile, 'r+')
    f.truncate()
    f.write(returnString)
    f.close()

    print returnString

if __name__ == '__main__':
    main()
