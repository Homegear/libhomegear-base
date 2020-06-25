#ifndef RAPIDXML_HPP_INCLUDED
#define RAPIDXML_HPP_INCLUDED

// Copyright (C) 2006, 2009 Marcin Kalicinski
// Version 1.13
// Revision $DateTime: 2009/05/13 01:46:17 $
//! \file rapidxml.hpp This file contains rapidxml parser and DOM implementation

#include <cstdlib>      // For std::size_t
#include <cassert>      // For assert
#include <new>          // For placement new

#include <exception>    // For std::exception
#include <ostream>
#include <iterator>

namespace rapidxml
{

    //! Parse error exception. 
    //! This exception is thrown by the parser when an error occurs. 
    //! Use what() function to get human-readable error message. 
    //! Use where() function to get a pointer to position within source text where error was detected.
    //! <br><br>
    //! If throwing exceptions by the parser is undesirable, 
    //! it can be disabled by defining RAPIDXML_NO_EXCEPTIONS macro before rapidxml.hpp is included.
    //! This will cause the parser to call rapidxml::parse_error_handler() function instead of throwing an exception.
    //! This function must be defined by the user.
    //! <br><br>
    //! This class derives from <code>std::exception</code> class.
    class parse_error: public std::exception
    {
    
    public:
    
        //! Constructs parse error
        parse_error(const char *what, void *where);

        //! Gets human readable description of error.
        //! \return Pointer to null terminated description of the error.
        const char *what() const noexcept override;

        //! Gets pointer to character data where error happened.
        //! Ch should be the same as char type of xml_document that produced the error.
        //! \return Pointer to location within the parsed string where error occured.
        char *where() const;

    private:  

        const char *m_what;
        void *m_where;

    };
}

///////////////////////////////////////////////////////////////////////////
// Pool sizes

#ifndef RAPIDXML_STATIC_POOL_SIZE
    // Size of static memory block of memory_pool.
    // Define RAPIDXML_STATIC_POOL_SIZE before including rapidxml.hpp if you want to override the default value.
    // No dynamic memory allocations are performed by memory_pool until static memory is exhausted.
    #define RAPIDXML_STATIC_POOL_SIZE (64 * 1024)
#endif

#ifndef RAPIDXML_DYNAMIC_POOL_SIZE
    // Size of dynamic memory block of memory_pool.
    // Define RAPIDXML_DYNAMIC_POOL_SIZE before including rapidxml.hpp if you want to override the default value.
    // After the static block is exhausted, dynamic blocks with approximately this size are allocated by memory_pool.
    #define RAPIDXML_DYNAMIC_POOL_SIZE (64 * 1024)
#endif

#ifndef RAPIDXML_ALIGNMENT
    // Memory allocation alignment.
    // Define RAPIDXML_ALIGNMENT before including rapidxml.hpp if you want to override the default value, which is the size of pointer.
    // All memory allocations for nodes, attributes and strings will be aligned to this value.
    // This must be a power of 2 and at least 1, otherwise memory_pool will not work.
    #define RAPIDXML_ALIGNMENT sizeof(void *)
#endif

namespace rapidxml
{
    class xml_node;
    class xml_attribute;
    class xml_document;

    //! Enumeration listing all node types produced by the parser.
    //! Use xml_node::type() function to query node type.
    enum node_type
    {
        node_document,      //!< A document node. Name and value are empty.
        node_element,       //!< An element node. Name contains element name. Value contains text of first data node.
        node_data,          //!< A data node. Name is empty. Value contains data text.
        node_cdata,         //!< A CDATA node. Name is empty. Value contains data text.
        node_comment,       //!< A comment node. Name is empty. Value contains comment text.
        node_declaration,   //!< A declaration node. Name and value are empty. Declaration parameters (version, encoding and standalone) are in node attributes.
        node_doctype,       //!< A DOCTYPE node. Name is empty. Value contains DOCTYPE text.
        node_pi             //!< A PI node. Name contains target. Value contains instructions.
    };

    ///////////////////////////////////////////////////////////////////////
    // Parsing flags

    //! Parse flag instructing the parser to not create data nodes. 
    //! Text of first data node will still be placed in value of parent element, unless rapidxml::parse_no_element_values flag is also specified.
    //! Can be combined with other flags by use of | operator.
    //! <br><br>
    //! See xml_document::parse() function.
    const int parse_no_data_nodes = 0x1;

    //! Parse flag instructing the parser to not use text of first data node as a value of parent element.
    //! Can be combined with other flags by use of | operator.
    //! Note that child data nodes of element node take precendence over its value when printing. 
    //! That is, if element has one or more child data nodes <em>and</em> a value, the value will be ignored.
    //! Use rapidxml::parse_no_data_nodes flag to prevent creation of data nodes if you want to manipulate data using values of elements.
    //! <br><br>
    //! See xml_document::parse() function.
    const int parse_no_element_values = 0x2;
    
