/*
Updated to use FFTW library instead of the outdated GPU FFT
*/
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <fftw3.h>  // Include FFTW header

static fftw_plan fft_plan;
static fftw_complex *fft_in, *fft_out;
static int fft_size;
static int BANDS_COUNT;
static float* all_results;

// Circular Queue for storing results
#define QUEUE_ELEMENTS 50
#define QUEUE_SIZE (QUEUE_ELEMENTS + 1)
float *Queue[QUEUE_ELEMENTS];
int QueueIn;

void QueueInit(int bands_count)
{
    int i, j;
    for (i = 0; i < QUEUE_ELEMENTS; i++) {
        Queue[i] = (float*) malloc(bands_count * sizeof(float));
        for (j = 0; j < bands_count; j++) {
            Queue[i][j] = 12.0;
        }
    }
    QueueIn = 0;
}

void QueueRelease()
{
    int i;
    for (i = 0; i < QUEUE_ELEMENTS; i++) {
        free(Queue[i]);
    }
}

int QueuePut(float* new_data)
{
    Queue[QueueIn] = new_data;
    QueueIn = (QueueIn + 1) % QUEUE_ELEMENTS;
    return 0;
}

float* QueuePop()
{
    return Queue[QueueIn];
}

/* Initial method to call before being able to compute audio levels */
int prepare(int size, int bands_count) {
    printf("prepare fftw\n");

    fft_size = size;
    BANDS_COUNT = bands_count;

    // Allocate input and output arrays for FFTW
    fft_in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * fft_size);
    fft_out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * fft_size);

    // Create FFTW plan
    fft_plan = fftw_plan_dft_1d(fft_size, fft_in, fft_out, FFTW_FORWARD, FFTW_ESTIMATE);

    if (!fft_plan) {
        printf("Error: FFTW plan creation failed!\n");
        return -1;
    }

    QueueInit(bands_count);

    // Buffer to store levels + current mean for each band + current standard deviation for each band
    all_results = (float*) malloc(3 * bands_count * sizeof(float));

    return 0;
}

/* Get the audio levels for the given data and frequency-band indexes, using FFTW to compute the FFT */
float* compute(float* data, int** bands_indexes) {
    int i, j;
    float s;
    float mean, std_deviation;

    // Copy data into fft_in (real part) and set imaginary part to 0
    for (i = 0; i < fft_size; i++) {
        fft_in[i][0] = data[i];  // Real part
        fft_in[i][1] = 0;        // Imaginary part
    }

    // Execute FFTW plan
    fftw_execute(fft_plan);

    // Get result buffer from the queue
    float *result = QueuePop();

    for (i = 0; i < BANDS_COUNT; i++) {
        s = 0.0;
        for (j = bands_indexes[i][0]; j < bands_indexes[i][1]; j++) {
            // Calculate the power spectrum (magnitude squared)
            s += fft_out[j][0] * fft_out[j][0] + fft_out[j][1] * fft_out[j][1];
        }
        // Logarithmic scaling (similar to human hearing perception)
        result[i] = all_results[i] = log10(s);
    }

    QueuePut(result);

    // Calculate mean and standard deviation
    for (i = 0; i < BANDS_COUNT; i++) {
        mean = 0.0;
        std_deviation = 0.0;

        // Compute mean
        for (j = 0; j < QUEUE_ELEMENTS; j++) {
            if (Queue[j][i] > 0) {
                mean += Queue[j][i];
            }
        }
        mean /= QUEUE_ELEMENTS;

        // Compute standard deviation
        for (j = 0; j < QUEUE_ELEMENTS; j++) {
            if (Queue[j][i] > 0) {
                std_deviation += (Queue[j][i] - mean) * (Queue[j][i] - mean);
            }
        }
        std_deviation = sqrt(std_deviation / QUEUE_ELEMENTS);

        // Store results
        all_results[BANDS_COUNT + i] = mean;
        all_results[(BANDS_COUNT << 1) + i] = std_deviation;
    }

    return all_results;
}

/* Free resources when computing is not needed anymore */
int release(void) {
    printf("release fftw\n");

    // Free FFTW resources
    fftw_destroy_plan(fft_plan);
    fftw_free(fft_in);
    fftw_free(fft_out);

    free(all_results);
    QueueRelease();

    return 0;
}
