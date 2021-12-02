import sys
from string           import Template
from params           import *
from classMeasurement import *
from classPrime       import *
from classPattern     import *

strides = range(1,6)

def generateCodeFiles(headerStr, functionStr, accessStr):
  hdrStr  = "#ifndef PRIME_H\n"
  hdrStr += "#define PRIME_H\n"
  hdrStr += "\n"
  hdrStr += "#include <stdint.h>\n"
  hdrStr += "\n"
  hdrStr += "$content"
  hdrStr += "\n"
  hdrStr += "#endif // PRIME_H"
  tmp = Template(hdrStr)
  hdrStr = tmp.substitute(content = headerStr)
  
  f = open(HdrFileName, "w")
  f.write(hdrStr)
  f.close()
  
  funStr  = "#include \"prime.h\"\n"
  funStr += "#include \"../../evsets/list/list_utils.h\"\n"
  funStr += "#include \"../../utils/cache_utils.h\"\n"
  funStr += "\n"
  funStr += "$content"
  tmp = Template(funStr)
  funStr = tmp.substitute(content = functionStr)

  f = open(FunFileName, "w")
  f.write(funStr)
  f.close()

  accStr  = "$content"
  tmp = Template(accStr)
  accStr = tmp.substitute(content = accessStr)

  f = open(AccFileName, "w")
  f.write(accStr)
  f.close()

def checkMeasurementFiles(newFileName, oldFileName):
  PatternsFile = open(newFileName, 'r')
  newMeasurements = [float(x.split(' ')[3]) for x in filter(None, PatternsFile.read().split("\n")) if x.split(' ')[0][0] != 'D']
  PatternsFile.close()
  if len(newMeasurements) == 0:
    return 0
  avgNew = sum(newMeasurements) / len(newMeasurements)

  PatternsFile = open(oldFileName, 'r')
  oldMeasurements = [float(x.split(' ')[3]) for x in filter(None, PatternsFile.read().split("\n")) if x.split(' ')[0][0] != 'D']
  PatternsFile.close()
  avgOld = sum(oldMeasurements) / len(oldMeasurements)
  if len(newMeasurements) == 0:
    return 0

  return (avgNew+3>avgOld)

def generatePrimes(strides, patterns):
  primes = []
  for key, pattern in patterns.items():
    varNo = 0
    for stride in strides:
      loop = LLC_WAYS - max([int(i) if str(i).isdigit() else 0 for i in pattern])
      name = "{a}_S{b}".format(a=key, b=stride)
      primes.append(classPrime(name, MIN_ITERATIONS, MAX_ITERATIONS, loop, stride, pattern))
      varNo += 1
  return primes

def var0_Primes(strides):
  
  # Create a dictionary of `classPattern`` type
  b = classPattern(dict())
  
  # Start with primitives. It will create simple access pattern constructions. 
  # If lenght is five, first only two accesses 01, then three 012, then four 
  # 0123, and finally five 01234.
  b.primitives(5)
  
  # Generate various products of the primitive patterns. Here product means, 
  # repeated accesses, and reverse-ordered accesses withing those repetitions
  # With max=3, there will be at most three repetitions of each primitive. 
  # With withReverse=1, reverse ordered accesses are also included
  # For example, take primitive pattern 01, the following products are created:
  #   B502       : 0 1
  #   B502_PR300 : 0 1 0 1
  #   B502_PR301 : 0 1 1 0
  #   B502_PR302 : 1 0 0 1
  #   B502_PR303 : 1 0 1 0
  #   B502_PR304 : 0 1 0 1 0 1
  #   B502_PR305 : 0 1 0 1 1 0
  #   B502_PR306 : 0 1 1 0 0 1
  #   B502_PR307 : 0 1 1 0 1 0
  #   B502_PR308 : 1 0 0 1 0 1
  #   B502_PR309 : 1 0 0 1 1 0
  #   B502_PR310 : 1 0 1 0 0 1
  #   B502_PR311 : 1 0 1 0 1 0
  b.concat(b.prod(max=3, withReverse=1).dictionary())

  # Apply intersperse the scope line access into the given patterns.
  # kind=0 for a sophisticated version, interspersing different variations of 
  # scope line accesses. For example, given a pattern 0123:
  #  * with access len=1, it will create patterns like 0S123
  #  * with access len=2, it will create patterns like 0SS123
  #  * with access len=1, gap=1, it will create patterns like 0S1S23
  #  * with access len=2, gap=1, it will create patterns like 0SS1SS23
  #  * with access len=2, gap=2, it will create patterns like 0SS12SS3
  # kind=1 for a simple version, interspersing only a single scope line access
  # exexmplified with the first bullet point above.
  e = b.evc(kind=0)

  # Create dummy access patterns, which are used before the actual patterns
  # to minimise the effect of residue cache-state
  d = classPattern(dict())
  d.dummies(50) 
  d.concat(e.dictionary())

  return generatePrimes(strides, d)