    //! Parse flag instructing the parser to not place zero terminators after strings in the source text.
    //! By default zero terminators are placed, modifying source text.
    //! Can be combined with other flags by use of | operator.
    //! <br><br>
    //! See xml_document::parse() function.
    const int parse_no_string_terminators = 0x4;
    
    //! Parse flag instructing the parser to not translate entities in the source text.
    //! By default entities are translated, modifying source text.
    //! Can be combined with other flags by use of | operator.
    //! <br><br>
    //! See xml_document::parse() function.
    const int parse_no_entity_translation = 0x8;
    
    //! Parse flag instructing the parser to disable UTF-8 handling and assume plain 8 bit characters.
    //! By default, UTF-8 handling is enabled.
    //! Can be combined with other flags by use of | operator.
    //! <br><br>
    //! See xml_document::parse() function.
    const int parse_no_utf8 = 0x10;
    
    //! Parse flag instructing the parser to create XML declaration node.
    //! By default, declaration node is not created.
    //! Can be combined with other flags by use of | operator.
    //! <br><br>
    //! See xml_document::parse() function.
    const int parse_declaration_node = 0x20;
    
    //! Parse flag instructing the parser to create comments nodes.
    //! By default, comment nodes are not created.
    //! Can be combined with other flags by use of | operator.
    //! <br><br>
    //! See xml_document::parse() function.
    const int parse_comment_nodes = 0x40;
    
    //! Parse flag instructing the parser to create DOCTYPE node.
    //! By default, doctype node is not created.
    //! Although W3C specification allows at most one DOCTYPE node, RapidXml will silently accept documents with more than one.
    //! Can be combined with other flags by use of | operator.
    //! <br><br>
    //! See xml_document::parse() function.
    const int parse_doctype_node = 0x80;
    
    //! Parse flag instructing the parser to create PI nodes.
    //! By default, PI nodes are not created.
    //! Can be combined with other flags by use of | operator.
    //! <br><br>
    //! See xml_document::parse() function.
    const int parse_pi_nodes = 0x100;
    
    //! Parse flag instructing the parser to validate closing tag names. 
    //! If not set, name inside closing tag is irrelevant to the parser.
    //! By default, closing tags are not validated.
    //! Can be combined with other flags by use of | operator.
    //! <br><br>
    //! See xml_document::parse() function.
    const int parse_validate_closing_tags = 0x200;
    
    //! Parse flag instructing the parser to trim all leading and trailing whitespace of data nodes.
    //! By default, whitespace is not trimmed. 
    //! This flag does not cause the parser to modify source text.
    //! Can be combined with other flags by use of | operator.
    //! <br><br>
    //! See xml_document::parse() function.
    const int parse_trim_whitespace = 0x400;

    //! Parse flag instructing the parser to condense all whitespace runs of data nodes to a single space character.
    //! Trimming of leading and trailing whitespace of data is controlled by rapidxml::parse_trim_whitespace flag.
    //! By default, whitespace is not normalized. 
    //! If this flag is specified, source text will be modified.
    //! Can be combined with other flags by use of | operator.
    //! <br><br>
    //! See xml_document::parse() function.
    const int parse_normalize_whitespace = 0x800;

    // Compound flags
    
    //! Parse flags which represent default behaviour of the parser. 
    //! This is always equal to 0, so that all other flags can be simply ored together.
    //! Normally there is no need to inconveniently disable flags by anding with their negated (~) values.
    //! This also means that meaning of each flag is a <i>negation</i> of the default setting. 
    //! For example, if flag name is rapidxml::parse_no_utf8, it means that utf-8 is <i>enabled</i> by default,
    //! and using the flag will disable it.
    //! <br><br>
    //! See xml_document::parse() function.
    const int parse_default = 0;
    
    //! A combination of parse flags that forbids any modifications of the source text. 
    //! This also results in faster parsing. However, note that the following will occur:
    //! <ul>
    //! <li>names and values of nodes will not be zero terminated, you have to use xml_base::name_size() and xml_base::value_size() functions to determine where name and value ends</li>
    //! <li>entities will not be translated</li>
    //! <li>whitespace will not be normalized</li>
    //! </ul>
    //! See xml_document::parse() function.
    const int parse_non_destructive = parse_no_string_terminators | parse_no_entity_translation;
    
    //! A combination of parse flags resulting in fastest possible parsing, without sacrificing important data.
    //! <br><br>
    //! See xml_document::parse() function.
    const int parse_fastest = parse_non_destructive | parse_no_data_nodes;
    
