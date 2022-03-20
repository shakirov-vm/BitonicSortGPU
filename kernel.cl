
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