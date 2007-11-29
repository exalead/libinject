#!/usr/bin/python

## @defgroup GraphLog Generate graph from logs
#
# This script parse logs from the output of the log action or the
# output of an injecthexdump on the result of a dump action. The log
# analyzed are used to generates graphes of the network activity. This
# graphes are grouped in a simple html page allowing a fast access to
# the result.
#
# Running this script is very simple:
# <pre>injectgraph [options] logfile1 [logfile2 [logfile3 ...]]</pre>
#
# Options are:
#   - m mask: set the host mask. Pattern are:
#     -  %%proto: protocole (tcp or udp)
#     -  %%lport: local port
#     -  %%addr:  remote ip address
#     -  %%rport: remote port
#   - h: print help
#   - b browser: open a browser after graph generation
#   - B: open the default browser after graph generation
#
# The default mask is %proto:%lport:[%addr]:%rport. Mask is useful to
# group remote port talking to the same local port (or vice-versa),
# protocoles...
#
# If a file is "-", the script will read stdin. A call the this script
# generate one timeline grouping the content of all the files. To get
# one timeline per file, you must call the script on each file separately.
#
# This script requires gnuplot to be installed on the computer.
#
# @sa ATT_Log, ATT_Dump, ReadDump
# @{

import sys, re, os, getopt

mask = "%proto:%lport:[%addr]:%rport"
min = 0
max = 0
data  = { }
gdata = { 'write': { }, 'read': { } }
browser = "firefox"
browsing = False

def buildGraph(name, filename, data):
  """Build the graph for the given data

  Generates a graph with p name written in p filename .png from p data.
  A data table. This table has two members data[write] and data[read] both of
  them being a table of dictionnary indexed by second having two members
  data[...][time][length] and data[...][time][packets]
  """
  global min
  global max
  r_length = { }
  r_packet = { }
  w_length = { }
  w_packet = { }
  datafile = open(filename + '_data', "w")
  cmdfile  = open(filename + '_cmd',  "w")
  cmdfile.write("""set term png small xFFFFFF size 800, 700
                   set output \"""" + filename + """.png\"
                   set xr [0:""" + str(max - min + 1) + """]
                   set multiplot layout 2,1 title \"""" + name + """ rate\"
                   set key outside left bottom horizontal Right
                   plot \"""" + filename + """_data\" using 1:2 title 'Receiving B/s """ + name + """' with boxes, \\
                        \"""" + filename + """_data\" using 1:3 title 'Receiving B/s (on last minute) """ + name + """' with lines, \\
                        \"""" + filename + """_data\" using 1:4 title 'Sending B/s """ + name + """' with boxes, \\
                        \"""" + filename + """_data\" using 1:5 title 'Sending B/s (on last minute) """ + name + """' with lines
                   plot \"""" + filename + """_data\" using 1:6 title 'Receiving calls/s """ + name + """' with boxes, \\
                        \"""" + filename + """_data\" using 1:7 title 'Receiving calls/s (on last minute) """ + name + """' with lines, \\
                        \"""" + filename + """_data\" using 1:8 title 'Sending calls/s """ + name + """' with boxes, \\
                        \"""" + filename + """_data\" using 1:9 title 'Sending calls/s (on last minute) """ + name + """' with lines
                   unset multiplot;\n""")
  for i in range(min, max + 1):
    pos = i%60
    try:
      r_length[pos] = data['read'][i]['length']
      r_packet[pos] = data['read'][i]['packets']
    except:
      r_length[pos] = 0
      r_packet[pos] = 0
    try:
      w_length[pos] = data['write'][i]['length']
      w_packet[pos] = data['write'][i]['packets']
    except:
      w_length[pos] = 0
      w_packet[pos] = 0
    datafile.write(str(i - min) + " " + str(r_length[pos]) + " " + str(sum(r_length.values()) / 60)
                                + " -" + str(w_length[pos]) + " -" + str(sum(w_length.values()) / 60)
                                + " " + str(r_packet[pos]) + " " + str(sum(r_packet.values()) / 60)
                                + " -" + str(w_packet[pos]) + " -" + str(sum(w_packet.values()) / 60)
                                + "\n")
  datafile.close()
  cmdfile.close()
  os.system("gnuplot " + filename + "_cmd")
  os.unlink(filename + '_data')
  os.unlink(filename + '_cmd')

