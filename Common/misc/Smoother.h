//==============================================================================================
//【文件说明】Smoother.h —— 滑动平均模板(给数值/向量做平滑)
//
//【这个文件是干什么的?】
//  维护一个 SampleSize 大小的历史环形缓冲区:每来一个新值就覆盖最旧的那个,
//  然后返回所有历史值的平均。常用于平滑帧率、平滑转向向量,避免画面抖动。
//  要求元素类型支持 += 和 / 运算(Vector2D 就支持)。
//
//【谁在使用这个文件?】
//  Raven 的瞄准方向平滑、第 5 章演示程序的帧率统计。
//==============================================================================================
#ifndef SMOOTHER
#define SMOOTHER
//--------------------------------------------------------------------------------
// 包含保护原理详见 Buckland_Chapter4-SimpleSoccer/Goal.h。
//--------------------------------------------------------------------------------
//------------------------------------------------------------------------
//
//  Name: Smoother.h
//
//  Desc: Template class to help calculate the average value of a history
//        of values. This can only be used with types that have a 'zero'
//        value and that have the += and / operators overloaded.
//
//        Example: Used to smooth frame rate calculations.
//
//  Author: Mat Buckland (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
#include <vector>

//--------------------------------------------------------------------------------
// template <class T> —— 元素类型(可 double 也可 Vector2D)。
// class Smoother —— 滑动平均器。
//--------------------------------------------------------------------------------
template <class T>
class Smoother
{
private:

  // m_History:历史环形数组;m_iNextUpdateSlot:下一个要覆盖的槽位;m_ZeroValue:类型零值。
  //this holds the history
  std::vector<T>  m_History;

  int           m_iNextUpdateSlot;

  //an example of the 'zero' value of the type to be smoothed. This
  //would be something like Vector2D(0,0)
  T             m_ZeroValue;

public:

  //to instantiate a Smoother pass it the number of samples you want
  //to use in the smoothing, and an exampe of a 'zero' type
  // 构造:指定缓冲大小和零值(用零值填满缓冲区,避免开头不平均)。
  Smoother(int SampleSize, T ZeroValue):m_History(SampleSize, ZeroValue),
                                        m_ZeroValue(ZeroValue),
                                        m_iNextUpdateSlot(0)
  {}

  //each time you want to get a new average, feed it the most recent value
  //and this method will return an average over the last SampleSize updates
//--------------------------------------------------------------------------------
// Update:① 把新值写到当前槽,槽位 +1 并回卷(环形);
// ② 遍历所有历史值累加,再除以个数返回平均值。
//--------------------------------------------------------------------------------
  T Update(const T& MostRecentValue)
  {  
    //overwrite the oldest value with the newest
    m_History[m_iNextUpdateSlot++] = MostRecentValue;

    //make sure m_iNextUpdateSlot wraps around. 
    if (m_iNextUpdateSlot == m_History.size()) m_iNextUpdateSlot = 0;

    //now to calculate the average of the history list
    T sum = m_ZeroValue;

    std::vector<T>::iterator it = m_History.begin();

    for (it; it != m_History.end(); ++it)
    {
      sum += *it;
    }

    return sum / (double)m_History.size();
  }
};


#endif