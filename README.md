# Audio Levels Calculation with FFTW on Raspberry Pi

This project allows you to calculate audio levels by frequency using the **FFTW** library on a Raspberry Pi. It is designed to process audio data and compute power spectrum levels across different frequency bands, leveraging **FFTW** for fast Fourier transformations (FFT).

## Features

- Perform real-time audio processing using FFT.
- Divide audio frequencies into custom bands and compute power levels.
- Efficient computation using the highly optimized **FFTW** library.

## Prerequisites

Before using this library, you will need to install the following dependencies:

### 1. FFTW library:
You can install **FFTW** on a Raspberry Pi with:

```bash
sudo apt-get install libfftw3-dev
```
### 2. Cython:
Install **Cython** with pip:
```bash
pip install cython
```

### 3. NumPy:
Install **NumPy**
```bash
pip install numpy
```

## Installation
After installing the necessary dependencies, you can build the Python extension with the following command:
```bash
python setup.py build_ext --inplace
```
This generates the `rpi_audio_levels.so`, be sure to add its directory to the PYTHONPATH
(or install it using ``sudo python setup.py install`` instead of the above command).

## Usage

```python
import numpy as np
from rpi_audio_levels import AudioLevels

# Create an instance of AudioLevels, with an FFT size of 10 and a bands count of 5
audio_levels = AudioLevels(10, 5)

# Define the frequency bands you want to analyze (e.g., 5 bands)
frequency_bands_indexes = [[0, 200], [200, 400], [400, 600], [600, 800], [800, 1000]]

# Generate some dummy audio data for testing
data = np.random.rand(1024).astype(np.float32)  # Simulated audio data

# Compute the audio levels, means, and standard deviations for the specified bands
levels, means, stds = audio_levels.compute(data, frequency_bands_indexes)

print("Levels:", levels)
print("Means:", means)
print("Standard Deviations:", stds)
```

The returned value is a tuple of tuple:
  - levels: power spectrum levels for each frequency band
  - means: means for each band
  - stds: standard deviation for each band

``data`` must be a numpy array of 2**DATA_SIZE real data with float dtype (np.float32),
with only 1 dimension.  

### How It Works
**FFTW**: The **FFTW** library is used for fast Fourier transform (FFT) computations, which convert time-domain audio signals into their frequency-domain representations. This allows us to analyze the power levels across specific frequency bands.  
**Frequency Bands**: You define the frequency bands you're interested in (e.g., low, mid, high frequencies), and the library computes the audio levels for each band.  

### Methods
**prepare(fft_size, bands_count)**: Initializes the FFT computation with the given size and number of bands.  
**compute(data, frequency_bands_indexes)**: Computes the audio levels, means, and standard deviations for the given frequency bands.  
**release()**: Frees up the resources when FFT computation is no longer needed.  
