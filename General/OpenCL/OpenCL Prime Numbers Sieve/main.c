/*
Name: Liam Conway
SID: 4066716
Last Modified: 9/2017
Contents: Assignment 2 CSCI376 (Multicore and GPU programming) 
				Part 2:
						Task A: Program accepts a pair of numbers to form a range and calculates all the primes
								using a Sieve of Eratosthenes to offload the work to the GPU.

						Task B: Program accepts a number and calculates all the Prime factors. It does this using 
								a Sieve of Eratosthenes to generate all the primes and a then uses trial comparison
								to check if all the primes up to half the input values are it's prime factors.
								
Exit Codes:
1: An error pertaining to the OpenCL section of the code
2: An error pertaining to the data input by the user
*/

#define _CRT_SECURE_NO_WARNINGS
#define PROGRAM_FILE "Primes.cl"


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#ifdef MAC
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

/*FUNCTION Declaration*/

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
	/*Variable Declaration OpenCL*/
	/* OpenCL data structures */
	cl_device_id device;
	cl_context context;
	cl_command_queue queue;
	cl_program program;
	cl_kernel *kernels;
	cl_int err;
	size_t offset, global_size;
	/* Data and events */
	cl_mem data_buffer;
	cl_mem primes_buffer;
	cl_mem factors_buffer;
	/*Num Kernels Variables*/
	cl_uint num_kernels;

	/*General Variables*/
	int Version = 0;
	int userVals[] = { 0,0 };
	int *Numbers, *Primes, *Factors, *Prime_Values, *lastNum, *lastPrime;
	int *Front_Numbers, *Front_Primes, *Front_Factors, *FrontPValues;
	size_t Size = 0;
	int inVal = -1, counter = 0, uVal=0, count =0;
	int NumIttoAllPrimes = 0;

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
	printf("Program Versions avaliable to Run:\n Version 1: Primes between 2 values\n Version 2: Prime Factors of a number\n Quit 9: Enter NINE to quit\n");
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
	case 1: //Calculate the primes given a range
		printf("Please enter a SMALL number then a LARGE number. \n");
		for (; inVal == -1;)
		{
			printf("Please enter the %d Value:", counter);
			scanf("%d", &inVal);
			//if a valid number was put in store it in userVals
			if (inVal != -1) { 
				userVals[counter] = inVal; 
				counter++;
				//if we are not finished getting input then invalidate inVal
				if (counter < 2) { inVal = -1; }
			}
			//consumes any characters not read by scanf
			while (getchar() != '\n');
		}

		if (userVals[0] > userVals[1] || userVals[0] < 0) {
			printf("Invalid range.\n");
			exit(2);
		}
		if(userVals[1] > INT_MAX){
			printf("Large number exceeds the capacity of this program.\n This program will quit");
			exit(2);
		}
		/*Assign enough memory for the range of ints up to N where N=userVals[1] - 3. e.g. 0-100 != 0,1,100 = 2-99*/
		Size = userVals[1] - 2;
		Numbers = (int*)malloc(Size * sizeof(int));
		Primes = (int*)malloc(Size * sizeof(int));
		Front_Numbers = Numbers;
		Front_Primes = Primes;
		//populate the array
		for (size_t i = 0; i < Size; i++) {
			//Fill Numbers wil the numbers up to the max limit starting from 2 to the Size e.g. (2,3,4,5....97,98,99)
			*Numbers = i + 2;
			//Initialise the primes arr as being all prime e.g 1
			*Primes = 1;
			//printf("Numbers: %d,  Primes: %d\n", *Numbers, *Primes);
			Numbers++;
			Primes++;
		}
		Numbers = Front_Numbers;
		Primes = Front_Primes;
		/*Sieving section*/
		/*Get the Maximum number of times we need to Iterate through to have only primes left*/
		NumIttoAllPrimes = (int)sqrt(userVals[1]);
		/*Calculate the global size*/
		for (int p = 0; p <NumIttoAllPrimes; p++) {
			if (*Numbers == NumIttoAllPrimes)
			{
				global_size = p;
				break;
			}
			Numbers++;
		}
		Numbers = Front_Numbers;
		/*Calculate all primes to sqrt(N)*/
		for (int f = 0; f < NumIttoAllPrimes; f++) {
			if (*Primes == 1) {
				int x = *Numbers;
				for (int j = x*x; j <= NumIttoAllPrimes; j += x)
				{
					int *temp = Primes;
					Primes += x;
					*Primes = 0;
					Primes = temp;
				}
			}
			//printf("Numbers: %d Primes%d\n", *Numbers,*Primes);
			Primes++;
			Numbers++;
		}
		Numbers = Front_Numbers;
		Primes = Front_Primes;
		//check that the last value is not prime
		int *lastNum = Numbers + Size - 1;
		int *lastPrime = Primes + Size - 1;
		for (int f = 0; f <= NumIttoAllPrimes; f++) {
			if (*Primes == 1)
			{
				int x = *Numbers;
				if (*lastNum % x == 0)
				{
					*lastPrime = 0;
				}
			}
			Primes++;
			Numbers++;
		}
		Numbers = Front_Numbers;
		Primes = Front_Primes;

		/* Create a buffer Numbers and A Buffer to hold Primes
					  clCreateBuffer (context,				cl_mem_flags,					   size,		   host_ptr, errcode_ret);*/
		data_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, Size * sizeof(int), Numbers, &err);
		if (err < 0) {
			perror("Couldn't create a buffer");
			exit(1);
		};
		primes_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, Size * sizeof(int), Primes, &err);
		if (err < 0) {
			perror("Couldn't create a buffer");
			exit(1);
		};

		/* CREATE kernel argument for Numbers,Primes and Size*/
		err = clSetKernelArg(kernels[0], 0, sizeof(cl_mem), &data_buffer);
		if (err < 0) {
			perror("Couldn't set a kernel argument");
			exit(1);
		};
		err = clSetKernelArg(kernels[0], 1, sizeof(cl_mem), &primes_buffer);
		if (err < 0) {
			perror("Couldn't set a kernel argument");
			exit(1);
		};
		err = clSetKernelArg(kernels[0], 2, sizeof(int), &Size);
		if (err < 0) {
			perror("Couldn't set a kernel argument");
			exit(1);
		};

		/* ENQUEUE the kernel 
		The third argument is the global size and it specifies a wish to execute X instances of the work-item specified in the kernel source.
		The fourth argument is the local size and it specifies how many of the work-items should be grouped into a work-group. 
		Local_size note 
		In this case because we are working with a variable global size and we cannot be sure that global_size can be equally divided into
		local size we let the complier choose at runtime
		*/
		offset = 0;

		err = clEnqueueNDRangeKernel(queue, kernels[0], 1, &offset, &global_size, NULL, 0, NULL, NULL);
		if (err < 0) {
			perror("Couldn't enqueue the kernel");
			exit(1);
		}

		/* READ the buffer */
		err = clEnqueueReadBuffer(queue, primes_buffer, CL_TRUE, 0,
			Size * sizeof(int), Primes, 0, NULL, NULL);
		if (err < 0) {
			perror("Couldn't read the buffer");
			exit(1);
		}
		//make sure primes is an the front of the arr
		Primes = Front_Primes;
		Numbers = Front_Numbers;
		//Iterate through the array comparing i to Prime[i] and if it is print Numbers[i]
		printf("List of Primes between %d and %d:\n", userVals[0],userVals[1]);
		counter = 0;
		for (size_t y = userVals[0]; y < Size-1; y++) {
			if (*Primes != 1) {
				//printf("Numbers %d, Primes %d, \n", *Numbers, *Primes);
				Primes++;
				Numbers++;
			}
			else {
				printf("%d, ", *Numbers);
				Primes++;
				Numbers++;
				counter++;
			}
		}
		printf("The total number of brimes between %d and %d is: %d \n", userVals[0], userVals[1], counter);

		//Free malloc memory
		free(Front_Numbers);
		free(Front_Primes);
		//RELEASE the memory assigned to data_buffer and primes_buffer before we reallocate it 
		clReleaseMemObject(data_buffer);
		clReleaseMemObject(primes_buffer);
		break;
	case 2: //Task B Prime Factors < Number
		/*First we sieve for all the primes less than the input value
		Then we use attempt to divide the input by the primes starting from the smallest up to input/2
		*/
		printf("Please enter an Integer. \n");
		for (; inVal == -1;)
		{
			printf("Please enter the %d Value:", counter);
			scanf("%d", &inVal);
			//if a valid number was put in store it in userVals
			if (inVal != -1) {
				userVals[1] = inVal;
				counter++;
			}
			//consumes any characters not read by scanf
			while (getchar() != '\n');
		}
		userVals[0] = 0;
		if (userVals[1] > INT_MAX) {
			printf("Large number exceeds the capacity of this program.\n This program will quit");
			exit(2);
		}
		/*Assign enough memory for the range of ints up to N where N=userVals[1] - 3. e.g. 0-100 != 0,1,100 = 2-99*/
		Size = userVals[1] - 2;
		Numbers = (int*)malloc(Size * sizeof(int));
		Primes = (int*)malloc(Size * sizeof(int));
		Front_Numbers = Numbers;
		Front_Primes = Primes;
		//populate the array
		for (size_t i = 0; i < Size; i++) {
			//Fill Numbers wil the numbers up to the max limit starting from 2 to the Size e.g. (2,3,4,5....97,98,99)
			*Numbers = i + 2;
			//Initialise the primes arr as being all prime e.g 1
			*Primes = 1;
			//printf("Numbers: %d,  Primes: %d\n", *Numbers, *Primes);
			Numbers++;
			Primes++;
		}
		Numbers = Front_Numbers;
		Primes = Front_Primes;

		/*Get the Maximum number of times we need to Iterate through to have only primes left*/
		NumIttoAllPrimes = (int)sqrt(userVals[1]);
		/*Calculate the global size*/
		for (int p = 0; p <= NumIttoAllPrimes; p++) {
			if (*Numbers == NumIttoAllPrimes)
			{
				global_size = p;
				break;
			}
			Numbers++;
		}
		Numbers = Front_Numbers;
		/*Calculate all primes to sqrt(N)*/
		for (int f = 0; f <= NumIttoAllPrimes; f++) {
			if (*Primes == 1) {
				int x = *Numbers;
				for (int j = x+x; j <= NumIttoAllPrimes; j += x)
				{
					int *temp = Primes;
					Primes += x;
					*Primes = 0;
					Primes = temp;
				}
			}
			//printf("Numbers: %d Primes%d\n", *Numbers,*Primes);
			Primes++;
			Numbers++;
		}
		Numbers = Front_Numbers;
		Primes = Front_Primes;
		//check that the last value is not prime
		lastNum = Numbers + Size-1;
		lastPrime = Primes + Size-1;
		for (int f = 0; f <= NumIttoAllPrimes; f++) {
			if (*Primes == 1)
			{
				int x = *Numbers;
				if (*lastNum % x == 0)
				{
					*lastPrime = 0;
				}
			}
			Primes++;
			Numbers++;
		}
		Numbers = Front_Numbers;
		Primes = Front_Primes;

		/* Create a buffer Numbers and A Buffer to hold Primes
		clCreateBuffer (context,				cl_mem_flags,					   size,		   host_ptr, errcode_ret);*/
		data_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, Size * sizeof(int), Numbers, &err);
		if (err < 0) {
			perror("Couldn't create a buffer");
			exit(1);
		};
		primes_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, Size * sizeof(int), Primes, &err);
		if (err < 0) {
			perror("Couldn't create a buffer");
			exit(1);
		};

		/* CREATE kernel argument for Numbers,Primes and Size*/
		err = clSetKernelArg(kernels[0], 0, sizeof(cl_mem), &data_buffer);
		if (err < 0) {
			perror("Couldn't set a kernel argument");
			exit(1);
		};
		err = clSetKernelArg(kernels[0], 1, sizeof(cl_mem), &primes_buffer);
		if (err < 0) {
			perror("Couldn't set a kernel argument");
			exit(1);
		};
		err = clSetKernelArg(kernels[0], 2, sizeof(int), &Size);
		if (err < 0) {
			perror("Couldn't set a kernel argument");
			exit(1);
		};
		/* Kernel Section */
		/* ENQUEUE the kernel
		The third argument is the global size and it specifies a wish to execute X instances of the work-item specified in the kernel source.
		The fourth argument is the local size and it specifies how many of the work-items should be grouped into a work-group.
		Local_size note
		In this case because we are working with a variable global size and we cannot be sure that global_size can be equally divided into
		local size we let the complier choose at runtime
		*/
		offset = 0;

		err = clEnqueueNDRangeKernel(queue, kernels[0], 1, &offset, &global_size, NULL, 0, NULL, NULL);
		if (err < 0) {
			perror("Couldn't enqueue the kernel");
			exit(1);
		}

		/* READ the buffer */
		err = clEnqueueReadBuffer(queue, primes_buffer, CL_TRUE, 0,
			Size * sizeof(int), Primes, 0, NULL, NULL);
		if (err < 0) {
			perror("Couldn't read the buffer");
			exit(1);
		}
		//make sure primes is an the front of the arr
		Primes = Front_Primes;
		Numbers = Front_Numbers;
		counter = 0;
		//Iterate through the array comparing i to Prime[i] and if it is incriment counter
		for (size_t y = userVals[0]; y < Size; y++) {
			if (*Primes != 1) {
				Primes++;
				Numbers++;
			}
			else {
				//printf("%d, ", *Numbers);
				counter++;
				Primes++;
				Numbers++;
			}
		}
		//RELEASE the memory assigned to data_buffer and primes_buffer before we reallocate it 
		clReleaseMemObject(data_buffer);
		clReleaseMemObject(primes_buffer);

		/*Beginning the Factors section*/
		//create an pair of arrays to hold only the primes and an array to act as out factor checker
		Prime_Values = (int*)malloc(counter * sizeof(int));
		Factors = (int*)malloc(counter*sizeof(int));
		/*Set two pointer to always be pointer to the front to the array*/
		FrontPValues = Prime_Values;
		Front_Factors = Factors;
		/*Set Primes and Numbers back to the front of the arrays*/
		Primes = Front_Primes;
		Numbers = Front_Numbers;

		//copy primes into the array
		for (size_t y = 0; y < Size; y++) {
			if (*Primes != 1) {
				Primes++;
				Numbers++;
			}
			else {
				//printf("%d, ", *Numbers);
				*Prime_Values = *Numbers;
				Primes++;
				Numbers++;
				Prime_Values++;
			}
		}
		//set Factors to all be false
		for (int y = userVals[0]; y < counter; y++) {
			*Factors = 0;
			Factors++;
		}
		Factors = Front_Factors;
		Prime_Values = FrontPValues;
		
		//Set the buffers
		data_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, counter * sizeof(int), Prime_Values, &err);
		if (err < 0) {
			perror("Couldn't create a buffer");
			exit(1);
		};
		factors_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, counter * sizeof(int), Factors, &err);
		if (err < 0) {
			perror("Couldn't create a buffer");
			exit(1);
		};
		uVal = userVals[1];
		/* CREATE kernel argument for Numbers,Primes and Size*/
		err = clSetKernelArg(kernels[1], 0, sizeof(cl_mem), &data_buffer);
		if (err < 0) {
			perror("Couldn't set a kernel argument");
			exit(1);
		};
		err = clSetKernelArg(kernels[1], 1, sizeof(cl_mem), &factors_buffer);
		if (err < 0) {
			perror("Couldn't set a kernel argument");
			exit(1);
		};
		err = clSetKernelArg(kernels[1], 2, sizeof(int), &uVal);
		if (err < 0) {
			perror("Couldn't set a kernel argument");
			exit(1);
		};
		/* Kernel Section */
		/* ENQUEUE the kernel
		The third argument is the global size and it specifies a wish to execute X instances of the work-item specified in the kernel source.
		The fourth argument is the local size and it specifies how many of the work-items should be grouped into a work-group.
		Local_size note
		In this case because we are working with a variable global size and we cannot be sure that global_size can be equally divided into
		local size we let the complier choose at runtime
		*/
		offset = 0;
		global_size = counter;
		err = clEnqueueNDRangeKernel(queue, kernels[1], 1, &offset, &global_size, NULL, 0, NULL, NULL);
		if (err < 0) {
			perror("Couldn't enqueue the kernel");
			exit(1);
		}

		/* READ the buffer */
		err = clEnqueueReadBuffer(queue, factors_buffer, CL_TRUE, 0,
			counter * sizeof(int), Factors, 0, NULL, NULL);
		if (err < 0) {
			perror("Couldn't read the buffer");
			exit(1);
		}
		//Ensure that Prime_Values and Factors are pointed at the start
		Factors = Front_Factors;
		Prime_Values = FrontPValues;
		/*Print all the factors to the screen*/
		printf("The prime factors of %d are: ", userVals[1]);
		for (int y = userVals[0]; y < counter; y++) {
			if (*Factors != 1)
			{
				Factors++;
				Prime_Values++;
			}
			else
			{
				printf("%d, ", *Prime_Values);
				Factors++;
				Prime_Values++;
				count++;
			}
		}
		printf("\n The total number of prime factors are: %d\n", count);

		/*Memory Deallocate*/
		//Free malloc memory
		free(Front_Numbers);
		free(Front_Primes);
		free(FrontPValues);
		free(Front_Factors);
		//RELEASE the memory assigned to data_buffer and primes_buffer before we reallocate it 
		clReleaseMemObject(data_buffer);
		clReleaseMemObject(factors_buffer);
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
	for (size_t i = 0; i<num_kernels; i++)
		clReleaseKernel(kernels[i]);
	free(kernels);

	//Release OpenCl Structures
	clReleaseProgram(program);
	clReleaseContext(context);
	clReleaseCommandQueue(queue);
	return 0;
}