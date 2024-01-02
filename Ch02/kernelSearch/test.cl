kernel void add(global float *a,
                global float *b,
                global float *c) {
   *c = *a + *b;
}

kernel void sub(global float *a,
                global float *b,
                global float *c) {
   *c = *a - *b;
}

kernel void mult(global float *a,
                 global float *b,
                 global float *c) {
   *c = *a * *b;
}

kernel void div(global float *a,
                global float *b,
                global float *c) {
   *c = *a / *b;
}