    //! A combination of parse flags resulting in largest amount of data being extracted. 
    //! This usually results in slowest parsing.
    //! <br><br>
    //! See xml_document::parse() function.
    const int parse_full = parse_declaration_node | parse_comment_nodes | parse_doctype_node | parse_pi_nodes | parse_validate_closing_tags;

    ///////////////////////////////////////////////////////////////////////
    // Memory pool
    
    //! This class is used by the parser to create new nodes and attributes, without overheads of dynamic memory allocation.
    //! In most cases, you will not need to use this class directly. 
    //! However, if you need to create nodes manually or modify names/values of nodes, 
    //! you are encouraged to use memory_pool of relevant xml_document to allocate the memory. 
    //! Not only is this faster than allocating them by using <code>new</code> operator, 
    //! but also their lifetime will be tied to the lifetime of document, 
    //! possibly simplyfing memory management. 
    //! <br><br>
    //! Call allocate_node() or allocate_attribute() functions to obtain new nodes or attributes from the pool. 
    //! You can also call allocate_string() function to allocate strings.
    //! Such strings can then be used as names or values of nodes without worrying about their lifetime.
    //! Note that there is no <code>free()</code> function -- all allocations are freed at once when clear() function is called, 
    //! or when the pool is destroyed.
    //! <br><br>
    //! It is also possible to create a standalone memory_pool, and use it 
    //! to allocate nodes, whose lifetime will not be tied to any document.
    //! <br><br>
    //! Pool maintains <code>RAPIDXML_STATIC_POOL_SIZE</code> bytes of statically allocated memory. 
    //! Until static memory is exhausted, no dynamic memory allocations are done.
    //! When static memory is exhausted, pool allocates additional blocks of memory of size <code>RAPIDXML_DYNAMIC_POOL_SIZE</code> each,
    //! by using global <code>new[]</code> and <code>delete[]</code> operators. 
    //! This behaviour can be changed by setting custom allocation routines. 
    //! Use set_allocator() function to set them.
    //! <br><br>
    //! Allocations for nodes, attributes and strings are aligned at <code>RAPIDXML_ALIGNMENT</code> bytes.
    //! This value defaults to the size of pointer on target architecture.
    //! <br><br>
    //! To obtain absolutely top performance from the parser,
    //! it is important that all nodes are allocated from a single, contiguous block of memory.
    //! Otherwise, cache misses when jumping between two (or more) disjoint blocks of memory can slow down parsing quite considerably.
    //! If required, you can tweak <code>RAPIDXML_STATIC_POOL_SIZE</code>, <code>RAPIDXML_DYNAMIC_POOL_SIZE</code> and <code>RAPIDXML_ALIGNMENT</code> 
    //! to obtain best wasted memory to performance compromise.
    //! To do it, define their values before rapidxml.hpp file is included.
    //! \param Ch Character type of created nodes.
    class memory_pool
    {
        
    public:

        //! \cond internal
        typedef void *(alloc_func)(std::size_t);       // Type of user-defined function used to allocate memory
        typedef void (free_func)(void *);              // Type of user-defined function used to free memory
        //! \endcond
        
        //! Constructs empty pool with default allocator functions.
        memory_pool();

        //! Destroys pool and frees all the memory. 
        //! This causes memory occupied by nodes allocated by the pool to be freed.
        //! Nodes allocated from the pool are no longer valid.
        ~memory_pool();

        //! Allocates a new node from the pool, and optionally assigns name and value to it. 
        //! If the allocation request cannot be accomodated, this function will throw <code>std::bad_alloc</code>.
        //! If exceptions are disabled by defining RAPIDXML_NO_EXCEPTIONS, this function
        //! will call rapidxml::parse_error_handler() function.
        //! \param type Type of node to create.
        //! \param name Name to assign to the node, or 0 to assign no name.
        //! \param value Value to assign to the node, or 0 to assign no value.
        //! \param name_size Size of name to assign, or 0 to automatically calculate size from name string.
        //! \param value_size Size of value to assign, or 0 to automatically calculate size from value string.
        //! \return Pointer to allocated node. This pointer will never be NULL.
        xml_node* allocate_node(node_type type, const char *name = nullptr, const char *value = nullptr, std::size_t name_size = 0, std::size_t value_size = 0);

        //! Allocates a new attribute from the pool, and optionally assigns name and value to it.
        //! If the allocation request cannot be accomodated, this function will throw <code>std::bad_alloc</code>.
        //! If exceptions are disabled by defining RAPIDXML_NO_EXCEPTIONS, this function
        //! will call rapidxml::parse_error_handler() function.
        //! \param name Name to assign to the attribute, or 0 to assign no name.
        //! \param value Value to assign to the attribute, or 0 to assign no value.
        //! \param name_size Size of name to assign, or 0 to automatically calculate size from name string.
        //! \param value_size Size of value to assign, or 0 to automatically calculate size from value string.
        //! \return Pointer to allocated attribute. This pointer will never be NULL.
        xml_attribute *allocate_attribute(const char *name = nullptr, const char *value = nullptr, std::size_t name_size = 0, std::size_t value_size = 0);

