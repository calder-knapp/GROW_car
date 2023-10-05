
from oscpy.client import OSCClient
import time
from msvcrt import getch


address = "192.168.0.100"
port = 9002
osc = OSCClient(address, port)

while True:
    #Check for a keypress
    key = getch()
    print(key)
    if key == b'\x1b':  # ESC
        break
    elif key == b'\x00':  # Special keys (arrows, f keys, ins, del, etc.)
        key = ord(getch())
        print(key)
        if key == 75:  # Left arrow
            print("Left")
            msg = [0.9, 0.1]
        elif key == 77:  # Right arrow
            print("Right")
            msg = [0.1, 0.9]
        elif key == 72:  # Right arrow
            print("Up")
            msg = [0.9, 0.9]
        elif key == 80:
            print("Down")
            msg = [0.1, 0.1]
        else:
            msg = [0.5, 0.5]
    else:
        msg = [0.5, 0.5]
    osc.send_message(b'/neuropype', msg)
    print(msg)
    time.sleep(.1)
