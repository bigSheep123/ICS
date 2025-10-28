#include <iostream>
using namespace std;
#include <vector>
int removeElement(vector<int> &nums, int val)
{
    int counter = 0;
    int j = nums.size() - 1;
    for (int i = 0; i <= j;)
    {
        if (nums[i] == val)
        {
            counter++;
            nums[i] = nums[j--];
        }
        else
        {
            i++;
        }
    }
    return counter;
}

int main() {
    vector<int> nums = {0,1,2,2,3,0,4,2};
    removeElement(nums,2);
    for (auto& e : nums) {
        printf("%d ",e);
    }
    printf("\n");
    return 0;
}