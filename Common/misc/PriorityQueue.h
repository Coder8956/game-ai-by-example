//==============================================================================================
//【文件说明】PriorityQueue.h —— 优先队列(堆实现,Dijkstra/A* 用)
//
//【这个文件是干什么的?】
//  3 个基于二叉堆的优先队列:
//    PriorityQ         —— 最大值优先(每次 pop 出最大);
//    PriorityQLow      —— 最小值优先(Dijkstra/A* 用,代价小的先出);
//    IndexedPriorityQLow —— 索引优先队列:堆里存"节点编号",实际键值在外部数组里,
//                          支持 ChangePriority(某节点代价变了就调整它在堆中的位置)。
//
//【谁在使用这个文件?】
//  GraphAlgorithms.h(Dijkstra/A* 内部用 IndexedPriorityQLow 排序节点)。
//==============================================================================================
#ifndef PRIORITYQUEUE_H
#define PRIORITYQUEUE_H
//--------------------------------------------------------------------------------
// 包含保护原理详见 Buckland_Chapter4-SimpleSoccer/Goal.h。
//--------------------------------------------------------------------------------


#include <ostream>
#include <vector>
#include <cassert>

//----------------------- Swap -------------------------------------------
//
//  used to swap two values
//------------------------------------------------------------------------
template<class T>
//--------------------------------------------------------------------------------
// Swap:交换两个值(堆操作反复用)。
// ReorderUpwards/ReorderDownwards:上浮/下沉——堆排序的核心调整函数。
//--------------------------------------------------------------------------------
void Swap(T &a, T &b)
{
  T temp = a;
  a = b;
  b = temp;
}

//-------------------- ReorderUpwards ------------------------------------
//
//  given a heap and a node in the heap, this function moves upwards
//  through the heap swapping elements until the heap is ordered
//------------------------------------------------------------------------
template<class T>
void ReorderUpwards(std::vector<T>& heap, int nd)
{
  //move up the heap swapping the elements until the heap is ordered
  while ( (nd>1) && (heap[nd/2] < heap[nd]))
  {
    Swap(heap[nd/2], heap[nd]);

    nd /= 2;
  }
}

//--------------------- ReorderDownwards ---------------------------------
//
//  given a heap, the heapsize and a node in the heap, this function
//  reorders the elements in a top down fashion by moving down the heap
//  and swapping the current node with the greater of its two children
//  (provided a child is larger than the current node)
//------------------------------------------------------------------------
template<class T>
void ReorderDownwards(std::vector<T>& heap, int nd, int HeapSize)
{
  //move down the heap from node nd swapping the elements until
  //the heap is reordered
  while (2*nd <= HeapSize)
  {
    int child = 2 * nd;

    //set child to largest of nd's two children
    if ( (child < HeapSize) && (heap[child] < heap[child+1]) )
    {
      ++child;
    }

    //if this nd is smaller than its child, swap
    if (heap[nd] < heap[child])
    {
      Swap(heap[child], heap[nd]);

      //move the current node down the tree
      nd = child;
    }

    else
    {
      break;
    }
  }
}



  // PriorityQ:最大值堆(本工程几乎不用,A* 用的是 Low 版)。
//--------------------- PriorityQ ----------------------------------------
//
//  basic heap based priority queue implementation
//------------------------------------------------------------------------
template<class T>
class PriorityQ
{
private:

  std::vector<T>  m_Heap;

  int             m_iSize;

  int             m_iMaxSize;

  //given a heap and a node in the heap, this function moves upwards
  //through the heap swapping elements until the heap is ordered
  void ReorderUpwards(std::vector<T>& heap, int nd)
  {
    //move up the heap swapping the elements until the heap is ordered
    while ( (nd>1) && (heap[nd/2] < heap[nd]))
    {
      Swap(heap[nd/2], heap[nd]);

      nd /= 2;
    }
  }

