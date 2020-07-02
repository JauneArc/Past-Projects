

#define INPUT_FILE "lenna.bmp"
#define OUTPUT_FILE "output.bmp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmpfuncs.h"


int main(int argc, char **argv) {

	/* Image data */
	unsigned char* inputImage;
	int w, h;
	int maxSize;
	float LumosityAvg = 0.0;
	char*start;
	char*end;

	/* Open input file and read image data */
	inputImage = readRGBImage(INPUT_FILE, &w, &h);
	start = inputImage;
	end = inputImage + (w*h * 4);
	maxSize = w*h *4;
	/*Sum all image lumosity values*/
	printf("Val1 : = %f .\n", inputImage[0]/255.0f * 0.299f);
	for (int i = 0; i < maxSize; i++) {
		//R channel
		LumosityAvg += inputImage[i]/255.0f * 0.299f;
		i++;
		//G channel

		LumosityAvg += inputImage[i] / 255.0f  * 0.587f;
		i++;
		//B channel
		LumosityAvg += inputImage[i] / 255.0f  * 0.114f;
		//move iterator to the alpha channel
		i++;
		LumosityAvg += 0;
	}
	//LumosityAvg = LumosityAvg / 4;
	printf("Lumosity Total: =%f .\n", LumosityAvg);
	printf("Lumosity Average: =%f .\n", (LumosityAvg/maxSize));

	printf("Done.");
	getchar();

	/* Deallocate resources */
	free(inputImage);
	return 0;
}