def var1_Primes(strides):
  b = classPattern(dict())
  b.primitives(4)
  b = b.evc(kind=1)
  
  d = classPattern(dict())
  d.dummies(50) 
  d.concat(b.dictionary())

  return generatePrimes(strides, d)

def mutated_Primes(primes, keepStride):
  p = []
  for prime in primes:
    m  = classPattern({prime.name: prime.accessPattern}).mutation()
    p += (generatePrimes( ([prime.name[-1]] if keepStride==1 else strides), m))
  return p
  
def main(argv):

  if (len(argv)==0):
    return 0

  primes = dict()

  if argv[0]=="createPrimes":

    if argv[1]=="var0":
      primes = var0_Primes(strides)
    elif argv[1]=="var1":
      primes = var1_Primes(strides)
      
    # [print(p) for p in primes]

    print("Writing Patterns [to " + argv[2] + "]")
    PatternsFile = open(argv[2], 'w')
    [PatternsFile.write(str(p) + "\n") for p in primes]
    PatternsFile.close()

  if argv[0]=="executionPrepare":
    print("Reading Primes [from " + argv[1] + "]")
    PatternsFile = open(argv[1], 'r')
    primes = [classPrime.fromString(x) for x in filter(None, PatternsFile.read().split("\n"))]
    PatternsFile.close()

    print("Writing Patterns [to " + PatternsFileName + "]")
    PatternsFile = open(PatternsFileName, 'w')
    [PatternsFile.write(str(p) + "\n") for p in primes]
    PatternsFile.close()

    print("Creating Code Files")
    generateCodeFiles(
      ''.join([p.createHeader()   for p in primes]),
      ''.join([p.createFunction() for p in primes]),
      ''.join([p.createAccess()   for p in primes]))
  
  if argv[0]=="executionCheck":
    sys.exit(checkMeasurementFiles(argv[1], argv[2]))

  if argv[0]=="filterByEVRate":
    # min_evc_rate     = float(argv[4])
    # duration_of_slow = int(argv[5])
    count     = int(argv[4])
    
    print("Reading Primes [from " + argv[1] + "]")
    PatternsFile = open(argv[1], 'r')
    primes = [classPrime.fromString(x) for x in filter(None, PatternsFile.read().split("\n"))]
    PatternsFile.close()

    print("Reading Measurements [from " + argv[2] + "]")
    MeasurementsFile = open(argv[2], 'r')
    allmeasurements = [classMeasurement.fromString(x) for x in filter(None, MeasurementsFile.read().split("\n"))]
    MeasurementsFile.close()
    
    print("Sorting Measurements")
    fastestMeasurements = sorted(allmeasurements, key=lambda m: m.EV)[::-1]
    # Get the list of good pattern names
    fastestMeasurementsName = [m.name for m in fastestMeasurements]
    # Remove dummy accesses
    fastestMeasurementsName = [m.name for m in fastestMeasurements if m.name[0] != 'D']
    # Remove duplicates
    fastestMeasurementsName = list(dict.fromkeys(fastestMeasurementsName))

    print("Filtering Measurements")
    filteredPrimes = []
    while len(filteredPrimes) < count and len(fastestMeasurementsName) > 0:
      f = fastestMeasurementsName.pop(0)
      filteredPrimes += [p for p in primes if p.name == f]

    print("Adding Dummies")
    d = classPattern(dict())
    d.dummies(50) 
    dummyPrimes = generatePrimes(strides, d)
    primes = dummyPrimes + filteredPrimes[::-1]

    print("Writing Patterns [to " + argv[3] + "]")
    PatternsFile = open(argv[3], 'w')
    [PatternsFile.write(str(p) + "\n") for p in primes]
    PatternsFile.close()

  if argv[0]=="filterByEVRateAbsolute":
    # min_evc_rate     = float(argv[4])
    # duration_of_slow = int(argv[5])
    count     = int(argv[4])
    
    print("Reading Primes [from " + argv[1] + "]")
    PatternsFile = open(argv[1], 'r')
    primes = [classPrime.fromString(x) for x in filter(None, PatternsFile.read().split("\n"))]
    PatternsFile.close()

    print("Reading Measurements [from " + argv[2] + "]")
    MeasurementsFile = open(argv[2], 'r')
    allmeasurements = [classMeasurement.fromString(x) for x in filter(None, MeasurementsFile.read().split("\n"))]
    MeasurementsFile.close()
    
    print("Sorting Measurements")
    fastestMeasurements = list(filter(lambda m: m.EV >= count, sorted(allmeasurements, key=lambda m: m.EV)[::-1]))
    # Get the list of good pattern names
    fastestMeasurementsName = [m.name for m in fastestMeasurements]
    # Remove dummy accesses
    fastestMeasurementsName = [m.name for m in fastestMeasurements if m.name[0] != 'D']
    # Remove duplicates
    fastestMeasurementsName = list(dict.fromkeys(fastestMeasurementsName))

    print("Filtering Measurements")
    filteredPrimes = []
    while len(fastestMeasurementsName) > 0:
      f = fastestMeasurementsName.pop(0)
      filteredPrimes += [p for p in primes if p.name == f]

    print("Adding Dummies")
    d = classPattern(dict())
    d.dummies(50) 
    dummyPrimes = generatePrimes(strides, d)
    primes = dummyPrimes + filteredPrimes[::-1]

    print("Writing Patterns [to " + argv[3] + "]")
    PatternsFile = open(argv[3], 'w')
    [PatternsFile.write(str(p) + "\n") for p in primes]
    PatternsFile.close()


  if argv[0]=="filterByEVCRate":
    # min_evc_rate     = float(argv[4])
    # duration_of_slow = int(argv[5])
    count     = int(argv[4])
    
    print("Reading Primes [from " + argv[1] + "]")
    PatternsFile = open(argv[1], 'r')
    primes = [classPrime.fromString(x) for x in filter(None, PatternsFile.read().split("\n"))]
    PatternsFile.close()

    print("Reading Measurements [from " + argv[2] + "]")
    MeasurementsFile = open(argv[2], 'r')
    allmeasurements = [classMeasurement.fromString(x) for x in filter(None, MeasurementsFile.read().split("\n"))]
    MeasurementsFile.close()
    
    print("Sorting Measurements")
    fastestMeasurements = sorted(allmeasurements, key=lambda m: m.EVC)[::-1]
    # Get the list of good pattern names
    fastestMeasurementsName = [m.name for m in fastestMeasurements]
    # Remove dummy accesses
    fastestMeasurementsName = [m.name for m in fastestMeasurements if m.name[0] != 'D']
    # Remove duplicates
    fastestMeasurementsName = list(dict.fromkeys(fastestMeasurementsName))

    print("Filtering Measurements")
    filteredPrimes = []
    while len(filteredPrimes) < count and len(fastestMeasurementsName) > 0:
      f = fastestMeasurementsName.pop(0)
      filteredPrimes += [p for p in primes if p.name == f]

    print("Adding Dummies")
    d = classPattern(dict())
    d.dummies(50) 
    dummyPrimes = generatePrimes(strides, d)
    primes = dummyPrimes + filteredPrimes[::-1]

    print("Writing Patterns [to " + argv[3] + "]")
    PatternsFile = open(argv[3], 'w')
    [PatternsFile.write(str(p) + "\n") for p in primes]
    PatternsFile.close()

  if argv[0]=="filterBySpeed":
    count     = int(argv[4])
    
    print("Reading Primes [from " + argv[1] + "]")
    PatternsFile = open(argv[1], 'r')
    primes = [classPrime.fromString(x) for x in filter(None, PatternsFile.read().split("\n"))]
    PatternsFile.close()

    print("Reading Measurements [from " + argv[2] + "]")
    MeasurementsFile = open(argv[2], 'r')
    allmeasurements = [classMeasurement.fromString(x) for x in filter(None, MeasurementsFile.read().split("\n"))]
    MeasurementsFile.close()

    print("Sorting Measurements")
    fastestMeasurements = sorted(allmeasurements, key=lambda m: m.duration[2])
    # Get the list of good pattern names
    fastestMeasurementsName = [m.name for m in fastestMeasurements]
    # Remove dummy accesses
    fastestMeasurementsName = [m.name for m in fastestMeasurements if m.name[0] != 'D']
    # Remove duplicates
    fastestMeasurementsName = list(dict.fromkeys(fastestMeasurementsName))

    print("Filtering Measurements")
    filteredPrimes = []
    while len(filteredPrimes) < count and len(fastestMeasurementsName) > 0:
      f = fastestMeasurementsName.pop(0)
      filteredPrimes += [p for p in primes if p.name == f]

    print("Adding Dummies")
    d = classPattern(dict())
    d.dummies(50) 
    dummyPrimes = generatePrimes(strides, d)
    primes =  dummyPrimes + filteredPrimes[::-1]

    print("Writing Patterns [to " + argv[3] + "]")
    PatternsFile = open(argv[3], 'w')
    [PatternsFile.write(str(p) + "\n") for p in primes]
    PatternsFile.close()

  if argv[0]=="sortByEVCr":

    print("Reading Primes [from " + argv[1] + "]")
    PatternsFile = open(argv[1], 'r')
    primes = [classPrime.fromString(x) for x in filter(None, PatternsFile.read().split("\n"))]
    PatternsFile.close()
    
    print("Reading Measurements [from " + argv[2] + "]")
    MeasurementsFile = open(argv[2], 'r')
    allmeasurements = [classMeasurement.fromString(x) for x in filter(None, MeasurementsFile.read().split("\n"))]
    MeasurementsFile.close()
    
    print("Sorting Measurements")
    fastestMeasurements = sorted(allmeasurements, key=lambda m: m.duration[2])

    filteredMeasurements = []

    step = 0.01
    for i in range(int((100-99.50)/step)):
      upper = 100-i*step
      lower = 100-(i+1)*step
      filteredMeasurements.append("EVCr from " + str(upper) + " to " + str(lower))
      for m in fastestMeasurements:
        if m.EVC<=upper and m.EVC>lower and m.name[0]!='D':
          pattern = [p for p in primes if p.name == m.name]
          if len(pattern) != 0:
            filteredMeasurements.append(\
              m.toString() + \
              " " + \
              ''.join([str(elem) for elem in pattern[0].accessPattern]))
      filteredMeasurements.append("")
      
    print("Writing Patterns [to " + argv[3] + "]")
    PatternsFile = open(argv[3], 'w')
    [PatternsFile.write(m + "\n") for m in filteredMeasurements]
    PatternsFile.close()
    return

  if argv[0]=="sortByEVr":

    print("Reading Primes [from " + argv[1] + "]")
    PatternsFile = open(argv[1], 'r')
    primes = [classPrime.fromString(x) for x in filter(None, PatternsFile.read().split("\n"))]
    PatternsFile.close()
    
    print("Reading Measurements [from " + argv[2] + "]")
    MeasurementsFile = open(argv[2], 'r')
    allmeasurements = [classMeasurement.fromString(x) for x in filter(None, MeasurementsFile.read().split("\n"))]
    MeasurementsFile.close()
    
    print("Sorting Measurements")
    fastestMeasurements = sorted(allmeasurements, key=lambda m: m.duration[2])

    filteredMeasurements = []

    step = 0.05
    for i in range(int((100-97.50)/step)):
      upper = 100-i*step
      lower = 100-(i+1)*step
      filteredMeasurements.append("EVr from " + str(upper) + " to " + str(lower))
      for m in fastestMeasurements:
        if m.EV<=upper and m.EV>lower and m.name[0]!='D':
          pattern = [p for p in primes if p.name == m.name]
          if len(pattern) != 0:
            filteredMeasurements.append(\
              m.toString() + \
              " " + \
              ''.join([str(elem) for elem in pattern[0].accessPattern]))
      filteredMeasurements.append("")
      
    print("Writing Patterns [to " + argv[3] + "]")
    PatternsFile = open(argv[3], 'w')
    [PatternsFile.write(m + "\n") for m in filteredMeasurements]
    PatternsFile.close()
    return


  if argv[0]=="mutation":
    
    print("Reading Primes [from " + argv[1] + "]")
    PatternsFile = open(argv[1], 'r')
    primes = [classPrime.fromString(x) for x in filter(None, PatternsFile.read().split("\n"))]
    PatternsFile.close()

    print("Mutating Primes")
    primes = mutated_Primes(primes, keepStride=1)
    
    print("Writing Patterns [to " + argv[2] + "]")
    PatternsFile = open(argv[2], 'w')
    [PatternsFile.write(str(p) + "\n") for p in primes]
    PatternsFile.close()
  
  print("Number of primes: " + str(len(primes)))
  sys.exit(len(primes))
    
if __name__ == '__main__':
  main(sys.argv[1:])