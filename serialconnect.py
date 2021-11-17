import argparse
import serial
import threading
from time import sleep

# Convert hex string to byte stream
def hexstr2byte(hexstream):

  if (len(hexstream)%2) != 0:
    print('Error converting hex string to bytes: odd number of chars')
    print('-> {}'.format(hexstream))
    return b''

  bytestream = b''
  for i in range(len(hexstream)):
    if (i%2)==0:
      curr_int = int(hexstream[i:(i+2)],16)
      bytestream = bytestream + (curr_int).to_bytes(1,'big')

  return bytestream

# Parser
parser = argparse.ArgumentParser()

parser.add_argument("srcfile", help="Source text file data to send (expressed in hex")
parser.add_argument("-o","--outputfile", help="Output file: UART Logs")
parser.add_argument("-d","--device", help="Serial device")
parser.add_argument("-t","--timedelay", help="Time delay (in secs)  before program termination")

args = parser.parse_args()

srcfile = args.srcfile

if args.outputfile:
  outputfile = args.outputfile
else:
  outputfile = 'serial_output.log'

if args.device:
  serdev = args.device
else:
  serdev = '/dev/ttyUSB1'

if args.timedelay:
  timedelay = int(args.timedelay)
else:
  timedelay = 5

# Initialize serial
baud = 9600 
timeout = 1
try:
  ser = serial.Serial(port=serdev, baudrate=baud, timeout=timeout)
  print('Connected to {}'.format(serdev))
except:
  print('Cannot find serial device')
  quit()

# Thread for logging data
def uart_rx(ser,outputfile):
  while True:
    rxdata = ser.readline().decode()
    if rxdata != '':
      fo = open(outputfile,'a')
      print('Rx: {}'.format(rxdata.strip()))
      fo.write(rxdata)
      fo.close()
    global stop_threads
    if stop_threads:
      break

stop_threads = False
thread = threading.Thread(target=uart_rx, args=(ser,outputfile,))
thread.start()

# Open source file for commands
fi = open(srcfile,'r')
lines = fi.readlines()

# Transmit commands to UART
for line in lines:

  line_bytes = hexstr2byte(line.strip())
  print('Tx: 0x{}'.format(line.strip()))
  ser.write(line_bytes)
  sleep(1)

sleep(timedelay) 

fi.close
print('Terminating python script')
stop_threads = True
thread.join()
quit()
