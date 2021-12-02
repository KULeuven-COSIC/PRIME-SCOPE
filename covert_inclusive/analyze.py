import matplotlib.pyplot as plt
import matplotlib as mpl

from math import floor, ceil, log2
import os.path
from os import path


def file_len(f):
    for i, l in enumerate(f):
        pass
    return i + 1

def prOrange(skk): print("\033[33m {}\033[00m" .format(skk))

GRAY1 = ['0', '1']
GRAY2 = ['00', '01', '11', '10']
GRAY3 = ['000', '001', '011', '010', '110', '111', '101', '100']
GRAY4 = ['0000', '0001', '0011', '0010', '0110', '0111', '0101', '0100', '1100', '1101', '1111', '1110', '1010', '1011', '1001', '1000']

def encodeGray(num, nbBits):
    if num == 2**nbBits:
        return nbBits*'0'
    if nbBits == 4:
        return GRAY4[num]
    if nbBits == 3:
        return GRAY3[num]
    if nbBits == 2:
        return GRAY2[num]
    if nbBits == 1:
        return GRAY1[num]

def main():
    
    #####################################
    # Determine accuracy of printed logs
    #####################################

    # Ground Truth
    fileV = open("log/cov_V.log", "r")
    v = fileV.read().replace("\n", "")
    v = [int(x) for x in filter(None, v.split(" "))]
    BITS = ceil(log2(max(v)))
    vBit = "".join([bin(x)[2:].zfill(BITS) for x in v])
    vBitGray = "".join([encodeGray(x, BITS) for x in v])
    fileV.close()

    # Received
    fileA = open("log/cov_A.log", "r")
    a = fileA.read().replace("\n", "")
    a = [int(x) for x in filter(None, a.split(" "))]
    notReceived = sum([x==2**BITS for x in a])
    aBit = "".join([bin(x)[2:].zfill(BITS)[:BITS] for x in a])
    aBitGray = "".join([encodeGray(x, BITS) for x in a])

    correctSymbol = sum([a[i] == v[i] for i in range(len(a))])*1.0/len(a)
    correctBit = sum([aBit[i] == vBit[i] for i in range(len(aBit))])*1.0/len(aBit)
    correctBitGray = sum([aBitGray[i] == vBitGray[i] for i in range(len(aBitGray))])*1.0/len(aBitGray)
    SER = (1-correctSymbol)*100
    BER = (1-correctBit)*100
    BERGray = (1-correctBitGray)*100
    incorrectZeroFraction = sum([1 if (a[i] != v[i] and a[i] == 0) else 0 for i in range(len(a))])*1.0/len(a)
    notReceivedFraction = notReceived*1.0/(len(a))

    prOrange("\tAccuracy: {:.2f}%".format(100-BER))
    print("\t\tSymbol Error Rate: {:.2f}% (of which {:.2f}% not received and {:.2f}% received as zero)".format(SER, notReceivedFraction*100, incorrectZeroFraction*100))
    print("\t\tBit Error Rate: {:.2f}%".format(BER))

    prOrange("\tAccuracy (Gray): {:.2f}%".format(100-BERGray))

    
if __name__ == "__main__":
    main()
