
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>

const int ARR_SIZE = 8;

int main() {

    std::vector<int> arr(ARR_SIZE);
    
    std::ifstream in_num("./test.txt");

    for(int i = 0; i < ARR_SIZE; i++) {
        in_num >> arr[i];
    }

    for (int i = 0; i < ARR_SIZE; i++) {
        std::cout << arr[i] << " ";
    }
    std::cout << std::endl;

    //std::cout << "We print array" << std::endl;

    /*int l = 0;

    for (int k = 2; k <= ARR_SIZE; k *= 2) {// k is doubled every iteration
        for (int j = k/2; j > 0; j /= 2) {// j is halved at every iteration, with truncation of fractional parts
            for (int i = 0; i < ARR_SIZE; i++) {
                l = i ^ j; // in C-like languages this is "i ^ j"
                std::cout << l << std::endl;
                if (l > i) {
                    if (  (((i ^ k) == 0) && (arr[i] > arr[l]))
                       || (((i ^ k) != 0) && (arr[i] < arr[l])) ) 
                    
                    {
                   		
                   		int tmp = arr[i];
						arr[i] = arr[l];
						arr[l] = tmp;
                      	//swap the elements arr[i] and arr[l]
                    }
                }
    		}
    	}
    }*/

    for (int biton_size = 2; biton_size < ARR_SIZE + 1; biton_size *= 2) {

		for (int bucket_size = biton_size; bucket_size > 1; bucket_size /= 2) {

		    for (int i = 0; i < (ARR_SIZE / biton_size); i++) {

                if ((i % 2) == 0) {

    		    	for (int j = 0; j < (biton_size / bucket_size); j++) {

    		    		for (int k = 0; k < bucket_size / 2; k++) {

    		    			int up = i * biton_size + bucket_size * j + k;
    		    			int low = i * biton_size + bucket_size * j + k + bucket_size / 2;

    			    		if (arr[up] > arr[low]) {

    			    			int tmp = arr[up];
    			    			arr[up] = arr[low];
    			    			arr[low] = tmp;
    		    			}
    		    		}
    		    	}
                } 
                else {

                    for (int j = 0; j < (biton_size / bucket_size); j++) {

                        for (int k = 0; k < bucket_size / 2; k++) {

                            int up = i * biton_size + bucket_size * j + k;
                            int low = i * biton_size + bucket_size * j + k + bucket_size / 2;

                            if (arr[up] < arr[low]) {
     
                                int tmp = arr[up];
                                arr[up] = arr[low];
                                arr[low] = tmp;
                            }
                        }
                    }
                }
		    }

            for (int i = 0; i < ARR_SIZE; i++) {
                std::cout << arr[i] << " ";
            }

            std::cout << "biton_size - " << biton_size << ", bucket_size - " << bucket_size;
            std::cout << std::endl;
		}
	}

    std::cout << std::endl;

    for (int i = 0; i < ARR_SIZE; i++) {
        std::cout << arr[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "We print array" << std::endl;

    std::sort(arr.begin(), arr.end());


    for (int i = 0; i < ARR_SIZE; i++) {
        std::cout << arr[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "We print array" << std::endl;

}