        //! Allocates a char array of given size from the pool, and optionally copies a given string to it.
        //! If the allocation request cannot be accomodated, this function will throw <code>std::bad_alloc</code>.
        //! If exceptions are disabled by defining RAPIDXML_NO_EXCEPTIONS, this function
        //! will call rapidxml::parse_error_handler() function.
        //! \param source String to initialize the allocated memory with, or 0 to not initialize it.
        //! \param size Number of characters to allocate, or zero to calculate it automatically from source string length; if size is 0, source string must be specified and null terminated.
        //! \return Pointer to allocated char array. This pointer will never be NULL.
        char *allocate_string(const char *source = nullptr, std::size_t size = 0);

        //! Clones an xml_node and its hierarchy of child nodes and attributes.
        //! Nodes and attributes are allocated from this memory pool.
        //! Names and values are not cloned, they are shared between the clone and the source.
        //! Result node can be optionally specified as a second parameter, 
        //! in which case its contents will be replaced with cloned source node.
        //! This is useful when you want to clone entire document.
        //! \param source Node to clone.
        //! \param result Node to put results in, or 0 to automatically allocate result node
        //! \return Pointer to cloned node. This pointer will never be NULL.
        xml_node *clone_node(const xml_node *source, xml_node *result = nullptr);

        //! Clears the pool. 
        //! This causes memory occupied by nodes allocated by the pool to be freed.
        //! Any nodes or strings allocated from the pool will no longer be valid.
        void clear();

        //! Sets or resets the user-defined memory allocation functions for the pool.
        //! This can only be called when no memory is allocated from the pool yet, otherwise results are undefined.
        //! Allocation function must not return invalid pointer on failure. It should either throw,
        //! stop the program, or use <code>longjmp()</code> function to pass control to other place of program. 
        //! If it returns invalid pointer, results are undefined.
        //! <br><br>
        //! User defined allocation functions must have the following forms:
        //! <br><code>
        //! <br>void *allocate(std::size_t size);
        //! <br>void free(void *pointer);
        //! </code><br>
        //! \param af Allocation function, or 0 to restore default function
        //! \param ff Free function, or 0 to restore default function
        void set_allocator(alloc_func *af, free_func *ff);

    private:

        struct header
        {
            char *previous_begin;
        };

        void init();
        
        char *align(char *ptr);
        
        char *allocate_raw(std::size_t size);
        
        void *allocate_aligned(std::size_t size);

        char *m_begin;                                      // Start of raw memory making up current pool
        char *m_ptr;                                        // First free byte in current pool
        char *m_end;                                        // One past last available byte in current pool
        char m_static_memory[RAPIDXML_STATIC_POOL_SIZE];    // Static raw memory
        alloc_func *m_alloc_func;                           // Allocator function, or 0 if default is to be used
        free_func *m_free_func;                             // Free function, or 0 if default is to be used
    };

    ///////////////////////////////////////////////////////////////////////////
    // XML base

    //! Base class for xml_node and xml_attribute implementing common functions: 
    //! name(), name_size(), value(), value_size() and parent().
    //! \param Ch Character type to use
    class xml_base
    {

    public:
        
        ///////////////////////////////////////////////////////////////////////////
        // Construction & destruction
    
        // Construct a base with empty name, value and parent
        xml_base();

        ///////////////////////////////////////////////////////////////////////////
        // Node data access
    
        //! Gets name of the node. 
        //! Interpretation of name depends on type of node.
        //! Note that name will not be zero-terminated if rapidxml::parse_no_string_terminators option was selected during parse.
        //! <br><br>
        //! Use name_size() function to determine length of the name.
        //! \return Name of node, or empty string if node has no name.
        char *name() const;

        //! Gets size of node name, not including terminator character.
        //! This function works correctly irrespective of whether name is or is not zero terminated.
        //! \return Size of node name, in characters.
        std::size_t name_size() const;

        //! Gets value of node. 
        //! Interpretation of value depends on type of node.
        //! Note that value will not be zero-terminated if rapidxml::parse_no_string_terminators option was selected during parse.
        //! <br><br>
        //! Use value_size() function to determine length of the value.
        //! \return Value of node, or empty string if node has no value.
        char *value() const;

        //! Gets size of node value, not including terminator character.
        //! This function works correctly irrespective of whether value is or is not zero terminated.
        //! \return Size of node value, in characters.
        std::size_t value_size() const;

