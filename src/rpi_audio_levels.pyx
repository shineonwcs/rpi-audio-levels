#cython: wraparound=False
#cython: boundscheck=False
#cython: cdivision=True

import numpy as np
cimport numpy as np
cimport cython
from libc.stdlib cimport malloc, free

FLOAT32_DTYPE = np.float32
ctypedef np.float32_t FLOAT32_DTYPE_t

# Import C functions from your updated _rpi_audio_levels.h
cdef extern from "_rpi_audio_levels.h":
    int prepare(int size, int bands_count)
    float* compute(float* data, int** bands_indexes)
    int release()

cdef class AudioLevels:
    """ 
    Allows calculating audio levels by frequency using the FFTW library.

    a = AudioLevels(10, <bands_count>)
    ...
    frequency_bands_indexes = [[0,200], [200,400], ...]
    levels, means, stds = a.compute(<float_numpy_data_1D>, frequency_bands_indexes)
    ...
    """

    def __cinit__(self, int fft_size, int bands_count):
        """ Prepare the FFT computation with the given size and number of bands. """
        if prepare(fft_size, bands_count) != 0:
            raise RuntimeError("Failed to initialize FFT computation")

    def __dealloc__(self):
        """ Release any resources when the object is deleted. """
        release()

    @cython.boundscheck(False)
    @cython.wraparound(False)
    def compute(self,
                np.ndarray[FLOAT32_DTYPE_t, ndim=1, mode='c'] data not None,
                list bands_indexes not None):
        """ 
        Perform FFT on the input data and return audio levels, means, and standard deviations 
        for the specified frequency bands.
        """
        cdef:
            unsigned int i
            int **c_bands_indexes
            unsigned int bands_count = len(bands_indexes)
            float[:] data_memview = data
            float *c_data = &data_memview[0]  # Pointer to the numpy data
            float *result  # Pointer to the raw result from C code

            py_levels = [None] * bands_count  # Levels to return to Python
            py_means = [None] * bands_count   # Means to return to Python
            py_stds = [None] * bands_count    # Standard deviations to return to Python

        # Allocate memory for the C array from Python array (2D array)
        c_bands_indexes = <int**> malloc(bands_count * sizeof(int*))
        if not c_bands_indexes:
            raise MemoryError("Failed to allocate memory for band indexes")

        for i in range(bands_count):
            c_bands_indexes[i] = <int*> malloc(2 * sizeof(int))
            if not c_bands_indexes[i]:
                raise MemoryError(f"Failed to allocate memory for band index {i}")
            c_bands_indexes[i][0] = bands_indexes[i][0]
            c_bands_indexes[i][1] = bands_indexes[i][1]

        # Execute the FFT and compute the audio levels
        result = compute(c_data, c_bands_indexes)

        if result is NULL:
            raise RuntimeError("FFT computation failed")

        # Convert the result array to Python lists
        for i in range(bands_count):
            py_levels[i] = result[i]
            py_means[i] = result[bands_count + i]
            py_stds[i] = result[(bands_count << 1) + i]

        # Free the C arrays allocated earlier
        for i in range(bands_count):
            free(c_bands_indexes[i])
        free(c_bands_indexes)

        # Return levels, means, and standard deviations as Python lists
        return py_levels, py_means, py_stds
