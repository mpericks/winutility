#include <windows.h>
#include <shlwapi.h>
#include <xmllite.h>
#include <stack>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include "KeyValueNode.h"
#include <strsafe.h>

DWORD KeyValueNodeToXml( const KeyValueNode& root_node, IXmlWriter* xml_writer_ptr )
{
  HRESULT hr = xml_writer_ptr->WriteStartElement(NULL, root_node.GetKey().c_str(), NULL ) ;
  if ( root_node.GetValue().length() > 0 )
  {
    hr = xml_writer_ptr->WriteString( root_node.GetValue().c_str() ) ;
  }
  if (FAILED(hr))
  {
    ::OutputDebugString(TEXT("failed to create xmlwriter interface"));
    return HRESULT_CODE(hr);
  }
  KeyValueNode::TSubNodeVector child_nodes = root_node.GetSubNodes() ;

  if ( child_nodes.size() )
  {
    KeyValueNode::TSubNodeVector::iterator sub_node_iter = child_nodes.begin() ;
    for ( ; sub_node_iter !=  child_nodes.end() ; sub_node_iter++ )
    {
      hr = KeyValueNodeToXml( *sub_node_iter, xml_writer_ptr ) ;
    }
  }

  hr = xml_writer_ptr->WriteEndElement() ;
  return hr ;
}
DWORD KeyValueRootNodeToXml( const KeyValueNode& root_node, std::basic_string<TCHAR>* xml_out_ptr ) 
{
  if ( NULL == xml_out_ptr )
  {
    _RPTF0( _CRT_ERROR, TEXT("Null Pointer passed in when creating XML from root KeyValueNode") ) ;
    return ERROR_INVALID_PARAMETER ;
  }
  IStream* pFileStream = NULL;
  HRESULT hr = ::CreateStreamOnHGlobal( NULL, TRUE, &pFileStream ) ;
  if ( FAILED(hr) )
  {
    ::OutputDebugString(TEXT("failed to createstream on HGLOBAL"));
    return HRESULT_CODE(hr);
  }
  IXmlWriter* pWriter = NULL ;
  hr = CreateXmlWriter( IID_IXmlWriter, (void**) &pWriter, NULL ) ;
  if (FAILED(hr))
  {
    ::OutputDebugString(TEXT("failed to create xmlwriter interface"));
    pFileStream->Release();
    return HRESULT_CODE(hr);
  }
  IXmlWriterOutput* xml_output_ptr = NULL ;
  hr = CreateXmlWriterOutputWithEncodingName( pFileStream, NULL, L"utf-16", &xml_output_ptr ) ;
  if (FAILED(hr))
  {
    ::OutputDebugString(TEXT("failed to create xmlwriter utf-16 output"));
    pFileStream->Release();
    pWriter->Release() ;
    return HRESULT_CODE(hr);
  }

  hr = pWriter->SetOutput( xml_output_ptr ) ;
  if (FAILED(hr))
  {
    ::OutputDebugString(TEXT("failed to set xmlwriter output"));
    pFileStream->Release();
    pWriter->Release() ;
    xml_output_ptr->Release() ;
    return HRESULT_CODE(hr);
  }
  hr = pWriter->WriteStartDocument( XmlStandalone_Omit ) ;
  if (FAILED(hr))
  {
    ::OutputDebugString(TEXT("failed WriteStartDocument when writing xml"));
    pFileStream->Release();
    pWriter->Release() ;
    return HRESULT_CODE(hr);
  }
  hr = KeyValueNodeToXml( root_node, pWriter ) ;
  pWriter->WriteEndDocument();
  pWriter->Flush() ;
  //get the size of the data in the stream
  STATSTG stat_buffer ;
  pFileStream->Stat( &stat_buffer, STATFLAG_NONAME ) ;
  ULARGE_INTEGER size = stat_buffer.cbSize ;

  //put the xml into a vector
  std::vector<TCHAR> char_vector;
  //zero the vector first, also allocs enough memory
  char_vector.assign( (std::vector<TCHAR>::size_type)size.LowPart, (TCHAR)0x0 ) ;
  LARGE_INTEGER move_zero = { 0,0 } ;
  pFileStream->Seek( move_zero, STREAM_SEEK_SET, NULL ) ;
  ULONG bytes_read = 0 ;
  hr = pFileStream->Read( (void*)&char_vector[0], size.LowPart, &bytes_read ) ;
  if ( SUCCEEDED( hr ) )
  {
    UINT index = 0 ;
    BOOL byte_order_mark = FALSE ;
    hr = pWriter->GetProperty( XmlWriterProperty_ByteOrderMark, (LONG_PTR*)&byte_order_mark ) ;
    if ( byte_order_mark ) 
    {
      index = 1 ;
    }
    xml_out_ptr->assign( (TCHAR*) &char_vector[index] ) ;
  }
  pWriter->Release() ;
  pFileStream->Release();
  xml_output_ptr->Release() ;
  return hr ;
}
DWORD XmlReaderToKeyValueNode( IXmlReader* pReader, KeyValueNode* root_node )
{
  XmlNodeType nodeType;  
  TCHAR* strLocalName = NULL;
  TCHAR* strValue = NULL;
  std::stack<KeyValueNode*> objStack;
  KeyValueNode* pCurrentObj = NULL;
  std::basic_stringstream<TCHAR> debug_str;
  //UINT current_depth = 0;
  //read until there are no more nodes
  HRESULT hr = S_OK ;
  while (S_OK == (hr = pReader->Read(&nodeType)))
  {
    switch (nodeType)
    {
      case XmlNodeType_Element:
      {
        if (FAILED(hr = pReader->GetLocalName((LPCTSTR*)&strLocalName, NULL)))
        {
            ::OutputDebugString(TEXT("Error getting local name\n"));
            return HRESULT_CODE(hr);
        }
        if ( NULL == pCurrentObj )
        {
          pCurrentObj = new KeyValueNode( std::basic_string <TCHAR>( strLocalName ) ) ;
          objStack.push(pCurrentObj);
        }
        else
        {
          pCurrentObj = pCurrentObj->NewSubNode( strLocalName ) ;
          objStack.push(pCurrentObj);
          debug_str << TEXT("start of ") << strLocalName << TEXT("\n");
          ::OutputDebugString(debug_str.str().c_str());
        }
      }
        break;
      case XmlNodeType_EndElement:
        if (FAILED(hr = pReader->GetLocalName((LPCTSTR*)&strLocalName, NULL)))
        {
            ::OutputDebugString(TEXT("Error getting local name\n"));
            return HRESULT_CODE(hr);
        }
        if (NULL != strLocalName)
        {
          if ( NULL != pCurrentObj && pCurrentObj->IsKeyEqual(strLocalName))
          {
            objStack.pop();
            if (! objStack.empty())
            {
              pCurrentObj = objStack.top();
            }
          }
          debug_str << TEXT("end of ") << strLocalName << TEXT("\n");
          ::OutputDebugString(debug_str.str().c_str());
        }
        strLocalName = NULL;
        break;
      case XmlNodeType_Whitespace:
        break;
      case XmlNodeType_Text:
        if (FAILED(hr = pReader->GetValue((LPCTSTR*)&strValue, NULL)))
        {
          ::OutputDebugString(TEXT("Error getting value"));
          return HRESULT_CODE(hr);
        }
        if (NULL != strValue)
        {
          if ( NULL != pCurrentObj)
          {
            pCurrentObj->SetValue( strValue );
          }      
          debug_str << TEXT("value of ") << strLocalName << TEXT(" is '") << strValue << TEXT("\n");
          ::OutputDebugString(debug_str.str().c_str());
        }
        break;
    }
  }
  if ( NULL != pCurrentObj )
  {
    *root_node = *pCurrentObj;
  }
  return ERROR_SUCCESS;
}
DWORD XmlStringToKeyValueRootNode( std::basic_string<TCHAR> string_in_buffer, KeyValueNode* root_node )
{
  if (NULL == root_node)
  {
    _RPTF0( _CRT_ERROR, TEXT("Null Pointer passed in when creating root KeyValueNode from XML") ) ;
    return ERROR_INVALID_PARAMETER ;
  }
  IStream* stream_ptr = NULL;
  HRESULT hr = ::CreateStreamOnHGlobal( NULL, TRUE, &stream_ptr ) ;
  if ( FAILED(hr) )
  {
    ::OutputDebugString(TEXT("failed to createstream on HGLOBAL"));
    return HRESULT_CODE(hr);
  }
  ULONG bytes_written = 0 ;
  ULONG size = string_in_buffer.length()*sizeof( std::basic_string<TCHAR>::value_type ) ;
  hr = stream_ptr->Write( (const void*)string_in_buffer.data(), size, &bytes_written ) ;
  if (FAILED(hr))
  {
    ::OutputDebugString(TEXT("failed to copy string into stream for xml reader"));
    stream_ptr->Release();
    return HRESULT_CODE(hr);
  }
  //reset seek pointer to beginning after the write
  LARGE_INTEGER move_zero = { 0,0 } ;
  stream_ptr->Seek( move_zero, STREAM_SEEK_SET, NULL ) ;

  IXmlReaderInput* reader_input_ptr = NULL ;
  ::CreateXmlReaderInputWithEncodingName( stream_ptr, NULL, L"utf-16", TRUE, NULL, &reader_input_ptr ) ;
  IXmlReader* xml_reader_ptr = NULL;
  hr = CreateXmlReader(IID_IXmlReader, (void**) &xml_reader_ptr, NULL);
  if (FAILED(hr))
  {
    ::OutputDebugString(TEXT("failed to create xmlreader interface"));
    stream_ptr->Release();
    return HRESULT_CODE(hr);
  }
  xml_reader_ptr->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit) ;
  hr = xml_reader_ptr->SetInput(reader_input_ptr);
  if (FAILED(hr))
  {
    ::OutputDebugString(TEXT("failed to set stream on xml reader"));
    stream_ptr->Release();
    reader_input_ptr->Release() ;
    xml_reader_ptr->Release();
    return HRESULT_CODE(hr);
  }
  DWORD dwRet = XmlReaderToKeyValueNode( xml_reader_ptr, root_node ) ;
  stream_ptr->Release();
  reader_input_ptr->Release() ;
  xml_reader_ptr->Release();
  return dwRet ;
}
DWORD XmlFileToKeyValueRootNode( std::basic_string<TCHAR> filename, KeyValueNode* root_node )
{
  if (NULL == root_node)
  {
    _RPTF0( _CRT_ERROR, TEXT("Null Pointer passed in when creating root KeyValueNode from XML") ) ;
    return ERROR_INVALID_PARAMETER ;
  }
  IStream* pFileStream = NULL;
  HRESULT hr = ::SHCreateStreamOnFile(filename.c_str(), STGM_READ, &pFileStream);
  if (FAILED(hr))
  {
    ::OutputDebugString(TEXT("failed to createstream on file"));
    return HRESULT_CODE(hr);
  }
  IXmlReader* pReader = NULL;
  hr = CreateXmlReader(IID_IXmlReader, (void**) &pReader, NULL);
  if (FAILED(hr))
  {
    ::OutputDebugString(TEXT("failed to create xmlreader interface"));
    pFileStream->Release();
    return HRESULT_CODE(hr);
  }
  pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit);
  hr = pReader->SetInput(pFileStream);
  if (FAILED(hr))
  {
    ::OutputDebugString(TEXT("failed to set stream on xml reader"));
    pFileStream->Release();
    pReader->Release();
    return HRESULT_CODE(hr);
  }
  DWORD dwRet = XmlReaderToKeyValueNode( pReader, root_node ) ;
  pFileStream->Release();
  pReader->Release();
  return dwRet ;
}

