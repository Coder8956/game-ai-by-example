//==============================================================================================
//【文件说明】Transformations.h —— "局部坐标 ↔ 世界坐标"互相转换
//
//【这个文件是干什么的?】
//  画一辆小车时,通常先在"小车自己的坐标系"里画好形状(车头朝右、原点在中心),
//  再按它在世界里的位置和朝向整体搬过去。本文件提供这一组转换函数:
//  点/向量 在"局部空间(以角色为中心)"和"世界空间(屏幕坐标)"之间来回换算,
//  并顺手提供"绕原点旋转向量"和"生成一圈探路触须(whisker)"两个工具。
//
//【谁在使用这个文件?】
//  Vehicle.cpp、MovingEntity.h、SteeringBehaviors —— 第 3 章车辆的绘制与操控;
//  第 4 章球员也类似使用。
//
//【本文件包含了谁?】
//  <vector>        —— std::vector 动态数组;
//  "Vector2D.h"   —— 二维向量;
//  "C2DMatrix.h"   —— 3×3 变换矩阵(所有转换都靠它做乘法)。
//==============================================================================================
#ifndef TRANSFORMATIONS_H
#define TRANSFORMATIONS_H
//------------------------------------------------------------------------
//
//  Name:   Transformations.h
//
//  Desc:   Functions for converting 2D vectors between World and Local
//          space.
//
//  Author: Mat Buckland (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
// ↓↓↓ 原文翻译【Transformations —— 坐标变换】:
//   在"世界空间"和"局部空间"之间转换二维向量的一组函数。作者:Mat Buckland。
//------------------------------------------------------------------------
// 注意:第 17 行 #include "Transformations.h" 是原作者对自己文件的重复包含(无害,
// 有包含保护兜底),原样保留。
#include <vector>

#include "Vector2D.h"
#include "C2DMatrix.h"
#include "Transformations.h"






//--------------------------- WorldTransform -----------------------------
//
//  given a std::vector of 2D vectors, a position, orientation and scale,
//  this function transforms the 2D vectors into the object's world space
//------------------------------------------------------------------------
inline std::vector<Vector2D> WorldTransform(std::vector<Vector2D> &points,
                                            const Vector2D   &pos,
                                            const Vector2D   &forward,
                                            const Vector2D   &side,
                                            const Vector2D   &scale)
{
	//copy the original vertices into the buffer about to be transformed
  std::vector<Vector2D> TranVector2Ds = points;
  
  //create a transformation matrix
	C2DMatrix matTransform;
	
	//scale
  if ( (scale.x != 1.0) || (scale.y != 1.0) )
  {
	  matTransform.Scale(scale.x, scale.y);
  }

	//rotate
	matTransform.Rotate(forward, side);

	//and translate
	matTransform.Translate(pos.x, pos.y);
	
  //now transform the object's vertices
  matTransform.TransformVector2Ds(TranVector2Ds);

  return TranVector2Ds;
}

//--------------------------- WorldTransform -----------------------------
//
// ↓↓↓ 原文翻译【WorldTransform(无缩放重载)】:上面那个的简化版——不缩放,只按位置和
//   朝向把整组点摆到世界中。
//  given a std::vector of 2D vectors, a position and  orientation
//  this function transforms the 2D vectors into the object's world space
//------------------------------------------------------------------------
inline std::vector<Vector2D> WorldTransform(std::vector<Vector2D> &points,
                                 const Vector2D   &pos,
                                 const Vector2D   &forward,
                                 const Vector2D   &side)
{
	//copy the original vertices into the buffer about to be transformed
  std::vector<Vector2D> TranVector2Ds = points;
  
  //create a transformation matrix
	C2DMatrix matTransform;

	//rotate
	matTransform.Rotate(forward, side);

	//and translate
	matTransform.Translate(pos.x, pos.y);
	
  //now transform the object's vertices
  matTransform.TransformVector2Ds(TranVector2Ds);

  return TranVector2Ds;
}

//--------------------- PointToWorldSpace --------------------------------
//
//  Transforms a point from the agent's local space into world space
// ↓↓↓ 原文翻译【PointToWorldSpace】:把一个点从角色的局部空间变换到世界空间。
//------------------------------------------------------------------------
inline Vector2D PointToWorldSpace(const Vector2D &point,
                                    const Vector2D &AgentHeading,
                                    const Vector2D &AgentSide,
                                    const Vector2D &AgentPosition)
{
	//make a copy of the point
  Vector2D TransPoint = point;
  
  //create a transformation matrix
	C2DMatrix matTransform;

	//rotate
	matTransform.Rotate(AgentHeading, AgentSide);

	//and translate
	matTransform.Translate(AgentPosition.x, AgentPosition.y);
	
  //now transform the vertices
  matTransform.TransformVector2Ds(TransPoint);

  return TransPoint;
}