def readFile(name):
  """Parse the content of a log file.

  This append the data parsed in the given file to the global data buffer data and gdata.
  The data extracted are the number of syscalls per second and the length of data per
  second.
  """
  global min
  global max
  global data
  global gdata
  global mask
  try:
    if name == "-":
      file = sys.stdin
      name = "stdin"
    else:
      file = open(name, "r")
  except:
    return False
  rex = re.compile(r"\[ts (\d+)\] \[line \d+\] (write|read) (tcp|udp):(\d+):\[(.+)\]:(\d+) length=(\d+)")
  for line in file.readlines():
    match = rex.search(line)
    if match:
      address = mask.replace('%proto', match.group(3)).replace('%lport', match.group(4)).replace('%addr', match.group(5)).replace('%rport', match.group(6))
      direct  = match.group(2)
      time    = int(match.group(1)) / 1000
      length  = int(match.group(7))
      if min == 0 or min > time:
        min = time
      if max == 0 or max < time:
        max = time
      try:
        data[address]
      except:
        data[address] = { 'write': { }, 'read': { } }
      try:
        data[address][direct][time]
      except:
        data[address][direct][time] = { 'length' : 0, 'packets' : 0 }
      try:
        gdata[direct][time]
      except:
        gdata[direct][time] = { 'length' : 0, 'packets' : 0 }
      data[address][direct][time]['length']  += length
      data[address][direct][time]['packets'] += 1
      gdata[direct][time]['length'] += length
      gdata[direct][time]['packets'] += 1
  file.close()
  return True

def buildOutput(filename):
  """Build the output of the function.

  The output contains the html page and a set of graphes: one per address
  (an address being a protocole + an ip + a port) and one "global view"
  that merge all the other graphes.
  """
  global data
  global gdata
  html = open(filename + ".html", "w")
  html.write(
  """<html>
      <body style="width: 820; margin-right: auto; margin-left: auto">
        <h1 id="index">Index</h1>
        <ul>
          <li><a href="#global">Global</a></li>""")
  imgs  = """<h1 id="global">Global</h1>
             <p><img src=\"""" + filename + """_global.png" alt="Global rate" /></p>
             <p style="font-size: smaller; text-align: right"><a href="#index">back to index</a></p>"""
  buildGraph("global", filename + "_global", gdata)
  for (address, datarate) in data.iteritems():
    filename = address.replace('[', '_').replace(']', '_').replace(':', '')
    html.write("<li><a href=\"#" + filename + "\">" + address + "</a></li>\n")
    imgs += """<h1 id=\"""" + filename + """\">""" + address + """</h1>
               <p><img src=\"""" + filename + """.png" alt="Rate for """ + address + """\" /></p>
               <p style="font-size: smaller; text-align: right"><a href="#index">back to index</a></p>"""
    buildGraph(address, filename, datarate)
  html.write("</ul>" + imgs + "</body></html>")
  html.close()

def usage():
  """Print usage informations"""
  print "Usage: " + sys.argv[0] + " [-h] [-m mask] [-b browser|-B] logfile1 [logfile2 [logfile3 ...]]"
  print "   -h print this help"
  print "   -m mask: set the address mask. The default mask is %proto:%lproto:[%addr]:%rport"
  print "            in order to group all accesses from the same proto, just set -m \"%proto\""
  print "            to group by remote host, set -m \"%addr\"..."
  print "         Patterns:"
  print "           %proto: protocole (tcp or udp)"
  print "           %lport: local port"
  print "           %addr:  remote ip address"
  print "           %rport: remote port"
  print "   -b browser: activate browsing with the given browser"
  print "   -B: activate browsing with the default browser (firefox)"
  print "   logfile is the output of the log action or of the processing of a dump by injecthexdump -s"
  print "   \"-\" means stdin"


try:
  opt, args = getopt.getopt(sys.argv[1:], "m:hb:B")
except:
  usage()
  sys.exit(1)

for o, a in opt:
  if o == '-m':
    mask = a
  elif o == '-h':
    usage()
    sys.exit(1)
  elif o == '-B':
    browsing = True
  elif o == '-b':
    if len(a) > 0:
      browser = a
    browsing = True

ok = False
for filename in args:
  if readFile(filename):
    ok = True
if ok:
  if filename == "-":
    filename = "stdin"
  buildOutput(filename)
  if browsing:
    os.system(browser + " " + filename + ".html")
else:
  usage()
  sys.exit(1)

## @}
