#include <iostream>
using namespace std;

// *******************************************************
// Ronnie Smith — QuickSort Demo for 3 Test Arrays
// Goal: Sort arrays in ascending order using QuickSort
// *******************************************************

// swap helper (clean readability)
void swapValues(int &a, int &b) {
    int temp = a;
    a = b;
    b = temp;
}

// partition function (this breaks array around pivot)
// pivot = last element, everything smaller moves left
int splitArray(int arr[], int start, int end) {

    int pivot = arr[end];     // take last value as pivot
    int leftIndex = start - 1; // track area for "small values"

    // walk through section and push small values to left side
    for (int i = start; i < end; i++) {

        if (arr[i] <= pivot) {    
            leftIndex++;           
            swapValues(arr[leftIndex], arr[i]);
        }
    }

    // move pivot into final sorted position
    swapValues(arr[leftIndex + 1], arr[end]);
    return leftIndex + 1;
}

// main QuickSort (recursive: break > sort > merge idea)
void doQuickSort(int arr[], int start, int end) {

    if (start < end) {

        int pivotSpot = splitArray(arr, start, end);

        // left side of pivot
        doQuickSort(arr, start, pivotSpot - 1);

        // right side of pivot
        doQuickSort(arr, pivotSpot + 1, end);
    }
}

// print helper function
void showArray(int arr[], int size) {
    for (int i = 0; i < size; i++) {
        cout << arr[i] << " ";
    }
    cout << endl;
}

// ****************************
// main program
// ****************************
int main() {

    // test arrays from assignment
    int arr1[] = {10, 7, 8, 9, 1, 5};
    int arr2[] = {3, 6, 8, 10, 1, 2, 1};
    int arr3[] = {5, 4, 5, 4, 5, 4, 5, 4};

    int size1 = sizeof(arr1) / sizeof(arr1[0]);
    int size2 = sizeof(arr2) / sizeof(arr2[0]);
    int size3 = sizeof(arr3) / sizeof(arr3[0]);


    cout << "Given Array 1: ";
    showArray(arr1, size1);
    doQuickSort(arr1, 0, size1 - 1);
    cout << "Sorted Array 1:   ";
    showArray(arr1, size1);

    cout << "\nGiven Array 2: ";
    showArray(arr2, size2);
    doQuickSort(arr2, 0, size2 - 1);
    cout << "Sorted Array 2:   ";
    showArray(arr2, size2);

    cout << "\nGiven Array 3: ";
    showArray(arr3, size3);
    doQuickSort(arr3, 0, size3 - 1);
    cout << "After  Array 3:   ";
    showArray(arr3, size3);

    return 0;
}