//--------------------- VectorToWorldSpace --------------------------------
//
//  Transforms a vector from the agent's local space into world space
// ↓↓↓ 原文翻译【VectorToWorldSpace】:把一个向量(方向)从局部变到世界(只旋转不平移,
//   因为向量没有"位置")。
//------------------------------------------------------------------------
inline Vector2D VectorToWorldSpace(const Vector2D &vec,
                                     const Vector2D &AgentHeading,
                                     const Vector2D &AgentSide)
{
	//make a copy of the point
  Vector2D TransVec = vec;
  
  //create a transformation matrix
	C2DMatrix matTransform;

	//rotate
	matTransform.Rotate(AgentHeading, AgentSide);

  //now transform the vertices
  matTransform.TransformVector2Ds(TransVec);

  return TransVec;
}


//--------------------- PointToLocalSpace --------------------------------
//
//------------------------------------------------------------------------
// ↓↓↓ 原文翻译【PointToLocalSpace】:反向——把世界里的点变回"以角色为中心"的局部坐标。
//   做法:先平移到角色位置的反方向(Tx、Ty 用点积算出),再旋转回来。
inline Vector2D PointToLocalSpace(const Vector2D &point,
                             Vector2D &AgentHeading,
                             Vector2D &AgentSide,
                              Vector2D &AgentPosition)
{

	//make a copy of the point
  Vector2D TransPoint = point;
  
  //create a transformation matrix
	C2DMatrix matTransform;

  double Tx = -AgentPosition.Dot(AgentHeading);
  double Ty = -AgentPosition.Dot(AgentSide);

  //create the transformation matrix
  matTransform._11(AgentHeading.x); matTransform._12(AgentSide.x);
  matTransform._21(AgentHeading.y); matTransform._22(AgentSide.y);
  matTransform._31(Tx);           matTransform._32(Ty);
	
  //now transform the vertices
  matTransform.TransformVector2Ds(TransPoint);

  return TransPoint;
}

//--------------------- VectorToLocalSpace --------------------------------
//
//------------------------------------------------------------------------
// VectorToLocalSpace:反向——把世界向量变回局部向量(只旋转,不平移)。
inline Vector2D VectorToLocalSpace(const Vector2D &vec,
                             const Vector2D &AgentHeading,
                             const Vector2D &AgentSide)
{ 

	//make a copy of the point
  Vector2D TransPoint = vec;
  
  //create a transformation matrix
	C2DMatrix matTransform;

  //create the transformation matrix
  matTransform._11(AgentHeading.x); matTransform._12(AgentSide.x);
  matTransform._21(AgentHeading.y); matTransform._22(AgentSide.y);
	
  //now transform the vertices
  matTransform.TransformVector2Ds(TransPoint);

  return TransPoint;
}

//-------------------------- Vec2DRotateAroundOrigin --------------------------
//
//  rotates a vector ang rads around the origin
// ↓↓↓ 原文翻译【Vec2DRotateAroundOrigin】:把向量 v 绕原点旋转 ang 弧度。
//-----------------------------------------------------------------------------
inline void Vec2DRotateAroundOrigin(Vector2D& v, double ang)
{
  //create a transformation matrix
  C2DMatrix mat;

  //rotate
  mat.Rotate(ang);
	
  //now transform the object's vertices
  mat.TransformVector2Ds(v);
}

//------------------------ CreateWhiskers ------------------------------------
//
//  given an origin, a facing direction, a 'field of view' describing the 
//  limit of the outer whiskers, a whisker length and the number of whiskers
//  this method returns a vector containing the end positions of a series
//  of whiskers radiating away from the origin and with equal distance between
//  them. (like the spokes of a wheel clipped to a specific segment size)
//----------------------------------------------------------------------------
// ↓↓↓ 原文翻译【CreateWhiskers —— 生成探路触须】:
//   给定原点、朝向、视野张角 fov、触须长度和触须条数,返回一组从原点向外扇形
//   均匀辐射的触须末端点(像车轮辐条截在一段扇形里)。车辆避障时就用这些触须
//   去探测前方有没有墙。
inline std::vector<Vector2D> CreateWhiskers(unsigned int  NumWhiskers,
                                            double        WhiskerLength,
                                            double        fov,
                                            Vector2D      facing,
                                            Vector2D      origin)
{
  //this is the magnitude of the angle separating each whisker
  double SectorSize = fov/(double)(NumWhiskers-1);

  std::vector<Vector2D> whiskers;
  Vector2D temp;
  double angle = -fov*0.5; 

  for (unsigned int w=0; w<NumWhiskers; ++w)
  {
    //create the whisker extending outwards at this angle
    temp = facing;
    Vec2DRotateAroundOrigin(temp, angle);
    whiskers.push_back(origin + WhiskerLength * temp);

    angle+=SectorSize;
  }

  return whiskers;
}


#endif