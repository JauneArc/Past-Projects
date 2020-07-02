/*
Name: Liam Conway
Student Number: 4066716
Last Modified: 15/8/2017
*/
#define _CRT_SECURE_NO_WARNINGS
#define PROGRAM_FILE "source.cl"
#include <stdio.h>
#include <stdlib.h>
#define NUM_FILES 1
#ifdef MAC
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

int main() {
	//Error checking
	cl_int err;
	
	// Program/kernel data structures
	cl_device_id device;
	cl_program program;
	FILE *program_handle;
	char *program_buffer[NUM_FILES];
	char *program_log;
	const char *file_name[] = { PROGRAM_FILE};
	const char options[] = "-cl-finite-math-only -cl-no-signed-zeros";
	size_t program_size[NUM_FILES];
	size_t log_size;
	cl_int i;
	
	//Kernel Data Structures
	char kernel_name[20];
	cl_kernel *kernels;
	cl_uint num_kernels;


	//Display Platform & Device Structures
	cl_uint x, j, a;
	char* info;
	size_t infoSize;
	cl_context context;

	// pointer for the structure
	cl_platform_id *platforms;
	cl_uint num_platforms;
	cl_device_id *devices;
	cl_uint num_devices;


	//Simplifying getting platform info
	const char* attributeNames[3] = { "Name", "Vendor", "Version"};
	const cl_platform_info attributeTypes[3] = { CL_PLATFORM_NAME, CL_PLATFORM_VENDOR, CL_PLATFORM_VERSION};
	const int attributeCount = sizeof(attributeNames) / sizeof(char*);

	
	//PLATFORM SETUP
	// First call to get the number of platforms available
	err = clGetPlatformIDs(5, NULL, &num_platforms);
	if (err < 0)
	{
		perror("could not find any platforms");
		exit(1);
	}
	// Allocate sufficient memory for the platform objects
	platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id)
		* num_platforms);
	// Second call to retrieve the platform objects
	clGetPlatformIDs(num_platforms, platforms, NULL);
	

	// for each platform print all attributes
	for (x = 0; x < num_platforms; x++) {
		cl_char string[10240] = { 0 };
		printf("\n %d. Platform \n", x + 1);

		for (j = 0; j < attributeCount; j++) {

			// get platform attribute value size
			clGetPlatformInfo(platforms[x], attributeTypes[j], 0, NULL, &infoSize);
			info = (char*)malloc(infoSize);

			// get platform attribute value
			clGetPlatformInfo(platforms[x], attributeTypes[j], infoSize, info, NULL);

			//Print Platform Data
			printf("  %d.%d %-11s: %s\n", x + 1, j + 1, attributeNames[j], info);
			free(info);


		}
		//DEVICE SETUP
		// First call to get the number of devices available
		err = clGetDeviceIDs(platforms[x], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
		if (err < 0)
		{
			perror("could not find any devices");
			exit(1);
		}
		// Allocate sufficient memory for the device objects
		devices = (cl_device_id*)malloc(sizeof(cl_device_id)* num_devices);
		// Second call to retrieve the device objects
		clGetDeviceIDs(platforms[x], CL_DEVICE_TYPE_ALL, num_devices, devices, NULL);
		printf("Number of devices: %d\n", num_devices);

		//BEGIN PRINT PLATFORM & DEVICES INFO

		//Display the number of Devices & Device Info
		for (a = 0; a < num_devices; a++) {
			cl_device_type temp;
			//Print Device Info
			//Device Number
			printf("\n %d. Device\n", a + 1);
			//Device Name
			clGetDeviceInfo(devices[a], CL_DEVICE_NAME, sizeof(string), &string, NULL);
			printf("Name: %s\n", string);
			printf("\n");
			//Print the Device's Type
			printf("Type:");
			clGetDeviceInfo(devices[a], CL_DEVICE_TYPE, sizeof(temp), &temp, NULL);
			if (temp == CL_DEVICE_TYPE_CPU) {
				printf("CL_DEVICE_TYPE_CPU");
			}
			else if (temp == CL_DEVICE_TYPE_GPU) {
				printf("CL_DEVICE_TYPE_GPU");
			}
			printf("\n");
			//Print Max Compute Units
			cl_uint maxcompunits;
			printf("Max Compute Units:");
			clGetDeviceInfo(devices[a], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &maxcompunits, NULL);
			printf("Max Compute Units: %d\n", maxcompunits);

			//Print Max Work Group Size
			size_t size;
			clGetDeviceInfo(devices[a], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size), &size, NULL);
			printf("Max Work-group Size: %ld\n", size);

			// Max WORK ITEM DIMENSIONS
			cl_uint max_wrk_itm_dims;
			err = clGetDeviceInfo(devices[a], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(cl_uint), &max_wrk_itm_dims, NULL);
			if (err < 0) { 
				printf("Error getting device max work-item dims \n");
			}
			printf("Max Work Item Dimensions: %ld\n", max_wrk_itm_dims);

			// MAX WORK ITEM SIZES
			size_t* Itm_size;
			err = clGetDeviceInfo(devices[a], CL_DEVICE_MAX_WORK_ITEM_SIZES, 0, NULL, &infoSize);
			if (err < 0) {
				printf("Failed getting Max Work Item Sizes\n");
			}
			
			Itm_size = (size_t*)malloc(infoSize);
			clGetDeviceInfo(devices[a], CL_DEVICE_MAX_WORK_ITEM_SIZES, infoSize, Itm_size, NULL);
			for (size_t k = 0; k < max_wrk_itm_dims; k++) {
				printf("Max Work Item Sizes: %ld\n", Itm_size[k]);
			}
			printf("\n");
			free(Itm_size);
		}

	}
	printf("\n");
	//END PRINT PLATFORMS & DEVICES INFO
	//BEGIN CREATE A CONTEXT

	/* Access the first GPU/CPU */
	err = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, 1, &device, NULL);
	if (err == CL_DEVICE_NOT_FOUND) {
		err = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_CPU, 1, &device, NULL);
	}
	if (err < 0) {
		perror("Couldn't find any devices");
		exit(1);
	}

	/* Create a context */
	context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
	if (err < 0) {
		perror("Couldn't create a context");
		exit(1);
	}

	/* Read each program file and place content into buffer array */
	for (i = 0; i<NUM_FILES; i++) {

		program_handle = fopen(file_name[i], "r");
		if (program_handle == NULL) {
			perror("Couldn't find the program file");
			exit(1);
		}
		fseek(program_handle, 0, SEEK_END);
		program_size[i] = ftell(program_handle);
		rewind(program_handle);
		program_buffer[i] = (char*)malloc(program_size[i] + 1);
		program_buffer[i][program_size[i]] = '\0';
		fread(program_buffer[i], sizeof(char), program_size[i],
			program_handle);
		fclose(program_handle);
	}

	/* Create a program containing all program content */
	program = clCreateProgramWithSource(context, NUM_FILES,
		(const char**)program_buffer, program_size, &err);
	if (err < 0) {
		perror("Couldn't create the program");
		exit(1);
	}

	/* Build program */
	err = clBuildProgram(program, 1, &device, options, NULL, NULL);
	if (err < 0) {

		/* Find size of log and print to std output */
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
			0, NULL, &log_size);
		program_log = (char*)malloc(log_size + 1);
		program_log[log_size] = '\0';
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
			log_size + 1, program_log, NULL);
		printf("%s\n", program_log);

		/* Wait until a key is pressed before exiting */
		getchar();

		free(program_log);
		exit(1);
	}

	/* Output message */
	printf("Successfully built program \n\n");
	if (err >= 0) {
		printf("Begin Build Log \n\n");
		/* Find size of log and print to std output */
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
			0, NULL, &log_size);
		program_log = (char*)malloc(log_size + 1);
		program_log[log_size] = '\0';
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
			log_size + 1, program_log, NULL);
		printf("%s\n", program_log);
		free(program_log);
		printf("End Build Log \n\n");
	}

	//**FORGOT TO ADD TO ASS BEFORE SUBMITION**/
	/* Find out how many kernels are in the source file */
	err = clCreateKernelsInProgram(program, 0, NULL, &num_kernels);
	if (err < 0) {
		perror("Couldn't find any kernels");
		exit(1);
	}

	/* Create a kernel for each function */
	kernels = (cl_kernel*)malloc(num_kernels * sizeof(cl_kernel));
	clCreateKernelsInProgram(program, num_kernels, kernels, NULL);

	// Print Total Number of Kernels
	printf("Total Number of Kernels: %ld\n", num_kernels);
	//Print Kernel Names
	for (cl_uint count = 0; count < num_kernels; count++) {
		clGetKernelInfo(kernels[count], CL_KERNEL_FUNCTION_NAME, sizeof(kernel_name), kernel_name, NULL);
		printf("Kernel Names: %s\n", kernel_name);
	}

	/**END FORGOT TO ADD TO ASS SUBMITION**/

	/* Deallocate resources */
	for (i = 0; i<NUM_FILES; i++) {
		free(program_buffer[i]);
	}
	clReleaseProgram(program);
	clReleaseContext(context);

	

	//Wait for character before ending
	printf("Press any key to End \n");
	getchar();

	//deallocate resources
	free(platforms);
	free(devices);

	return 0;

}