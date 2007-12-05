#!/usr/bin/python

## @defgroup ForgeDump Build a dump from user input
#
# This can be used to build a dump file from the user input. The file generated
# can be read using injecthexdump and can be used to source the 'replay' action
# of inject.
#
# @sa ATT_Replay, ReadDump
# @{

import os, sys, getopt, re
from array import *
from struct import *

commandrex = re.compile(r"(text|hexa)(\((.*)\))$")
outfile    = "-"

def readCommand(file):
  """Read a command from the user and then call the apropriate handler"""
  global commandrex

  while True:
    line = file.readline()
    if len(line) == 0:
      return None
    match = commandrex.search(line)
    if match:
      command = match.group(1)
      args = match.group(3).split(",")
      params = { }
      for arg in args:
        kv = arg.split('=')
        if len(kv) == 2:
          params[kv[0]] = kv[1]
      return callCommand(command, file, params)
    elif len(line.strip()) > 0:
      print "Unknown command"

def callCommand(command, file, params):
  """Call the apropriate handler"""
  if command == "text":
    return textHandler(file, params)
  elif command == "hexa":
    return hexaHandler(file, params)

def mergeArguments(given, default):
  """Merge given value to program default values"""
  val = { }
  for (name, value) in default.iteritems():
    try:
      val[name] = given[name]
    except:
      val[name] = value
  return val

def textHandler(file, params):
  args = mergeArguments(params, { "eol": "\\n" })
  args["eol"] = args["eol"].replace("\\n", "\n").replace("\\r", "\r")

  first = True
  output = ""
  while True:
    line = file.readline().rstrip("\r\n")
    empty = (len(line) == 0)
    line += args["eol"]
    if (not empty) or first:
      output += line
    if empty:
      break
    first = False
  return output

def writeEntry(file, type, data):
  """Write a dump entry"""
  a = array('B')
  a.fromstring(data)
  file.write(pack('=QbbHBBBBHbi', 0, type, 1, 0, 0, 0, 0, 0, 0, 1, len(a)))
  a.tofile(file)
  return True

def usage():
  """Print usage instructions"""
  print "Command:", sys.argv[0], "[-h] [-o file] file"
  print "  -h: show this help"
  print "  -o file: write output to file instead of stdout"

try:
  opt, args = getopt.getopt(sys.argv[1:], "ho:")
except:
  usage()
  sys.exit(1)

for o, a in opt:
  if o == '-h':
    usage()
    sys.exit()
  elif o == '-o':
    outfile = a

if outfile == "-":
  dest = sys.stdout
else:
  dest = open(outfile, "w")

while True:
  data = readCommand(sys.stdin)
  if data == None:
    break
  writeEntry(dest, 1, data)

if outfile != "-":
  dest.close()

# @}
