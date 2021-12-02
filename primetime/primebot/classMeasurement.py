class classMeasurement:

  def __init__(self, name, iteration, EV, EVC, duration):
    self.name      = name
    self.iteration = iteration
    self.EV        = EV
    self.EVC       = EVC
    self.duration  = duration

  @classmethod
  def fromString(cls, s):
    s = s.split()
    name = s[0]
    iteration = s[1]
    EV = float(s[2])
    EVC = float(s[3])
    duration = [int(s[4]), int(s[5]), int(s[6])]
    return cls(name, iteration, EV, EVC, duration)

  def toString(self):
    txt = "{:22s} {} {} {} {}".format(\
      self.name, \
      self.iteration, \
      self.EV, \
      self.EVC, \
      self.duration)
    return txt
    
  def __str__(self):
    return  "name      : {}\n"\
            "iteration : {}\n"\
            "EV        : {:3.3f}%\n"\
            "EVC       : {:3.3f}%\n"\
            "Cycles    : {} {} {}".format(\
      self.name, 
      self.iteration, 
      self.EV, 
      self.EVC, 
      self.duration[0], self.duration[1], self.duration[2])

  def isGood(self, min_evc_rate, max_99_duration):
    return (self.EVC > min_evc_rate) and (self.duration[2] < max_99_duration)

  def get99(self):
    return self.duration[2]