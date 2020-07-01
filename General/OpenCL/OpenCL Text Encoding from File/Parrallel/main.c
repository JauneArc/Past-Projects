#define _CRT_SECURE_NO_WARNINGS
#define TEXT_FILE_1 "plaintext"
#define PROGRAM_FILE "OneVal.cl"
//#define KERNEL_FUNC "OneVal"
#define LOOKUP_SIZE 26

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>

#ifdef MAC
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

/*FUNCTION Declaration*/
void RemoveSpaces(char* source)
{
	char* i = source;
	char* j = source;
	while (*j != 0)
	{
		*i = *j++;
		if (*i != ' ')
			i++;
	}
	*i = '\0';
}
void strip(char* str)
{
	int from, to;
	for (from = 0, to = 0; str[from] != '\0'; ++from) {
		if (isascii(str[from])) {
			if ((!isdigit(str[from])) && (!ispunct(str[from])) )
			{
				str[to] = toupper(str[from]);
				++to;
			}
		}
	}
	str[to] = '\0';
}
void stripNewline(char* str)
{
	int from, to;
	for (from = 0, to = 0; str[from] != '\0'; ++from)
	{
		if (str[from] != '\n')
		{
			str[to] = str[from];
			++to;
		}
	}
	str[to] = '\0';
}
char* spaceFive(char* str, size_t file_size) 
{
	int size = 0;
	for (size = 0; str[size] != 0; ++size) {

	}
	size = size / 5;
	file_size += size;

	char *buff = (char*)malloc(file_size + 1);
	int from, to, counter = 0;

	for (from = 0, to = 0; str[from] != '\0'; ++from)
	{
			//add a space every 5 chars e.g. 01234 56789 ....
			if (counter == 5)
			{
				buff[to] = ' ';
				++to;
				counter = 0;
			}
		buff[to] = str[from];
		++to;
		counter++;
	}
	buff[to] = '\0';
	return buff;
}

int GetUserValues(int *UserValues) {
	int MAX_VALID = 4;
	int valid_Entries = 0;
	int userVal = -1;
	printf("Enter a Number when prompted. Enter zero's to not use that entry.\n");
	for (;valid_Entries < MAX_VALID;) {
		printf("Enter the %d Number: ", valid_Entries + 1);
		scanf("%d", &userVal);
		if (userVal > -1)
		{
			UserValues[valid_Entries] = userVal;
			valid_Entries++;
			userVal = -1;
		}
		//consumes any characters not read by scanf
		while (getchar() != '\n');
	}
	//count the number of non-zero entries
	valid_Entries = 0;
	for (int i = 0; i < MAX_VALID; i++) {
		if (UserValues[i] > 0)
			valid_Entries++;
	}
	return valid_Entries;
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
		exit(1);
	}

	return program;
}