  //given a heap, the heapsize and a node in the heap, this function
  //reorders the elements in a top down fashion by moving down the heap
  //and swapping the current node with the greater of its two children
  //(provided a child is larger than the current node)
  void ReorderDownwards(std::vector<T>& heap, int nd, int HeapSize)
  {
    //move down the heap from node nd swapping the elements until
    //the heap is reordered
    while (2*nd <= HeapSize)
    {
     int child = 2 * nd;

      //set child to largest of nd's two children
      if ( (child < HeapSize) && (heap[child] < heap[child+1]) )
     {
        ++child;
      }

      //if this nd is smaller than its child, swap
      if (heap[nd] < heap[child])
      {
        Swap(heap[child], heap[nd]);

        //move the current node down the tree
        nd = child;
      }
  
      else
      {
        break;
      }
    }
  }

public:

  PriorityQ(int MaxSize):m_iMaxSize(MaxSize), m_iSize(0)
  {
    m_Heap.assign(MaxSize+1, T());
  }

  bool empty()const{return (m_iSize==0);}

  //to insert an item into the queue it gets added to the end of the heap
  //and then the heap is reordered
  void insert(const T item)
  {

    assert (m_iSize+1 <= m_iMaxSize);

    ++m_iSize;

    m_Heap[m_iSize] = item;

    ReorderUpwards(m_Heap, m_iSize);
  }

  //to get the max item the first element is exchanged with the lowest
  //in the heap and then the heap is reordered from the top down. 
  T pop()
  {
    Swap(m_Heap[1], m_Heap[m_iSize]);

    ReorderDownwards(m_Heap, 1, m_iSize-1);

    return m_Heap[m_iSize--];
  }

  //so we can take a peek at the first in line
  const T& Peek()const{return m_Heap[1];}
};

  // PriorityQLow:最小值堆,把上面的比较方向反过来即可。
//--------------------- PriorityQLow -------------------------------------
//
//  basic 2-way heap based priority queue implementation. This time the priority
//  is given to the lowest valued key
//------------------------------------------------------------------------
template<class T>
class PriorityQLow
{
private:

  std::vector<T>  m_Heap;

  int             m_iSize;

  int             m_iMaxSize;

  //given a heap and a node in the heap, this function moves upwards
  //through the heap swapping elements until the heap is ordered
  void ReorderUpwards(std::vector<T>& heap, int nd)
  {
    //move up the heap swapping the elements until the heap is ordered
    while ( (nd>1) && (heap[nd/2] > heap[nd]))
    {
      Swap(heap[nd/2], heap[nd]);

      nd /= 2;
    }
  }

  //given a heap, the heapsize and a node in the heap, this function
  //reorders the elements in a top down fashion by moving down the heap
  //and swapping the current node with the smaller of its two children
  //(provided a child is larger than the current node)
  void ReorderDownwards(std::vector<T>& heap, int nd, int HeapSize)
  {
    //move down the heap from node nd swapping the elements until
    //the heap is reordered
    while (2*nd <= HeapSize)
    {
     int child = 2 * nd;

      //set child to largest of nd's two children
      if ( (child < HeapSize) && (heap[child] > heap[child+1]) )
     {
        ++child;
      }

      //if this nd is smaller than its child, swap
      if (heap[nd] > heap[child])
      {
        Swap(heap[child], heap[nd]);

        //move the current node down the tree
        nd = child;
      }
  
      else
      {
        break;
      }
    }
  }

public:

  PriorityQLow(int MaxSize):m_iMaxSize(MaxSize), m_iSize(0)
  {
    m_Heap.assign(MaxSize+1, T());
  }

  bool empty()const{return (m_iSize==0);}

  //to insert an item into the queue it gets added to the end of the heap
  //and then the heap is reordered
  void insert(const T item)
  {
    assert (m_iSize+1 <= m_iMaxSize);

    ++m_iSize;

    m_Heap[m_iSize] = item;

    ReorderUpwards(m_Heap, m_iSize);
  }

