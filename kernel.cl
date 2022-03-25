
__kernel void bitonic_hard(__global int *arr, const int biton_size, const int bucket_size) {

	int i = get_global_id(0);
	int j = get_global_id(1);
	int k = get_global_id(2);

	int local_i = get_local_id(0);

	__local int vec[16];

	printf("bucket_size - %d, biton_size - %d\n", bucket_size, biton_size);

	printf("i: %d, %d\n", i, local_i);

	printf("global id: %d, %d, %d\n", i, j, k);
	printf("global size: %d, %d, %d\n", get_global_size(0), get_global_size(1), get_global_size(2));


	int up = (i * biton_size + bucket_size * j + k) % (8 * 2); // ??
	int low = (i * biton_size + bucket_size * j + k + bucket_size / 2) % (8 * 2); // ??
	
	int pack = (i / 8) * 8 * biton_size + local_i;

	vec[local_i] = arr[pack]; //?
	vec[local_i + 8] = arr[pack + 8]; //?

	printf("pos: %d, %d - [%d;%d] and %d\n", pack, pack + 8, i, local_i, pack);
	printf("vec: %d, %d - %d\n", vec[local_i], vec[local_i + 8], local_i);


	printf("not slice up and low: %d, %d - %d\n", i * biton_size + bucket_size * j + k, i * biton_size + bucket_size * j + k + bucket_size / 2, pack);
	printf("up and low: %d, %d - %d\n", up, low, pack);
	//printf("arr up and low: %d, %d - %d\n", arr[up], arr[low], pack);
	printf("vec up and low: %d, %d - %d\n", vec[up], vec[low], pack);

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

	arr[pack] = vec[local_i]; //?
	arr[pack + 8] = vec[local_i + 8]; //?

}


__kernel void bitonic_simple(__global int *arr, const int biton_size, const int bucket_size) {

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