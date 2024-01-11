kernel void atomic(global int* x) {
   local int a, b;
   a = 0;
   b = 0;

   // increment WITHOUT atomic add
   a++;

   // increment WITH atomic add
   atomic_inc(&b);

   x[0] = a;
   x[1] = b;
}