KeyValueNode::KeyValueNode(void)
{
}

KeyValueNode::~KeyValueNode(void)
{
}
KeyValueNode::KeyValueNode( const KeyValueNode& in )
{
  key = in.key ;
  value = in.value ;

  std::vector< KeyValueNode > sub_nodes_vect = in.GetSubNodes() ;
  std::vector< KeyValueNode >::const_iterator iter = sub_nodes_vect.begin() ;
  for ( iter; iter != sub_nodes_vect.end(); iter++ )
  {
    child_nodes.insert( std::make_pair(iter->GetKey(), (*iter) ) );
  }
}
KeyValueNode::KeyValueNode( const std::basic_string< TCHAR >& key_init )
{
  key = key_init ;
}
KeyValueNode::KeyValueNode( const std::basic_string< TCHAR >& key_init, const std::basic_string< TCHAR >& value_init )
{
  key = key_init ;
  value = value_init ;
}
KeyValueNode::KeyValueNode( const std::basic_string< TCHAR >& key_init, 
                const std::basic_string< TCHAR >& value_init,
                const std::vector< KeyValueNode >& child_nodes_init ) 
{
  key = key_init ;
  value = value_init ;

  std::vector< KeyValueNode >::const_iterator iter = child_nodes_init.begin() ;
  for ( iter; iter != child_nodes_init.end(); iter++ )
  {
    child_nodes.insert( std::make_pair( iter->GetKey(), (*iter) ) ) ;
  }
  

}
bool KeyValueNode::IsKeyEqual( const std::basic_string< TCHAR >& key_in ) const
{
  return key == key_in ;
}
std::basic_string< TCHAR > KeyValueNode::GetKey() const
{
  return key ;
}
KeyValueNode::TValueType KeyValueNode::GetValue() const
{
  return value ;
}
std::pair< std::basic_string< TCHAR >, KeyValueNode::TValueType > KeyValueNode::GetKeyValuePair() const
{
  return std::make_pair( key, value ) ;
}
void KeyValueNode::SetValue( const KeyValueNode::TValueType& value_in ) 
{
  value = value_in ;
}
void KeyValueNode::SetValueForSingletonNode( const std::basic_string< TCHAR >& sub_key,
                                             const KeyValueNode::TValueType& sub_value ) 
{
  std::vector<KeyValueNode*> nodes = this->GetSubNodesByReference( sub_key ) ;
  if ( nodes.size() > 0 )
  {
    nodes[0]->SetValue( sub_value ) ;
  }
}
bool KeyValueNode::IsSubNode( const std::basic_string< TCHAR >& subnode_name ) const
{
  std::multimap< std::basic_string<TCHAR>, KeyValueNode >::const_iterator iter ;
  iter = child_nodes.find( subnode_name ) ;
  if ( iter != child_nodes.end() )
  {
    return true ;
  }
  return false ;
}
std::vector<KeyValueNode*> KeyValueNode::GetSubNodesByReference( const std::basic_string< TCHAR >& subnode_name ) const
{
  //bool found = true ;
  std::vector< KeyValueNode* > nodes ;
  std::multimap< std::basic_string<TCHAR>, KeyValueNode >::const_iterator iter ;
  iter = child_nodes.find( subnode_name ) ;

  for ( iter ; iter != child_nodes.end() ; iter ++ )
  {
    if ( iter->first == subnode_name )
    {
      nodes.push_back( (KeyValueNode*)&( (*iter).second ) ) ;
    }
  }
  return nodes ;
}
std::vector<KeyValueNode*> KeyValueNode::GetSubNodesByReference() const
{
//  bool found = true ;
  std::vector< KeyValueNode* > nodes ;
  std::multimap< std::basic_string<TCHAR>, KeyValueNode >::const_iterator iter ;
  iter = child_nodes.begin() ;

  for ( iter ; iter != child_nodes.end() ; iter ++ )
  {
    nodes.push_back( (KeyValueNode*)&( (*iter).second ) ) ;
  }
  return nodes ;
}
std::vector<KeyValueNode> KeyValueNode::GetSubNodes( const std::basic_string< TCHAR >& subnode_key ) const
{
//  bool found = true ;
  std::vector< KeyValueNode > nodes ;
  std::multimap< std::basic_string<TCHAR>, KeyValueNode >::const_iterator iter ;
  iter = child_nodes.find( subnode_key ) ;

  for ( iter ; iter != child_nodes.end() ; iter ++ )
  {
    if ( iter->first == subnode_key )
    {
      nodes.push_back( iter->second  ) ;
    }
  }
  return nodes ;
}
KeyValueNode::TValueType KeyValueNode::GetValueForSingletonNode( const std::basic_string< TCHAR >& sub_key ) const
{
  KeyValueNode::TValueType return_value ;
  KeyValueNode::TSubNodeVector just_one = GetSubNodes( sub_key ) ;
  if ( just_one.size() > 0 )
  {
    return_value = just_one.at( 0 ).GetValue() ;
  }
  return return_value ;
}
std::vector<KeyValueNode> KeyValueNode::GetSubNodes() const
{
//  bool found = true ;
  std::vector< KeyValueNode > nodes ;
  std::multimap< std::basic_string<TCHAR>, KeyValueNode >::const_iterator iter ;
  iter = child_nodes.begin() ;

  for ( iter ; iter != child_nodes.end() ; iter ++ )
  {
    nodes.push_back( iter->second ) ;
  }
  return nodes ;
}
KeyValueNode* KeyValueNode::NewSubNode( const std::basic_string< TCHAR >& key_init )
{
  KeyValueNode new_node( key_init ) ;
  std::multimap< std::basic_string<TCHAR>, KeyValueNode >::iterator iter ;
  iter = child_nodes.insert( std::make_pair( key_init, new_node ) ) ;
  return &( (*iter).second ) ;
}

KeyValueNode* KeyValueNode::NewSubNode( const std::basic_string< TCHAR >& key_init, const std::basic_string< TCHAR >& value_init )
{
  KeyValueNode new_node( key_init, value_init ) ;
  std::multimap< std::basic_string<TCHAR>, KeyValueNode >::iterator iter ;
  iter = child_nodes.insert( std::make_pair( key_init, new_node ) ) ;
  return &( (*iter).second ) ;
}
void KeyValueNode::AddSubNode( const KeyValueNode& new_node ) 
{
  child_nodes.insert( std::make_pair( new_node.GetKey(), new_node ) ) ;
}