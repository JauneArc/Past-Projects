__kernel void Primes(__global int* Numbers,__global int* Primes, int Size) {
	int global_id = get_global_id(0);	
	int i =2 + global_id;
		for(int j = i+i; j <= Size; j+=i)
		{
			if(Numbers[j-2] == j){
				Primes[j-2] = 0;
			}
		}	
}

__kernel void PrimeFactors(__global int* Primes,__global int* Factors, int UserVal) {
	int global_id = get_global_id(0);
	if(Primes[global_id] > UserVal/2)
		return;

	if(UserVal % Primes[global_id] == 0){
		Factors[global_id] = 1;
	}
	else {Factors[global_id] =0;
	}
}