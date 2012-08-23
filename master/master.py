import socket
import binascii
import time
import random

# SLIP special characters

# Character used to signal the start or end of a frame
SLIP_END           = chr(0xC0)
# Character used to escape the use of special characters in the data
SLIP_ESC           = chr(0xDB)
# Character used to replace a SLIP_END character in the data
SLIP_ESC_END       = chr(0xDC)
# Character used to replace a SLIP_ESC character in the data
SLIP_ESC_ESC       = chr(0xDD)

MASTER_ADDRESS             = 0xAAAA
MESSAGE_HEADER_LEN         = 2 + 2 + 1 #bytes

# Message Types
MESSAGE_HEADER             = chr(0x00)
MESSAGE_COLOR_CHANGE       = chr(0x01)
MESSAGE_SWITCH_CHANGE      = chr(0x02)

MSG_TYPES                  = [MESSAGE_HEADER, MESSAGE_COLOR_CHANGE, MESSAGE_SWITCH_CHANGE]

# Message Templates
TEMPLATES = {
    MESSAGE_HEADER         : ['source','destination','message_type']
    MESSAGE_COLOR_CHANGE   : ['switch_index', 'red', 'green', 'blue'],
    MESSAGE_SWITCH_CHANGE  : ['switch_index', 'switch_state'],
}

# Modes
MODE_CALIB         = 0x01
MODE_NORMAL        = 0x02

HOST = ''                 # Symbolic name meaning all available interfaces
PORT = 50008              # Arbitrary non-privileged port

""" Group Indexes
0 1 2
3 4 5
6 7 8
"""

""" Button Indexes
0  1
2  3
"""



"""
  == Initializing ==
(Master in MODE_CALIB)
Slaves set some default color on power on
Master waits for button press

  == Normal Operation ==
master <- BTN_PRESS <- humans                   humans stepping on buttons trigger BTN_PRESS messages
master -> SET_COLOR -> response to BTN_PRESS    in response to button press, SET_COLOR message(s) are sent
"""


s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))
#s.settimeout(0.00001) #really shouldn't be setting a timeout at all, but it allows a sig interupt to be fired

class Button(object):
    rnd = None
    position = None
    colors = [(0x00, 0x00, 0x00), (0x00, 0x00, 0x00), (0x00, 0x00, 0x00), (0x00, 0x00, 0x00)]
    button_config = []
    # buf = bytearray()
    # prev_byte = None
    
    def set_color(self, switch_index, red, green, blue):
        global conn
        header = {'source':MASTER_ADDRESS, 'destination':self.rnd, 'message_type':MESSAGE_COLOR_CHANGE}
        msg = {'switch_index':switch_index, 'red':red, 'green':green, 'blue':blue}
        self.colors[switch_index] = (red, green, blue)
        bytes = self.slip_encode(self.encode_msg(header, msg))
        
        print "SET_COLOR  "+str(self.rnd)+"  "+str(switch_index)+"  "+str(self.colors[switch_index])
        
        #send bytes over tcp connection
        conn.send(bytes)


BUTTON_STATES = {
    '0':{
        '0': None,
        '1': None,
        '2': None,
    },
    '1':{
        '0': None,
        '1': None,
        '2': None,
    },
    '2':{
        '0': None,
        '1': None,
        '2': None,
    }
}
GROUP_ADDRESSES = dict()
MODE = MODE_CALIB




buf = bytearray()
prev_byte = None
def handle_byte(byte):
    global buf, prev_byte
    
    if byte == SLIP_END:
        #if it's an empty message
        # or message type field is not expected
        # or the length isn't correct for the message type
        #throw it out
        if len(buf) == 0 or chr(buf[0]) not in MSG_TYPES or len(buf[1:]) != len(TEMPLATES[chr(buf[0])]):
            if len(buf) > 0:
                print "Throwing out: "+binascii.hexlify(buf)
            buf = bytearray()
        else:
            process_data(buf)
            buf = bytearray()
    elif byte == SLIP_ESC:
        pass
    elif prev_byte == SLIP_ESC and byte == SLIP_ESC_END:
        buf.append(SLIP_END)
    elif prev_byte == SLIP_ESC and byte == SLIP_ESC_ESC:
        buf.append(SLIP_ESC)
    else:
        buf.append(byte)
        
    prev_byte = byte

def process_data(data):
    #if chr(data[0]) not in MSG_TYPES or len(data[1:]) != len(TEMPLATES[chr(data[0])]):
    if len(data) < len(MESSAGE_HEADER_LEN):
        print "Throwing out header: "+binascii.hexlify(data)
        return
    header = self.decode_header(data)
    if header['destination'] != MASTER_ADDRESS:
        print "Throwing out message because it's not meant for me "+binascii.hexlify(data)
        return
    if header['message_type'] not in MSG_TYPES:
        print "Throwing out message because message type not recognized "+binascii.hexlify(data)
        return
    tmpl = TEMPLATES[header['message_type']]
    if len(data) < MESSAGE_HEADER_LEN + len(tmpl):
        print "Throwing out message: "+binascii.hexlify(data)
        return
    msg = self.decode_msg(tmpl, data)
    if chr(header['message_type']) == MESSAGE_SWITCH_CHANGE:
        handle_MESSAGE_SWITCH_CHANGE(header['source'], msg)
    if chr(header['message_type']) == MESSAGE_COLOR_CHANGE:
        print "Master doesn't handle MESSAGE_COLOR_CHANGE"


