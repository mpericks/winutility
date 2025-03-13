#pragma once

class FileSystemPath
{
public:
  FileSystemPath(void);
  FileSystemPath( std::vector<std::wstring> components_init, bool is_unc ) ;
  FileSystemPath( std::wstring string_init ) ;
  FileSystemPath( const FileSystemPath& init ) ; 
  ~FileSystemPath(void);

public:
  std::wstring string() const ;
  std::string utf8_string() const ;
  std::wstring filename() const ;
  void AddComponent( std::wstring new_component ) ;
  void append( std::wstring new_component ) ;
  std::wstring::size_type length() ;
  void RemoveLastComponent() ;
  void RemoveComponents( unsigned int how_deep ) ;
  std::wstring LastComponent() ;
  FileSystemPath operator = ( const FileSystemPath& new_path ) ;
  FileSystemPath operator = ( const std::wstring& new_path ) ;

private:
  void TokenizeToComponents( std::wstring string_init ) ;

private:
  std::vector<std::wstring> components ;
  bool unc ;
};
