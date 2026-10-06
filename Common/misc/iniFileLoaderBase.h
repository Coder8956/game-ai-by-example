//==============================================================================================
//【文件说明】iniFileLoaderBase.h —— 简单的参数文件读取基类
//
//【这个文件是干什么的?】
//  游戏常把可调参数(如避障权重、最大速度)放在 .ini 文本文件里。继承这个基类,
//  构造时传入文件名,然后按调用顺序依次 GetNextParameterInt/Double/... 读出参数。
//  会自动跳过 // 注释和空行,按空格/分号/等号/逗号分隔。
//
//【谁在使用这个文件?】
//  Raven 各工程的参数文件读取类(Raven_Game 等)继承它。
//==============================================================================================
#ifndef INIFILELOADERBASE
#define INIFILELOADERBASE
//--------------------------------------------------------------------------------
// #pragma warning(disable:4800) 原理详见 SoccerPitch.h;包含保护原理详见 Goal.h。
//--------------------------------------------------------------------------------
#pragma warning(disable:4800)
//------------------------------------------------------------------------
//
//  Name: IniFileLoaderBase.h
//  
//  Desc: Inherit from this to create a class capable of reading
//        parameters from an ascii file
//
//        instantiate this class with the name of the parameter file. Then
//        call the helper functions to retrieve the data. 
//
//  Author: Mat Buckland (fup@ai-junkie.com)
//
//------------------------------------------------------------------------

#include <fstream>
#include <string>
#include <cassert>


//--------------------------------------------------------------------------------
// class iniFileLoaderBase —— 参数读取基类。
//   file:输入文件流;CurrentLine:当前尚未取完的行;m_bGoodFile:文件能否打开。
//--------------------------------------------------------------------------------
class iniFileLoaderBase
{

private:

  //the file the parameters are stored in
  std::ifstream file;

  std::string   CurrentLine;

  void        GetParameterValueAsString(std::string& line);

  std::string GetNextParameter();

  //this ignores any comments and finds the next delimited string 
  std::string GetNextToken();

  //this is set to true if the file specified by the user is valid
  bool        m_bGoodFile;

public:

  //helper methods. They convert the next parameter value found into the 
  //relevant type
  // GetNextParameterXxx:每次读一行,取出等号右边的值转成对应类型;读错抛 runtime_error。
  double      GetNextParameterDouble(){if (m_bGoodFile) return atof(GetNextParameter().c_str());throw std::runtime_error("bad file");}
  float       GetNextParameterFloat(){if (m_bGoodFile) return (float)atof(GetNextParameter().c_str());throw std::runtime_error("bad file");}
  int         GetNextParameterInt(){if (m_bGoodFile) return atoi(GetNextParameter().c_str());throw std::runtime_error("bad file");}
  bool        GetNextParameterBool(){return (bool)(atoi(GetNextParameter().c_str()));throw std::runtime_error("bad file");}

  // GetNextTokenAsXxx:把一行按分隔符逐个 token 取出来(适合一行多值)。
  double      GetNextTokenAsDouble(){if (m_bGoodFile) return atof(GetNextToken().c_str()); throw std::runtime_error("bad file");}
  float       GetNextTokenAsFloat(){if (m_bGoodFile) return (float)atof(GetNextToken().c_str()); throw std::runtime_error("bad file");}
  int         GetNextTokenAsInt(){if (m_bGoodFile) return atoi(GetNextToken().c_str()); throw std::runtime_error("bad file");}
  std::string GetNextTokenAsString(){if (m_bGoodFile) return GetNextToken(); throw std::runtime_error("bad file");}

  bool        eof()const{if (m_bGoodFile) return file.eof(); throw std::runtime_error("bad file");}
  bool        FileIsGood()const{return m_bGoodFile;}

  // 构造:打开文件;失败把 m_bGoodFile 置 false,后续读取抛异常。
  iniFileLoaderBase(char* filename):CurrentLine(""), m_bGoodFile(true)
  {
    file.open(filename);

    if (!file){m_bGoodFile = false;}
  }

};




  

#endif



