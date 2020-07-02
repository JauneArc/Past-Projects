#define _CRT_SECURE_NO_WARNINGS
#define PROGRAM_FILE "reduction_complete.cl"
#define KERNEL_FUNC "simple_conv"

#define INPUT_FILE "lenna.bmp"
#define OUTPUT_FILE "output.bmp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmpfuncs.h"

#ifdef MAC
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif


/* Find a GPU or CPU associated with the first available platform */
cl_device_id create_device() {

	cl_platform_id platform;
	cl_device_id dev;
	int err;
	
	/* Identify a platform */
	err = clGetPlatformIDs(1, &platform, NULL);
	if (err < 0) {
		perror("Couldn't identify a platform");
		exit(1);
	}
	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
	if (err == CL_DEVICE_NOT_FOUND) {
		err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
	}
	if (err < 0) {
		perror("Couldn't access any devices");
		exit(1);
	}
	return dev;
}

/* Create program from a file and compile it */
cl_program build_program(cl_context ctx, cl_device_id dev, const char* filename) {

	cl_program program;
	FILE *program_handle;
	char *program_buffer, *program_log;
	size_t program_size, log_size;
	int err;

	/* Read program file and place content into buffer */
	program_handle = fopen(filename, "rb");
	if (program_handle == NULL) {
		perror("Couldn't find the program file");
		exit(1);
	}
	fseek(program_handle, 0, SEEK_END);
	program_size = ftell(program_handle);
	rewind(program_handle);
	program_buffer = (char*)malloc(program_size + 1);
	program_buffer[program_size] = '\0';
	fread(program_buffer, sizeof(char), program_size, program_handle);
	fclose(program_handle);

	/* Create program from file */
	program = clCreateProgramWithSource(ctx, 1,
		(const char**)&program_buffer, &program_size, &err);
	if (err < 0) {
		perror("Couldn't create the program");
		exit(1);
	}
	free(program_buffer);

	/* Build program */
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err < 0) {

		/* Find size of log and print to std output */
		clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG,
			0, NULL, &log_size);
		program_log = (char*)malloc(log_size + 1);
		program_log[log_size] = '\0';
		clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG,
			log_size + 1, program_log, NULL);
		printf("%s\n", program_log);
		free(program_log);
		getchar();
		exit(1);
	}

	return program;
}



