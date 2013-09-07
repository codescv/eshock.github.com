---
layout: post
title: "排列问题"
date: 2013-09-07 12:47
comments: true
categories: 
---

## Kth Permutation
有一组数例如"123456", 它们可以组成 `n!`种排列, 将这些数从小到大排列:
    "123456"
    "123465"
    "123546"
    ...
问题: 在这些排列中,如何求出第k大的数(k从0开始)?

思考: 考虑简单的情况 n = 3:
    perm   k   k / (n-1)!
    123    0   0
    132    1   0
    213    2   1
    231    3   1
    312    4   2
    321    5   2
由于在n个元素组成的排列中, 以某个元素i开头的排列共有(n-1)!个. 不难得出结论:
1. 第k大的排列以 perm[k/(n-1)!] 开头.
2. 第k大的n个元素的排列, 等于以perm[k/(n-1)!]开头的元素, 加上去掉这个元素后剩下的元素组成的第 k % (n-1)! 大的排列.
于是就有了一个很简单的递归关系:
    kth_perm(perm, k) = perm[k/(n-1)!]  +  kth_perm(perm[0..i-1] + perm[i..n-1], k % (n-1)!)
根据这个关系可以写出kth_permutation的代码:
{% codeblock lang:c++ %}
    vector<int> kth_permutation(const vector<int> &vec, int k)
    {
        if (k == 0)
            return vec;

        int n = vec.size();
        k %= fact(n);
        int t = fact(n-1);

        int i = k / t;
        int kk = k % t;

        vector<int> result;
        result.push_back(vec[i]);
        vector<int> left;
        copy(vec.begin(), vec.begin()+i, back_inserter(left));
        copy(vec.begin()+i+1, vec.end(), back_inserter(left));
        left = kth_permutation(left, kk);
        copy(left.begin(), left.end(), back_inserter(result));
        return result;
    }
{% endcodeblock %}

## Permutation Rank
给定一个排列"231", 它是"123"的排列中第几大的?

由于2是第1大的, 因此有 `2! * 1` 个首位比 2 小的.

由于3是剩下的数中第1大的, 因此有 `1! * 1` 个第二位比3小的.

因此比"231"小的 有 `2! * 1 + 1 ! * 1 = 3`个, 因此"231"是第3大的. (均从0开始)

一个比较naive的实现：
{% codeblock lang:c++ %}
    int permutation_rank(vector<int> &vec)
    {
        vector<int> vec2(vec.begin(), vec.end());
        sort(vec2.begin(), vec2.end());
        map<int, int> ranks;
        for (int i = 0; i < vec2.size(); i++) {
            ranks[vec2[i]] = i;
        }

        int rank = 0;
        int n = vec.size();
        for (int i = 0; i < vec.size(); i++) {
            rank += fact(n-1) * ranks[vec[i]];
            for (map<int, int>::iterator it = ranks.begin(); it != ranks.end(); ++it) {
                if (it->first > vec[i])
                    it->second--;
            }
            n--;
        }
        return rank;
    }
{% endcodeblock %}

## Next permutation
已知一个排列"146532", 如何求出比它大的下一个排列? 这个问题被称为next_permutation.

一个简单的方法是, 使用前面的结论, 首先求出rank, 然后求第rank+1的排列.
    next_permutation(vec):
        k = rank(vec)
        return kth_permutation(vec, k+1)

不过也有一个更加subtle, 效率也更高的方法:

首先找出最大的i ,使得 `a[i] < a[i+1]`, 例如
    1 4 6 5 3 2
      i
此时, a是所有以a[0..i]开头的排列中最大的. 也就是说,
1 4 6 5 3 2是所有 1 4 开头排列中最大的. (如果这一步找不到, 说明原序列倒序排列, 已经是最大的一个排列了.)

现在我们要使a变大, 但是变大的幅度要尽可能小, 因此我们要找到一个刚好比a[i]大的数, 也就是比a[i]大的数中最小的一个.

因此,我们找出最大的j, 使得 `a[i] < a[j]`:
    1 4 6 5 3 2
      i   j

a[j]就是比a[i]大的数中最小的一个. 我们把它和a[i]交换:
    1 5 6 4 3 2
      i   j

此时, a是 a[0..i]开头的数中最大的一个, 这不满足我们的要求, 我们希望它是最小的一个. 因此, 我们将a[i+1..n-1]倒序:
    1 5 2 3 4 6

即为所求. c++实现:
{% codeblock lang:c++ %}
    void reverse(vector<int> &num, int i) {
        for(int j = num.size() - 1; j > i; j--) {
            swap(num[i], num[j]);
            i++;
        }
    }
    
    void nextPermutation(vector<int> &num) {
        if (num.size() <= 1) {
            return;
        }
        
        // find max i where num[i] < num[i+1]
        int i;
        for (i = num.size()-2; i >= 0 && num[i] >= num[i+1]; i--);

        if (num[i] >= num[i+1]) {
            reverse(num, 0);
            return;
        }

        // find max j where num[i] < num[j]
        int j;
        for (j = num.size()-1; num[i] >= num[j]; j--);

        swap(num[i], num[j]);

        reverse(num, i+1);
    }
{% endcodeblock %}
