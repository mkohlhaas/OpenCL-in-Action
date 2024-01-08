kernel void blank(global int4 *x) {
   for(int i=0; i<25; i++) {
      x[i] *= 2;
   }
}