        ///////////////////////////////////////////////////////////////////////////
        // Node modification
    
        //! Sets name of node to a non zero-terminated string.
        //! See \ref ownership_of_strings.
        //! <br><br>
        //! Note that node does not own its name or value, it only stores a pointer to it. 
        //! It will not delete or otherwise free the pointer on destruction.
        //! It is reponsibility of the user to properly manage lifetime of the string.
        //! The easiest way to achieve it is to use memory_pool of the document to allocate the string -
        //! on destruction of the document the string will be automatically freed.
        //! <br><br>
        //! Size of name must be specified separately, because name does not have to be zero terminated.
        //! Use name(const Ch *) function to have the length automatically calculated (string must be zero terminated).
        //! \param name Name of node to set. Does not have to be zero terminated.
        //! \param size Size of name, in characters. This does not include zero terminator, if one is present.
        void name(const char *name, std::size_t size);

        //! Sets name of node to a zero-terminated string.
        //! See also \ref ownership_of_strings and xml_node::name(const Ch *, std::size_t).
        //! \param name Name of node to set. Must be zero terminated.
        void name(const char *name);

        //! Sets value of node to a non zero-terminated string.
        //! See \ref ownership_of_strings.
        //! <br><br>
        //! Note that node does not own its name or value, it only stores a pointer to it. 
        //! It will not delete or otherwise free the pointer on destruction.
        //! It is reponsibility of the user to properly manage lifetime of the string.
        //! The easiest way to achieve it is to use memory_pool of the document to allocate the string -
        //! on destruction of the document the string will be automatically freed.
        //! <br><br>
        //! Size of value must be specified separately, because it does not have to be zero terminated.
        //! Use value(const Ch *) function to have the length automatically calculated (string must be zero terminated).
        //! <br><br>
        //! If an element has a child node of type node_data, it will take precedence over element value when printing.
        //! If you want to manipulate data of elements using values, use parser flag rapidxml::parse_no_data_nodes to prevent creation of data nodes by the parser.
        //! \param value value of node to set. Does not have to be zero terminated.
        //! \param size Size of value, in characters. This does not include zero terminator, if one is present.
        void value(const char *value, std::size_t size);

        //! Sets value of node to a zero-terminated string.
        //! See also \ref ownership_of_strings and xml_node::value(const Ch *, std::size_t).
        //! \param value Vame of node to set. Must be zero terminated.
        void value(const char *value);

        ///////////////////////////////////////////////////////////////////////////
        // Related nodes access
    
        //! Gets node parent.
        //! \return Pointer to parent node, or 0 if there is no parent.
        xml_node *parent() const;

    protected:

        // Return empty string
        static char *nullstr();

        char *m_name;                         // Name of node, or 0 if no name
        char *m_value;                        // Value of node, or 0 if no value
        std::size_t m_name_size;            // Length of node name, or undefined of no name
        std::size_t m_value_size;           // Length of node value, or undefined if no value
        xml_node *m_parent;             // Pointer to parent node, or 0 if none

    };

    //! Class representing attribute node of XML document. 
    //! Each attribute has name and value strings, which are available through name() and value() functions (inherited from xml_base).
    //! Note that after parse, both name and value of attribute will point to interior of source text used for parsing. 
    //! Thus, this text must persist in memory for the lifetime of attribute.
    //! \param Ch Character type to use.
    class xml_attribute: public xml_base
    {

        friend class xml_node;
    
    public:

        ///////////////////////////////////////////////////////////////////////////
        // Construction & destruction
    
        //! Constructs an empty attribute with the specified type. 
        //! Consider using memory_pool of appropriate xml_document if allocating attributes manually.
        xml_attribute() = default;

        ///////////////////////////////////////////////////////////////////////////
        // Related nodes access
    
        //! Gets document of which attribute is a child.
        //! \return Pointer to document that contains this attribute, or 0 if there is no parent document.
        xml_document *document() const;

        //! Gets previous attribute, optionally matching attribute name. 
        //! \param name Name of attribute to find, or 0 to return previous attribute regardless of its name; this string doesn't have to be zero-terminated if name_size is non-zero
        //! \param name_size Size of name, in characters, or 0 to have size calculated automatically from string
        //! \param case_sensitive Should name comparison be case-sensitive; non case-sensitive comparison works properly only for ASCII characters
        //! \return Pointer to found attribute, or 0 if not found.
        xml_attribute *previous_attribute(const char *name = nullptr, std::size_t name_size = 0, bool case_sensitive = true) const;

