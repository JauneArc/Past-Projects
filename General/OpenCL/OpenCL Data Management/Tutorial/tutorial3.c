#define _CRT_SECURE_NO_WARNINGS
#define PROGRAM_FILE "blank_kernel.cl"
#define KERNEL_FUNC "blank"

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
   float float_data1[40], float_data2[40], float_result[40];	// float arrays
   int int_data[40], int_result[40];							// int arrays
   int num = 0;
   int buffer_size[3], buffer_mem[3];							// for buffer info
   cl_mem buffer[3];											// buffer objects
   void* mapped_memory;											// pointer to mapped memory
   const size_t buffer_origin[3] = {5*sizeof(float), 2, 0};		// for reading rectangle data
   const size_t host_origin[3] = {1*sizeof(float), 1, 0};
   const size_t region[3] = {4*sizeof(float), 2, 1};

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
   for(i=0; i<40; i++) {
      float_data1[i] = i+1.0f;			// Set the values in the array from 1 to 40
	  float_data2[i] = -1.0f*(i+1);		// Set the values in the array from -1 to -40
	  int_data[i] = i;					// Set the values in the array from 0 to 39
   }
   memset(float_result, 0.0f, sizeof(float)*40);	// Set all values in the array to 0
   memset(int_result, 0, sizeof(int)*40);			// Set all values in the array to 0

///////////////////////////////////////////////////////////////////////////////////////////////////////
   // SECTION 3
   // Create buffers
   buffer[0] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, 
	   sizeof(float_data1), float_data1, &err);
   if(err < 0)
      handle_error("Couldn't create a buffer object 1");

   buffer[1] = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
	   sizeof(float_data2), float_data2, &err);
   if(err < 0)
      handle_error("Couldn't create a buffer object 2");

   buffer[2] = clCreateBuffer(context, CL_MEM_READ_WRITE, 
	   sizeof(int_data), NULL, &err);
   if(err < 0)
      handle_error("Couldn't create a buffer object 3");

   // Obtain and display size and host pointer information
   clGetMemObjectInfo(buffer[0], CL_MEM_SIZE, sizeof(buffer_size[0]), &buffer_size[0], NULL);
   clGetMemObjectInfo(buffer[1], CL_MEM_SIZE, sizeof(buffer_size[1]), &buffer_size[1], NULL);
   clGetMemObjectInfo(buffer[2], CL_MEM_SIZE, sizeof(buffer_size[2]), &buffer_size[2], NULL);
   clGetMemObjectInfo(buffer[0], CL_MEM_HOST_PTR, sizeof(buffer_mem[0]), &buffer_mem[0], NULL);
   clGetMemObjectInfo(buffer[1], CL_MEM_HOST_PTR, sizeof(buffer_mem[1]), &buffer_mem[1], NULL);
   clGetMemObjectInfo(buffer[2], CL_MEM_HOST_PTR, sizeof(buffer_mem[2]), &buffer_mem[2], NULL);
   printf("buffer 1 size: %lu\n", buffer_size[0]);
   printf("buffer 2 size: %lu\n", buffer_size[1]);
   printf("buffer 3 size: %lu\n", buffer_size[2]);
   printf("float_data1 memory address: %p\n", float_data1);
   printf("float_data2 memory address: %p\n", float_data2);
   printf("int_data memory address: %p\n", int_data);
   printf("buffer 1 memory address: %p\n", buffer_mem[0]);
   printf("buffer 2 memory address: %p\n", buffer_mem[1]);
   printf("buffer 3 memory address: %p\n", buffer_mem[2]);

///////////////////////////////////////////////////////////////////////////////////////////////////////
   // SECTION 4
   // Set buffers as arguments to the kernel
   err = clSetKernelArg(kernel, 0, sizeof(int), &num);
   err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &buffer[0]);
   err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &buffer[1]);
   err |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &buffer[2]);
   if(err < 0)
      handle_error("Couldn't set kernel argument");

   // Enqueue kernel
   err = clEnqueueTask(queue, kernel, 0, NULL, NULL);
   if(err < 0)
      handle_error("Couldn't enqueue the kernel");

///////////////////////////////////////////////////////////////////////////////////////////////////////
   // SECTION 5
/*
   // Enqueue command to write to buffer 3
   err = clEnqueueWriteBuffer(queue, buffer[2], CL_TRUE, 0,
         sizeof(int_data), int_data, 0, NULL, NULL); 
   if(err < 0)
      handle_error("Couldn't write to buffer object 3");

   // Enqueue command to read from buffer 3
   err = clEnqueueReadBuffer(queue, buffer[2], CL_TRUE, 
		 sizeof(int)*10, sizeof(int)*20, &int_result[10], 0, NULL, NULL); 
   if(err < 0)
      handle_error("Couldn't read from buffer object 3");

   // Display int_result contents
   printf("\nContents of int_result after the read command:\n");
   for(i=0; i<4; i++) {
     for(j=0; j<10; j++) {
         printf("%d ", int_result[j+i*10]);
      }
      printf("\n");
   }
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////
   // SECTION 6
/*
   // Enqueue command to map buffer 2 to host memory
   mapped_memory = clEnqueueMapBuffer(queue, buffer[1], CL_TRUE,
         CL_MAP_READ, 0, sizeof(float_result), 0, NULL, NULL, &err);
   if(err < 0)
      handle_error("Couldn't map the buffer 2 to host memory");

   // Copy data from mapped memory
   memcpy(float_result, mapped_memory, sizeof(float_result));

   // Unmap the buffer
   err = clEnqueueUnmapMemObject(queue, buffer[1], mapped_memory, 0, NULL, NULL);
   if(err < 0)
      handle_error("Couldn't unmap the buffer");

   // Display contents
   printf("\nContents of float_result after copying memory mapped data:\n");
   for(i=0; i<4; i++) {
      for(j=0; j<10; j++) {
         printf("%6.1f", float_result[j+i*10]);
      }
      printf("\n");
   }
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////
   // SECTION 7
/*
   // Enqueue command to copy contents of buffer 1 to buffer 2
   err = clEnqueueCopyBuffer(queue, buffer[0], buffer[1], 0, 0,
         sizeof(float)*40, 0, NULL, NULL); 
   if(err < 0)
      handle_error("Couldn't copy content from buffer 1 to buffer 2");

   // Enqueue command to read rectangle of data from buffer 2
   err = clEnqueueReadBufferRect(queue, buffer[1], CL_TRUE, 
         buffer_origin, host_origin, region, 10*sizeof(float), 0, 
         10*sizeof(float), 0, float_result, 0, NULL, NULL);
   if(err < 0)
      handle_error("Couldn't read from buffer object 3");

   // Display contents
   printf("\nContents of float_result after copying data from buffer 1 to buffer 2\nand reading rectangle data from buffer 2:\n");
   for(i=0; i<4; i++) {
      for(j=0; j<10; j++) {
         printf("%6.1f", float_result[j+i*10]);
      }
      printf("\n");
   }
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////

   /* Wait until a key is pressed before exiting */
   getchar();	

   /* Deallocate resources */
   clReleaseMemObject(buffer[0]);
   clReleaseMemObject(buffer[1]);
   clReleaseMemObject(buffer[2]);
   clReleaseKernel(kernel);
   clReleaseCommandQueue(queue);
   clReleaseProgram(program);
   clReleaseContext(context);

   return 0;
}
