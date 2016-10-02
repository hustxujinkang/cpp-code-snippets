#include<iostream>
using namespace std;

using Align = double;

union Header
{
    struct
    {
        Header* next;
        size_t size;
    }s;
    Align x;
};

Header base;

void* malloc(size_t nbytes)
{
    int block_siz = sizeof(Header);
    int nunits = (nbytes + block_siz - 1) / block_siz + 1;
    //now we can check the list to find whether there's enough memory already
    Header* p = &base;
    while(p->next != NULL && p->next->size < nunits)
    {
         p = p->next;
    }
    if(p->next == NULL|| p->next->size < nunits)
    {
         char* res = new char[nunits * sizeof(Header)];
         return (void*) res;
    }
    else
    {
        if(p->next->size == nunits)
        {
            Header* res = p->next + 1;
            p->next = p->next->next;
            return (void*)res;
        }
        else
        {
            Header* node = p->next;
            Header* res = node + 1;
            node->size -= nunits;
            node += nunits;
            return (void*)res;
        }
    }
    return NULL;
}

void free(void* p)
{
    Header* node = p;
    node -= 1;
    Header* cur = &base;
    while(cur->next != NULL && cur->next < node)
    {
         cur = cur->next;
    }
    if(cur->next != NULL && node + node->size == cur->next)
    {
         node->size += cur->next->size;
         cur->next = node;
    }
    if(cur != &base && cur->size + cur == node)
    {
         cur->size += node->size;
    }
    else
    {
        node->next = cur->next;
        cur->next = node;
    }
    return;
}
int main()
{
    return 0;
}
