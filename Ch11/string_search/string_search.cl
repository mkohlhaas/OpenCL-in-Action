kernel void string_search(char16       pattern,
                          global char* text,
                          int          chars_per_item,
                          local int*   local_result,
                          global int*  global_result) {

   char16 text_vector, check_vector;

   // initialize local data
   local_result[0] = 0;
   local_result[1] = 0;
   local_result[2] = 0;
   local_result[3] = 0;

   // make sure previous processing has completed
   barrier(CLK_LOCAL_MEM_FENCE);

   int item_offset = get_global_id(0) * chars_per_item;

   // iterate through characters in text
   for(int i=item_offset; i < item_offset + chars_per_item; i++) {

      // load global text into private buffer
      text_vector = vload16(0, text + i);

      // compare text vector and pattern
      check_vector = text_vector == pattern;

      // Check for 'that'
      if(all(check_vector.s0123)) {
         atomic_inc(local_result);
      }

      // check for 'with'
      if(all(check_vector.s4567)) {
         atomic_inc(local_result + 1);
      }

      // check for 'have'
      if(all(check_vector.s89AB)) {
         atomic_inc(local_result + 2);
      }

      // check for 'from'
      if(all(check_vector.sCDEF)) {
         atomic_inc(local_result + 3);
      }
   }

   // make sure local processing has completed
   barrier(CLK_GLOBAL_MEM_FENCE);

   // perform global reduction
   if(get_local_id(0) == 0) {
      atomic_add(global_result, local_result[0]);
      atomic_add(global_result + 1, local_result[1]);
      atomic_add(global_result + 2, local_result[2]);
      atomic_add(global_result + 3, local_result[3]);
   }
}
