/* Enable the double type if available */
#ifdef FP_64
#pragma OPENCL EXTENSION cl_khr_fp64: enable
#endif

kernel void double_test(global float* a,
                        global float* b,
                        global float* out)
{
/* Declare a double variable if possible. */
/* Use floats if doubles are unavailable. */

#ifdef FP_64
   double c = (double)(*a / *b);
   *out = (float)c;
#else
   *out = *a * *b;
#endif
}
