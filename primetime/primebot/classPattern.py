from itertools import product 
import re
import random

class classPattern(dict):

  def __init__(self, d):
    self._dict = d

  def __str__(self):
    selfStr = ""
    for key, pattern in self._dict.items():
      ptrnStr  = ' '.join('X' if value == 'S' else str(value) for value in pattern)
      selfStr += '  %-10s : %s' % (key, ptrnStr) + '\n'
    return selfStr[:-1]

  def dictionary(self):
    return self._dict

  def items(self):
    return self._dict.items()

  def concat(self, d):
    self._dict = {**self._dict , **d}

  def dummies(self, len):
    l = ['0', '1', '2', '3', 'S']
    self._dict = {}
    for i in range(len):
      self._dict['D{:03}'.format(i)] = random.sample(l, 4)
      
  def primitives(self, maxlen):
    self._dict = {}
    for i in range(2,maxlen+1):
      self._dict['B{}{:02}'.format(maxlen, i)] = list(range(i))

  def prod(self, max, withReverse):
    r = classPattern(dict())
    for name, pattern in self._dict.items():  
      d = dict()
      key = 'P' + ('R' if withReverse else 'S') + str(max)
      index = 0
      if withReverse:
        products = [list(product([pattern, pattern[::-1]], repeat=x)) for x in range(2,max+1)]
      else:
        products = [list(product([pattern], repeat=x)) for x in range(2,max+1)]
      for p in products: 
        a = [sum(i, []) for i in p]
        for i in a:
          d[name+'_' + key + '{:02}'.format(index)] = i
          index += 1
      r._dict = {**r._dict, **d}
    return r

  def evc(self, kind):

    def evc_param(key, pattern, name, lenght, gap, repeat):
      access = ['S']*lenght
      mask = access
      [mask.append(' ') for i in range(gap)]  
      mask = mask*(repeat+1)
      if gap != 0:
        mask = mask[:-gap]
      
      mask    = [' ']*len(pattern) + mask + [' ']*len(pattern)
      pattern = [str(p) if isinstance(p, int) else p for p in pattern]

      plen    = len(pattern)
      mlen    = len(mask)

      patterns = []
      for offset in range(mlen):
        plist = pattern + [' ']*(len(mask)-plen)
        l  = [' ' if i<plen else mask[i] for i in range(offset)]
        l += ['S' if mask[offset+i]=='S' else plist.pop(0) for i in range(len(mask)-offset)]

        txt = ''.join(l).strip()
        if (txt.find(' ') == -1):
          l = list(txt)
          patterns.append(l) if l not in patterns else patterns
          
      d = dict()
      index = 0
      for p in patterns:
        d[key+'_' + name + '{:02}'.format(index)] = p
        index += 1
      return d

    r = classPattern(dict())
    for name, pattern in self._dict.items():  
      d = dict()
      if kind==0:
        d =         evc_param(name, pattern, 'E'+str(kind)+'0', lenght=1, gap=0, repeat=0)
        d = {**d, **evc_param(name, pattern, 'E'+str(kind)+'1', lenght=2, gap=0, repeat=0)}
        d = {**d, **evc_param(name, pattern, 'E'+str(kind)+'2', lenght=1, gap=1, repeat=1)}
        d = {**d, **evc_param(name, pattern, 'E'+str(kind)+'3', lenght=1, gap=2, repeat=1)}
        d = {**d, **evc_param(name, pattern, 'E'+str(kind)+'4', lenght=2, gap=1, repeat=1)}
        d = {**d, **evc_param(name, pattern, 'E'+str(kind)+'5', lenght=2, gap=2, repeat=1)}
      elif kind==1:
        d  = evc_param(name, pattern, 'E'+str(kind)+'0', lenght=1, gap=0, repeat=0)
      r._dict = {**r._dict, **d}
    return r

  def mutation(self):
    r = classPattern(dict())
    for key, pattern in self._dict.items():
      keys = key.split('_')

      d = dict()
      if keys[1][0:2] == 'PS':
        # B402_PS208_E1006_S2 -> B402_PR2**_E1006_S2
        
        blen  = int(keys[0][1])
        plen  = int(keys[1][2])
        ekind = int(keys[2][1])

        b = primitives(blen).get(keys[0])
        p = prod(keys[0], b, max=plen, withReverse=1)
        e = dict()
        for key, pattern in p.items():
          e = {**e, **evc(key, pattern, kind=ekind)}
        
        exp = "{}_PR{}\d\d_{}".format(keys[0], plen, keys[2])
        d = {k: v for k, v in e.items() if re.match(exp, k)}

      elif keys[2][0:2] == 'E0':
        # B402_PR308_E0***_S2 -> B402_PR308_E2***_S2
        
        blen  = int(keys[0][1])
        plen  = int(keys[1][2])

        b = primitives(blen).get(keys[0])
        p = prod(keys[0], b, max=plen, withReverse=1)
        e = dict()
        for key, pattern in p.items():
          e = {**e, **evc(key, pattern, kind=1)}

        exp = "{}_{}_E1\d\d\d".format(keys[0], keys[1])
        d = {k: v for k, v in e.items() if re.match(exp, k)}

      # elif ...:
        # Can consider swaps
        # B402_E0006_S2 -> B402_E0006_SW*_S2
      
      else:
        d ={**d, **{key[:-3]: pattern}}

      r._dict = {**r._dict, **d}
    return r
    