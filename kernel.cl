
__kernel void bitonic_simple(__global int *arr, int k, int j) {

	int i = get_global_id(0);

	int l = i ^ j;
	if (l > i) {

		if ((i ^ k == 0) && (arr[i] > arr[l]) 
		 || (i ^ k != 0) && (arr[i] < arr[l])) {
			
			int tmp = arr[i];
			arr[i] = arr[j];
			arr[j] = tmp;
		}
	}
}