        //! Gets next attribute, optionally matching attribute name. 
        //! \param name Name of attribute to find, or 0 to return next attribute regardless of its name; this string doesn't have to be zero-terminated if name_size is non-zero
        //! \param name_size Size of name, in characters, or 0 to have size calculated automatically from string
        //! \param case_sensitive Should name comparison be case-sensitive; non case-sensitive comparison works properly only for ASCII characters
        //! \return Pointer to found attribute, or 0 if not found.
        xml_attribute *next_attribute(const char *name = nullptr, std::size_t name_size = 0, bool case_sensitive = true) const;

    private:

        xml_attribute *m_prev_attribute;        // Pointer to previous sibling of attribute, or 0 if none; only valid if parent is non-zero
        xml_attribute *m_next_attribute;        // Pointer to next sibling of attribute, or 0 if none; only valid if parent is non-zero
    
    };

    ///////////////////////////////////////////////////////////////////////////
    // XML node

    //! Class representing a node of XML document. 
    //! Each node may have associated name and value strings, which are available through name() and value() functions. 
    //! Interpretation of name and value depends on type of the node.
    //! Type of node can be determined by using type() function.
    //! <br><br>
    //! Note that after parse, both name and value of node, if any, will point interior of source text used for parsing. 
    //! Thus, this text must persist in the memory for the lifetime of node.
    //! \param Ch Character type to use.
    class xml_node : public xml_base
    {

    public:

        ///////////////////////////////////////////////////////////////////////////
        // Construction & destruction
    
        //! Constructs an empty node with the specified type. 
        //! Consider using memory_pool of appropriate document to allocate nodes manually.
        //! \param type Type of node to construct.
        xml_node(node_type type);

        ///////////////////////////////////////////////////////////////////////////
        // Node data access
    
        //! Gets type of node.
        //! \return Type of node.
        node_type type() const;

        ///////////////////////////////////////////////////////////////////////////
        // Related nodes access
    
        //! Gets document of which node is a child.
        //! \return Pointer to document that contains this node, or 0 if there is no parent document.
        xml_document *document() const;

        //! Gets first child node, optionally matching node name.
        //! \param name Name of child to find, or 0 to return first child regardless of its name; this string doesn't have to be zero-terminated if name_size is non-zero
        //! \param name_size Size of name, in characters, or 0 to have size calculated automatically from string
        //! \param case_sensitive Should name comparison be case-sensitive; non case-sensitive comparison works properly only for ASCII characters
        //! \return Pointer to found child, or 0 if not found.
        xml_node *first_node(const char *name = nullptr, std::size_t name_size = 0, bool case_sensitive = true) const;

        //! Gets last child node, optionally matching node name. 
        //! Behaviour is undefined if node has no children.
        //! Use first_node() to test if node has children.
        //! \param name Name of child to find, or 0 to return last child regardless of its name; this string doesn't have to be zero-terminated if name_size is non-zero
        //! \param name_size Size of name, in characters, or 0 to have size calculated automatically from string
        //! \param case_sensitive Should name comparison be case-sensitive; non case-sensitive comparison works properly only for ASCII characters
        //! \return Pointer to found child, or 0 if not found.
        xml_node *last_node(const char *name = nullptr, std::size_t name_size = 0, bool case_sensitive = true) const;

        //! Gets previous sibling node, optionally matching node name. 
        //! Behaviour is undefined if node has no parent.
        //! Use parent() to test if node has a parent.
        //! \param name Name of sibling to find, or 0 to return previous sibling regardless of its name; this string doesn't have to be zero-terminated if name_size is non-zero
        //! \param name_size Size of name, in characters, or 0 to have size calculated automatically from string
        //! \param case_sensitive Should name comparison be case-sensitive; non case-sensitive comparison works properly only for ASCII characters
        //! \return Pointer to found sibling, or 0 if not found.
        xml_node *previous_sibling(const char *name = nullptr, std::size_t name_size = 0, bool case_sensitive = true) const;

        //! Gets next sibling node, optionally matching node name. 
        //! Behaviour is undefined if node has no parent.
        //! Use parent() to test if node has a parent.
        //! \param name Name of sibling to find, or 0 to return next sibling regardless of its name; this string doesn't have to be zero-terminated if name_size is non-zero
        //! \param name_size Size of name, in characters, or 0 to have size calculated automatically from string
        //! \param case_sensitive Should name comparison be case-sensitive; non case-sensitive comparison works properly only for ASCII characters
        //! \return Pointer to found sibling, or 0 if not found.
        xml_node *next_sibling(const char *name = nullptr, std::size_t name_size = 0, bool case_sensitive = true) const;

