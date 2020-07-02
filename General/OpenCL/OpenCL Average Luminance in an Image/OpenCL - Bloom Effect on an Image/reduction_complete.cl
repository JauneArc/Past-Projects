__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

//Gaussian Blurring Filters
__constant float Blurringx3Filter[3] = {0.27901, 0.44198, 0.27901};
__constant float Blurringx5Filter[5] = {0.06136, 0.24477, 0.38774, 0.24477, 0.06136};
__constant float Blurringx7Filter[7] = {0.00598, 0.060626, 0.241843, 0.383103, 0.241843, 0.060626, 0.00598};

//Kernel 0
__kernel void ImgToFloat(read_only image2d_t src_image, __global float* outputLuminance){
	int2 coord = (int2)(get_global_id(0), get_global_id(1));
	float4 pixel = read_imagef(src_image, sampler, coord);
	int index = get_global_id(0) * get_global_size(0) + get_global_id(1);
	outputLuminance[index] = 0.299f * (pixel.x * 255) + 0.587f * (pixel.y * 255) + 0.114f * (pixel.z * 255);
}
//Kernel 1
__kernel void reduction_vector(__global float4* data,
                   __local float4* partialSums)
{
	int lid = get_local_id(0);
	int groupSize = get_local_size(0);

	partialSums[lid] = data[get_global_id(0)];
	barrier(CLK_LOCAL_MEM_FENCE);

	for (int i = groupSize / 2; i > 0; i >>= 1)
	{
		if (lid < i)
		{
			partialSums[lid] += partialSums[lid + i];
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}

	if (lid == 0)
	{
		data[get_group_id(0)] = partialSums[0];
}
}
//Kernel 2
__kernel void reduction_complete(__global float4* data,
                       __local float4* partialSums,
                       __global float* sum)
{
	int lid = get_local_id(0);
	int groupSize = get_local_size(0);

	partialSums[lid] = data[get_local_id(0)];
	barrier(CLK_LOCAL_MEM_FENCE);

	for (int i = groupSize / 2; i > 0; i >>= 1)
	{
		if (lid < i)
		{
			partialSums[lid] += partialSums[lid + i];
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}

	if (lid == 0)
	{
		*sum = partialSums[0].s0 + partialSums[0].s1 +
			partialSums[0].s2 + partialSums[0].s3;
}
}
//Kernel 3
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
//Kernel 4
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
//Kernel 5
__kernel void PixelBlacking(read_only image2d_t src_image,
					write_only image2d_t dst_image, float luminanceAverage) {
   	int2 coord = (int2)(get_global_id(0), get_global_id(1));
	float4 pixel = read_imagef(src_image, sampler, coord);
	float luminance = 0.299f * (pixel.x * 255) + 0.587f * (pixel.y * 255) + 0.114f * (pixel.z * 255);

	if (luminance < luminanceAverage)
	{
		pixel.xyz = 0.0f;
	}

	write_imagef(dst_image, coord, pixel);
}
//Kernel 6
__kernel void ImgAdd(read_only image2d_t src_image, read_only image2d_t src_image2,
					write_only image2d_t dst_image) {

	int2 coord = (int2)(get_global_id(0), get_global_id(1));
	float4 pixelA = read_imagef(src_image, sampler, coord);
	float4 pixelB = read_imagef(src_image2, sampler, coord);
	write_imagef(dst_image, coord, pixelA + pixelB);

}