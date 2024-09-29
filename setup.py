import os
import numpy
from Cython.Distutils import build_ext
from distutils.core import setup
from distutils.extension import Extension

# Remove the GPU FFT source files (gpu_fft_source_files)
# and path (gpu_fft_src_path)

# Keep your Python and C source files
sources = ['src/rpi_audio_levels.pyx', 'src/_rpi_audio_levels.c']

# Update the README text
README = """Python binding allowing to retrieve audio levels by frequency bands given
audio samples (power spectrum in fact), on a Raspberry Pi, using FFTW"""

setup(
    name="rpi_audio_levels",
    version='0.1.1',
    url='https://github.com/colin-guyon/rpi-audio-levels',
    author='Colin Guyon',
    description='Python binding for Raspberry Pi FFT using FFTW',
    long_description=README,
    install_requires=[
        'cython>=0.19.1',
        'numpy',
    ],
    cmdclass={'build_ext': build_ext},
    include_dirs=[numpy.get_include(), '/usr/include'],
    ext_modules=[
        Extension(
            'rpi_audio_levels',
            sources=sources,
            libraries=['fftw3'],  # Link to FFTW library
            extra_compile_args=['-O3'],  # Optimization for FFTW
            extra_link_args=['-lfftw3', '-lm'],  # Link FFTW and math library
        )
    ]
)
