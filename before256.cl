
// Догоняем размер битонной последовательности до 256
// Начинает с 256 просто элементов, на выход должен давать битонную последовательность из 256 элементов

#define WORK_GROUP_SIZE 256

__kernel void before256(__global int *arr) {

	int i = get_global_id(0);
	//int j = get_global_id(1);
	//int k = get_global_id(2);

	__local int tile[WORK_GROUP_SIZE * 2];

	int local_i = get_local_id(0);
	int local_j = get_local_id(1);
	int local_k = get_local_id(2);

	int local_size_i = get_local_size(0);
	int local_size_j = get_local_size(1);
	int local_size_k = get_local_size(2);

	int place = local_i * local_size_i + local_j * local_size_j + local_k; // ??

	tile[place] = arr[i * WORK_GROUP_SIZE + place];
	tile[place + WORK_GROUP_SIZE] = arr[i * WORK_GROUP_SIZE + WORK_GROUP_SIZE + place];

	for (int biton_size = 2; biton_size < WORK_GROUP_SIZE * 2 + 1; biton_size *= 2) {

        for (int bucket_size = biton_size; bucket_size > 1; bucket_size /= 2) {

        	local_i = WORK_GROUP_SIZE / biton_size;
        	local_j = biton_size / bucket_size;
        	local_k = bucket_size / 2;


			if ((local_i % 2) == 0) {

				int up = local_i * biton_size + bucket_size * local_j + local_k;
				int low = local_i * biton_size + bucket_size * local_j + local_k + bucket_size / 2;

				if (arr[up] > arr[low]) {

					int tmp = arr[up];
					arr[up] = arr[low];
					arr[low] = tmp;
				}
		    } 
		    else {

		        int up = local_i * biton_size + bucket_size * local_j + local_k;
		        int low = local_i * biton_size + bucket_size * local_j + local_k + bucket_size / 2;

		        if (arr[up] < arr[low]) {

		            int tmp = arr[up];
		            arr[up] = arr[low];
		            arr[low] = tmp;
		        }
		    }

		    barrier(CLK_LOCAL_MEM_FENCE);
        }
    }

    arr[i * WORK_GROUP_SIZE + place] = tile[place];
	arr[i * WORK_GROUP_SIZE + WORK_GROUP_SIZE + place] = tile[place + WORK_GROUP_SIZE];
}
