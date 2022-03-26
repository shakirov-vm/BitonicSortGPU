
#define WORK_GROUP_SIZE 8


__kernel void bitonic_hard(__global int *arr, const int biton_size, const int bucket_size) {

	int i = get_global_id(0);
	int j = get_global_id(1);
	int k = get_global_id(2);

	int local_i = get_local_id(0);
	int local_j = get_local_id(1);
	int local_k = get_local_id(2); 

	__local int vec[2 * WORK_GROUP_SIZE];

	int local_stupid = local_i * get_local_size(0) + local_j * get_local_size(1) + local_k * get_local_size(2);

	int local_up = local_i * biton_size + bucket_size * local_j + local_k;
	int local_low = local_i * biton_size + bucket_size * local_j + local_k + bucket_size / 2;

	int group = get_group_id(0); // ??

	int up = 2 * WORK_GROUP_SIZE * group + local_up;
	int low = 2 * WORK_GROUP_SIZE * group + local_low;

	vec[local_up] = arr[up]; // Not cash frendly
	vec[local_low] = arr[low];

	printf("global: up - %d, low - %d, arr[up] - %d, arr[low] - %d\n", up, low, arr[up], arr[low]);
	printf("local: l_up - %d, l_low - %d, vec[l_up] - %d, vec[l_low] - %d\n", local_up, local_low, vec[local_up], vec[local_low]);

	barrier(CLK_LOCAL_MEM_FENCE);
//       ???????????
	if ((local_stupid % 2) == 0) {

		printf("equal : %d > %d - %d\n", vec[local_up], vec[local_low], local_i);

		if (vec[local_up] > vec[local_low]) {

			int tmp = vec[local_up];
			vec[local_up] = vec[local_low];
			vec[local_low] = tmp;
		}
    } 
    else {

		printf("equal : %d < %d - %d\n", vec[local_up], vec[local_low], local_i);

        if (vec[local_up] < vec[local_low]) {

            int tmp = vec[local_up];
            vec[local_up] = vec[local_low];
            vec[local_low] = tmp;
        }
    }

   	arr[up] = vec[local_up]; // Not cash frendly
	arr[low] = vec[local_low];
}

__kernel void bitonic_hard_1(__global int *arr, const int biton_size, const int bucket_size) {

	int i = get_global_id(0);
	int j = get_global_id(1);
	int k = get_global_id(2);

	int local_i = get_local_id(0);
	int local_j = get_local_id(1);
	int local_k = get_local_id(2);

	__local int vec[WORK_GROUP_SIZE * 2];


	//int local_pos = local_i * get_local_size(1) + local_j * get_local_size(2) + local_k; //? ?? ?? ??? ?? 
	//int global_pos = i * get_global_size(1) + j * get_global_size(2) + k; // ?? ?  ?? ? ? ?? 
	//int local_pos = local_i + local_j * get_local_size(0) + local_k * get_local_size(1);
	//int global_pos = get_group_id() * WORK_GROUP_SIZE + local_pos;

	int gr = get_group_id(0);
										// ??????????
	//int global_pos = gr * WORK_GROUP_SIZE + local_pos;
	
	// for printf
	int global_pos = 0;
	int local_pos = 0;

	printf("global id: %d, %d, %d - %d\n", i, j, k, global_pos);
	printf("global size: %d, %d, %d\n", get_global_size(0), get_global_size(1), get_global_size(2));
	printf("local size %d: local id: %d, %d, %d [%d, %d, %d]\n", local_pos, local_i, local_j, local_k, get_local_size(0), get_local_size(1), get_local_size(2));

	int local_up = local_i * biton_size + bucket_size * local_j + local_k; // ??
	int local_low = local_i * biton_size + bucket_size * local_j + local_k + bucket_size / 2; // ??
								// ??????????
	//int up = 2 * WORK_GROUP_SIZE * (gr) + local_pos;
	//int low = 2 * WORK_GROUP_SIZE * (gr) + local_pos + WORK_GROUP_SIZE;

	int up = 2 * WORK_GROUP_SIZE * gr + local_up;
	int low = 2 * WORK_GROUP_SIZE * gr + local_low;

	printf("global: %d, %d - [%d;%d]\n", up, low, local_pos, global_pos);
	printf("local: %d, %d - [%d;%d]\n", local_up, local_low, local_pos, global_pos);

	vec[local_up] = arr[up]; //?
	vec[local_low] = arr[low]; //?

	//printf("global up and low: %d, %d - [%d;%d]\n", arr[up], arr[low], local_pos, global_pos);
	//printf("local up and low: %d, %d - [%d;%d]\n", vec[local_up], vec[local_low], local_pos, global_pos);

	//printf("not slice up and low: %d, %d - %d\n", i * biton_size + bucket_size * j + k, i * biton_size + bucket_size * j + k + bucket_size / 2, pack);
	//printf("up and low: %d, %d - %d\n", up, low, pack);

	barrier(CLK_LOCAL_MEM_FENCE);

	if ((local_i % 2) == 0) {

		printf("vec > : %d, %d - %d : [%d;%d]\n", vec[local_up], vec[local_low], local_i, local_pos, global_pos);

		if (vec[local_up] > vec[local_low]) {

			int tmp = vec[local_up];
			vec[local_up] = vec[local_low];
			vec[local_low] = tmp;
		}
    } 
    else {

		printf("vec < : %d, %d - %d : [%d;%d]\n", vec[local_up], vec[local_low], local_i, local_pos, global_pos);

		if (vec[local_up] < vec[local_low]) {

			int tmp = vec[local_up];
			vec[local_up] = vec[local_low];
			vec[local_low] = tmp;
		}
    }

    printf("in end: %d, %d - [%d;%d]\n", vec[local_up], vec[local_low], local_pos, global_pos);

	barrier(CLK_LOCAL_MEM_FENCE);

	arr[up] = vec[local_up]; //?
	arr[low] = vec[local_low]; //?

}

__kernel void bitonic_simple(__global int *arr, const int biton_size, const int bucket_size) {

	int i = get_global_id(0);
	int j = get_global_id(1);
	int k = get_global_id(2);

	if ((i % 2) == 0) {

		int up = i * biton_size + bucket_size * j + k;
		int low = i * biton_size + bucket_size * j + k + bucket_size / 2;

		printf("simple equal : %d > %d - %d\n", arr[up], arr[low], i);

		if (arr[up] > arr[low]) {

			int tmp = arr[up];
			arr[up] = arr[low];
			arr[low] = tmp;
		}
    } 
    else {

        int up = i * biton_size + bucket_size * j + k;
        int low = i * biton_size + bucket_size * j + k + bucket_size / 2;

		printf("simple equal : %d > %d - %d\n", arr[up], arr[low], i);

        if (arr[up] < arr[low]) {

            int tmp = arr[up];
            arr[up] = arr[low];
            arr[low] = tmp;
        }
    }

	/*int global_pos = get_global_id(0) * get_global_size(1) + get_global_id(1) * get_global_size(2) + get_global_id(2); // ?? ?  ?? ? ? ?? 
	int a = get_global_size(0);
	int b = get_global_size(1);
	int c = get_global_size(2);
	printf("global id: %d - %d, %d - %d, %d - %d : %d - [%d;%d]\n", i, a, j, b, k, c, global_pos, arr[i * biton_size + bucket_size * j + k], arr[i * biton_size + bucket_size * j + k + bucket_size / 2]);
	*/
	//printf("global size: %d, %d, %d\n", get_global_size(0), get_global_size(1), get_global_size(2));
}