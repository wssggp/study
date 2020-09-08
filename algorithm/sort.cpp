#include <algorithm>
#include <iostream>
#include <vector>

void bubble_sort(std::vector<int> &s)
{
    bool has_changed = true;
    int last_change = s.size() - 1;
    int pos = 0;                                          //pos变量用来标记循环里最后一次交换的位置
    for (int i = s.size() - 1; i > 0 && has_changed; i--) //一共要排序size-1次
    {
        has_changed = false;
        for (int j = 1; j <= last_change; j++)
        {
            if (s[j] < s[j - 1])
            {
                std::swap(s[j], s[j - 1]);
                has_changed = true;
                pos = j - 1;
            }
        }
        last_change = pos;
    }
}

void selection_sort(std::vector<int> &s)
{
    for (int i = 0; i < s.size() - 1; i++)
    {
        int min_pos = i;
        for (int j = i + 1; j < s.size(); j++)
        {
            if (s[min_pos] > s[j])
            {
                min_pos = j;
            }
        }
        std::swap(s[i], s[min_pos]);
    }
}
void selection_sort2(std::vector<int> &s)
{
    int left = 0;
    int right = s.size() - 1;
    while (left < right)
    {
        int min = left;
        int max = right;
        for (int i = left + 1; i <= right; i++) // 这里要等于right
        {
            if (s[max] < s[i])
            {
                max = i;
            }

            if (s[min] > s[i])
            {
                min = i;
            }
        }
        std::swap(s[left], s[min]);
        std::swap(s[right], s[max]);

        left++;
        right--;
    }
}

void insert_sort(std::vector<int> &s)
{
    for (int i = 1; i < s.size(); i++)
    {
        if (s[i] < s[i - 1])
        {
            int tmp = s[i];
            int j = i - 1;
            for (; j >= 0 && s[j] > tmp; j--)
            {
                s[j + 1] = s[j];
            }
            s[j + 1] = tmp;
        }
    }
}

void shell_sort(std::vector<int> &s)
{
    for (int gap = s.size() / 2; gap > 0; gap /= 2)
    {
        for (int i = gap; i < s.size(); i++)
        {
            if (s[i] < s[i - gap])
            {
                int tmp = s[i];
                int j = i - gap;
                for (; j >= 0 && s[j] > tmp; j -= gap)
                {
                    s[j + gap] = s[j];
                }
                s[j + gap] = tmp;
            }
        }
    }
}

void merge_arry(std::vector<int> &s, int start, int mid, int end, std::vector<int> &tmp)
{
    int left = start;
    int right = mid + 1;
    int k = 0;
    while (left <= mid && right <= end)
    {
        if (s[left] < s[right])
        {
            tmp[k++] = s[left++];
        }
        else
        {
            tmp[k++] = s[right++];
        }
        // 也可以这样写
        // tmp[k++] = s[left] < s[right] ? s[left++] : s[right++];
    }
    while (left <= mid)
    {
        tmp[k++] = s[left++];
    }
    while (right <= end)
    {
        tmp[k++] = s[right++];
    }

    for (int i = 0; i < k; i++)
    {
        s[start + i] = tmp[i];
    }
}

void merge(std::vector<int> &s, int start, int end, std::vector<int> &tmp)
{
    if (start >= end)
    {
        return;
    }
    int mid = (start + end) / 2;
    merge(s, start, mid, tmp);
    merge(s, mid + 1, end, tmp);
    merge_arry(s, start, mid, end, tmp);
}

void merge_sort(std::vector<int> &s)
{
    std::vector<int> tmp;
    tmp.resize(s.size());
    merge(s, 0, s.size() - 1, tmp);
}

void merge_sort2(std::vector<int> &s)
{
    std::vector<int> tmp;
    tmp.resize(s.size());
    int step = 1;

    while (step < s.size())
    {
        int start = 0;
        while (start + step <= s.size() - 1)
        {
            int mid = start + step - 1;
            int end = mid + step;
            if (end > s.size() - 1)
            {
                end = s.size() - 1; //第二个序列个数不足step
            }
            merge_arry(s, start, mid, end, tmp);
            // 将i和i+step这两个有序序列进行合并
            // 当i以后的长度小于或者等于step时，退出
            start = end + 1;
        }
        step *= 2;
    }
}

int partition(std::vector<int> &s, int start, int end)
{
    int pos = start;
    for (int i = start + 1; i <= end; i++)
    {
        if (s[start] > s[i])
        {
            std::swap(s[++pos], s[i]);
        }
    }
    std::swap(s[pos], s[start]);
    return pos;
}

int partition2(std::vector<int> &s, int start, int end)
{
    int left = start;
    int right = end;
    int key = s[start];
    while (left < right)
    {
        while (right > left && s[right] > key)
        {
            right--;
        }
        if (right > left)
        {
            s[left++] = s[right];
        }

        while (right > left && s[left] <= key)
        {
            left++;
        }
        if (right > left)
        {
            s[right--] = s[left];
        }
    }
    s[left] = key;

    return left;
}

