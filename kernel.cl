
__kernel void bitonic_hard(__global int *arr, const int biton_size, const int bucket_size) {

	int i = get_global_id(0);
	int j = get_global_id(1);
	int k = get_global_id(2);

	int local_i = get_local_id(0);

	__local int vec[16];

	int up = i * biton_size + bucket_size * j + k;
	int low = i * biton_size + bucket_size * j + k + bucket_size / 2;
	
	vec[local_i] = arr[local_i]; //?
	vec[local_i + 8] = arr[local_i + 8]; //?

	printf("pos: %d, %d - %d\n", local_i, local_i + 8);
	printf("vec: %d, %d - %d\n", vec[local_i], vec[local_i + 8], local_i);


	printf("up and low: %d, %d - %d\n", up, low, local_i);
	printf("vec up and low: %d, %d - %d\n", vec[up], vec[low], local_i);

	barrier(CLK_LOCAL_MEM_FENCE);

	if ((local_i % 2) == 0) {

		printf("vec > : %d, %d - %d\n", vec[up], vec[low], local_i);

		if (vec[up] > vec[low]) {

			int tmp = vec[up];
			vec[up] = vec[low];
			vec[low] = tmp;
		}
    } 
    else {

		printf("vec < : %d, %d - %d\n", vec[up], vec[low], local_i);

		if (vec[up] < vec[low]) {

			int tmp = vec[up];
			vec[up] = vec[low];
			vec[low] = tmp;
		}
    }

    printf("in end: %d, %d - %d\n", vec[up], vec[low], local_i);

	barrier(CLK_LOCAL_MEM_FENCE);

	arr[local_i] = vec[local_i]; //?
	arr[local_i + 8] = vec[local_i + 8]; //?
	printf("End\n");
}


__kernel void bitonic_simple(__global int *arr, const int biton_size, const int bucket_size) {
	printf("Start\n");
	int i = get_global_id(0);
	int j = get_global_id(1);
	int k = get_global_id(2);

	if ((i % 2) == 0) {

		int up = i * biton_size + bucket_size * j + k;
		int low = i * biton_size + bucket_size * j + k + bucket_size / 2;

		if (arr[up] > arr[low]) {

			int tmp = arr[up];
			arr[up] = arr[low];
			arr[low] = tmp;
		}
    } 
    else {

        int up = i * biton_size + bucket_size * j + k;
        int low = i * biton_size + bucket_size * j + k + bucket_size / 2;

        if (arr[up] < arr[low]) {

            int tmp = arr[up];
            arr[up] = arr[low];
            arr[low] = tmp;
        }
    }
}