int main(int argc, char **argv) {

	/* Host/device data structures */
	cl_device_id device;
	cl_context context;
	cl_command_queue queue;
	cl_program program;
	cl_kernel *kernel;
	cl_uint num_kernels;
	cl_int err;
	size_t global_size[2];
	//cl_event start_event, end_event;
	//cl_int i;
	size_t local_size, gl_size;

	/* Sum data & Buffers */
	float sum;
	cl_mem sum_buffer;
	//cl_ulong time_start, time_end, total_time;
	float temp = 0;
	/* Image data */
	unsigned char* inputImage;
	cl_image_format img_format;
	cl_mem input_image, data_buffer;
	//size_t origin[3], region[3];
	size_t width, height;
	int w, h;
	float* data;

	/* Open input file and read image data */
	inputImage = readRGBImage(INPUT_FILE, &w, &h);
	width = w;
	height = h;
	data = (float*)malloc(sizeof(float)*w*h * 4);
	/* Create a device and context */
	device = create_device();
	context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
	if (err < 0) {
		perror("Couldn't create a context");
		exit(1);
	}
	/* Determine local size */
	err = clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE,
		sizeof(local_size), &local_size, NULL);
	if (err < 0) {
		perror("Couldn't obtain device information");
		exit(1);
	}
	printf("Local Size:%d\n", local_size);
	/* Build the program and create a kernel */
	program = build_program(context, device, PROGRAM_FILE);
	/* Find out how many kernels there are in the source file*/
	err = clCreateKernelsInProgram(program, 0, NULL, &num_kernels);
	if (err < 0) {
		perror("Couldn't find any kernels");
		exit(1);
	}

	/* CREATE a kernel for each function */
	kernel = (cl_kernel*)malloc(num_kernels * sizeof(cl_kernel));
	clCreateKernelsInProgram(program, num_kernels, kernel, NULL);
	/*START Get Luminance Data*/
	/* Create image object */
	img_format.image_channel_order = CL_RGBA;
	img_format.image_channel_data_type = CL_UNORM_INT8;
	//pass the kernel a 2d image 
	input_image = clCreateImage2D(context,
		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		&img_format, width, height, 0, (void*)inputImage, &err);
	//The kernel returns float data in buffer
	data_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE |
		CL_MEM_USE_HOST_PTR, width*height * 4 * sizeof(float), data, &err);
	if (err < 0) {
		perror("Couldn't create the image object");
		exit(1);
	};

	/* Create kernel arguments */
	err = clSetKernelArg(kernel[0], 0, sizeof(cl_mem), &input_image);
	err |= clSetKernelArg(kernel[0], 1, sizeof(cl_mem), &data_buffer);
	if (err < 0) {
		printf("Couldn't set a kernel argument");
		exit(1);
	};

	/* Create a command queue */
	queue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &err);
	if (err < 0) {
		perror("Couldn't create a command queue");
		exit(1);
	};

	/* Enqueue kernel */
	global_size[0] = height; global_size[1] = width;
	err = clEnqueueNDRangeKernel(queue, kernel[0], 2, NULL, global_size,
		NULL, 0, NULL, NULL);
	if (err < 0) {
		perror("Couldn't enqueue the kernel");
		exit(1);
	}

	/* Read the final sum */
	err = clEnqueueReadBuffer(queue, data_buffer, CL_TRUE, 0,
		height*width * 4 * sizeof(float), data, 0, NULL, NULL);
	if (err < 0) {
		perror("Couldn't read the buffer");
		exit(1);
	}
	/*END Get Luminance Data*/

	/*START Parallel Luminance Encode*/
	//free data_buffer so we can re-use it
	clReleaseMemObject(data_buffer);
	/*Initialise Buffers*/
	data_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE |
		CL_MEM_USE_HOST_PTR, (height*width * 4) * sizeof(float), data, &err);
	sum_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
		sizeof(float), NULL, &err);
	if (err < 0) {
		perror("Couldn't create a buffer");
		exit(1);
	};
	/* Set arguments for vector kernel */
	err = clSetKernelArg(kernel[1], 0, sizeof(cl_mem), &data_buffer);
	err |= clSetKernelArg(kernel[1], 1, local_size * 4 * sizeof(float), NULL);

	/* Set arguments for complete kernel */
	err = clSetKernelArg(kernel[2], 0, sizeof(cl_mem), &data_buffer);
	err |= clSetKernelArg(kernel[2], 1, local_size * 4 * sizeof(float), NULL);
	err |= clSetKernelArg(kernel[2], 2, sizeof(cl_mem), &sum_buffer);
	if (err < 0) {
		perror("Couldn't create a kernel argument");
		exit(1);
	}

	/* Enqueue kernels */
	gl_size = height*width * 4;
	printf("Global size = %lu\n", gl_size);
	/* Perform successive stages of the reduction */
	while (gl_size / local_size > local_size) {
		gl_size = gl_size / local_size;
		err = clEnqueueNDRangeKernel(queue, kernel[1], 1, NULL, &gl_size,
			&local_size, 0, NULL, NULL);
		printf("Global size = %lu\n", gl_size);
		if (err < 0) {
			perror("Couldn't enqueue the kernel");
			exit(1);
		}
	}
	gl_size = gl_size / local_size;
	err = clEnqueueNDRangeKernel(queue, kernel[2], 1, NULL, &gl_size,
		NULL, 0, NULL, NULL);
	printf("Global size = %lu\n", gl_size);

	/* Read the result */
	err = clEnqueueReadBuffer(queue, sum_buffer, CL_TRUE, 0,
		sizeof(float), &sum, 0, NULL, NULL);
	if (err < 0) {
		perror("Couldn't read the buffer");
		exit(1);
	}
	sum = sum / (w*h * 4);

	/*END Parallel Luminance Encode*/
	//	printf("Total time = %lu\n", total_time);
	printf("Lumosity Average = %f\n", sum);

	printf("Done.");
	getchar();

	/* Deallocate resources */
	free(inputImage);
	clReleaseMemObject(input_image);
	clReleaseMemObject(data_buffer);
	//Release Dynamically allocated Kernels
	for (cl_uint i = 0; i<num_kernels; i++)
		clReleaseKernel(kernel[i]);
	free(kernel);
	//Release OpenCL Strutures
	clReleaseCommandQueue(queue);
	clReleaseProgram(program);
	clReleaseContext(context);
	return 0;
}