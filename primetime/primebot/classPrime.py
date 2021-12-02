class classPrime:

  def __init__(self, name, iteration_min, iteration_max, loop, stride, accessPattern):
    self.name          = name
    self.iteration_min = iteration_min                
    self.iteration_max = iteration_max                
    self.loop          = loop      
    self.stride        = stride             
    self.accessPattern = accessPattern               

  def __str__(self):
    selfStr  = self.name
    selfStr += " {a}".format(a=self.iteration_min)
    selfStr += " {a}".format(a=self.iteration_max)
    selfStr += " {a}".format(a=self.loop)
    selfStr += " {a}".format(a=self.stride)
    selfStr += " {a}".format(a=len(self.accessPattern))
    selfStr += " {a}".format(a=''.join([str(elem) for elem in self.accessPattern]))
    return selfStr

  @classmethod
  def fromString(cls, s):
    s = s.split()
    name = s[0]
    iteration_min = s[1]
    iteration_max = s[2]
    loop = s[3]
    stride = s[4]
    accessPattern = list(s[6])
    return cls(name, iteration_min, iteration_max, loop, stride, accessPattern)
  
  def hasName(self, name):
    return (self.name == name)

  def createHeader(self):
    tStr = "void traverse_{a}(uint64_t* arr);\n".format(a=self.name)
    return tStr

  def createFunction(self):
    tStr  = "void traverse_{a}(uint64_t* arr) {{\n".format(a=self.name)
    tStr += "  int i;\n"
    inc = 1 if self.loop == 1 else self.stride
    tStr += "  for(i=0; i<{a}; i+={b}) {{\n".format(a=self.loop, b=inc)
    for access in self.accessPattern:
      if access == 'S':
        tStr += "    maccess((void *) arr[  0]);\n"
      else:
        tStr += "    maccess((void *) arr[i+{a}]);\n".format(a=access)
    tStr += "  }\n"
    tStr += "}\n\n"
    return tStr

  def createAccess(self):
    tStr  = "config.traverse = &traverse_{a};\n".format(a=self.name)
    tStr += "for(i={a}; i<={b}; i++) {{\n".format(a=self.iteration_min, b=self.iteration_max)
    tStr += "  memcpy(config.name, \"{a}\", 20);\n".format(a=self.name)
    tStr += "  config.rounds = i;\n"
    tStr += "  test_pattern(evset_ptr, target_index, config, SW_THR, SW_LLC, file_measurements);\n"
    tStr += "}\n\n"
    return tStr