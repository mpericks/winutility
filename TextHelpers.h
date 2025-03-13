#pragma once

#include <algorithm> 
#include <functional> 
#include <cctype>
#include <vector>

std::string utf8_encode(const std::wstring &wstr) ;
std::wstring utf8_decode(const std::string &str) ;
std::vector< std::string > PutFileIntoVectorOfStrings( std::wstring path ) ;

template< class stringtype >
std::vector< stringtype >  TokenizeToComponents( stringtype string_init, stringtype delimiters )
{
  std::vector< stringtype > components ;
  // Skip delimiters at beginning.
  size_t lastPos = string_init.find_first_not_of(delimiters, 0);
  // Find first "non-delimiter".
  size_t pos     = string_init.find_first_of(delimiters, lastPos);

  while (stringtype::npos != pos || stringtype::npos != lastPos)
  {
    // Found a token, add it to the vector.
    components.push_back(string_init.substr(lastPos, pos - lastPos));
    // Skip delimiters.  Note the "not_of"
    lastPos = string_init.find_first_not_of(delimiters, pos);
    // Find next "non-delimiter"
    pos = string_init.find_first_of(delimiters, lastPos);
  }
  return components ;
};

template< class stringtype >
stringtype LeftTrimString( const stringtype &s ) 
{
  stringtype temp = s ;
  temp.erase(temp.begin(), std::find_if(temp.begin(), temp.end(), [](int c) {return !std::isspace(c);} ) ) ;
  return temp;
};

template< class stringtype >
stringtype RightTrimString( const stringtype &s ) 
{
  stringtype temp = s ;
  temp.erase(std::find_if(temp.rbegin(), temp.rend(), [](int c) {return !std::isspace(c);}, temp.end() ));
  return temp;
};

template< class stringtype >
stringtype TrimString( const stringtype &s ) 
{
  return LeftTrimString(RightTrimString(s));
};

template< class stringtype> 
stringtype ReplaceString( const stringtype& value, const stringtype& search, const stringtype& replace)
{
  stringtype temp = value ;  
  size_t  next;

    for(next = temp.find(search);        // Try and find the first match
        next != stringtype::npos;        // next is npos if nothing was found
        next = temp.find(search,next)    // search for the next match starting after
                                          // the last match that was found.
       )
    {
        // Inside the loop. So we found a match.
        temp.replace(next, search.length(), replace);   // Do the replacement.
        next += replace.length();                      // Move to just after the replace
                                                       // This is the point were we start
                                                       // the next search from. 
    }
    return temp ;
}

template< class stringtype>
stringtype ReplaceMultipleStrings( const stringtype& value, std::vector< std::pair< stringtype, stringtype > > args) 
{
  stringtype temp = value ;
  typename std::vector< std::pair< stringtype, stringtype > >::iterator iter = args.begin() ;
  while ( iter != args.end() )
  {
    temp = ReplaceString( temp, iter->first, iter->second ) ;
    iter++ ;
  }
  return temp ;
}