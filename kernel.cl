
#define WORK_GROUP_SIZE 64

// i - bitons_counter, j - buckets_counter, k - count in bucket

int get_local_linear_id_std() {
	return  (get_local_id(2) * get_local_size(1) * get_local_size(0)) + 
			(get_local_id(1) * get_local_size(0)) + 
			(get_local_id(0));
}

__kernel void small_biton(__global int *arr, const int biton_size, const int bucket_size) {

	int direction = get_global_id(0);

	int local_i = get_local_id(0);
	int local_j = get_local_id(1);
	int local_k = get_local_id(2);

	__local int vec[WORK_GROUP_SIZE * 2];
	
	int global_pos = 2 * WORK_GROUP_SIZE * get_group_id(0);
	int local_pos = get_local_linear_id_std();

	int local_up = local_i * biton_size + bucket_size * local_j + local_k;
	int local_low = local_i * biton_size + bucket_size * local_j + local_k + bucket_size / 2;

	int up = global_pos + local_up;
	int low = global_pos + local_low;

	vec[local_up] = arr[up];
	vec[local_low] = arr[low];

	barrier(CLK_LOCAL_MEM_FENCE);

	if ((direction % 2) == 0) {

		if (vec[local_up] > vec[local_low]) {

			int tmp = vec[local_up];
			vec[local_up] = vec[local_low];
			vec[local_low] = tmp;
		}
    } 
    else {

		if (vec[local_up] < vec[local_low]) {

			int tmp = vec[local_up];
			vec[local_up] = vec[local_low];
			vec[local_low] = tmp;
		}
    }

	barrier(CLK_LOCAL_MEM_FENCE);

	arr[up] = vec[local_up];
	arr[low] = vec[local_low];
}


__kernel void big_biton(__global int *arr, const int biton_size, const int bucket_size) {

	int direction = get_global_id(0);

	int local_i = get_local_id(0);
	int local_j = get_local_id(1);
	int local_k = get_local_id(2);

	__local int vec[WORK_GROUP_SIZE * 2];

	int global_pos = get_group_id(0) * biton_size + get_group_id(1) * 2 * WORK_GROUP_SIZE;
	int local_pos = get_local_linear_id_std();

	int local_up = local_i * biton_size + bucket_size * local_j + local_k;
	int local_low = local_i * biton_size + bucket_size * local_j + local_k + bucket_size / 2; 

	int up = global_pos + local_up;
	int low = global_pos + local_low;

	vec[local_up] = arr[up]; 
	vec[local_low] = arr[low]; 

	barrier(CLK_LOCAL_MEM_FENCE);

	if ((direction % 2) == 0) {

		if (vec[local_up] > vec[local_low]) {

			int tmp = vec[local_up];
			vec[local_up] = vec[local_low];
			vec[local_low] = tmp;
		}
    } 
    else {

		if (vec[local_up] < vec[local_low]) {

			int tmp = vec[local_up];
			vec[local_up] = vec[local_low];
			vec[local_low] = tmp;
		}
    }

	barrier(CLK_LOCAL_MEM_FENCE);

	arr[up] = vec[local_up];
	arr[low] = vec[local_low];
}

__kernel void big_bucket(__global int *arr, const int biton_size, const int bucket_size) {

	int i = get_global_id(0);
	int j = get_global_id(1);
	int k = get_global_id(2);

	__local int vec[WORK_GROUP_SIZE * 2];

	int local_pos = get_local_linear_id_std();

    int up = i * biton_size + bucket_size * j + k;
    int low = i * biton_size + bucket_size * j + k + bucket_size / 2;

	int local_up = local_pos;
	int local_low = local_pos + WORK_GROUP_SIZE;

	vec[local_up] = arr[up];
	vec[local_low] = arr[low];

	barrier(CLK_LOCAL_MEM_FENCE);

	if ((i % 2) == 0) {

		if (vec[local_up] > vec[local_low]) {

			int tmp = vec[local_up];
			vec[local_up] = vec[local_low];
			vec[local_low] = tmp;
		}
    } 
    else {

		if (vec[local_up] < vec[local_low]) {

			int tmp = vec[local_up];
			vec[local_up] = vec[local_low];
			vec[local_low] = tmp;
		}
    }

   	barrier(CLK_LOCAL_MEM_FENCE);

   	arr[up] = vec[local_up]; 
	arr[low] = vec[local_low]; 
}