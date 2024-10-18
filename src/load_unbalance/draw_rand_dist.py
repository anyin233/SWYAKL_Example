import numpy as np
import matplotlib.pyplot as plt
import math

# Initialize parameters
N = 2048
gen = np.random.default_rng(10)
d = np.random.normal(loc=N, scale=1.0, size=(N, N))

# Calculate cnt values based on the distribution
cnt = np.zeros((2 * N + 1,))

for i in range(1, N+1):
    for j in range(1, N+1):
        cnt[i + j] =  N - abs(N * math.sin(d[i-1, j-1]))

# Plot the frequency distribution of the generated cnt values
plt.hist(cnt, bins=20, edgecolor='black')
plt.title('Frequency Distribution of Generated cnt Values')
plt.xlabel('cnt Values')
plt.ylabel('Frequency')
plt.grid(True)
plt.show()