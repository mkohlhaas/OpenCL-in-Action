kernel void add(global float *a,
                global float *b,
                global float *c) {
   *c = *a + *b;
}

kernel void subtract(global float *a,
                     global float *b,
                     global float *c) {
   *c = *a - *b;
}

kernel void multiply(global float *a,
                     global float *b,
                     global float *c) {
   *c = *a * *b;
}

kernel void divide(global float *a,
                   global float *b,
                   global float *c) {
   *c = *a / *b;
}