int main() {
	/*VARIABLE Declaration General*/
	FILE *fptr;
	char *file_buffer, *spaced = NULL;
	size_t file_size;

	/*Variable Declaration OpenCL*/
	/* OpenCL data structures */
	cl_device_id device;
	cl_context context;
	cl_command_queue queue;
	cl_program program;
	cl_kernel *kernels;
	cl_int err;
	size_t offset, global_size, local_size;

	/* Data and events */
	cl_mem data_buffer;
	cl_mem int_buffer;
	cl_mem lookup_buffer;
							//	 A, B, C, D, E, F, G, H,  I, J,  K, L, M, N, O, P,  Q,  R,  S, T, U,  V,  W,  X, Y,  Z
	int inverseLookupTable[] = { 9,16,-2,20,17, 1, 7, 8, -7,16,  8, 8,-1,-8,-7, 1,-13,  3,-16,-7,-6,-17,-14, -13,-2,  -1 };
	       int lookupTable[] = { 2, 7,16,13,17, 8,-1, 7, 14,-9, 13, 1, 7,-7, 6,-8, -1,-16, -8,-8,-3,-17,  2,-20, 1,-16 };
	/*User Value Variables*/
	int userVal = 0;
	int Version = 0;
	int UserVals[] = { 0,0,0,0 };
	int valid_Values;
	/*Num Kernels Variables*/
	cl_uint num_kernels;

	/* Create a device and context */
	device = create_device();
	context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
	if (err < 0) {
		perror("Couldn't create a context");
		exit(1);
	}

	/* Create a command queue */
	queue = clCreateCommandQueue(context, device, 0, &err);
	if (err < 0) {
		perror("Couldn't create a command queue");
		exit(1);
	};


	/*OPEN file for reading*/
	fptr = fopen(TEXT_FILE_1, "r");
	if (fptr == NULL)
	{
		printf("Cannot open file \n");
		exit(0);
	}
	//Read file into buffer
	fseek(fptr, 0, SEEK_END);
	file_size = ftell(fptr);
	rewind(fptr);
	file_buffer = (char*)malloc(file_size + 1); //remember to free
	file_buffer[file_size] = '\0';
	fread(file_buffer, sizeof(char), file_size, fptr);
	/*CLOSE file*/
	fclose(fptr);

	/* REMOVE spaces */
	RemoveSpaces(file_buffer);

	/* REMOVE non-letters & toUpper */
	strip(file_buffer);

	/*Strip Newline char's*/
	stripNewline(file_buffer);

	/*Add space every 5 char's*/
	spaced = spaceFive(file_buffer,file_size);



	/* BUILD the program and CREATE a kernel */
	program = build_program(context, device, PROGRAM_FILE);

	/* Find out how many kernels there are in the source file*/
	err = clCreateKernelsInProgram(program, 0, NULL, &num_kernels);
	if (err < 0) {
		perror("Couldn't find any kernels");
		exit(1);
	}

	/* CREATE a kernel for each function */
	kernels = (cl_kernel*)malloc(num_kernels * sizeof(cl_kernel));
	clCreateKernelsInProgram(program, num_kernels, kernels, NULL);
	/* MENU WRAPPER */
	printf("Program Versions avaliable to Run:\n Version 1: Single Integer Encryption\n Version 2: Up to 4 Integer Encryption\n Version 3: Lookup table Encryption\n Quit 9: Enter NINE to quit\n");
	//Prompt user to enter a vaild Version
	for (; Version == 0;)
	{
		printf("Please enter a Version:");
		scanf("%d", &Version);
		//consumes any characters not read by scanf
		while (getchar() != '\n');
		//printf("Number Entered:%d", userVal);
	}
	switch (Version)
	{
	case 1: //Task b
			//Prompt user for a value
		for (; userVal == 0;)
		{
			printf("Please enter a Value:");
			scanf("%d", &userVal);
			//consumes any characters not read by scanf
			while (getchar() != '\n');
		}
		/* Create a buffer to hold data And a buffer to hold the int you are passing
					  clCreateBuffer (context,   cl_mem_flags,			 size,		      host_ptr, errcode_ret);*/
		data_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, file_size * sizeof(char), spaced, &err);
		if (err < 0) {
			perror("Couldn't create a buffer");
			exit(1);
		};

		/* CREATE kernel argument */
		err = clSetKernelArg(kernels[0], 0, sizeof(cl_mem), &data_buffer);
		if (err < 0) {
			perror("Couldn't set a kernel argument");
			exit(1);
		};
		err = clSetKernelArg(kernels[0], 1, sizeof(int), &userVal);
		if (err < 0) {
			perror("Couldn't set a kernel argument");
			exit(1);
		};
		/* ENCRYPTION Section */
		/* ENQUEUE the kernel 
		The third argument is the global size and it specifies a wish to execute X instances of the work-item specified in the kernel source.
		The fourth argument is the local size and it specifies how many of the work-items should be grouped into a work-group. 
		Local_size note 
		In this case because we are working with a variable global size and we cannot be sure that global_size can be equally divided into
		local size we let the complier choose at runtime
		*/
		offset = 0;
		global_size = file_size;

		err = clEnqueueNDRangeKernel(queue, kernels[0], 1, &offset, &global_size, NULL, 0, NULL, NULL);
		if (err < 0) {
			perror("Couldn't enqueue the kernel");
			exit(1);
		}

		/* READ the buffer */
		err = clEnqueueReadBuffer(queue, data_buffer, CL_TRUE, 0,
			file_size * sizeof(char), spaced, 0, NULL, NULL);
		if (err < 0) {
			perror("Couldn't read the buffer");
			exit(1);
		}

		//DISPLAY encrypted File
		printf("Encrypted File:");
		printf("%s\n", spaced);

		//RELEASE the memory assigned to data_buffer before we reallocate it 
		clReleaseMemObject(data_buffer);
		/*DECRYPT File Section*/
		data_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, file_size * sizeof(char), spaced, &err);
		if (err < 0) {
			perror("Couldn't create a buffer");
			exit(1);
		};

		/* CREATE kernel argument */
		err = clSetKernelArg(kernels[1], 0, sizeof(cl_mem), &data_buffer);
		if (err < 0) {
			perror("Couldn't set a kernel argument");
			exit(1);
		};
		err = clSetKernelArg(kernels[1], 1, sizeof(int), &userVal);
		if (err < 0) {
			perror("Couldn't set a kernel argument");
			exit(1);
		};

		/* ENQUEUE the  kernel */
		offset = 0;
		/*N.B THE KERNEL AND THE INPUT can be better optimised for large multi threaded files
		E.g use trial modulo to get the offset; assuming that once divided < MAX_local_work_size
		and have each execution instance do: 
		global_id(0)+1*offset-offset -> global_id(0)+1*offset
		*/ 
		global_size = file_size;

		err = clEnqueueNDRangeKernel(queue, kernels[1], 1, &offset, &global_size, NULL, 0, NULL, NULL);
		if (err < 0) {
			perror("Couldn't enqueue the kernel");
			exit(1);
		}

		/* READ the buffer */
		err = clEnqueueReadBuffer(queue, data_buffer, CL_TRUE, 0,
			file_size * sizeof(char), spaced, 0, NULL, NULL);
		if (err < 0) {
			perror("Couldn't read the buffer");
			exit(1);
		}

		//DISPLAY decrypted File
		printf("Decrypted File:");
		printf("%s\n", spaced);


		//Free OpenCL stuff;
		clReleaseMemObject(data_buffer);
		break;
	case 2: //Task C (Up to 4 User Values)

		valid_Values = GetUserValues(UserVals);

		data_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, file_size * sizeof(char), spaced, &err);
		if (err < 0) {
			perror("Couldn't create a buffer");
			exit(1);
		};
		int_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, 4 * sizeof(int), UserVals, &err);

		/* CREATE kernel argument */
		err = clSetKernelArg(kernels[2], 0, sizeof(cl_mem), &data_buffer);
		if (err < 0) {
			perror("Couldn't set a kernel argument");
			exit(1);
		};
		err = clSetKernelArg(kernels[2], 1, sizeof(cl_mem), &int_buffer);
		if (err < 0) {
			perror("Couldn't set a kernel argument");
			exit(1);
		};
		err = clSetKernelArg(kernels[2], 2, sizeof(int), &valid_Values);
		if (err < 0) {
			perror("Couldn't set a kernel argument");
			exit(1);
		};
		/* ENCRYPTION Section */
		/* ENQUEUE the kernel
		The third argument is the global size and it specifies a wish to execute X instances of the work-item specified in the kernel source.
		The fourth argument is the local size and it specifies how many of the work-items should be grouped into a work-group.
		Local_size note
		In this case because we are working with a variable global size and we cannot be sure that global_size can be equally divided into
		local size we let the complier choose at runtime
		*/
		offset = 0;
		global_size = file_size;

		err = clEnqueueNDRangeKernel(queue, kernels[2], 1, &offset, &global_size, NULL, 0, NULL, NULL);
		if (err < 0) {
			perror("Couldn't enqueue the kernel");
			exit(1);
		}

		/* READ the buffer */
		err = clEnqueueReadBuffer(queue, data_buffer, CL_TRUE, 0,
			file_size * sizeof(char), spaced, 0, NULL, NULL);
		if (err < 0) {
			perror("Couldn't read the buffer");
			exit(1);
		}

		//DISPLAY encrypted File
		printf("Encrypted File:");
		printf("%s\n", spaced);

		//RELEASE the memory assigned to data_buffer before we reallocate it 
		clReleaseMemObject(data_buffer);
		clReleaseMemObject(int_buffer);
		/*DECRYPT File Section*/
		data_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, file_size * sizeof(char), spaced, &err);
		if (err < 0) {
			perror("Couldn't create a buffer");
			exit(1);
		};
		int_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, 4 * sizeof(int), UserVals, &err);
		/* CREATE kernel argument */
		err = clSetKernelArg(kernels[3], 0, sizeof(cl_mem), &data_buffer);
		if (err < 0) {
			perror("Couldn't set a kernel argument");
			exit(1);
		};
		err = clSetKernelArg(kernels[3], 1, sizeof(cl_mem), &int_buffer);
		if (err < 0) {
			perror("Couldn't set a kernel argument");
			exit(1);
		};
		err = clSetKernelArg(kernels[3], 2, sizeof(int), &valid_Values);
		if (err < 0) {
			perror("Couldn't set a kernel argument");
			exit(1);
		};

		/* ENQUEUE the  kernel */
		offset = 0;
		global_size = file_size;

		err = clEnqueueNDRangeKernel(queue, kernels[3], 1, &offset, &global_size, NULL, 0, NULL, NULL);
		if (err < 0) {
			perror("Couldn't enqueue the kernel");
			exit(1);
		}

		/* READ the buffer */
		err = clEnqueueReadBuffer(queue, data_buffer, CL_TRUE, 0,
			file_size * sizeof(char), spaced, 0, NULL, NULL);
		if (err < 0) {
			perror("Couldn't read the buffer");
			exit(1);
		}

		//DISPLAY decrypted File
		printf("Decrypted File:");
		printf("%s\n", spaced);


		//Free OpenCL stuff;
		clReleaseMemObject(data_buffer);
		break;
	case 3: //Task D (Using Lookup Tables to Encrypt)
		printf("Table Lookup Encryption Method\n *****UNDER TESTING*****");
		/* Create a buffer to hold data And a buffer to hold the int you are passing
		clCreateBuffer (context,   cl_mem_flags,			 size,		      host_ptr, errcode_ret);*/
		data_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, file_size * sizeof(char), spaced, &err);
		if (err < 0) {
			perror("Couldn't create a buffer");
			exit(1);
		};
		lookup_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, LOOKUP_SIZE * sizeof(int), lookupTable, &err);
		if (err < 0) {
			perror("Couldn't create a buffer");
			exit(1);
		};

		/* CREATE kernel argument */
		err = clSetKernelArg(kernels[4], 0, sizeof(cl_mem), &data_buffer);
		if (err < 0) {
			perror("Couldn't set a kernel argument");
			exit(1);
		};
		err = clSetKernelArg(kernels[4], 1, sizeof(cl_mem), &lookup_buffer);
		if (err < 0) {
			perror("Couldn't set a kernel argument");
			exit(1);
		};
		/* ENCRYPTION Section */
		/* ENQUEUE the kernel
		The third argument is the global size and it specifies a wish to execute X instances of the work-item specified in the kernel source.
		The fourth argument is the local size and it specifies how many of the work-items should be grouped into a work-group.
		Local_size note
		In this case because we are working with a variable global size and we cannot be sure that global_size can be equally divided into
		local size we let the complier choose at runtime
		*/
		offset = 0;
		global_size = file_size;

		err = clEnqueueNDRangeKernel(queue, kernels[4], 1, &offset, &global_size, NULL, 0, NULL, NULL);
		if (err < 0) {
			perror("Couldn't enqueue the kernel");
			exit(1);
		}

		/* READ the buffer */
		err = clEnqueueReadBuffer(queue, data_buffer, CL_TRUE, 0,
			file_size * sizeof(char), spaced, 0, NULL, NULL);
		if (err < 0) {
			perror("Couldn't read the buffer");
			exit(1);
		}

		//DISPLAY encrypted File
		printf("Encrypted File: \n");
		printf("%s\n", spaced);

		//RELEASE the memory assigned to data_buffer before we reallocate it 
		clReleaseMemObject(data_buffer);
		clReleaseMemObject(lookup_buffer);
		/*DECRYPT File Section*/
		data_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, file_size * sizeof(char), spaced, &err);
		if (err < 0) {
			perror("Couldn't create a buffer");
			exit(1);
		};
		lookup_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, LOOKUP_SIZE * sizeof(int), inverseLookupTable, &err);
		if (err < 0) {
			perror("Couldn't create a buffer");
			exit(1);
		};

		/* CREATE kernel argument */
		err = clSetKernelArg(kernels[5], 0, sizeof(cl_mem), &data_buffer);
		if (err < 0) {
			perror("Couldn't set a kernel argument");
			exit(1);
		};
		err = clSetKernelArg(kernels[5], 1, sizeof(cl_mem), &lookup_buffer);
		if (err < 0) {
			perror("Couldn't set a kernel argument");
			exit(1);
		};

		/* ENQUEUE the  kernel */
		offset = 0;
		/*N.B THE KERNEL AND THE INPUT can be better optimised for large multi threaded files
		E.g use trial modulo to get the offset; assuming that once divided < MAX_local_work_size
		and have each execution instance do:
		global_id(0)+1*offset-offset -> global_id(0)+1*offset
		*/
		global_size = file_size;

		err = clEnqueueNDRangeKernel(queue, kernels[5], 1, &offset, &global_size, NULL, 0, NULL, NULL);
		if (err < 0) {
			perror("Couldn't enqueue the kernel");
			exit(1);
		}

		/* READ the buffer */
		err = clEnqueueReadBuffer(queue, data_buffer, CL_TRUE, 0,
			file_size * sizeof(char), spaced, 0, NULL, NULL);
		if (err < 0) {
			perror("Couldn't read the buffer");
			exit(1);
		}

		//DISPLAY decrypted File
		printf("Decrypted File: \n");
		printf("%s\n", spaced);


		//Free OpenCL stuff;
		clReleaseMemObject(data_buffer);
		clReleaseMemObject(lookup_buffer);
		break;
	case 9:
		printf("Thank You for using this Program.\n");
		break;
	default:
		printf("Invalid version selection (%d) was entered. The program will quit.\n", Version);
		break;
	}

	printf("Press ENTER to quit");
	getchar();

	//Deallocate Memory
	//release the memory for each kernel then free dynamic memory
	for (int i = 0; i<num_kernels; i++)
		clReleaseKernel(kernels[i]);
	free(kernels);

	//Free Dynamically allocated memory
	free(file_buffer);
	free(spaced);
	//Release OpenCl Structures
	clReleaseProgram(program);
	clReleaseContext(context);
	clReleaseCommandQueue(queue);
	return 0;
}