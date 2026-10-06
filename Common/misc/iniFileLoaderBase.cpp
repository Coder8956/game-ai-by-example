//==============================================================================================
//【文件说明】iniFileLoaderBase.cpp —— 参数读取基类的实现
//  RemoveCommentingFromLine:把一行中 // 之后的注释切掉;
//  GetNextParameter:读下一行非空、非注释的内容,取出等号右边的值;
//  GetParameterValueAsString:在一行里找"参数名=值",留下值;
//  GetNextToken:从当前行按分隔符切出下一个 token,切完就置空。
//==============================================================================================
#include "misc/iniFileLoaderBase.h"
using std::string;


//removes any commenting from a line of text
  // 工具函数:找 // 出现的位置,截掉后面的注释。
void RemoveCommentingFromLine(string& line)
{
   //search for any comment and remove
   string::size_type idx = line.find('//');

   if (idx != string::npos)
   {
     //cut out the comment
     line = line.substr(0, idx);
   }
}
//----------------------- GetNextParameter ------------------------------------
//
//  searches the text file for the next valid parameter. Discards any comments
//  and returns the value as a string
//-----------------------------------------------------------------------------
  // 循环 getline,跳过空行/注释行,再把值部分截出来。
string iniFileLoaderBase::GetNextParameter()
{
 
  //this will be the string that holds the bext parameter
  std::string line;
  
  std::getline(file, line);
   
  RemoveCommentingFromLine(line);

  //if the line is of zero length, get the next line from
  //the file
  if (line.length() == 0)
  {
    return GetNextParameter();
  }

  GetParameterValueAsString(line);  
    
  return line;
}


//-------------------------- GetParameterValueAsString ------------------------
//
// given a line of text this function removes the parameter description
// and returns just the parameter as a std::string
//-----------------------------------------------------------------------------
  // 用 find_first_not_of/find_first_of 在 " ;=," 分隔符间,先找到参数名,再找到值。
void iniFileLoaderBase::GetParameterValueAsString(string& line)
{
  //find beginning of parameter description
  string::size_type begIdx;
  string::size_type endIdx;

  //define some delimiters
  const string delims(" \;=,");

  begIdx = line.find_first_not_of(delims);

  //find the end of the parameter description
  if (begIdx != string::npos)
  {
    endIdx = line.find_first_of(delims, begIdx);

    //end of word is the end of the line
    if (endIdx == string::npos)
    {
      endIdx = line.length();
    }
  }   

  //find the beginning of the parameter value
  begIdx = line.find_first_not_of(delims, endIdx);
  //find the end of the parameter value
  if(begIdx != string::npos)
  {
    endIdx = line.find_first_of(delims, begIdx);

    //end of word is the end of the line
    if (endIdx == string::npos)
    {
      endIdx = line.length();
    }
  }
    
  line = line.substr(begIdx, endIdx);
}

//--------------------------- GetNextToken ------------------------------------
//
//  ignores any commenting and gets the next string
//-----------------------------------------------------------------------------
  // 取出当前行第一个 token 并把它从 CurrentLine 剩下的部分切掉,下次继续。
std::string iniFileLoaderBase::GetNextToken()
{ 
  //strip the line of any commenting
  while (CurrentLine.length() == 0)
  {
    std::getline(file, CurrentLine);
   
    RemoveCommentingFromLine(CurrentLine);
  }

   //find beginning of parameter description
  string::size_type begIdx; 
  string::size_type endIdx;

  //define some delimiters
  const string delims(" \;=,");

  begIdx = CurrentLine.find_first_not_of(delims);

  //find the end of the parameter description
  if (begIdx != string::npos)
  {
    endIdx = CurrentLine.find_first_of(delims, begIdx);

    //end of word is the end of the line
    if (endIdx == string::npos)
    {
      endIdx = CurrentLine.length();
    }
  }
    
  string s = CurrentLine.substr(begIdx, endIdx);

  if (endIdx != CurrentLine.length())
  {
    //strip the token from the line
    CurrentLine = CurrentLine.substr(endIdx+1, CurrentLine.length());
  }

  else { CurrentLine = "";}

  return s;
  
}

