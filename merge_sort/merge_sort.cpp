#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <thread>
#include <random>
#include <vector>
#include <algorithm>
#include <cmath>  
#include <chrono>
#include <future>

using namespace std;
using namespace std::chrono;
 
void merge(std::vector<int>& arr, int l, int m, int r)
{
    int i, j, k;
    int n1 = m - l + 1;
    int n2 =  r - m;
 
    int L[n1], R[n2];
 
    for (i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[m + 1+ j];
 
    i = 0;
    j = 0;
    k = l;
    while (i < n1 && j < n2)
    {
        if (L[i] <= R[j])
        {
            arr[k] = L[i];
            i++;
        }
        else
        {
            arr[k] = R[j];
            j++;
        }
        k++;
    }
 
    while (i < n1)
    {
        arr[k] = L[i];
        i++;
        k++;
    }
 
    while (j < n2)
    {
        arr[k] = R[j];
        j++;
        k++;
    }
}
 
void mergeSort(std::vector<int>& arr, int l, int r)
{
    if (l < r)
    {
        int m = l+(r-l)/2;
 
        mergeSort(arr, l, m);
        mergeSort(arr, m+1, r);
 
        merge(arr, l, m, r);
    }
}

void parallelMergeSort(std::vector<int>& arr, int l, int r, int n)
{
    if (l < r)
    {
        int m = l+(r-l)/2;

        if(n>0)
        {
            future<void> p1 = async(parallelMergeSort, ref(arr), l, m, n-2);
            future<void> p2 = async(parallelMergeSort, ref(arr), m+1, r, n-2);
            p1.get();
            p2.get();
        }
        else
        {    
            mergeSort(arr, l, m);
            mergeSort(arr, m+1, r);   
        }

        merge(arr, l, m, r); 
    }
}

void print_arr(std::vector<int>& arr)
{
	for (int& item: arr)
    {
    	cout << item << " ";
    }
    cout << endl;
    cout.flush();
}
 
int main()
{
    unsigned int n = std::thread::hardware_concurrency();
    std::cout << n << " concurrent threads are supported.\n";
    
    std::random_device rd; 
    std::mt19937 gen(rd()); 
    std::uniform_int_distribution<> dis(1, 1000);

    size_t size = 1000000;;
    std::vector<int> arr(size);

    std::generate(arr.begin(), arr.end(), [&](){return dis(gen);});    
    
    vector<int> copy1(arr.begin(), arr.end());
    high_resolution_clock::time_point start1 = high_resolution_clock::now();
    parallelMergeSort(copy1, 0, copy1.size()-1, n);
    high_resolution_clock::time_point end1 = high_resolution_clock::now();

    cout << "Last member = " << copy1[size-1] << endl;
    cout << "parallelPrefixSum execution time " << duration_cast<microseconds>( end1 - start1 ).count() << endl;    

    //print_arr(copy2);
    vector<int> copy2(arr.begin(), arr.end());
    high_resolution_clock::time_point start2 = high_resolution_clock::now();
    mergeSort(copy2, 0, copy2.size());
    high_resolution_clock::time_point end2 = high_resolution_clock::now();

    cout << "Last member = " << copy2[size-1] << endl;
    cout << "normalPrefixSum execution time " << duration_cast<microseconds>( end2 - start2 ).count() << endl;
    
    std::cout.flush();
}