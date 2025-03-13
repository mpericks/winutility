#pragma once

//to use this header:
/*
#include <string>
#include <vector>
#include <map>
*/
class KeyValueNode ;

DWORD XmlFileToKeyValueRootNode( std::basic_string<TCHAR> filename, KeyValueNode* root_node ) ;
DWORD XmlStringToKeyValueRootNode( std::basic_string<TCHAR> string_in_buffer, KeyValueNode* root_node ) ;
DWORD KeyValueRootNodeToXml( const KeyValueNode& root_node, std::basic_string<TCHAR>* xml_out_ptr ) ;

class KeyValueNode
{
public:
  typedef std::basic_string<TCHAR> TValueType ;
  typedef std::vector<KeyValueNode> TSubNodeVector ;
  typedef std::pair< std::basic_string< TCHAR >, TValueType > TKeyValuePairType ;
public:
  KeyValueNode(void);
  KeyValueNode( const KeyValueNode& in ) ;

  KeyValueNode( const std::basic_string< TCHAR >& key_init, 
                const std::basic_string< TCHAR >& value_init ) ;

  KeyValueNode( const std::basic_string< TCHAR >& key_init, 
                const std::basic_string< TCHAR >& value_init,
                const std::vector< KeyValueNode >& child_nodes) ;
  explicit KeyValueNode( const std::basic_string< TCHAR >& key_init ) ;
  ~KeyValueNode(void);
public:
  bool IsKeyEqual( const std::basic_string< TCHAR >& key_in ) const;
  std::basic_string< TCHAR > GetKey() const ;
  TValueType GetValue() const;
  TKeyValuePairType GetKeyValuePair() const ;
  void SetValue( const TValueType& value_in ) ;
  void SetValueForSingletonNode( const std::basic_string< TCHAR >& sub_key,
                                 const std::basic_string< TCHAR >& sub_value ) ;
  TValueType GetValueForSingletonNode( const std::basic_string< TCHAR >& sub_key ) const ;
  bool IsSubNode( const std::basic_string< TCHAR >& subnode_name ) const ;
  std::vector<KeyValueNode*> GetSubNodesByReference( const std::basic_string< TCHAR >& subnode_key ) const;
  std::vector<KeyValueNode*> GetSubNodesByReference() const;
  TSubNodeVector GetSubNodes( const std::basic_string< TCHAR >& subnode_key ) const;
  TSubNodeVector GetSubNodes() const;
  KeyValueNode* NewSubNode( const std::basic_string< TCHAR >& key_init ) ;
  KeyValueNode* NewSubNode( const std::basic_string< TCHAR >& key_init, const std::basic_string< TCHAR >& value_init ) ;
  void AddSubNode( const KeyValueNode& new_node ) ;
private:
  std::multimap< std::basic_string<TCHAR>, KeyValueNode > child_nodes ;
  std::basic_string<TCHAR> key ;
  TValueType value ;
};

