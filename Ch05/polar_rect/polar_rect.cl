kernel void polar_rect(global float4* r_vals,
                       global float4* angles,
                       global float4* x_coords,
                       global float4* y_coords) {

   *y_coords = sincos(*angles, x_coords);
   *x_coords *= *r_vals;
   *y_coords *= *r_vals;
}
