__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

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