def decode_header(data):
    msg = dict()
    msg['source'] = (data[0] << 8) + data[1]
    msg['destination'] = (data[2] << 8) + data[3]
    msg['message_type'] = data[4]
    return msg

def encode_header(msg):
    data = list()
    data += list(struct.pack(">I", msg['source'][2:]))
    data += list(struct.pack(">I", msg['destination'][2:]))
    data += list(msg['message_type'])
    return msg

def decode_msg(tmpl, data):
    msg = dict()
    for i in range(len(data[1:])):
        field = tmpl[i]
        msg[field] = data[i+1]
    return msg

def encode_msg(header, msg):
    data = bytearray(self.encode_header(header))
    for field in TEMPLATES[header['message_type']]:
        data.append(chr(data[field]))
    return data

def slip_encode(data):
    encoded = bytearray([SLIP_END])
    for byte in data:
        if byte == SLIP_END:
            encoded.append(SLIP_ESC)
            encoded.append(SLIP_ESC_END)
        elif byte == SLIP_ESC:
            encoded.append(SLIP_ESC)
            encoded.append(SLIP_ESC_ESC)
        else:
            encoded.append(byte)
    encoded.append(SLIP_END)
    return encoded

def handle_MESSAGE_SWITCH_CHANGE(source, msg):
    global MODE
    if MODE == MODE_CALIB:
        print 'RCV MESSAGE_SWITCH_CHANGE (calibration) '+str(source)+"  "+str(msg)
        if source in GROUP_ADDRESSES:
            print "Already calibrated this button"
            return
        btn = Button()
        btn.rnd = source
        GROUP_ADDRESSES[str(source)] = btn
        # find first unoccupied spot by scanning through
        for r in sorted(BUTTON_STATES.keys()):
            for c in sorted(BUTTON_STATES[r].keys()):
                if not BUTTON_STATES[r][c]:
                    btn.position = (r, c)
                    # infer group orientation
                    btn.button_config = [0,1,2,3][msg['switch_index']:] + [0,1,2,3][:msg['switch_index']]
                    BUTTON_STATES[r][c] = btn
                    btn.set_color(switch_index=msg['switch_index'], red=0xFF, green=0x00, blue=0x00)
                    break
            if btn.position: break
        if len(GROUP_ADDRESSES) == 9:
            MODE = MODE_NORMAL
            print_pixel_config()
            # TODO: write config to disk
    else:
        print 'RCV MESSAGE_SWITCH_CHANGE '+str(source)+"  "+str(msg)
        btn = GROUP_ADDRESSES[source]
        new_color = (random.randrange(0x00, 0xFF), random.randrange(0x00, 0xFF), random.randrange(0x00, 0xFF))
        btn.set_color(switch_index=msg['switch_index'], red=new_color[0], green=new_color[1], blue=new_color[2])

def calibration_mode():
    global MODE
    MODE = MODE_CALIB
    print "Calibrating - step on button groups in order, top left button"
    
    for btn in GROUP_ADDRESSES.values():
        for switch_index in range(4):
            btn.set_color(switch_index=switch_index, red=new_color[0], green=new_color[1], blue=new_color[2])

def pixel_to_button(x, y):
    btn = BUTTON_STATES[str(y/2)][str(x/2)]
    switch_index = btn.button_config[(y%2)*2 + x%2]
    return (btn, switch_index)

def print_button_config():
    for r in sorted(BUTTON_STATES.keys()):
        row_str = ""
        for c in sorted(BUTTON_STATES[r].keys()):
            btn = BUTTON_STATES[r][c]
            row_str += str(btn.rnd)+" "+str(btn.button_config)+"\t"
            # row_str += str(btn.rnd)+"\t"
        print row_str

def print_pixel_config():
    for y in range(6):
        row_str = ""
        for x in range(6):
            btn, switch_index = pixel_to_button(x, y)
            row_str += str(btn.rnd)+" "+str(btn.button_config[switch_index])+"\t"
        print row_str

def set_pattern_outline():
    # Send colors for an outline of the floor
    for y in range(6):
        for x in range(6):
            if x in [0,5] or y in [0, 5]:
                new_color = (0xFF, 0xFF, 0xFF)
                btn, switch_index = pixel_to_button(x, y)
                btn.set_color(switch_index=switch_index, red=new_color[0], green=new_color[1], blue=new_color[2])


def listen():
    print "Listening"
    while True:
        try:
            byte = s.recv(1)
            if len(byte)==0:
                s.connect((HOST, PORT))
                continue
            handle_byte(byte)
        except socket.timeout:
            time.sleep(0.00001)
            continue



# TODO: try to load config from disk
config = None
if config:
    pass
else:
    calibration_mode()


listen()


s.close()
