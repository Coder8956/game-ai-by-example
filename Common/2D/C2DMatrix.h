//==============================================================================================
//【文件说明】C2DMatrix.h —— "3×3 变换矩阵":批量平移/缩放/旋转图形
//
//【这个文件是干什么的?】
//  计算机图形学里,"平移、缩放、旋转"三种几何变换都能写成一个 3×3 矩阵,再把矩阵
  //乘到每个点上。本类封装这种矩阵:先 Identity()(清零成单位阵),再依次 Translate /
//  Scale / Rotate 叠加变换,最后 TransformVector2Ds 把一整批点全部变换到位。
//  (点用齐次坐标(x,y,1),所以需要 3×3 才能把平移也写成乘法。)
//
//【谁在使用这个文件?】
//  2D/Transformations.h、2D/geometry.h —— 用它做屏幕绘制前的坐标变换;
//  Vehicle.h/.cpp、MovingEntity.h —— 第 3 章车辆,把"局部朝向"旋到世界坐标;
//  PlayerBase.cpp、FieldPlayer.cpp —— 第 4 章球员转身/绘制。
//
//【本文件包含了谁?】
//  <math.h>       —— sin/cos 三角函数(旋转用);
//  <vector>       —— std::vector 动态数组(一批点);
//  "misc/utils.h" —— 工具库;
//  "2d/Vector2D.h"—— 二维向量类。
//==============================================================================================
#ifndef C2DMATRIX_H
#define C2DMATRIX_H
//------------------------------------------------------------------------
//
//  Name:   C2DMatrix.h
//
//  Author: Mat Buckland 2002
//
//  Desc:   2D Matrix class 
//
//------------------------------------------------------------------------
// ↓↓↓ 原文翻译【C2DMatrix —— 2D 矩阵】:一个 2D 矩阵类。作者:Mat Buckland,2002 年。
//------------------------------------------------------------------------
#include <math.h>
#include <vector>

#include "misc/utils.h"
#include "2d/Vector2D.h"




//--------------------------------------------------------------------------------
// class C2DMatrix —— 2D 变换矩阵类。
class C2DMatrix
{
private:
  
  // private 里先嵌套定义一个小结构体 Matrix,装 3×3 共 9 个数字。
  // 命名 _11 _12 _13 = 第 1 行第 1/2/3 列;_21.._33 依此类推。构造函数把 9 个都清零。
  struct Matrix
  {

    double _11, _12, _13;
    double _21, _22, _23;
    double _31, _32, _33;

    Matrix()
    {
      _11=0.0; _12=0.0; _13=0.0;
      _21=0.0; _22=0.0; _23=0.0;
      _31=0.0; _32=0.0; _33=0.0;
    }

  };

  // m_Matrix:本类内部持有的那个 3×3 矩阵(实际存数据的成员)。
  Matrix m_Matrix;

  //multiplies m_Matrix with mIn
  //(原文注释:把 m_Matrix 与 mIn 相乘)—— 矩阵乘法,结果存回 m_Matrix。
  inline void  MatrixMultiply(Matrix &mIn);


public:

  C2DMatrix()
  {
    //initialize the matrix to an identity matrix
    //(原文注释:把矩阵初始化为单位矩阵)—— 单位阵 = 乘上去什么都不变(类似数字 1)。
    Identity();
  }

  //create an identity matrix
  inline void Identity();
  
  //create a transformation matrix
  inline void Translate(double x, double y);

  //create a scale matrix
  inline void Scale(double xScale, double yScale);

  //create a rotation matrix
  inline void  Rotate(double rotation);

  //create a rotation matrix from a fwd and side 2D vector
  //(原文注释:用一个前向 fwd、一个侧向 side 二维向量构造旋转矩阵)
  inline void  Rotate(const Vector2D &fwd, const Vector2D &side);

   //applys a transformation matrix to a std::vector of points
  //(原文注释:把变换矩阵作用到"一批点"上)—— 下面是两个同名重载:一个处理整个点数组,
  // 一个处理单个点。
  inline void TransformVector2Ds(std::vector<Vector2D> &vPoints);

  //applys a transformation matrix to a point
  inline void TransformVector2Ds(Vector2D &vPoint);

  //accessors to the matrix elements
  //(原文注释:矩阵元素的访问器)—— 下面一组用来直接设置矩阵第 i 行 j 列的数值。
  void _11(double val){m_Matrix._11 = val;}
  void _12(double val){m_Matrix._12 = val;}
  void _13(double val){m_Matrix._13 = val;}

  void _21(double val){m_Matrix._21 = val;}
  void _22(double val){m_Matrix._22 = val;}
  void _23(double val){m_Matrix._23 = val;}

  void _31(double val){m_Matrix._31 = val;}
  void _32(double val){m_Matrix._32 = val;}
  void _33(double val){m_Matrix._33 = val;}

};



