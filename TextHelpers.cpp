#include <windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <iterator>
#include "TextHelpers.h"

std::string utf8_encode(const std::wstring &wstr)
{
  int size_needed = ::WideCharToMultiByte( CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL ) ;
  if ( ! size_needed )
  {
    return std::string() ;
  }
  std::vector<std::string::value_type> char_vector( size_needed ) ;
  WideCharToMultiByte( CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &char_vector[0], size_needed, NULL, NULL ) ;
  std::string ret_val( char_vector.begin(), char_vector.end() );
  return ret_val;
}

// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string &str)
{
  int size_needed = ::MultiByteToWideChar( CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0 ) ;
  if ( ! size_needed )
  {
    return std::wstring() ;
  }
  std::vector<std::wstring::value_type> wchar_vector( size_needed ) ;
  MultiByteToWideChar( CP_UTF8, 0, str.c_str(), (int)str.size(), &wchar_vector[0], size_needed ) ;
  std::wstring ret_val( wchar_vector.begin(), wchar_vector.end() );
  return ret_val;
}

std::vector< std::string > PutFileIntoVectorOfStrings( std::wstring path ) 
{
  std::ifstream stream_file ;
  stream_file.open( path.c_str(), std::ios::in ) ;
  std::vector< std::string > file_contents_vector ;
  if ( stream_file.is_open() )
  {
    while ( stream_file.good() )
    {
      std::string line ;
      std::getline( stream_file, line ) ;
      if ( line.size() > 1 )
      {
        file_contents_vector.push_back( line ) ;
      }
    }
  }
  //std::istream_iterator< std::string > in_iter( stream_file );
  //std::istream_iterator<std::string> end_iter ;
  //std::vector< std::string > file_contents_vector( in_iter, end_iter ) ;
  return file_contents_vector ;
}
