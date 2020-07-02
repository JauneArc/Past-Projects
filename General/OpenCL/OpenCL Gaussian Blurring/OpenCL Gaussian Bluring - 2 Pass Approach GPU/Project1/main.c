#define _CRT_SECURE_NO_WARNINGS
#define PROGRAM_FILE "simple_conv.cl"
#define KERNEL_FUNC "simple_conv"

#define INPUT_FILE "lenna.bmp"
#define OUTPUT_FILE "output.bmp"
#define OUTPUT_FILE2 "output2.bmp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmpfuncs.h"

#ifdef MAC
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif


/*Handles the Fetching and verifying of user input*/
int userArguments(int Case) {
	//implement a switch statement to handle various user interactions in one statement
	switch (Case)
	{
	case 0:
		printf("Please enter a vaild mask for blurring. /n Valid mask values are: 3, 5 or 7.");
		for (int uVal = 0; uVal == 0;) {
			uVal = (int)getchar();
			if (uVal == 51 || uVal == 53 || uVal == 55) {
				return uVal;
			}
			uVal = 0;
			printf("Please enter a valid mask Value(3, 5 or 7)");
		}
		break;
	case 1:
		printf("Please choose which device to run the program on. /n1.)GPU /n 2.)CPU");
		for (int uVal = 0; uVal == 0;) {
			uVal = getchar();
			if (uVal == 49 || uVal == 50) {
				return uVal;
			}
			uVal = 0;
			printf("Please choose a valid device");
		}
		break;
	default:
		break;
	}
	return 0;
}

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

	/* Access a device */
	//Switch handles selecting the device (CPU or GPU)
	switch (userArguments(1))
	{
	case 49:
		err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
		if (err == CL_DEVICE_NOT_FOUND) {
			err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
		}
		if (err < 0) {
			perror("Couldn't access any devices");
			exit(1);
		}
		return dev;
		break;
	case 50:
		err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
		if (err == CL_DEVICE_NOT_FOUND) {
			err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
		}
		if (err < 0) {
			perror("Couldn't access any devices");
			exit(1);
		}
		return dev;
		break;
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
	program_handle = fopen(filename, "r");
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
	
	/*
	cl_event start_event, end_event;
	cl_ulong time_start, time_end, total_time;
	*/

	/* Image data */
	unsigned char* inputImage;
	unsigned char* outputImage;

	cl_image_format img_format;
	cl_mem input_image, output_image;
	size_t origin[3], region[3];
	size_t width, height;
	int w, h;
	int maskValue = userArguments(0);

	/* Open input file and read image data */
	inputImage = readRGBImage(INPUT_FILE, &w, &h);
	width = w;
	height = h;
	outputImage = (unsigned char*)malloc(sizeof(unsigned char)*w*h * 4);

	/* Create a device and context */
	device = create_device();
	context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
	if (err < 0) {
		perror("Couldn't create a context");
		exit(1);
	}

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
	/* Create image object */
	img_format.image_channel_order = CL_RGBA;
	img_format.image_channel_data_type = CL_UNORM_INT8;

	input_image = clCreateImage2D(context,
		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		&img_format, width, height, 0, (void*)inputImage, &err);
	output_image = clCreateImage2D(context,
		CL_MEM_WRITE_ONLY, &img_format, width, height, 0, NULL, &err);
	if (err < 0) {
		perror("Couldn't create the image object");
		exit(1);
	};

	/* Create kernel arguments */
	err = clSetKernelArg(kernel[0], 0, sizeof(cl_mem), &input_image);
	err |= clSetKernelArg(kernel[0], 1, sizeof(cl_mem), &output_image);
	err |= clSetKernelArg(kernel[0], 2, sizeof(int), &maskValue);
	if (err < 0) {
		printf("Couldn't set a kernel argument");
		exit(1);
	};

	/* Create a command queue */
	queue = clCreateCommandQueue(context, device, 0, &err);
	if (err < 0) {
		perror("Couldn't create a command queue");
		exit(1);
	};

	/* Enqueue kernel */
	global_size[0] = height; global_size[1] = width;
	/*err = clEnqueueNDRangeKernel(queue, kernel[0], 2, NULL, global_size,
	NULL, 0, NULL, &start_event);
	if (err < 0) {
	perror("Couldn't enqueue the kernel");
	exit(1);
	}*/
	//for (int i = 0; i < 1000; i++) {
	err = clEnqueueNDRangeKernel(queue, kernel[0], 2, NULL, global_size,
		NULL, 0, NULL, NULL);
	if (err < 0) {
		perror("Couldn't enqueue the kernel");
		exit(1);
	}
	//}
	/* Finish processing the queue and get profiling information
	err = clEnqueueNDRangeKernel(queue, kernel[0], 2, NULL, global_size,
	NULL, 0, NULL, &end_event);
	clFinish(queue);
	clGetEventProfilingInfo(start_event, CL_PROFILING_COMMAND_START,
	sizeof(time_start), &time_start, NULL);
	clGetEventProfilingInfo(end_event, CL_PROFILING_COMMAND_END,
	sizeof(time_end), &time_end, NULL);
	total_time = time_end - time_start;
	printf("Total time = %lu\n", total_time);
	getchar();*/
	/* Read the image object */
	origin[0] = 0; origin[1] = 0; origin[2] = 0;
	region[0] = width; region[1] = height; region[2] = 1;
	err = clEnqueueReadImage(queue, output_image, CL_TRUE, origin,
		region, 0, 0, (void*)outputImage, 0, NULL, NULL);
	if (err < 0) {
		perror("Couldn't read from the image object");
		exit(1);
	}
	//storeRGBImage(outputImage, OUTPUT_FILE2, h, w, INPUT_FILE);
	//Duplicate the above section for vertical pass
	clReleaseMemObject(input_image);
	clReleaseMemObject(output_image);
	img_format.image_channel_order = CL_RGBA;
	img_format.image_channel_data_type = CL_UNORM_INT8;

	input_image = clCreateImage2D(context,
		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		&img_format, width, height, 0, (void*)outputImage, &err);
	output_image = clCreateImage2D(context,
		CL_MEM_WRITE_ONLY, &img_format, width, height, 0, NULL, &err);
	if (err < 0) {
		perror("Couldn't create the image object");
		exit(1);
	};
	/* Create kernel arguments */
	err = clSetKernelArg(kernel[1], 0, sizeof(cl_mem), &input_image);
	err |= clSetKernelArg(kernel[1], 1, sizeof(cl_mem), &output_image);
	err |= clSetKernelArg(kernel[1], 2, sizeof(int), &maskValue);
	if (err < 0) {
		printf("Couldn't set a kernel argument");
		exit(1);
	};

	/* Create a command queue */
	queue = clCreateCommandQueue(context, device, 0, &err);
	if (err < 0) {
		perror("Couldn't create a command queue");
		exit(1);
	};
	//loop Here

	/* Enqueue kernel */
	global_size[0] = height; global_size[1] = width;
	/*err = clEnqueueNDRangeKernel(queue, kernel[1], 2, NULL, global_size,
		NULL, 0, NULL, &start_event);
	if (err < 0) {
		perror("Couldn't enqueue the kernel");
		exit(1);
	}*/
	//for (int i = 0; i < 1000; i++) {
		err = clEnqueueNDRangeKernel(queue, kernel[1], 2, NULL, global_size,
			NULL, 0, NULL, NULL);
		if (err < 0) {
			perror("Couldn't enqueue the kernel");
			exit(1);
		}
	//}
	/* Finish processing the queue and get profiling information
	err = clEnqueueNDRangeKernel(queue, kernel[1], 2, NULL, global_size,
		NULL, 0, NULL, &end_event);
	clFinish(queue);
	clGetEventProfilingInfo(start_event, CL_PROFILING_COMMAND_START,
		sizeof(time_start), &time_start, NULL);
	clGetEventProfilingInfo(end_event, CL_PROFILING_COMMAND_END,
		sizeof(time_end), &time_end, NULL);
	total_time += time_end - time_start;
	printf("Total time = %lu\n", total_time);
	getchar();*/
	/* Read the image object */
	origin[0] = 0; origin[1] = 0; origin[2] = 0;
	region[0] = width; region[1] = height; region[2] = 1;
	err = clEnqueueReadImage(queue, output_image, CL_TRUE, origin,
		region, 0, 0, (void*)outputImage, 0, NULL, NULL);
	if (err < 0) {
		perror("Couldn't read from the image object");
		exit(1);
	}

	/* Create output BMP file and write data */
	storeRGBImage(outputImage, OUTPUT_FILE, h, w, INPUT_FILE);

	printf("Done.");
	getchar();

	/* Deallocate resources */
	free(inputImage);
	free(outputImage);
	clReleaseMemObject(input_image);
	clReleaseMemObject(output_image);
	//Release Dynamically allocated Kernels
	for (int i = 0; i<num_kernels; i++)
		clReleaseKernel(kernel[i]);
	free(kernel);
	//Release OpenCL Strutures
	clReleaseCommandQueue(queue);
	clReleaseProgram(program);
	clReleaseContext(context);
	return 0;
}