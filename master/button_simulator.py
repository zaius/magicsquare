import socket
import binascii
import copy
import random
import time

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

HOST = 'localhost'        # The remote host
PORT = 50008              # The same port as used by the server

class Button(object):
    grp_idx = None
    position = None
    mode = MODE_INIT
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
            if len(self.buf) == 0 or chr(self.buf[0]) not in MSG_TYPES or len(self.buf[1:]) != len(TEMPLATES[chr(self.buf[0])]):
                if len(self.buf) > 0:
                    print "Throwing out: "+binascii.hexlify(self.buf)
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
        msg = decode_data(data)
        if chr(data[0]) == RST_ADDR:
            self.handle_RST_ADDR(msg)
        elif chr(data[0]) == REQ_ADDR:
            pass #only relevent for master
        elif chr(data[0]) == ASN_ADDR:
            self.handle_ASN_ADDR(msg)
        elif chr(data[0]) == BTN_PRESS:
            pass #only relevent for master
        elif chr(data[0]) == CALIB_MODE:
            self.handle_CALIB_MODE(msg)
        elif chr(data[0]) == SET_COLOR:
            self.handle_SET_COLOR(msg)
    
    def handle_RST_ADDR(self, msg):
        self.rnd = random.randrange(0x00, 0xFF)
        conn.send(encode_message(REQ_ADDR, {'rnd':self.rnd}))
    
    def handle_ASN_ADDR(self, msg):
        global GROUP_INDEXES
        
        if msg['rnd'] != self.rnd:
            #print "Message not meant for me!"
            return
        else:
            self.grp_idx = msg['grp_idx']
            GROUP_INDEXES[self.grp_idx] = self
            print "grp_idx "+str(self.grp_idx)
    
    def handle_CALIB_MODE(self, msg):
        if msg['grp_idx'] != self.grp_idx:
            #print "Message not meant for me!"
            return
        else:
            self.mode = MODE_CALIB
            print "grp_idx: "+str(self.grp_idx)+"  CALIB"
            
    def handle_SET_COLOR(self, msg):
        if msg['grp_idx'] != self.grp_idx:
            #print "Message not meant for me!"
            return
        else:
            if self.mode == MODE_CALIB:
                self.mode = MODE_NORMAL
            self.colors[msg['btn_idx']] = (msg['rv'], msg['gv'], msg['bv'])
            print "SET_COLOR grp: "+str(msg['grp_idx'])+"  btn: "+str(msg['btn_idx'])+"  color: "+str(self.colors[msg['btn_idx']])
            
        

def decode_data(data):
    msg = dict()
    tmpl = TEMPLATES[chr(data[0])]
    for i in range(len(data[1:])):
        field = tmpl[i]
        msg[field] = data[i+1]
    return msg


def encode_message(msg_type, msg):
    encoded = bytearray([SLIP_END, msg_type])
    for field in TEMPLATES[msg_type]:
        byte = msg[field]
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
print "Sending button press for "+str(btn.position)+"  grp_idx "+str(btn.grp_idx)
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

conn.close()
s.close()

