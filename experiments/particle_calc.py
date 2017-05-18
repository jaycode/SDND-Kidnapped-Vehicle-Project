import math
from decimal import Decimal


def meas_prob(tx, ty, lx, ly):
    std_x = 0.3
    std_y = 0.3
    a = ((tx-lx)**2)/(2*std_x**2)
    b = ((ty-ly)**2)/(2*std_y**2)
    return (1/(2*math.pi*std_x*std_y))*math.exp(-(a+b))

def final_weight(meas_probs):
    prod = 1
    for mp in meas_probs:
        prod *= mp
    return prod

mp = [meas_prob(6,3,5,3),
      meas_prob(2,2,2,1),
      meas_prob(0,5,4,7)
     ]

print('%.2E' % Decimal(mp[0]))
print('%.2E' % Decimal(mp[1]))
print('%.2E' % Decimal(mp[2]))

print('%.2E' % Decimal(final_weight(mp)))