        //! Gets first attribute of node, optionally matching attribute name.
        //! \param name Name of attribute to find, or 0 to return first attribute regardless of its name; this string doesn't have to be zero-terminated if name_size is non-zero
        //! \param name_size Size of name, in characters, or 0 to have size calculated automatically from string
        //! \param case_sensitive Should name comparison be case-sensitive; non case-sensitive comparison works properly only for ASCII characters
        //! \return Pointer to found attribute, or 0 if not found.
        xml_attribute *first_attribute(const char *name = nullptr, std::size_t name_size = 0, bool case_sensitive = true) const;

        //! Gets last attribute of node, optionally matching attribute name.
        //! \param name Name of attribute to find, or 0 to return last attribute regardless of its name; this string doesn't have to be zero-terminated if name_size is non-zero
        //! \param name_size Size of name, in characters, or 0 to have size calculated automatically from string
        //! \param case_sensitive Should name comparison be case-sensitive; non case-sensitive comparison works properly only for ASCII characters
        //! \return Pointer to found attribute, or 0 if not found.
        xml_attribute *last_attribute(const char *name = nullptr, std::size_t name_size = 0, bool case_sensitive = true) const;

        ///////////////////////////////////////////////////////////////////////////
        // Node modification
    
        //! Sets type of node.
        //! \param type Type of node to set.
        void type(node_type type);

        ///////////////////////////////////////////////////////////////////////////
        // Node manipulation

        //! Prepends a new child node.
        //! The prepended child becomes the first child, and all existing children are moved one position back.
        //! \param child Node to prepend.
        void prepend_node(xml_node *child);

        //! Appends a new child node. 
        //! The appended child becomes the last child.
        //! \param child Node to append.
        void append_node(xml_node *child);

        //! Inserts a new child node at specified place inside the node. 
        //! All children after and including the specified node are moved one position back.
        //! \param where Place where to insert the child, or 0 to insert at the back.
        //! \param child Node to insert.
        void insert_node(xml_node *where, xml_node *child);

        //! Removes first child node. 
        //! If node has no children, behaviour is undefined.
        //! Use first_node() to test if node has children.
        void remove_first_node();

        //! Removes last child of the node. 
        //! If node has no children, behaviour is undefined.
        //! Use first_node() to test if node has children.
        void remove_last_node();

        //! Removes specified child from the node
        // \param where Pointer to child to be removed.
        void remove_node(xml_node *where);

        //! Removes all child nodes (but not attributes).
        void remove_all_nodes();

        //! Prepends a new attribute to the node.
        //! \param attribute Attribute to prepend.
        void prepend_attribute(xml_attribute *attribute);

        //! Appends a new attribute to the node.
        //! \param attribute Attribute to append.
        void append_attribute(xml_attribute *attribute);

        //! Inserts a new attribute at specified place inside the node. 
        //! All attributes after and including the specified attribute are moved one position back.
        //! \param where Place where to insert the attribute, or 0 to insert at the back.
        //! \param attribute Attribute to insert.
        void insert_attribute(xml_attribute *where, xml_attribute *attribute);

        //! Removes first attribute of the node. 
        //! If node has no attributes, behaviour is undefined.
        //! Use first_attribute() to test if node has attributes.
        void remove_first_attribute();

        //! Removes last attribute of the node. 
        //! If node has no attributes, behaviour is undefined.
        //! Use first_attribute() to test if node has attributes.
        void remove_last_attribute();

        //! Removes specified attribute from node.
        //! \param where Pointer to attribute to be removed.
        void remove_attribute(xml_attribute *where);

        //! Removes all attributes of node.
        void remove_all_attributes();
        
    private:

        ///////////////////////////////////////////////////////////////////////////
        // Restrictions

        // No copying
        xml_node(const xml_node &);
        void operator =(const xml_node &);
    
        ///////////////////////////////////////////////////////////////////////////
        // Data members
    
        // Note that some of the pointers below have UNDEFINED values if certain other pointers are 0.
        // This is required for maximum performance, as it allows the parser to omit initialization of 
        // unneded/redundant values.
        //
        // The rules are as follows:
        // 1. first_node and first_attribute contain valid pointers, or 0 if node has no children/attributes respectively
        // 2. last_node and last_attribute are valid only if node has at least one child/attribute respectively, otherwise they contain garbage
        // 3. prev_sibling and next_sibling are valid only if node has a parent, otherwise they contain garbage

        node_type m_type;                       // Type of node; always valid
        xml_node *m_first_node;             // Pointer to first child node, or 0 if none; always valid
        xml_node *m_last_node;              // Pointer to last child node, or 0 if none; this value is only valid if m_first_node is non-zero
        xml_attribute *m_first_attribute;   // Pointer to first attribute of node, or 0 if none; always valid
        xml_attribute *m_last_attribute;    // Pointer to last attribute of node, or 0 if none; this value is only valid if m_first_attribute is non-zero
        xml_node *m_prev_sibling;           // Pointer to previous sibling of node, or 0 if none; this value is only valid if m_parent is non-zero
        xml_node *m_next_sibling;           // Pointer to next sibling of node, or 0 if none; this value is only valid if m_parent is non-zero

    };

///////////////////////////////////////////////////////////////////////////
    // XML document
    
