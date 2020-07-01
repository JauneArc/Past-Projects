#define _CRT_SECURE_NO_WARNINGS
#define PROGRAM_FILE "vecadd.cl"
#define KERNEL_FUNC "vecadd"
#define ARRAY_LENGTH 1000

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#ifdef MAC
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

/* Prints error message and exits */
void handle_error(const char* msg)
{
   perror(msg);

   /* Wait until a key is pressed before exiting */
   getchar();	
	
   exit(1);   
}

/* Find a GPU or CPU associated with the first available platform */
cl_device_id create_device() {

   cl_platform_id platform;
   cl_device_id dev;
   int err;

   /* Identify a platform */
   err = clGetPlatformIDs(1, &platform, NULL);
   if(err < 0)
      handle_error("Couldn't identify a platform");

   /* Access a device */
   err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
   if(err == CL_DEVICE_NOT_FOUND) {
      err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
   }
   if(err < 0)
      handle_error("Couldn't access any devices");

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
   if(program_handle == NULL)
      handle_error("Couldn't find the program file");

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
   if(err < 0)
      handle_error("Couldn't create the program");

   free(program_buffer);

   /* Build program */
   err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
   if(err < 0) {

      /* Find size of log and print to std output */
      clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 
            0, NULL, &log_size);
      program_log = (char*) malloc(log_size + 1);
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

   /* OpenCL data structures */
   cl_device_id device;
   cl_context context;
   cl_command_queue queue;
   cl_program program;
   cl_kernel kernel;
   cl_int i, j, err;

///////////////////////////////////////////////////////////////////////////////////////////////////////
   // SECTION 1
   // Declare data and buffers
   float array_a[ARRAY_LENGTH];	// float arrays
   float array_b[ARRAY_LENGTH];
   float array_result[ARRAY_LENGTH];			
   cl_mem buffer_a;				// buffer objects
   cl_mem buffer_b;				
   cl_mem buffer_c;				// c = a + b
   size_t num_of_work_items = ARRAY_LENGTH;
   cl_bool result_check = CL_TRUE;
//   int array_size = sizeof(float)*ARRAY_LENGTH;

///////////////////////////////////////////////////////////////////////////////////////////////////////

   /* Create a device and context */
   device = create_device();
   context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
   if(err < 0)
      handle_error("Couldn't create a context");

   /* Build the program and create the kernel */
   program = build_program(context, device, PROGRAM_FILE);
   kernel = clCreateKernel(program, KERNEL_FUNC, &err);
   if(err < 0)
      handle_error("Couldn't create a kernel");

   /* Create a command queue */
   queue = clCreateCommandQueue(context, device, 0, &err);
   if(err < 0)
      handle_error("Couldn't create a command queue");

///////////////////////////////////////////////////////////////////////////////////////////////////////
   // SECTION 2
   // Initialise arrays
   for(i=0; i<ARRAY_LENGTH; i++) {
      array_a[i] = i+2.0f;					// Set the values in the array from 2 to 1001
	  array_b[i] = -1.0f*(i+1);				// Set the values in the array from -1 to -1000
   }
   memset(array_result, 0.0f, sizeof(float)*ARRAY_LENGTH);	// Set all values in the array to 0

///////////////////////////////////////////////////////////////////////////////////////////////////////
   // SECTION 3
   // Create buffers
   // Buffers for the input, read-only
   buffer_a = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
	   sizeof(float)*ARRAY_LENGTH, array_a, &err);
   if(err < 0)
      handle_error("Couldn't create buffer a");

   buffer_b = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
	   sizeof(float)*ARRAY_LENGTH, array_b, &err);
   if(err < 0)
      handle_error("Couldn't create buffer b");

   // Buffer for the result
   buffer_c = clCreateBuffer(context, CL_MEM_WRITE_ONLY, 
	   sizeof(float)*ARRAY_LENGTH, NULL, &err);
   if(err < 0)
      handle_error("Couldn't create result1 buffer");

///////////////////////////////////////////////////////////////////////////////////////////////////////
   // SECTION 4
   // Set buffers as arguments to the kernel
   // Set kernel arguments for c = a + b
   err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer_a);
   err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &buffer_b);
   err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &buffer_c);
   if(err < 0)
      handle_error("Couldn't set kernel argument");

   // Enqueue kernel
//   err = clEnqueueTask(queue, kernel, 0, NULL, NULL);
   err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &num_of_work_items, NULL, 0, NULL, NULL);
   if(err < 0)
      handle_error("Couldn't enqueue the kernel");

///////////////////////////////////////////////////////////////////////////////////////////////////////
   // SECTION 5
   // Enqueue command to read from buffer c
   err = clEnqueueReadBuffer(queue, buffer_c, CL_TRUE, 
		 0, sizeof(float)*ARRAY_LENGTH, array_result, 0, NULL, NULL); 
   if(err < 0)
      handle_error("Couldn't read from buffer result");

   // Check the results
   result_check = CL_TRUE;

   for(i=0; i<ARRAY_LENGTH; i++)
   {
	   if(array_result[i] != 1)
	   {
		   printf("Error at array_result[%d]: %.1f\n", i, array_result[i]);
		   result_check = CL_FALSE;
	   }
   }
   if(result_check)
	   printf("Successfully processed %d array elements.\n", ARRAY_LENGTH);

///////////////////////////////////////////////////////////////////////////////////////////////////////

   /* Wait until a key is pressed before exiting */
   getchar();	

   /* Deallocate resources */
   clReleaseMemObject(buffer_a);
   clReleaseMemObject(buffer_b);
   clReleaseMemObject(buffer_c);
   clReleaseKernel(kernel);
   clReleaseCommandQueue(queue);
   clReleaseProgram(program);
   clReleaseContext(context);

   return 0;
}