void partition_sort(std::vector<int> &s, int start, int end)
{
    if (start >= end)
    {
        return;
    }
    int pos = partition2(s, start, end);

    partition_sort(s, start, pos - 1);
    partition_sort(s, pos + 1, end);
}

void quick_sort(std::vector<int> &s)
{
    partition_sort(s, 0, s.size() - 1);
}

void adjust_heap(std::vector<int> &s, int i, int len) // 这里的len由外面传进来，不能用数组的大小
{
    int tmp = s[i];
    for (int k = i * 2 + 1; k < len; k = k * 2 + 1) // 从i结点的左子结点开始，也就是2i+1处开始
    {
        if (k + 1 < len && s[k] < s[k + 1]) //如果左子结点小于右子结点，k指向右子结点, 这里主要是找子节点中值最大的一个
        {
            k++;
        }
        if (s[k] > tmp) //如果子节点大于父节点，将子节点值赋给父节点（不用进行交换）
        {
            s[i] = s[k];
            i = k;
        }
        else
        {
            break;
        }
    }
    s[i] = tmp; //将temp值放到最终的位置
}

void heap_sort(std::vector<int> &s)
{
    // 构建大顶堆
    //从第一个非叶子结点从下至上，从右至左调整结构
    for (int i = s.size() / 2 - 1; i >= 0; i--)
    {
        adjust_heap(s, i, s.size());
    }

    // 调整堆结构+交换堆顶元素与末尾元素
    for (int i = s.size() - 1; i > 0; i--)
    {
        std::swap(s[0], s[i]); //将堆顶元素与末尾元素进行交换
        adjust_heap(s, 0, i);  //重新对堆进行调整
    }
}

void counting_sort(std::vector<int> &s)
{
    if (s.size() <= 1)
    {
        return;
    }
    // 找出最大最小值
    int min = s[0];
    int max = s[0];
    for (auto v : s)
    {
        if (min > v)
            min = v;
        if (max < v)
            max = v;
    }
    if (min < 0)
        return;
    std::vector<int> count; // 用于计数
    count.resize(max + 1);

    std::vector<int> tmp; // 用于临时存储排序数据
    tmp.resize(s.size());

    for (int i = 0; i < s.size(); i++)
        count[s[i]]++;

    for (int i = 1; i < count.size(); i++)
        count[i] += count[i - 1];

    for (int i = s.size() - 1; i >= 0; i--)
        tmp[--count[s[i]]] = s[i]; // 这里需要--count[s[i]]，防止一个元素有多个

    for (int i = 0; i < s.size(); i++)
        s[i] = tmp[i];
}

void bucket_sort(std::vector<int> &s)
{
    const static int BUCKET_SIZE = 10; // 这里设置桶的大小为10
    if (s.size() <= 1)
        return;
    // 找出最大最小值
    int min = s[0];
    int max = s[0];
    for (auto v : s)
    {
        if (min > v)
            min = v;
        if (max < v)
            max = v;
    }

    int bucket_count = (max - min) / BUCKET_SIZE + 1;
    std::vector<std::vector<int>> bucket;
    bucket.resize(bucket_count);

    for (auto v : s)
        bucket[(v - min) / BUCKET_SIZE].push_back(v);

    s.clear();
    for (auto &b : bucket)
    {
        insert_sort(b);
        for (auto v : b)
            s.push_back(v);
    }
}

void radix_sort(std::vector<int> &s)
{
    if (s.size() <= 1)
        return;
    // 找出最大值
    int max = s[0];
    for (auto v : s)
    {
        if (max < v)
            max = v;
    }

    int max_digit = 1;
    int dev = 10;
    while (max / dev >= 1)
    {
        max_digit++;
        dev *= 10;
    }

    int mod = 10;
    dev = 1;

    for (int i = 0; i < max_digit; i++, dev *= 10, mod *= 10)
    {
        std::vector<std::vector<int>> bucket;
        bucket.resize(10);
        for (int j = 0; j < s.size(); j++)
        {
            int digit = s[j] % mod / dev;
            bucket[digit].push_back(s[j]);
        }
        s.clear();
        for (auto &b : bucket)
        {
            for (auto v : b)
            {
                s.push_back(v);
            }
        }
    }
}

void print(const std::vector<int> &s)
{
    for (auto v : s)
    {
        std::cout << v << " ";
    }
    std::cout << std::endl;
}

int main()
{
    std::vector<int> arr{5, 8, 2, 4, 22, 1, 0, 88, 99, 66, 87, 567, 88, 776, 44, 345, 76};
    //std::vector<int> arr{5, 8, 2};
    radix_sort(arr);
    print(arr);
}