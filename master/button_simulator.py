import socket
import binascii
import copy
import random
import time
import sys
import struct

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

HOST = 'localhost'        # The remote host
PORT = 50008              # The same port as used by the server

class Button(object):
    grp_idx = None
    position = None
    colors = [(0x00, 0x00, 0x00), (0x00, 0x00, 0x00), (0x00, 0x00, 0x00), (0x00, 0x00, 0x00)]
    button_config = [0, 1, 2, 3]
    buf = bytearray()
    prev_byte = None
    rnd = None
    
    def handle_byte(self, byte):
        if byte == SLIP_END:
            #if it's an empty message
            # or message type field is not expected
            # or the length isn't correct for the message type
            #throw it out
            if len(self.buf) == 0:
                #chr(self.buf[0]) not in MSG_TYPES or len(self.buf[1:]) != len(TEMPLATES[chr(self.buf[0])])
                # if len(self.buf) > 0:
                #     print "Throwing out: "+binascii.hexlify(self.buf)
                self.buf = bytearray()
            else:
                self.process_data(self.buf)
                self.buf = bytearray()
        elif byte == SLIP_ESC:
            pass
        elif self.prev_byte == SLIP_ESC and byte == SLIP_ESC_END:
            self.buf.append(SLIP_END)
        elif self.prev_byte == SLIP_ESC and byte == SLIP_ESC_ESC:
            self.buf.append(SLIP_ESC)
        else:
            self.buf.append(byte)
            
        self.prev_byte = byte
    
    def process_data(self, data):
        #if chr(data[0]) not in MSG_TYPES or len(data[1:]) != len(TEMPLATES[chr(data[0])]):
        if len(data) < len(MESSAGE_HEADER_LEN):
            print "Throwing out header: "+binascii.hexlify(data)
            return
        header = self.decode_header(data)
        if header['destination'] != self.rnd:
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
            print "Slave doesn't handle MESSAGE_SWITCH_CHANGE"
            pass
        if chr(header['message_type']) == MESSAGE_COLOR_CHANGE:
            self.handle_COLOR_CHANGE(msg)
        
            
    def handle_COLOR_CHANGE(self, msg):
        if msg['switch_index'] not in self.button_config:
            #print "Switch index not valid"
            return
        else:
            print "RCV MESSAGE_COLOR_CHANGE "+str(msg)
            self.colors[msg['switch_index']] = (msg['rv'], msg['gv'], msg['bv'])
            
    def trigger_button_press(self, switch_index):
        global conn
        header = {'source':self.rnd, 'destination':MASTER_ADDRESS, 'message_type':MESSAGE_SWITCH_CHANGE}
        msg = {'switch_index':switch_index, 'switch_state':True}
        bytes = self.slip_encode(self.encode_msg(header, msg))
        
        #send bytes over tcp connection
        conn.send(bytes)

    def decode_header(self, data):
        msg = dict()
        msg['source'] = (data[0] << 8) + data[1]
        msg['destination'] = (data[2] << 8) + data[3]
        msg['message_type'] = data[4]
        return msg

    def encode_header(self, msg):
        data = list()
        data += list(struct.pack(">I", msg['source'][2:]))
        data += list(struct.pack(">I", msg['destination'][2:]))
        data += list(msg['message_type'])
        return msg

    def decode_msg(self, tmpl, data):
        msg = dict()
        for i in range(len(data[1:])):
            field = tmpl[i]
            msg[field] = data[i+1]
        return msg

    def encode_msg(self, header, msg):
        data = bytearray(self.encode_header(header))
        for field in TEMPLATES[header['message_type']]:
            data.append(chr(data[field]))
        return data
    
    def slip_encode(self, data):
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
    
def pixel_to_button(x, y):
    grp_idx = (y/2)*3 + x/2
    btn = GROUP_INDEXES[grp_idx]
    btn_idx = btn.button_config[(y%2)*2 + x%2]
    # btn_idx = [0,1,2,3][(y%2)*2 + x%2]
    return (grp_idx, btn_idx)

def print_button_config():
    for r in sorted(BUTTON_STATES.keys()):
        row_str = ""
        for c in sorted(BUTTON_STATES[r].keys()):
            btn = BUTTON_STATES[r][c]
            # row_str += str(btn.grp_idx)+"\t"
            row_str += str(btn.grp_idx)+" "+str(btn.button_config)+"\t"
        print row_str

def print_pixel_config():
    for y in range(6):
        row_str = ""
        for x in range(6):
            grp_idx, btn_idx = pixel_to_button(x, y)
            btn = GROUP_INDEXES[grp_idx]
            # row_str += str(btn.grp_idx)+" "+str(btn.position)+"\t"
            row_str += str(btn.grp_idx)+" "+str(btn.button_config[btn_idx])+"\t"
            # row_str += str(btn.grp_idx)+"\t"
        print row_str

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST, PORT))
s.listen(1)

