kernel void mod_round(global float*  mod_input,
                      global float*  mod_output,
                      global float4* round_input,
                      global float4* round_output) {

   /* Use fmod and remainder: 317.0, 23.0 */
   mod_output[0] = fmod(mod_input[0], mod_input[1]);
   mod_output[1] = remainder(mod_input[0], mod_input[1]);

   /* Rounds the input values: -6.5, -3.5, 3.5, and 6.5 */
   round_output[0] = rint(*round_input);
   round_output[1] = round(*round_input);
   round_output[2] = ceil(*round_input);
   round_output[3] = floor(*round_input);
   round_output[4] = trunc(*round_input);
}
