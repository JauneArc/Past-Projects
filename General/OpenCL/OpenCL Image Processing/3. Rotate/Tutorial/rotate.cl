__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | 
      CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST; 

__kernel void rotate_image(read_only image2d_t src_image,
					write_only image2d_t dst_image,
					float sin_theta,
					float cos_theta) {

   /* Get pixel coordinate */
   int2 coord = (int2)(get_global_id(0), get_global_id(1));
 
   /* Calculate rotation offset */
   float x0 = get_global_size(0)/2.0f;
   float y0 = get_global_size(1)/2.0f;

   float xOff = coord.x - x0;
   float yOff = coord.y - y0; 

   /* Calculate rotated coordinates */
   int xpos = (int)(xOff*cos_theta + yOff*sin_theta + x0);
   int ypos = (int)(yOff*cos_theta - xOff*sin_theta + y0); 
   int2 rotated_coord = (int2)(xpos, ypos);

   /* Read pixel value */
   float4 pixel = read_imagef(src_image, sampler, rotated_coord);

   /* Write new pixel value to output */
   write_imagef(dst_image, coord, pixel);
}
