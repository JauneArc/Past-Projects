__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | 
      CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST; 

// x3 Gaussian Blurring Filter
__constant float Blurringx3Filter[3] = {0.27901, 0.44198, 0.27901};
// x5 Gaussian Blurring Filter
__constant float Blurringx5Filter[5] = {0.06136, 0.24477, 0.38774, 0.24477, 0.06136};
// x7 Gaussian Blurring Filter
__constant float Blurringx7Filter[7] = {0.00598, 0.060626, 0.241843, 0.383103, 0.241843, 0.060626, 0.00598};

__kernel void Hozizontal_Pass(read_only image2d_t src_image,
					write_only image2d_t dst_image, int weighting) {
   /* Get work-item’s row and column position */
   int column = get_global_id(0); 
   int row = get_global_id(1);
   int FiltertoUse = weighting;

   /* Accumulated pixel value */
   float4 sum = (float4)(0.0);

   /* Filter's current index */
   int filter_index =  0;

   int2 coord;
   float4 pixel;

   if(FiltertoUse == 51)
   {
	/* Iterate over the rows */
	for(int i = -1; i <= 1; i++) {
		  coord.y =  row + i;
		  coord.x = column;
		  /* Read value pixel from the image */ 		
		  pixel = read_imagef(src_image, sampler, coord);
		 /* Accumulate weighted sum */ 		
		sum.xyz += pixel.xyz * Blurringx3Filter[filter_index++];
	}
   }
   if(FiltertoUse == 53)
   {
	/* Iterate over the rows */
	for(int i = -2; i <= 2; i++) {
		  coord.y =  row + i;
		  coord.x = column;
		 /* Read value pixel from the image */ 		
		 pixel = read_imagef(src_image, sampler, coord);
		 /* Accumulate weighted sum */ 		
		sum.xyz += pixel.xyz * Blurringx5Filter[filter_index++];
	}   
   }
   if(FiltertoUse == 55)
   {
	/* Iterate over the rows */
	for(int i = -3; i <= 3; i++) {
		  coord.y =  row + i;
		  coord.x = column;
		 /* Read value pixel from the image */ 		
		 pixel = read_imagef(src_image, sampler, coord);
		 /* Accumulate weighted sum */ 		
		sum.xyz += pixel.xyz * Blurringx7Filter[filter_index++];
		
	}   
   }
   /* Write new pixel value to output */
   coord = (int2)(column, row); 
   write_imagef(dst_image, coord, sum);

}
__kernel void Vertical_Pass(read_only image2d_t src_image,
					write_only image2d_t dst_image, int weighting) {
   /* Get work-item’s row and column position */
   int column = get_global_id(0); 
   int row = get_global_id(1);
   int FiltertoUse = weighting;

   /* Accumulated pixel value */
   float4 sum = (float4)(0.0);

   /* Filter's current index */
   int filter_index =  0;

   int2 coord;
   float4 pixel;

   if(FiltertoUse == 51)
   {
	/* Iterate over the rows */
	for(int i = -1; i <= 1; i++) {
		  coord.x =  column + i;
		  coord.y = row;
		  /* Read value pixel from the image */ 		
		  pixel = read_imagef(src_image, sampler, coord);
		 /* Accumulate weighted sum */ 		
		sum.xyz += pixel.xyz * Blurringx3Filter[filter_index++];
	}
   }
   if(FiltertoUse == 53)
   {
	/* Iterate over the column */
	for(int i = -2; i <= 2; i++) {
		  coord.x =  column + i;
		  coord.y = row;
		 /* Read value pixel from the image */ 		
		 pixel = read_imagef(src_image, sampler, coord);
		 /* Accumulate weighted sum */ 		
		sum.xyz += pixel.xyz * Blurringx5Filter[filter_index++];
	}   
   }
   if(FiltertoUse == 55)
   {
	/* Iterate over the column */
	for(int i = -3; i <= 3; i++) {
		  coord.x =  column + i;
		  coord.y = row;
		 /* Read value pixel from the image */ 		
		 pixel = read_imagef(src_image, sampler, coord);
		 /* Accumulate weighted sum */ 		
		sum.xyz += pixel.xyz * Blurringx7Filter[filter_index++];
		
	}   
   }
   /* Write new pixel value to output */
   coord = (int2)(column, row); 
   write_imagef(dst_image, coord, sum);

}