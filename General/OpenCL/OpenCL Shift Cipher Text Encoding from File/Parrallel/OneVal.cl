__kernel void OneValEncrypt(__global char* file_input, int userVal) {
	int global_id = get_global_id(0);
	int uVal = userVal;
	int ascii = 0;
	
	ascii = (int) file_input[global_id];
	if(ascii >=65 && ascii <=90){
		ascii += uVal;
		if(ascii > 90){
			ascii -=25;
		}
		file_input[global_id] = ascii;
	}

}

__kernel void OneValDecrytpion(__global char* file_input, int userVal) {
	int global_id = get_global_id(0);
	int uVal = userVal;
	int ascii = 0;
	
	ascii = (int) file_input[global_id];
	if(ascii >=65 && ascii <=90){
			ascii -= uVal;
			if(ascii < 65){
				ascii +=25;
			}
		file_input[global_id] = ascii;
	}

}

__kernel void MultiValEncrypt(__global char* file_input,__global int* userVals, int valid_entries) {
	int global_id = get_global_id(0);
	int position = global_id % valid_entries;
	int ascii = 0;
	
	ascii = (int) file_input[global_id];

	if(ascii >=65 && ascii <=90){
			ascii += userVals[position];
			if(ascii > 90){
				ascii -=25;
			}
		file_input[global_id] = ascii;
	}

}
__kernel void MultiValDecrypt(__global char* file_input,__global int* userVals, int valid_entries) {
	int global_id = get_global_id(0);
	int position = global_id % valid_entries;
	int ascii = 0;
	
	ascii = (int) file_input[global_id];

	if(ascii >=65 && ascii <=90){
			ascii -= userVals[position];
			if(ascii < 65){
				ascii +=25;
			}
		file_input[global_id] = ascii;
	}

}
__kernel void LookupEncrypt(__global char* file_input,__global int* lookupTable) {
	int global_id = get_global_id(0);
	int ascii = (int)file_input[global_id]; 
	int offset = lookupTable[ascii-65];

	if(ascii !=32){
	file_input[global_id] +=offset;
	}

}
__kernel void LookupDecrypt(__global char* file_input,__global int* inverseLookupTable) {
	int global_id = get_global_id(0);
	int ascii = (int)file_input[global_id]; 
	int offset = inverseLookupTable[ascii-65];
	if(ascii !=32){
	file_input[global_id] +=offset;
	}


}