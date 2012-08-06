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

# Message Types
RST_ADDR           = chr(0x00)
REQ_ADDR           = chr(0x01)
ASN_ADDR           = chr(0x02)
BTN_PRESS          = chr(0x03)
CALIB_MODE         = chr(0x04)
SET_COLOR          = chr(0x05)
MSG_TYPES          = [RST_ADDR, REQ_ADDR, ASN_ADDR, BTN_PRESS, CALIB_MODE, SET_COLOR]

# Message Templates
TEMPLATES = {
    RST_ADDR      : [],
    REQ_ADDR      : ['rnd'],
    ASN_ADDR      : ['rnd', 'grp_idx'],
    BTN_PRESS     : ['grp_idx', 'btn_idx'],
    CALIB_MODE    : ['grp_idx'],
    SET_COLOR     : ['grp_idx', 'btn_idx', 'rv', 'gv', 'bv']
}

# Modes
MODE_INIT          = 0x00
MODE_CALIB         = 0x01
MODE_NORMAL        = 0x02

""" Button Indexes
0  1
2  3
"""



HOST = ''                 # Symbolic name meaning all available interfaces
PORT = 50008              # Arbitrary non-privileged port

"""
master -> RST_ADDR -> broadcast
master <- REQ_ADDR <- each button group         wait a random amount of time before requesting an address
master -> ASN_ADDR -> each button group         for each request, send back a sequential group address
master -> CALIB_MODE -> each button group       for each button, send a message to put it in calibration mode
master <- BTN_PRESS <- each button group        in raster scan fashion, a human will press the top left button of each group
master -> SET_COLOR -> each button group        after pressing the button in calibration mode, the master sends command to change the color

  now in normal operation
master <- BTN_PRESS <- humans                   humans stepping on buttons trigger BTN_PRESS messages
master -> SET_COLOR -> response to BTN_PRESS    in response to button press, SET_COLOR message(s) are sent
"""


s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))
s.settimeout(0.00001)

class Button(object):
    grp_idx = None
    position = None
    mode = MODE_INIT
    colors = [(0x00, 0x00, 0x00), (0x00, 0x00, 0x00), (0x00, 0x00, 0x00), (0x00, 0x00, 0x00)]
    button_config = []
    buf = bytearray()
    prev_byte = None


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
GROUP_INDEXES = list()
MODE = MODE_INIT


def encode_message(msg_type, msg):
    encoded = bytearray([SLIP_END, msg_type])
    for field in TEMPLATES[msg_type]:
        byte = msg[field]
        if type(byte) == int:
            byte = chr(byte)
        if byte == SLIP_END:
            encoded.append(SLIP_ESC)
            encoded.append(SLIP_ESC_END)
        elif byte == SLIP_ESC:
            encoded.append(SLIP_ESC)
            encoded.append(SLIP_ESC_ESC)
        else:
            try:
                encoded.append(byte)
            except:
                binascii.hexlify(byte)
    
    encoded.append(SLIP_END)
    
    return encoded

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

def decode_data(data):
    msg = dict()
    tmpl = TEMPLATES[chr(data[0])]
    for i in range(len(data[1:])):
        field = tmpl[i]
        msg[field] = data[i+1]
    return msg

def process_data(data):
    msg = decode_data(data)
    if chr(data[0]) == RST_ADDR:
        pass #only relevent for slave
    elif chr(data[0]) == REQ_ADDR:
        handle_REQ_ADDR(msg)
    elif chr(data[0]) == ASN_ADDR:
        pass #only relevent for slave
    elif chr(data[0]) == BTN_PRESS:
        handle_BTN_PRESS(msg)
    elif chr(data[0]) == CALIB_MODE:
        pass #only relevent for slave
    elif chr(data[0]) == SET_COLOR:
        pass #only relevent for slave

def handle_REQ_ADDR(msg):
    print "REQ_ADDR  "+str(msg)
    btn = Button()
    btn.grp_idx = len(GROUP_INDEXES)
    GROUP_INDEXES.append(btn)
    
    reply = {'rnd': msg['rnd'], 'grp_idx':btn.grp_idx}
    s.send(encode_message(ASN_ADDR, reply))

def handle_BTN_PRESS(msg):
    btn = GROUP_INDEXES[msg['grp_idx']]
    if btn.mode == MODE_CALIB:
        print 'BTN_PRESS (calibration) '+str(msg)
        for r in sorted(BUTTON_STATES.keys()):
            for c in sorted(BUTTON_STATES[r].keys()):
                if not BUTTON_STATES[r][c]:
                    btn.position = (r, c)
                    btn.mode = MODE_NORMAL
                    BUTTON_STATES[r][c] = btn
                    s.send(encode_message(SET_COLOR, {'grp_idx':btn.grp_idx, 'btn_idx':msg['btn_idx'], 'rv':0xFF, 'gv':0x00, 'bv':0x00}))
                    break
            if btn.position: break
    else:
        print 'BTN_PRESS '+str(msg)
        btn = GROUP_INDEXES[msg['grp_idx']]
        new_color = (random.randrange(0x00, 0xFF), random.randrange(0x00, 0xFF), random.randrange(0x00, 0xFF))
        btn.colors[msg['btn_idx']] = new_color
        s.send(encode_message(SET_COLOR, {'grp_idx':btn.grp_idx, 'btn_idx':msg['btn_idx'], 'rv':new_color[0], 'gv':new_color[1], 'bv':new_color[2]}))
        print "SET_COLOR  grp: "+str(btn.grp_idx)+"  btn: "+str(msg['btn_idx'])+"  color: "+str(new_color)

def initialize_buttons():
    global MODE
    MODE = MODE_INIT
    
    data = encode_message(RST_ADDR, {})
    s.send(data)

    while len(GROUP_INDEXES) < 9:
        try:
            byte = s.recv(1)
            if len(byte)==0:
                continue
            handle_byte(byte)
        except socket.timeout:
            time.sleep(0.00001)
            continue

def calibrate_buttons():
    global MODE
    MODE = MODE_CALIB
    
    for btn in GROUP_INDEXES:
        s.send(encode_message(CALIB_MODE, {'grp_idx':btn.grp_idx}))
        btn.mode = MODE_CALIB
    
    
    while not all(map(lambda b: b.mode == MODE_NORMAL, GROUP_INDEXES)):
        try:
            byte = s.recv(1)
            if len(byte)==0:
                continue
            handle_byte(byte)
        except socket.timeout:
            time.sleep(0.00001)
            continue
    
    MODE = MODE_NORMAL

def print_button_config():
    for r in sorted(BUTTON_STATES.keys()):
        row_str = ""
        for c in sorted(BUTTON_STATES[r].keys()):
            btn = BUTTON_STATES[r][c]
            row_str += str(btn.grp_idx)+" "+str(btn.position)+"\t"
        print row_str

def listen():
    while True:
        try:
            byte = s.recv(1)
            if len(byte)==0:
                continue
            handle_byte(byte)
        except socket.timeout:
            time.sleep(0.00001)
            continue


initialize_buttons()

calibrate_buttons()

print_button_config()

listen()

# data = encode_message({'source':0, 'destination':1, 'square_index':2, 'color':3})
# s.send(data)

# data = encode_message({'source':4, 'destination':5, 'square_index':6, 'color':7})
# s.send(data)
# s.send(bytearray('foo'))

# data = encode_message({'source':0, 'destination':1, 'square_index':2, 'color':3})
# s.send(data)


s.close()
