#include <windows.h>
#include <Shlwapi.h>
#include <crtdbg.h>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <iterator>
#include "FileSystemPath.h"
#include "TextHelpers.h"

// non-class functions for path utilities
std::wstring add_delimiter_to_path_component( std::wstring component )
{
  std::wstring backslashed( component ) ;
  if ( component.at( component.length() - 1 ) != '\\' )
	{
		backslashed.append(L"\\");
	}
	return backslashed;
}

FileSystemPath::FileSystemPath(void)
{
}
FileSystemPath::FileSystemPath( std::vector<std::wstring> components_init, bool is_unc ) : components(components_init), unc( is_unc ) 
{

}
FileSystemPath::FileSystemPath( const FileSystemPath& init ) : components(init.components), unc( init.unc )
{

}
FileSystemPath::FileSystemPath(std::wstring string_init ) : components(), unc( false ) 
{
  TokenizeToComponents( string_init ) ;
}
void FileSystemPath::TokenizeToComponents( std::wstring string_init )
{
  std::wstring delimiters(L"\\/") ;
  // Skip delimiters at beginning.
  std::wstring::size_type lastPos = string_init.find_first_not_of(delimiters, 0);
  if ( ( lastPos == 2 ) && ( components.size() == 0 ) )
  {
    unc = true ;
  }
  // Find first "non-delimiter".
  std::wstring::size_type pos     = string_init.find_first_of(delimiters, lastPos);

  while (std::wstring::npos != pos || std::wstring::npos != lastPos)
  {
    // Found a token, add it to the vector.
    components.push_back(string_init.substr(lastPos, pos - lastPos));
    // Skip delimiters.  Note the "not_of"
    lastPos = string_init.find_first_not_of(delimiters, pos);
    // Find next "non-delimiter"
    pos = string_init.find_first_of(delimiters, lastPos);
  }  
}
FileSystemPath FileSystemPath::operator = ( const FileSystemPath& new_path )
{
  unc = new_path.unc ; 
  components.erase( components.begin(), components.end() ) ;
  components = new_path.components ;
  return *this ;
}
FileSystemPath FileSystemPath::operator = ( const std::wstring& new_path ) 
{
  unc = false ;
  components.erase( components.begin(), components.end() ) ;
  TokenizeToComponents( new_path ) ;
  return *this ;
}
FileSystemPath::~FileSystemPath(void)
{
}
std::wstring FileSystemPath::string() const 
{
   std::vector<std::wstring> v;
   std::transform( components.begin(), components.end(), std::back_inserter(v), add_delimiter_to_path_component );
   std::wstring path = std::accumulate( v.begin(), v.end(), std::wstring() );
   //backslash is added to last component. remove it.
   path = path.substr( 0, path.length() - 1 ) ;
   std::wstring unc_mark( L"\\\\" ) ; 
   if ( unc )
   {
      path = unc_mark + path ;
   }
   return path ;
}
void FileSystemPath::append( std::wstring new_component ) 
{ 
  AddComponent(new_component) ; 
}

std::wstring::size_type FileSystemPath::length()
{
  return this->string().length() ;
}
std::string FileSystemPath::utf8_string() const
{
  std::wstring wide_path = this->string() ;
  return utf8_encode( wide_path ) ;
}
std::wstring FileSystemPath::filename() const
{
  return std::wstring( ::PathFindFileNameW( this->string().c_str() ) ) ;
}
void FileSystemPath::AddComponent( std::wstring new_component )
{
  if ( new_component.length() > 0 )
  {
    TokenizeToComponents( new_component ) ;
  }
}
void FileSystemPath::RemoveLastComponent()
{
  if ( components.size() ) 
  {
    components.pop_back() ;
  }
}
void FileSystemPath::RemoveComponents( unsigned int how_deep )
{
  for ( unsigned int i = 0; i < how_deep && (i < (components.size() - 1)); i++ )
  {
    RemoveLastComponent() ;
  }
}
std::wstring FileSystemPath::LastComponent()
{
  if ( components.size() )
  {
    return components.at( components.size() - 1 ) ;
  }
  return std::wstring() ;
}