  //to get the max item the first element is exchanged with the lowest
  //in the heap and then the heap is reordered from the top down. 
  T pop()
  {
    Swap(m_Heap[1], m_Heap[m_iSize]);

    ReorderDownwards(m_Heap, 1, m_iSize-1);

    return m_Heap[m_iSize--];
  }

  //so we can take a peek at the first in line
  const T& peek()const{return m_Heap[1];}
};

//--------------------------------------------------------------------------------
// IndexedPriorityQLow:索引优先队列。堆 m_Heap 存的是节点编号,
// 实际键值(代价)在外部数组 m_vecKeys 里;m_invHeap 是反向映射(节点→堆位置),
// 这样 ChangePriority(节点) 能直接定位并上浮调整。
//--------------------------------------------------------------------------------
//----------------------- IndexedPriorityQLow ---------------------------
//
//  Priority queue based on an index into a set of keys. The queue is
//  maintained as a 2-way heap.
//
//  The priority in this implementation is the lowest valued key
//------------------------------------------------------------------------
template<class KeyType>
class IndexedPriorityQLow
{
private:

  std::vector<KeyType>&  m_vecKeys;

  std::vector<int>       m_Heap;
 
  std::vector<int>       m_invHeap;

  int                    m_iSize,
                         m_iMaxSize;

  void Swap(int a, int b)
  {
    int temp = m_Heap[a]; m_Heap[a] = m_Heap[b]; m_Heap[b] = temp;

    //change the handles too
    m_invHeap[m_Heap[a]] = a; m_invHeap[m_Heap[b]] = b;
  }

  void ReorderUpwards(int nd)
  {
    //move up the heap swapping the elements until the heap is ordered
    while ( (nd>1) && (m_vecKeys[m_Heap[nd/2]] > m_vecKeys[m_Heap[nd]]) )
    {      
      Swap(nd/2, nd);

      nd /= 2;
    }
  }

  void ReorderDownwards(int nd, int HeapSize)
  {
    //move down the heap from node nd swapping the elements until
    //the heap is reordered
    while (2*nd <= HeapSize)
    {
      int child = 2 * nd;

      //set child to smaller of nd's two children
      if ((child < HeapSize) && (m_vecKeys[m_Heap[child]] > m_vecKeys[m_Heap[child+1]]))
      {
        ++child;
      }

      //if this nd is larger than its child, swap
      if (m_vecKeys[m_Heap[nd]] > m_vecKeys[m_Heap[child]])
      {
        Swap(child, nd);

        //move the current node down the tree
        nd = child;
      }

      else
      {
        break;
      }
    }
  }


public:
  
  //you must pass the constructor a reference to the std::vector the PQ
  //will be indexing into and the maximum size of the queue.
  // 构造:传入外部键值数组的引用;insert 把节点编号入堆;Pop 出代价最小的节点;
  IndexedPriorityQLow(std::vector<KeyType>& keys,
                      int              MaxSize):m_vecKeys(keys),
                                                m_iMaxSize(MaxSize),
                                                m_iSize(0)
  {
    m_Heap.assign(MaxSize+1, 0);
    m_invHeap.assign(MaxSize+1, 0);
  }

  bool empty()const{return (m_iSize==0);}

  //to insert an item into the queue it gets added to the end of the heap
  //and then the heap is reordered from the bottom up.
  void insert(const int idx)
  {
    assert (m_iSize+1 <= m_iMaxSize);

    ++m_iSize;

    m_Heap[m_iSize] = idx;

    m_invHeap[idx] = m_iSize;

    ReorderUpwards(m_iSize);
  }

  //to get the min item the first element is exchanged with the lowest
  //in the heap and then the heap is reordered from the top down. 
  int Pop()
  {
    Swap(1, m_iSize);

    ReorderDownwards(1, m_iSize-1);

    return m_Heap[m_iSize--];
  }

  //if the value of one of the client key's changes then call this with 
  //the key's index to adjust the queue accordingly
  // ChangePriority:某节点代价变小后,在堆中上浮它的位置。
  void ChangePriority(const int idx)
  {
    ReorderUpwards(m_invHeap[idx]);
  }
};


#endif