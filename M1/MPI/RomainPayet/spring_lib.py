import math
import random
import numpy as np
import matplotlib.pyplot as plt
from collections import namedtuple

ID, POSX, POSY, SPEEDX, SPEEDY, WEIGHT = range(6)
DIST, STIFFNESS, IDA, IDB = range(4)

UNIT=1000

def display_plot(data, springs=None):
    plt.gcf().clear()            # to remove to see the traces of the particules...
    plt.axis((-.2, 1.4, -.2, 1.4))

    xx = [ float(d[POSX])/UNIT  for d in data]
    yy = [ float(d[POSY])/UNIT  for d in data]
    plt.plot(xx, yy, 'ro')

    if not springs is None:
        for s in springs:
            plt.plot([float(data[s[IDA]][POSX])/UNIT,
                      float(data[s[IDB]][POSX])/UNIT],
                     [float(data[s[IDA]][POSY])/UNIT,
                      float(data[s[IDB]][POSY])/UNIT])
    plt.draw()
    plt.pause(0.00001)            # in order to see something otherwise too fast...

def generate_springs(nb_masses, nb_strings):
    random.seed(0)
    masses = np.zeros((nb_masses, 6), dtype='i')
    for i in range(nb_masses):
        masses[i][ID] = i
        masses[i][POSX] = random.uniform(0,1)*UNIT
        masses[i][POSY] = random.uniform(0,1)*UNIT
        masses[i][SPEEDX], masses[i][SPEEDY] = 0, 0
        masses[i][WEIGHT] = 10 + 10*random.uniform(0,1) + 10*i

    springs = [ (random.uniform(0.3,1)*UNIT, random.uniform(.1,1)*UNIT, i, j)
                for i in range(nb_masses) for j in range(nb_masses) if i>j
               ]
    if nb_strings != -1:
        springs = random.sample(springs, k=nb_strings)

    return masses, np.array(springs, dtype='i')


def interaction(spring, data):
    a = data[spring[IDA]]
    b = data[spring[IDB]]

    dx = a[POSX]-b[POSX]
    dy = a[POSY]-b[POSY]
    dist = math.sqrt(dx*dx + dy*dy)
    force = spring[STIFFNESS]/UNIT*(dist-spring[DIST]/UNIT)

    if dist < 0.1*UNIT:
        return np.zeros(2, dtype='i')
    else:
        return -np.array([dx* force / dist, dy* force / dist], dtype='i') 

def update(data, i, force):
    new_speed = data[i][SPEEDX:SPEEDY+1] + force / data[i][WEIGHT]
    new_pos = data[i][POSX:POSY+1] + new_speed

    data[i][POSX] = max(-10*UNIT, min(10*UNIT, new_pos[0]))
    data[i][POSY] = max(-10*UNIT, min(10*UNIT, new_pos[1]))
    data[i][SPEEDX] = new_speed[0]
    data[i][SPEEDY] = new_speed[1]

def signature(data):
    return sum([d[POSX] - d[POSY]  for d in data])
