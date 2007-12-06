#!/usr/bin/python

## @defgroup ForgeDump Build a dump from user input
#
# This can be used to build a dump file from the user input. The file generated
# can be read using injecthexdump and can be used to source the 'replay' action
# of inject.
#
# @sa ATT_Replay, ReadDump
# @{

import os, sys, getopt, re, string
from array import *
from struct import *

commandrex = re.compile(r"(text|hexa)(\((.*)\))$")
outfile    = "-"
type       = 1

def readCommand(file):
  """Read a command from the user and then call the apropriate handler"""
  global commandrex

  while True:
    if file.isatty():
      sys.stdout.write("> ")
      sys.stdout.flush()
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
  """Handler for text input

  Take every single line of the input, change the endofline to the given one
  (\\n by default), and write the whole to the dump. The input delimiter is the
  empty line: when an empty line is found, input stop, and the program wait for
  the next command. Empty lines are not included to the output until it is the
  first line of the command.

  Allowed params:
    - eol: end of line characters in output (default \\n)
  """
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

def hexaHandler(file, params):
  """Handler for hexadecimal input

  This recognize both suites of bytes (b01020304050938759320Fa0ba), or 0x prefixed
  integers. While raw suites are read byte per byte, integers are stored using the
  given endianess.

  End of input is detected at the first empty line. Non hexadecimal characters are
  non significants, but each group must always be a set of bytes (each bytes being
  2 hexadecimal charaters)

  Allowed parameters:
    - endianess: is ce, be or le (default ce)
  """
  args = mergeArguments(params, { "endianess": "ce" })
  input  = ""
  output = ""
  while True:
    line = file.readline().rstrip("\r\n")
    input += line
    if len(line) == 0:
      break;
  input = re.split("(?i)[^0-9a-fx]", input.lower())
  integerRex = re.compile("0x([0-9a-f]+)")
  if args["endianess"] == "be":
    pattern = ">"
  elif args["endianess"] == "le":
    pattern = "<"
  else:
    pattern = "="
  for batch in input:
    match = integerRex.search(batch)
    if match:
      i = string.atoi(match.group(1), base=16)
      size = len(match.group(1))
      if size == 2:
        symbol = 'B'
      elif size <= 4:
        symbol = 'H'
      elif size <= 8:
        symbol = 'I'
      else:
        symbol = 'Q'
      output += pack(pattern + symbol, i)
    else:
      if len(batch) % 2 != 0:
        print "Invalid batch " + batch
        break
      a = array('B')
      for i in range(0, (len(batch) / 2)):
        a.append(string.atoi(batch[2*i:2*i+2], base=16))
      output += a.tostring()
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
  print "Command:", sys.argv[0], "[-hrv] [-o file] file"
  print "  -h: show this help"
  print "  -r: in dump, data is considered has beeing read (default)"
  print "  -w: in dump, data is considered has beeing writen"
  print "  -o file: write output to file instead of stdout"
  print ""
  print "The program takes it input from stdin in the given format"
  print "    command(params)"
  print "    data..."
  print ""
  print "Params are given as a comma separated list of key=value list (spaces are"
  print "significant, so you should avoid them). The format for the data depends of the"
  print "command. Currently allowed commands are:"
  print "  - text(eol=.*): take ascii text"
  print "      eg: text(eol=\\r\\n) = take text and set line separator to \\r\\n"
  print "          text(eol=)       = take text and remove end of lines"
  print "      data finished when an empty line is met. Empty lines are ignored until they"
  print "      are not at the beginning of the data"
  print "  - hexa(endianess=ce|be|le): take hexadecimal raw data"
  print "      eg: hexa()"
  print "          01020f03 0x12345678 98a83ff794"
  print "      data finished when an empty line is met. You a define different kinds of"
  print "      data"
  print "      => integers 0x[0-9a-f]+: are stored has complete integer values (eg 1, 2,"
  print "         4 or 8 bytes, depending on the length of the string) and the storage is"
  print "         endianess dependent."
  print "      => flow ([0-9a-f][0-9a-f])+: this is interpreted as a raw suite of bytes"

try:
  opt, args = getopt.getopt(sys.argv[1:], "hrwo:")
except:
  usage()
  sys.exit(1)

for o, a in opt:
  if o == '-h':
    usage()
    sys.exit()
  elif o == '-o':
    outfile = a
  elif o == '-r':
    type = 1
  elif o == '-w':
    type = 2

if outfile == "-":
  dest = sys.stdout
else:
  dest = open(outfile, "w")

while True:
  data = readCommand(sys.stdin)
  if data == None:
    break
  writeEntry(dest, 2, data)

if outfile != "-":
  dest.close()

## @}
