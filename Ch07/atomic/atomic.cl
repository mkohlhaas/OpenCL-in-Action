kernel void atomic(global int* x) {

   local int a, b;

   a = 0;
   b = 0;

   /* Increment without atomic add */
   a++;

   /* Increment with atomic add */
   atomic_inc(&b);

   x[0] = a;
   x[1] = b;
}
