__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | 
      CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST; 

__kernel void ImgToFloat(read_only image2d_t src_image, __global float* data){
   int column = get_global_id(0); 
   int row = get_global_id(1);
   int gid = get_global_id(0); 
   if(gid != 0){
	  gid = gid * 4;
   }
   
   int2 coord;
   float4 pixel;
   for(int i = 0; i <= 1; i++) {
		coord.y = row +i;
		coord.x = column;
		/* Read value pixel from the image */ 		
		pixel = read_imagef(src_image, sampler, coord);
		/* Accumulate weighted sum */ 		
		data[gid  ] = pixel.x * 0.299f;
		data[gid+1] = pixel.y * 0.587f;
		data[gid+2] = pixel.z * 0.114f;
		data[gid+3] = 0.0f;
	}

}

