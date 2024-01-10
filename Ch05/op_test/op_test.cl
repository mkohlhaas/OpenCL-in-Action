kernel void op_test(global int4* output) {

   int4 vec = (int4)(1, 2, 3, 4);

   vec += 4;                                              // (5, 6, 7, 8)

   if(vec.s2 == 7) {
      vec &= (int4)(-1, -1, 0, -1);                       // (5, 6, 0, 8)
   }

   // the signed value for true is -1, for false it is 0
   vec.s01 = vec.s23 < 7;                                 // (-1, 0, 0, 8)

   while(vec.s3 > 7 && (vec.s0 < 16 || vec.s1 < 16)) {
      vec.s3 >>= 1;
   }

   *output = vec;                                         // (-1, 0, 0, 4)
}
