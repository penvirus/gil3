import time
from math import log
from collections import defaultdict

switch_on = False

def dbg(msg):
    print('[DEBUG] {0}'.format(msg))

class Entropy(object):
    def __init__(self):
        self._count = defaultdict(int)
        self._size = 0

    def update(self, data):
        for c in data:
            self._count[ord(c)] += 1
        self._size += len(data)

    def final(self):
        if not self._size:
            return 0.0

        ent = 0.0
        for i, c in self._count.items():
            prob = float(c) / float(self._size)
            ent -= prob * log(prob)
        return ent / log(2.0)

def run():
    current_on = False
    ent = None

    # ```mknod /dev/urandom c 1 9''' if the device doesn't exist
    with open('/dev/urandom') as rf:
        while True:
            if not switch_on:
                if current_on:
                    print('{0:.4f}'.format(ent.final()))
                    current_on = False
                    ent = None
                    dbg('switch off')

                time.sleep(1)
            else:
                if not current_on:
                    current_on = True
                    ent = Entropy()
                    dbg('switch on')

                data = rf.read(4096)
                ent.update(data)