//multiply two matrices together
// ↓↓↓ 原文注释翻译:两个矩阵相乘。下面按矩阵乘法逐格计算(新矩阵第 i 行第 j 列 =
//   左边第 i 行与右边第 j 列对应相乘再相加),最后结果存回 m_Matrix。
inline void C2DMatrix::MatrixMultiply(Matrix &mIn)
{
  C2DMatrix::Matrix mat_temp;
  
  //first row
  mat_temp._11 = (m_Matrix._11*mIn._11) + (m_Matrix._12*mIn._21) + (m_Matrix._13*mIn._31);
  mat_temp._12 = (m_Matrix._11*mIn._12) + (m_Matrix._12*mIn._22) + (m_Matrix._13*mIn._32);
  mat_temp._13 = (m_Matrix._11*mIn._13) + (m_Matrix._12*mIn._23) + (m_Matrix._13*mIn._33);

  //second
  mat_temp._21 = (m_Matrix._21*mIn._11) + (m_Matrix._22*mIn._21) + (m_Matrix._23*mIn._31);
  mat_temp._22 = (m_Matrix._21*mIn._12) + (m_Matrix._22*mIn._22) + (m_Matrix._23*mIn._32);
  mat_temp._23 = (m_Matrix._21*mIn._13) + (m_Matrix._22*mIn._23) + (m_Matrix._23*mIn._33);

  //third
  mat_temp._31 = (m_Matrix._31*mIn._11) + (m_Matrix._32*mIn._21) + (m_Matrix._33*mIn._31);
  mat_temp._32 = (m_Matrix._31*mIn._12) + (m_Matrix._32*mIn._22) + (m_Matrix._33*mIn._32);
  mat_temp._33 = (m_Matrix._31*mIn._13) + (m_Matrix._32*mIn._23) + (m_Matrix._33*mIn._33);

  m_Matrix = mat_temp;
}

//applies a 2D transformation matrix to a std::vector of Vector2Ds
// ↓↓↓ 原文注释翻译:把 2D 变换矩阵作用到一组 Vector2D 点上(逐个点变换)。
inline void C2DMatrix::TransformVector2Ds(std::vector<Vector2D> &vPoint)
{
  for (unsigned int i=0; i<vPoint.size(); ++i)
  {
    double tempX =(m_Matrix._11*vPoint[i].x) + (m_Matrix._21*vPoint[i].y) + (m_Matrix._31);

    double tempY = (m_Matrix._12*vPoint[i].x) + (m_Matrix._22*vPoint[i].y) + (m_Matrix._32);
  
    vPoint[i].x = tempX;

    vPoint[i].y = tempY;

  }
}

//applies a 2D transformation matrix to a single Vector2D
// ↓↓↓ 原文注释翻译:把矩阵作用到单个点上。公式:新 x = _11·x + _21·y + _31(平移项),
//   新 y = _12·x + _22·y + _32。
inline void C2DMatrix::TransformVector2Ds(Vector2D &vPoint)
{

  double tempX =(m_Matrix._11*vPoint.x) + (m_Matrix._21*vPoint.y) + (m_Matrix._31);

  double tempY = (m_Matrix._12*vPoint.x) + (m_Matrix._22*vPoint.y) + (m_Matrix._32);
  
  vPoint.x = tempX;

  vPoint.y = tempY;
}



//create an identity matrix
// Identity:把矩阵写成单位阵(对角线 1,其余 0),表示"不做任何变换"。
inline void C2DMatrix::Identity()
{
  m_Matrix._11 = 1; m_Matrix._12 = 0; m_Matrix._13 = 0;

  m_Matrix._21 = 0; m_Matrix._22 = 1; m_Matrix._23 = 0;

  m_Matrix._31 = 0; m_Matrix._32 = 0; m_Matrix._33 = 1;

  }

//create a transformation matrix
// Translate:搭一个平移矩阵(_31=x、_32=y 是平移量),再乘到当前矩阵上叠加。
inline void C2DMatrix::Translate(double x, double y)
{
  Matrix mat;
  
  mat._11 = 1; mat._12 = 0; mat._13 = 0;
  
  mat._21 = 0; mat._22 = 1; mat._23 = 0;
  
  mat._31 = x;    mat._32 = y;    mat._33 = 1;
  
  //and multiply
  MatrixMultiply(mat);
}

//create a scale matrix
// Scale:搭一个缩放矩阵(_11=xScale、_22=yScale),再叠加。
inline void C2DMatrix::Scale(double xScale, double yScale)
{
  C2DMatrix::Matrix mat;
  
  mat._11 = xScale; mat._12 = 0; mat._13 = 0;
  
  mat._21 = 0; mat._22 = yScale; mat._23 = 0;
  
  mat._31 = 0; mat._32 = 0; mat._33 = 1;
  
  //and multiply
  MatrixMultiply(mat);
}


//create a rotation matrix
// Rotate:搭一个旋转矩阵(用 sin/cos),再叠加。
inline void C2DMatrix::Rotate(double rot)
{
  C2DMatrix::Matrix mat;

  double Sin = sin(rot);
  double Cos = cos(rot);
  
  mat._11 = Cos;  mat._12 = Sin; mat._13 = 0;
  
  mat._21 = -Sin; mat._22 = Cos; mat._23 = 0;
  
  mat._31 = 0; mat._32 = 0;mat._33 = 1;
  
  //and multiply
  MatrixMultiply(mat);
}


//create a rotation matrix from a 2D vector
// Rotate(重载):用前向、侧向两个方向向量直接摆出旋转矩阵(用于"对齐朝向")。
inline void C2DMatrix::Rotate(const Vector2D &fwd, const Vector2D &side)
{
  C2DMatrix::Matrix mat;
  
  mat._11 = fwd.x;  mat._12 = fwd.y; mat._13 = 0;
  
  mat._21 = side.x; mat._22 = side.y; mat._23 = 0;
  
  mat._31 = 0; mat._32 = 0;mat._33 = 1;
  
  //and multiply
  MatrixMultiply(mat);
}





#endif
