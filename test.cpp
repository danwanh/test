/// heloooooooooooooooooooooo
#include <iostream>
using namespace std;

void test(int *a, int n){
	swap(a[0], a[2]);
}
void test_(int *&a, int n){
	a = a + 2;
}
int findMajority(int arr[], int n)
{
    int i, candidate = -1, votes = 0;
    // Finding majority candidate
    for (i = 0; i < n; i++) {
        if (votes == 0) {
            candidate = arr[i];
            votes = 1;
        }
        else {
            if (arr[i] == candidate)
                votes++;
            else
                votes--;
        }
    }
    return candidate;
}
int func(int *&a, int n){
	a = new int[n]{0};
	int sum = 0;
	for(int i = 0; i < n; i++){
		int tmp = 0;
		for(int j = 0; j <= i; j++){
			for(int k = 0; k <= j; k++){
				tmp += 1;
			}
			a[i] = sum + tmp;
			sum = tmp;
		}
	}
}
// Driver's code
int main()
{
   /*int *a = new int[3]{1, 2, 5};
   cout << a  << " " << *a << endl;
   test_(a, 3);
   cout << a[0] << " " << a[1] << " " << a[2];
   cout << endl << a << " " << *a; //dia chi khong doi nhung gia tri thay doi*/
   //int a[9] = {1, 5, 3, 1, 5, 1, 5, 6, 7};
   //cout << findMajority(a,9 );
   int a[10];
   int* ptr = a;
   func(ptr, 10);
   for(int i = 0; i < 10; i++){
   	cout << ptr[i] << " ";
   }
	return 0; 
}