BUTTON_STATES = {
    '0':{
        '0': Button(),
        '1': Button(),
        '2': Button(),
    },
    '1':{
        '0': Button(),
        '1': Button(),
        '2': Button(),
    },
    '2':{
        '0': Button(),
        '1': Button(),
        '2': Button(),
    }
}
GROUP_INDEXES = [
    None, None, None, 
    None, None, None, 
    None, None, None
]
RAND_BUTTONS = []
for r in sorted(BUTTON_STATES.keys()):
    for c in sorted(BUTTON_STATES[r].keys()):
        btn = BUTTON_STATES[r][c]
        btn.position = (r, c)
        rnd_orientation = random.choice(btn.button_config)
        btn.button_config = btn.button_config[rnd_orientation:] + btn.button_config[:rnd_orientation]
        RAND_BUTTONS.append(btn)



#Initialization phase
conn, addr = s.accept()
buf = bytearray()
prev_byte = None


# First, for testing SLIP is implemented correctly, expect to receive 256 bytes in sequence
for i in range(256):
    while True:
        byte = conn.recv(1)
    if len(byte) == 0: 
        conn, addr = s.accept()
        continue
    if byte == SLIP_END:
        #if it's an empty message
        # or message type field is not expected
        # or the length isn't correct for the message type
        #throw it out
        if len(buf) == 0:
            self.buf = bytearray()
        else:
            if buf[0] == i:
                print "received "+str(buf)
                continue
            else:
                print "Error: expected "+str(i)+" and got "+str(buf)
                sys.exit(1)
            self.buf = bytearray()
    elif byte == SLIP_ESC:
        pass
    elif self.prev_byte == SLIP_ESC and byte == SLIP_ESC_END:
        self.buf.append(SLIP_END)
    elif self.prev_byte == SLIP_ESC and byte == SLIP_ESC_ESC:
        self.buf.append(SLIP_ESC)
    else:
        self.buf.append(byte)
        
    self.prev_byte = byte


while not all(map(lambda b: b.mode == MODE_CALIB, RAND_BUTTONS)):
    byte = conn.recv(1)
    if len(byte) == 0: 
        conn, addr = s.accept()
        continue
    #send bytes to all buttons in a random order to simulate unpredictable timing on the wire
    random.shuffle(RAND_BUTTONS)
    for btn in RAND_BUTTONS:
        btn.handle_byte(byte)
    

#Calibration phase
for r in sorted(BUTTON_STATES.keys()):
    for c in sorted(BUTTON_STATES[r].keys()):
        time.sleep(0.2)
        btn = BUTTON_STATES[r][c]
        print "SND BTN_PRESS "+str({'grp_idx':btn.grp_idx, 'btn_idx':btn.button_config[0]})
        conn.send(encode_message(BTN_PRESS, {'grp_idx':btn.grp_idx, 'btn_idx':btn.button_config[0]}))
        

# print_button_config()
print_pixel_config()

while not all(map(lambda b: b.mode == MODE_NORMAL, RAND_BUTTONS)):
    byte = conn.recv(1)
    if len(byte) == 0: 
        conn, addr = s.accept()
        continue
    #send bytes to all buttons in a random order to simulate unpredictable timing on the wire
    random.shuffle(RAND_BUTTONS)
    for btn in RAND_BUTTONS:
        btn.handle_byte(byte)

# Buttons should now be in normal operation mode

time.sleep(1)

# Simulate pressing a few random buttons
btn = random.choice(RAND_BUTTONS)
btn_idx = random.choice([0,1,2,3])
current_color = btn.colors[btn_idx]
print "SND BTN_PRESS "+str({'grp_idx':btn.grp_idx, 'btn_idx':btn_idx})
conn.send(encode_message(BTN_PRESS, {'grp_idx':btn.grp_idx, 'btn_idx':btn_idx}))

# Wait for SET_COLOR
while btn.colors[btn_idx] == current_color:
    byte = conn.recv(1)
    if len(byte) == 0: 
        conn, addr = s.accept()
        continue
    #send bytes to all buttons in a random order to simulate unpredictable timing on the wire
    random.shuffle(RAND_BUTTONS)
    for btn in RAND_BUTTONS:
        btn.handle_byte(byte)

while True:
    byte = conn.recv(1)
    if len(byte) == 0: 
        conn, addr = s.accept()
        continue
    #send bytes to all buttons in a random order to simulate unpredictable timing on the wire
    random.shuffle(RAND_BUTTONS)
    for btn in RAND_BUTTONS:
        btn.handle_byte(byte)

conn.close()
s.close()