    //! This class represents root of the DOM hierarchy. 
    //! It is also an xml_node and a memory_pool through public inheritance.
    //! Use parse() function to build a DOM tree from a zero-terminated XML text string.
    //! parse() function allocates memory for nodes and attributes by using functions of xml_document, 
    //! which are inherited from memory_pool.
    //! To access root node of the document, use the document itself, as if it was an xml_node.
    //! \param Ch Character type to use.
    class xml_document: public xml_node, public memory_pool
    {
    
    public:

        //! Constructs empty XML document
        xml_document();

        //! Parses zero-terminated XML string according to given flags.
        //! Passed string will be modified by the parser, unless rapidxml::parse_non_destructive flag is used.
        //! The string must persist for the lifetime of the document.
        //! In case of error, rapidxml::parse_error exception will be thrown.
        //! <br><br>
        //! If you want to parse contents of a file, you must first load the file into the memory, and pass pointer to its beginning.
        //! Make sure that data is zero-terminated.
        //! <br><br>
        //! Document can be parsed into multiple times. 
        //! Each new call to parse removes previous nodes and attributes (if any), but does not clear memory pool.
        //! \param text XML data to parse; pointer is non-const to denote fact that this data may be modified by the parser.
        template<int Flags>
        void parse(char *text);

        //! Clears the document by deleting all nodes and clearing the memory pool.
        //! All nodes owned by document pool are destroyed.
        void clear();
        
    private:

        ///////////////////////////////////////////////////////////////////////
        // Internal character utility functions
        
        // Detect whitespace character
        struct whitespace_pred
        {
            static unsigned char test(char ch);
        };

        // Detect node name character
        struct node_name_pred
        {
            static unsigned char test(char ch);
        };

        // Detect attribute name character
        struct attribute_name_pred
        {
            static unsigned char test(char ch);
        };

        // Detect text character (PCDATA)
        struct text_pred
        {
            static unsigned char test(char ch);
        };

        // Detect text character (PCDATA) that does not require processing
        struct text_pure_no_ws_pred
        {
            static unsigned char test(char ch);
        };

        // Detect text character (PCDATA) that does not require processing
        struct text_pure_with_ws_pred
        {
            static unsigned char test(char ch);
        };

        // Detect attribute value character
        template<char Quote>
        struct attribute_value_pred
        {
            static unsigned char test(char ch);
        };

        // Detect attribute value character
        template<char Quote>
        struct attribute_value_pure_pred
        {
            static unsigned char test(char ch);
        };

        // Insert coded character, using UTF8 or 8-bit ASCII
        template<int Flags>
        static void insert_coded_character(char *&text, unsigned long code);

        // Skip characters until predicate evaluates to true
        template<class StopPred, int Flags>
        static void skip(char *&text);

        // Skip characters until predicate evaluates to true while doing the following:
        // - replacing XML character entity references with proper characters (&apos; &amp; &quot; &lt; &gt; &#...;)
        // - condensing whitespace sequences to single space character
        template<class StopPred, class StopPredPure, int Flags>
        static char *skip_and_expand_character_refs(char *&text);

        ///////////////////////////////////////////////////////////////////////
        // Internal parsing functions
        
        // Parse BOM, if any
        template<int Flags>
        void parse_bom(char *&text);

        // Parse XML declaration (<?xml...)
        template<int Flags>
        xml_node *parse_xml_declaration(char *&text);

        // Parse XML comment (<!--...)
        template<int Flags>
        xml_node *parse_comment(char *&text);

        // Parse DOCTYPE
        template<int Flags>
        xml_node *parse_doctype(char *&text);

        // Parse PI
        template<int Flags>
        xml_node *parse_pi(char *&text);

        // Parse and append data
        // Return character that ends data.
        // This is necessary because this character might have been overwritten by a terminating 0
        template<int Flags>
        char parse_and_append_data(xml_node *node, char *&text, char *contents_start);

        // Parse CDATA
        template<int Flags>
        xml_node *parse_cdata(char *&text);
        
        // Parse element node
        template<int Flags>
        xml_node *parse_element(char *&text);

        // Determine node type, and parse it
        template<int Flags>
        xml_node *parse_node(char *&text);

        // Parse contents of the node - children, data etc.
        template<int Flags>
        void parse_node_contents(char *&text, xml_node *node);
        
        // Parse XML attributes of the node
        template<int Flags>
        void parse_node_attributes(char *&text, xml_node *node);

    };
}

#endif