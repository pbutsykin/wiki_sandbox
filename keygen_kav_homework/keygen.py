# -*- coding: utf-8 -*-
"""keygen.py name"""

# Homework for kaspersky.com application @ 16.03.2017 - 17.03.2017

def generate_key(checksum, magic = 0xDEADBEEF ^ 1, key_max = 32):
    import random
    u5 = lambda x: x & 0x1f
    stage, key = checksum, ""
    for _ in range(random.randint(7, key_max)):
        stage ^= magic
        rand = random.randint(0, 7) << 5
        key = format(rand | u5(stage), '02x') + key
        stage = (stage ^ rand) >> 5
    return key

def checksum_by_name(name, magic = 0xFEC0135A):
    import struct
    s1 = struct.unpack('>I', name[:2] + name[-2:])[0]
    s2 = sum([ord(c) for c in name])
    return s1 ^ s2 ^ magic

def main(name):
    nmax, nmin = 260, 4

    if len(name) > nmax or len(name) < nmin:
        print("The name or email should be less then %u and more then %u." % (nmax, nmin))
        return

    if (filter(lambda c: ord(c) < 0x20 or ord(c) > 0x7e, name)):
        print("The name or email field should contains only ASCII chars.")
        return

    csum = checksum_by_name(name)
    print(generate_key(csum))

if __name__ == '__main__':
    import sys
    if len(sys.argv) != 2:
        print(__doc__)
    else:
        main(sys.argv[1])
