import matplotlib.pyplot as plt
import math
i_mag = []
i_phase = []

v_mag = []
v_phase = []

freq = []
C = 100e-12
L = 10e-3


with open("top_hierarchy.txt") as file:
    file.readline()
    for line in file:
        line = line.split()
        freq.append(float(line[0]))
        i_phasor = line[1]
        i_phasor = i_phasor.split(",")
        #print(i_phasor)
        i_mag.append(float("".join([c for c in list(i_phasor[0]) if c in "-+.0123456789e"])))
        i_phase.append(float("".join([c for c in list(i_phasor[1]) if c in "-+.0123456789e"])))

        v_phasor = line[2]
        v_phasor = v_phasor.split(",")
        #print(i_phasor)
        v_mag.append(float("".join([c for c in list(v_phasor[0]) if c in "-+.0123456789e"])))
        v_phase.append(float("".join([c for c in list(v_phasor[1]) if c in "-+.0123456789e"])))

gain_mag = [v_mag[i] - (i_mag[i]-80 - 28) for i in range(len(v_mag))]
gain_phase = [v_phase[i] - i_phase[i] for i in range(len(v_phase))]
gain_phase = [(val + 360) % 360 for val in gain_phase]
ideal_mag = [20*math.log10(1/(2*3.14159*freq[i]*C)) for i in range(len(v_mag))]
#ideal_mag = [20*math.log10(2*3.14159*freq[i]*L) for i in range(len(v_mag))]

plt.plot(freq, v_mag)
plt.plot(freq, i_mag)
plt.plot(freq, gain_mag)
plt.plot(freq, ideal_mag)
plt.xscale('log')
plt.show()



