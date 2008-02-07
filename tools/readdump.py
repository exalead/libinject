#!/usr/bin/python

## @defgroup ReadDump Read the content of a dump
#
# Read the content of dump action and display it in a human understanble way.
# The output combines the action description in the log action format (parsable
# using injectgraph command) and a hexdump -C like format (this means hexa +
# ASCII output, 16 bytes per line with offset at the beginning of the line).
#
# The script usage is very simple:
# <pre>injecthexdump [-sh] logfile</pre>
#
# - '-h' show the usage instructions
# - '-s' switch the simplified mode, without data dump. In this mode, only the
#        action description will be printed.
# - '-r file' read the dump and write it in the given file after inversing directions
#        (writing -> reading, reading -> writing)
#
# You can generate a graph of network activity using injecthexdump and injectgraph:
# <pre>injecthexdump -s logfile | injectgraph -</pre>
#
# @sa ATT_Log, ATT_Dump, GraphLog
# @{

import os, sys, getopt
from struct import *
from array import *

revert     = None
simplified = False

def showEntry(time, type, proto, lport, a1, a2, a3, a4, port, success, length, data):
  """Display the given entry

  Print the data to stdout according the informations given and the requested mode
  """
  if proto == 1:
    addr = "tcp:"
  else:
    addr = "udp:"
  addr += str(lport) + ":[" + str(a4) + "." + str(a3) + "." + str(a2) + "." + str(a1) + "]:" + str(port)
  if type == 1:
    text = "read"
  elif type == 2:
    text = "write"
  elif type == 4:
    text = "connect"
  elif type == 8:
    text = "close"
  else:
    text = "unknown-action"
  tlen = ""
  if success == -1:
    tlen = " error=\"" + os.strerror(length) + "\""
  else:
    if type <= 2:
      tlen = " length=" + str(length)
    if success == 0:
      tlen += " callnotperformed"
  print "[ts " + str(time) + "] [line 0] " + text + " " + addr + tlen
  if data != None and not simplified:
    pos = 0
    while len(data) > 0:
      text = "%(pos)08x " % { 'pos': pos }
      bout = "|"
      for i in range(0, 15):
        if i%8 == 0:
          text += ' '
        try:
          val  = data.pop(0)
        except:
          text += '   '
          bout += ' '
          continue
        text += "%(val)02x " % { 'val': val }
        if val >= 32 and val <= 126:
          bout += '%(val)c' % { 'val': val }
        else:
          bout += '.'
      print text + ' ' + bout
      pos += 16
    print ""
  return True

def readEntry(file):
  """Read an entry from the file

  Parse a file and extract data, the gives these data to readEntry to be displaid.
  """
  str = file.read(19)
  time, type, proto, lport, a1, a2, a3, a4, port, success = unpack('=QbbHBBBBHb', str)
  length = 0
  data = None
  if ( type != 4 and type != 8 ) or success == -1: # not connection, so w
    length = file.read(4)
    length = unpack('i', length)[0]
  if length > 0 and success != -1:
    data = array('B')
    data.fromfile(file, length)
    data = data.tolist()
  return (time, type, proto, lport, a1, a2, a3, a4, port, success, length, data)

def writeEntry(file, time, type, proto, lport, a1, a2, a3, a4, port, success, length, data):
  """Write a binary entry to a file"""
  str = pack('=QbbHBBBBHb', time, type, proto, lport, a1, a2, a3, a4, port, success)
  if ( type != 4 and type != 8 ) or success == -1:
    str += pack('i', length)
  file.write(str)
  if data != None:
    a = array('B')
    a.fromlist(data)
    a.tofile(file)
  return True

def usage():
  """Show usage instructions"""
  print "Command:", sys.argv[0], "[-hs] [-r dest] file"
  print "  -h: show this help"
  print "  -s: simplified output (without packet content)"
  print "  -r dest: revert data direction and write the result in dest"

try:
  opt, args = getopt.getopt(sys.argv[1:], 'shr:')
except:
  usage()
  sys.exit(1)

for o, a in opt:
  if o == '-h':
    usage()
    sys.exit(0)
  elif o == '-s':
    simplified = True
  elif o == '-r':
    revert = a
try:
  file = open(args[0], "r")
except:
  usage()
  sys.exit(1)

if revert != None:
  dest = open(revert, "w")

while True:
  try:
    time, type, proto, lport, a1, a2, a3, a4, port, success, length, data = readEntry(file)
  except:
    break
  if revert == None:
    showEntry(time, type, proto, lport, a1, a2, a3, a4, port, success, length, data)
  else:
    if type == 1:
      type = 2
    elif type == 2:
      type = 1
    writeEntry(dest, time, type, proto, lport, a1, a2, a3, a4, port, success, length, data)

## @}
