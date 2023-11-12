#!/usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt

COEFFICIENT_CNT = 32

sine_coefficients = np.array([])
cosine_coefficients = np.array([])

samples = np.sin(2*np.pi*np.arange(0, 1024) / 256)

sample_cnt = len(samples)
interval = np.arange(sample_cnt)

avg = np.sum(samples) / sample_cnt
reconstructed = np.full((sample_cnt,), avg)

for n in range(1, COEFFICIENT_CNT + 1):
    sinewave = np.sin(n * np.pi * interval / sample_cnt)
    coswave = np.cos(n * np.pi * interval / sample_cnt)

    sine_coefficients = np.append(sine_coefficients, np.sum(samples * sinewave) / sample_cnt)
    cosine_coefficients = np.append(cosine_coefficients, np.sum(samples * coswave) / sample_cnt)

    reconstructed = reconstructed + (sinewave*sine_coefficients[n-1]) + (coswave*cosine_coefficients[n-1])

fig, axes = plt.subplots(2, 2)
axes[0, 0].set_title("Sine coefficients")
axes[0, 0].bar(range(1, COEFFICIENT_CNT+1), sine_coefficients)
axes[0, 1].set_title("Cosine coefficients")
axes[0, 1].bar(range(1, COEFFICIENT_CNT+1), cosine_coefficients)
axes[1, 0].set_title("Original waveform")
axes[1, 0].plot(interval, samples)
axes[1, 1].set_title("Reconstructed waveform")
axes[1, 1].plot(interval, reconstructed)
plt.show()
