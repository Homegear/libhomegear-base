#include "rapidxml.h"

namespace rapidxml
{

//! \cond internal
namespace internal
{

// Struct that contains lookup tables for the parser
// It must be a template to allow correct linking (because it has static data members, which are defined in a header file).
template<int Dummy>
struct lookup_tables
{
    static const unsigned char lookup_whitespace[256];              // Whitespace table
    static const unsigned char lookup_node_name[256];               // Node name table
    static const unsigned char lookup_text[256];                    // Text table
    static const unsigned char lookup_text_pure_no_ws[256];         // Text table
    static const unsigned char lookup_text_pure_with_ws[256];       // Text table
    static const unsigned char lookup_attribute_name[256];          // Attribute name table
    static const unsigned char lookup_attribute_data_1[256];        // Attribute data table with single quote
    static const unsigned char lookup_attribute_data_1_pure[256];   // Attribute data table with single quote
    static const unsigned char lookup_attribute_data_2[256];        // Attribute data table with double quotes
    static const unsigned char lookup_attribute_data_2_pure[256];   // Attribute data table with double quotes
    static const unsigned char lookup_digits[256];                  // Digits
    static const unsigned char lookup_upcase[256];                  // To uppercase conversion table for ASCII characters
};

// Find length of the string
inline std::size_t measure(const char *p)
{
    const char *tmp = p;
    while (*tmp)
        ++tmp;
    return tmp - p;
}

// Compare strings for equality
inline bool compare(const char *p1, std::size_t size1, const char *p2, std::size_t size2, bool case_sensitive)
{
    if (size1 != size2)
        return false;
    if (case_sensitive)
    {
        for (const char *end = p1 + size1; p1 < end; ++p1, ++p2)
            if (*p1 != *p2)
                return false;
    }
    else
    {
        for (const char *end = p1 + size1; p1 < end; ++p1, ++p2)
            if (lookup_tables<0>::lookup_upcase[static_cast<unsigned char>(*p1)] != lookup_tables<0>::lookup_upcase[static_cast<unsigned char>(*p2)])
                return false;
    }
    return true;
}

// Whitespace (space \n \r \t)
template<int Dummy>
constexpr unsigned char lookup_tables<Dummy>::lookup_whitespace[256] =
        {
                // 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
                0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  0,  0,  1,  0,  0,  // 0
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 1
                1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 2
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 3
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 4
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 5
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 6
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 7
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 8
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 9
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // A
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // B
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // C
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // D
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // E
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0   // F
        };

// Node name (anything but space \n \r \t / > ? \0)
template<int Dummy>
constexpr unsigned char lookup_tables<Dummy>::lookup_node_name[256] =
        {
                // 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
                0,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  1,  1,  0,  1,  1,  // 0
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 1
                0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  // 2
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  // 3
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 4
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 5
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 6
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 7
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 8
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 9
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // A
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // B
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // C
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // D
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // E
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1   // F
        };

// Text (i.e. PCDATA) (anything but < \0)
template<int Dummy>
constexpr unsigned char lookup_tables<Dummy>::lookup_text[256] =
        {
                // 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
                0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 0
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 1
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 2
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  1,  1,  1,  // 3
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 4
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 5
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 6
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 7
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 8
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 9
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // A
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // B
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // C
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // D
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // E
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1   // F
        };

// Text (i.e. PCDATA) that does not require processing when ws normalization is disabled
// (anything but < \0 &)
template<int Dummy>
constexpr unsigned char lookup_tables<Dummy>::lookup_text_pure_no_ws[256] =
        {
                // 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
                0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 0
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 1
                1,  1,  1,  1,  1,  1,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 2
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  1,  1,  1,  // 3
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 4
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 5
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 6
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 7
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 8
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 9
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // A
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // B
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // C
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // D
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // E
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1   // F
        };

// Text (i.e. PCDATA) that does not require processing when ws normalizationis is enabled
// (anything but < \0 & space \n \r \t)
template<int Dummy>
constexpr unsigned char lookup_tables<Dummy>::lookup_text_pure_with_ws[256] =
        {
                // 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
                0,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  1,  1,  0,  1,  1,  // 0
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 1
                0,  1,  1,  1,  1,  1,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 2
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  1,  1,  1,  // 3
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 4
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 5
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 6
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 7
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 8
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 9
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // A
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // B
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // C
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // D
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // E
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1   // F
        };

// Attribute name (anything but space \n \r \t / < > = ? ! \0)
template<int Dummy>
constexpr unsigned char lookup_tables<Dummy>::lookup_attribute_name[256] =
        {
                // 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
                0,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  1,  1,  0,  1,  1,  // 0
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 1
                0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  // 2
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  // 3
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 4
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 5
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 6
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 7
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 8
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 9
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // A
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // B
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // C
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // D
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // E
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1   // F
        };

// Attribute data with single quote (anything but ' \0)
template<int Dummy>
constexpr unsigned char lookup_tables<Dummy>::lookup_attribute_data_1[256] =
        {
                // 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
                0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 0
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 1
                1,  1,  1,  1,  1,  1,  1,  0,  1,  1,  1,  1,  1,  1,  1,  1,  // 2
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 3
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 4
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 5
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 6
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 7
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 8
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 9
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // A
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // B
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // C
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // D
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // E
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1   // F
        };

// Attribute data with single quote that does not require processing (anything but ' \0 &)
template<int Dummy>
constexpr unsigned char lookup_tables<Dummy>::lookup_attribute_data_1_pure[256] =
        {
                // 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
                0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 0
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 1
                1,  1,  1,  1,  1,  1,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  // 2
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 3
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 4
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 5
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 6
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 7
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 8
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 9
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // A
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // B
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // C
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // D
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // E
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1   // F
        };

// Attribute data with double quote (anything but " \0)
template<int Dummy>
constexpr unsigned char lookup_tables<Dummy>::lookup_attribute_data_2[256] =
        {
                // 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
                0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 0
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 1
                1,  1,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 2
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 3
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 4
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 5
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 6
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 7
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 8
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 9
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // A
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // B
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // C
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // D
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // E
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1   // F
        };

// Attribute data with double quote that does not require processing (anything but " \0 &)
template<int Dummy>
constexpr unsigned char lookup_tables<Dummy>::lookup_attribute_data_2_pure[256] =
        {
                // 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
                0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 0
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 1
                1,  1,  0,  1,  1,  1,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 2
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 3
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 4
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 5
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 6
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 7
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 8
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 9
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // A
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // B
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // C
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // D
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // E
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1   // F
        };

// Digits (dec and hex, 255 denotes end of numeric character reference)
template<int Dummy>
constexpr unsigned char lookup_tables<Dummy>::lookup_digits[256] =
        {
                // 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
                255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // 0
                255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // 1
                255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // 2
                0,  1,  2,  3,  4,  5,  6,  7,  8,  9,255,255,255,255,255,255,  // 3
                255, 10, 11, 12, 13, 14, 15,255,255,255,255,255,255,255,255,255,  // 4
                255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // 5
                255, 10, 11, 12, 13, 14, 15,255,255,255,255,255,255,255,255,255,  // 6
                255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // 7
                255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // 8
                255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // 9
                255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // A
                255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // B
                255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // C
                255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // D
                255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // E
                255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255   // F
        };

// Upper case conversion
template<int Dummy>
constexpr unsigned char lookup_tables<Dummy>::lookup_upcase[256] =
        {
                // 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  A   B   C   D   E   F
                0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,   // 0
                16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,   // 1
                32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,   // 2
                48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,   // 3
                64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,   // 4
                80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,   // 5
                96, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,   // 6
                80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 123,124,125,126,127,  // 7
                128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,  // 8
                144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,  // 9
                160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,  // A
                176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,  // B
                192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,  // C
                208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,  // D
                224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,  // E
                240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255   // F
        };
}
//! \endcond

parse_error::parse_error(const char* what, void* where) : m_what(what), m_where(where)
{
}

const char* parse_error::what() const noexcept
{
    return m_what;
}

char* parse_error::where() const
{
    return reinterpret_cast<char*>(m_where);
}

memory_pool::memory_pool(): m_alloc_func(nullptr), m_free_func(nullptr)
{
    init();
}

memory_pool::~memory_pool()
{
    clear();
}

xml_node* memory_pool::allocate_node(node_type type, const char* name, const char* value, std::size_t name_size, std::size_t value_size)
{
    void *memory = allocate_aligned(sizeof(xml_node));
    xml_node *node = new(memory) xml_node(type);
    if (name)
    {
        if (name_size > 0)
            node->name(name, name_size);
        else
            node->name(name);
    }
    if (value)
    {
        if (value_size > 0)
            node->value(value, value_size);
        else
            node->value(value);
    }
    return node;
}

xml_attribute* memory_pool::allocate_attribute(const char* name, const char* value, std::size_t name_size, std::size_t value_size)
{
    void *memory = allocate_aligned(sizeof(xml_attribute));
    xml_attribute *attribute = new(memory) xml_attribute;
    if (name)
    {
        if (name_size > 0)
            attribute->name(name, name_size);
        else
            attribute->name(name);
    }
    if (value)
    {
        if (value_size > 0)
            attribute->value(value, value_size);
        else
            attribute->value(value);
    }
    return attribute;
}

char* memory_pool::allocate_string(const char* source, std::size_t size)
{
    assert(source || size);     // Either source or size (or both) must be specified
    if (size == 0)
        size = internal::measure(source) + 1;
    char *result = static_cast<char *>(allocate_aligned(size * sizeof(char)));
    if (source)
        for (std::size_t i = 0; i < size; ++i)
            result[i] = source[i];
    return result;
}

xml_node* memory_pool::clone_node(const xml_node* source, xml_node* result)
{
    // Prepare result node
    if (result)
    {
        result->remove_all_attributes();
        result->remove_all_nodes();
        result->type(source->type());
    }
    else
        result = allocate_node(source->type());

    // Clone name and value
    result->name(source->name(), source->name_size());
    result->value(source->value(), source->value_size());

    // Clone child nodes and attributes
    for (xml_node *child = source->first_node(); child; child = child->next_sibling())
        result->append_node(clone_node(child));
    for (xml_attribute *attr = source->first_attribute(); attr; attr = attr->next_attribute())
        result->append_attribute(allocate_attribute(attr->name(), attr->value(), attr->name_size(), attr->value_size()));

    return result;
}

void memory_pool::clear()
{
    while (m_begin != m_static_memory)
    {
        char *previous_begin = reinterpret_cast<header *>(align(m_begin))->previous_begin;
        if (m_free_func)
            m_free_func(m_begin);
        else
            delete[] m_begin;
        m_begin = previous_begin;
    }
    init();
}

void memory_pool::set_allocator(memory_pool::alloc_func* af, memory_pool::free_func* ff)
{
    assert(m_begin == m_static_memory && m_ptr == align(m_begin));    // Verify that no memory is allocated yet
    m_alloc_func = af;
    m_free_func = ff;
}

void memory_pool::init()
{
    m_begin = m_static_memory;
    m_ptr = align(m_begin);
    m_end = m_static_memory + sizeof(m_static_memory);
}

char* memory_pool::align(char* ptr)
{
    std::size_t alignment = ((RAPIDXML_ALIGNMENT - (std::size_t(ptr) & (RAPIDXML_ALIGNMENT - 1))) & (RAPIDXML_ALIGNMENT - 1));
    return ptr + alignment;
}

char* memory_pool::allocate_raw(std::size_t size)
{
    // Allocate
    void *memory;
    if (m_alloc_func)   // Allocate memory using either user-specified allocation function or global operator new[]
    {
        memory = m_alloc_func(size);
        assert(memory); // Allocator is not allowed to return 0, on failure it must either throw, stop the program or use longjmp
    }
    else
    {
        memory = new char[size];
#ifdef RAPIDXML_NO_EXCEPTIONS
        if (!memory)            // If exceptions are disabled, verify memory allocation, because new will not be able to throw bad_alloc
                    RAPIDXML_PARSE_ERROR("out of memory", 0);
#endif
    }
    return static_cast<char *>(memory);
}

void* memory_pool::allocate_aligned(std::size_t size)
{
    // Calculate aligned pointer
    char *result = align(m_ptr);

    // If not enough memory left in current pool, allocate a new pool
    if (result + size > m_end)
    {
        // Calculate required pool size (may be bigger than RAPIDXML_DYNAMIC_POOL_SIZE)
        std::size_t pool_size = RAPIDXML_DYNAMIC_POOL_SIZE;
        if (pool_size < size)
            pool_size = size;

        // Allocate
        std::size_t alloc_size = sizeof(header) + (2 * RAPIDXML_ALIGNMENT - 2) + pool_size;     // 2 alignments required in worst case: one for header, one for actual allocation
        char *raw_memory = allocate_raw(alloc_size);

        // Setup new pool in allocated memory
        char *pool = align(raw_memory);
        header *new_header = reinterpret_cast<header *>(pool);
        new_header->previous_begin = m_begin;
        m_begin = raw_memory;
        m_ptr = pool + sizeof(header);
        m_end = raw_memory + alloc_size;

        // Calculate aligned pointer again using new pool
        result = align(m_ptr);
    }

    // Update pool and return aligned pointer
    m_ptr = result + size;
    return result;
}

xml_base::xml_base(): m_name(nullptr), m_value(nullptr) , m_parent(nullptr)
{
}

char* xml_base::name() const
{
    return m_name ? m_name : nullstr();
}

std::size_t xml_base::name_size() const
{
    return m_name ? m_name_size : 0;
}

char* xml_base::value() const
{
    return m_value ? m_value : nullstr();
}

std::size_t xml_base::value_size() const
{
    return m_value ? m_value_size : 0;
}

void xml_base::name(const char* name, std::size_t size)
{
    m_name = const_cast<char *>(name);
    m_name_size = size;
}

void xml_base::name(const char* name)
{
    this->name(name, internal::measure(name));
}

void xml_base::value(const char* value, std::size_t size)
{
    m_value = const_cast<char *>(value);
    m_value_size = size;
}

void xml_base::value(const char* value)
{
    this->value(value, internal::measure(value));
}

xml_node* xml_base::parent() const
{
    return m_parent;
}

char* xml_base::nullstr()
{
    static char zero = '\0';
    return &zero;
}

xml_document* xml_attribute::document() const
{
    if (xml_node *node = this->parent())
    {
        while (node->parent())
            node = node->parent();
        return node->type() == node_document ? static_cast<xml_document *>(node) : 0;
    }
    else
        return nullptr;
}

xml_attribute* xml_attribute::previous_attribute(const char* name, std::size_t name_size, bool case_sensitive) const
{
    if (name)
    {
        if (name_size == 0)
            name_size = internal::measure(name);
        for (xml_attribute *attribute = m_prev_attribute; attribute; attribute = attribute->m_prev_attribute)
            if (internal::compare(attribute->name(), attribute->name_size(), name, name_size, case_sensitive))
                return attribute;
        return 0;
    }
    else
        return this->m_parent ? m_prev_attribute : nullptr;
}

xml_attribute* xml_attribute::next_attribute(const char* name, std::size_t name_size, bool case_sensitive) const
{
    if (name)
    {
        if (name_size == 0)
            name_size = internal::measure(name);
        for (xml_attribute *attribute = m_next_attribute; attribute; attribute = attribute->m_next_attribute)
            if (internal::compare(attribute->name(), attribute->name_size(), name, name_size, case_sensitive))
                return attribute;
        return nullptr;
    }
    else
        return this->m_parent ? m_next_attribute : nullptr;
}

xml_node::xml_node(node_type type): m_type(type), m_first_node(nullptr), m_first_attribute(nullptr)
{
}

node_type xml_node::type() const
{
    return m_type;
}

xml_document* xml_node::document() const
{
    xml_node *node = const_cast<xml_node *>(this);
    while (node->parent())
        node = node->parent();
    return node->type() == node_document ? static_cast<xml_document *>(node) : 0;
}

xml_node* xml_node::first_node(const char* name, std::size_t name_size, bool case_sensitive) const
{
    if (name)
    {
        if (name_size == 0)
            name_size = internal::measure(name);
        for (xml_node *child = m_first_node; child; child = child->next_sibling())
            if (internal::compare(child->name(), child->name_size(), name, name_size, case_sensitive))
                return child;
        return nullptr;
    }
    else
        return m_first_node;
}

xml_node* xml_node::last_node(const char* name, std::size_t name_size, bool case_sensitive) const
{
    assert(m_first_node);  // Cannot query for last child if node has no children
    if (name)
    {
        if (name_size == 0)
            name_size = internal::measure(name);
        for (xml_node *child = m_last_node; child; child = child->previous_sibling())
            if (internal::compare(child->name(), child->name_size(), name, name_size, case_sensitive))
                return child;
        return nullptr;
    }
    else
        return m_last_node;
}

xml_node* xml_node::previous_sibling(const char* name, std::size_t name_size, bool case_sensitive) const
{
    assert(this->m_parent);     // Cannot query for siblings if node has no parent
    if (name)
    {
        if (name_size == 0)
            name_size = internal::measure(name);
        for (xml_node *sibling = m_prev_sibling; sibling; sibling = sibling->m_prev_sibling)
            if (internal::compare(sibling->name(), sibling->name_size(), name, name_size, case_sensitive))
                return sibling;
        return nullptr;
    }
    else
        return m_prev_sibling;
}

xml_node* xml_node::next_sibling(const char* name, std::size_t name_size, bool case_sensitive) const
{
    assert(this->m_parent);     // Cannot query for siblings if node has no parent
    if (name)
    {
        if (name_size == 0)
            name_size = internal::measure(name);
        for (xml_node *sibling = m_next_sibling; sibling; sibling = sibling->m_next_sibling)
            if (internal::compare(sibling->name(), sibling->name_size(), name, name_size, case_sensitive))
                return sibling;
        return 0;
    }
    else
        return m_next_sibling;
}

xml_attribute* xml_node::first_attribute(const char* name, std::size_t name_size, bool case_sensitive) const
{
    if (name)
    {
        if (name_size == 0)
            name_size = internal::measure(name);
        for (xml_attribute *attribute = m_first_attribute; attribute; attribute = attribute->m_next_attribute)
            if (internal::compare(attribute->name(), attribute->name_size(), name, name_size, case_sensitive))
                return attribute;
        return nullptr;
    }
    else
        return m_first_attribute;
}

xml_attribute* xml_node::last_attribute(const char* name, std::size_t name_size, bool case_sensitive) const
{
    if (name)
    {
        if (name_size == 0)
            name_size = internal::measure(name);
        for (xml_attribute *attribute = m_last_attribute; attribute; attribute = attribute->m_prev_attribute)
            if (internal::compare(attribute->name(), attribute->name_size(), name, name_size, case_sensitive))
                return attribute;
        return nullptr;
    }
    else
        return m_first_attribute ? m_last_attribute : nullptr;
}

void xml_node::type(node_type type)
{
    m_type = type;
}

void xml_node::prepend_node(xml_node* child)
{
    assert(child && !child->parent() && child->type() != node_document);
    if (first_node())
    {
        child->m_next_sibling = m_first_node;
        m_first_node->m_prev_sibling = child;
    }
    else
    {
        child->m_next_sibling = nullptr;
        m_last_node = child;
    }
    m_first_node = child;
    child->m_parent = this;
    child->m_prev_sibling = nullptr;
}

void xml_node::append_node(xml_node* child)
{
    assert(child && !child->parent() && child->type() != node_document);
    if (first_node())
    {
        child->m_prev_sibling = m_last_node;
        m_last_node->m_next_sibling = child;
    }
    else
    {
        child->m_prev_sibling = nullptr;
        m_first_node = child;
    }
    m_last_node = child;
    child->m_parent = this;
    child->m_next_sibling = nullptr;
}

void xml_node::insert_node(xml_node* where, xml_node* child)
{
    assert(!where || where->parent() == this);
    assert(child && !child->parent() && child->type() != node_document);
    if (where == m_first_node)
        prepend_node(child);
    else if (where == 0)
        append_node(child);
    else
    {
        child->m_prev_sibling = where->m_prev_sibling;
        child->m_next_sibling = where;
        where->m_prev_sibling->m_next_sibling = child;
        where->m_prev_sibling = child;
        child->m_parent = this;
    }
}

void xml_node::remove_first_node()
{
    assert(first_node());
    xml_node *child = m_first_node;
    m_first_node = child->m_next_sibling;
    if (child->m_next_sibling)
        child->m_next_sibling->m_prev_sibling = nullptr;
    else
        m_last_node = nullptr;
    child->m_parent = nullptr;
}

void xml_node::remove_last_node()
{
    assert(first_node());
    xml_node *child = m_last_node;
    if (child->m_prev_sibling)
    {
        m_last_node = child->m_prev_sibling;
        child->m_prev_sibling->m_next_sibling = nullptr;
    }
    else
        m_first_node = nullptr;
    child->m_parent = nullptr;
}

void xml_node::remove_node(xml_node* where)
{
    assert(where && where->parent() == this);
    assert(first_node());
    if (where == m_first_node)
        remove_first_node();
    else if (where == m_last_node)
        remove_last_node();
    else
    {
        where->m_prev_sibling->m_next_sibling = where->m_next_sibling;
        where->m_next_sibling->m_prev_sibling = where->m_prev_sibling;
        where->m_parent = nullptr;
    }
}

void xml_node::remove_all_nodes()
{
    for (xml_node *node = first_node(); node; node = node->m_next_sibling)
        node->m_parent = nullptr;
    m_first_node = nullptr;
}

void xml_node::prepend_attribute(xml_attribute* attribute)
{
    assert(attribute && !attribute->parent());
    if (first_attribute())
    {
        attribute->m_next_attribute = m_first_attribute;
        m_first_attribute->m_prev_attribute = attribute;
    }
    else
    {
        attribute->m_next_attribute = nullptr;
        m_last_attribute = attribute;
    }
    m_first_attribute = attribute;
    attribute->m_parent = this;
    attribute->m_prev_attribute = nullptr;
}

void xml_node::append_attribute(xml_attribute* attribute)
{
    assert(attribute && !attribute->parent());
    if (first_attribute())
    {
        attribute->m_prev_attribute = m_last_attribute;
        m_last_attribute->m_next_attribute = attribute;
    }
    else
    {
        attribute->m_prev_attribute = nullptr;
        m_first_attribute = attribute;
    }
    m_last_attribute = attribute;
    attribute->m_parent = this;
    attribute->m_next_attribute = nullptr;
}

void xml_node::insert_attribute(xml_attribute* where, xml_attribute* attribute)
{
    assert(!where || where->parent() == this);
    assert(attribute && !attribute->parent());
    if (where == m_first_attribute)
        prepend_attribute(attribute);
    else if (where == 0)
        append_attribute(attribute);
    else
    {
        attribute->m_prev_attribute = where->m_prev_attribute;
        attribute->m_next_attribute = where;
        where->m_prev_attribute->m_next_attribute = attribute;
        where->m_prev_attribute = attribute;
        attribute->m_parent = this;
    }
}

void xml_node::remove_first_attribute()
{
    assert(first_attribute());
    xml_attribute *attribute = m_first_attribute;
    if (attribute->m_next_attribute)
    {
        attribute->m_next_attribute->m_prev_attribute = nullptr;
    }
    else
        m_last_attribute = nullptr;
    attribute->m_parent = nullptr;
    m_first_attribute = attribute->m_next_attribute;
}

void xml_node::remove_last_attribute()
{
    assert(first_attribute());
    xml_attribute *attribute = m_last_attribute;
    if (attribute->m_prev_attribute)
    {
        attribute->m_prev_attribute->m_next_attribute = nullptr;
        m_last_attribute = attribute->m_prev_attribute;
    }
    else
        m_first_attribute = nullptr;
    attribute->m_parent = nullptr;
}

void xml_node::remove_attribute(xml_attribute* where)
{
    assert(first_attribute() && where->parent() == this);
    if (where == m_first_attribute)
        remove_first_attribute();
    else if (where == m_last_attribute)
        remove_last_attribute();
    else
    {
        where->m_prev_attribute->m_next_attribute = where->m_next_attribute;
        where->m_next_attribute->m_prev_attribute = where->m_prev_attribute;
        where->m_parent = nullptr;
    }
}

void xml_node::remove_all_attributes()
{
    for (xml_attribute *attribute = first_attribute(); attribute; attribute = attribute->m_next_attribute)
        attribute->m_parent = nullptr;
    m_first_attribute = nullptr;
}

xml_document::xml_document() : xml_node(node_document)
{
}

template<int Flags>
void xml_document::parse(char* text)
{
    assert(text);

    // Remove current contents
    this->remove_all_nodes();
    this->remove_all_attributes();

    // Parse BOM, if any
    parse_bom<Flags>(text);

    // Parse children
    while (1)
    {
        // Skip whitespace before node
        skip<whitespace_pred, Flags>(text);
        if (*text == 0)
            break;

        // Parse and append new child
        if (*text == '<')
        {
            ++text;     // Skip '<'
            if (xml_node *node = parse_node<Flags>(text))
                this->append_node(node);
        }
        else
            throw parse_error("expected <", text);
    }
}

#ifndef DOXYGEN_SKIP
template void xml_document::parse<0>(char* text);
template void xml_document::parse<parse_no_entity_translation>(char* text);
template void xml_document::parse<parse_no_entity_translation | parse_validate_closing_tags>(char* text);
#endif

void xml_document::clear()
{
    this->remove_all_nodes();
    this->remove_all_attributes();
    memory_pool::clear();
}

unsigned char xml_document::whitespace_pred::test(char ch)
{
    return internal::lookup_tables<0>::lookup_whitespace[static_cast<unsigned char>(ch)];
}

unsigned char xml_document::node_name_pred::test(char ch)
{
    return internal::lookup_tables<0>::lookup_node_name[static_cast<unsigned char>(ch)];
}

unsigned char xml_document::attribute_name_pred::test(char ch)
{
    return internal::lookup_tables<0>::lookup_attribute_name[static_cast<unsigned char>(ch)];
}

unsigned char xml_document::text_pred::test(char ch)
{
    return internal::lookup_tables<0>::lookup_text[static_cast<unsigned char>(ch)];
}

unsigned char xml_document::text_pure_no_ws_pred::test(char ch)
{
    return internal::lookup_tables<0>::lookup_text_pure_no_ws[static_cast<unsigned char>(ch)];
}

unsigned char xml_document::text_pure_with_ws_pred::test(char ch)
{
    return internal::lookup_tables<0>::lookup_text_pure_with_ws[static_cast<unsigned char>(ch)];
}

template<char Quote>
unsigned char xml_document::attribute_value_pred<Quote>::test(char ch)
{
    if (Quote == '\'')
        return internal::lookup_tables<0>::lookup_attribute_data_1[static_cast<unsigned char>(ch)];
    if (Quote == '\"')
        return internal::lookup_tables<0>::lookup_attribute_data_2[static_cast<unsigned char>(ch)];
    return 0;       // Should never be executed, to avoid warnings on Comeau
}

template<char Quote>
unsigned char xml_document::attribute_value_pure_pred<Quote>::test(char ch)
{
    if (Quote == '\'')
        return internal::lookup_tables<0>::lookup_attribute_data_1_pure[static_cast<unsigned char>(ch)];
    if (Quote == '\"')
        return internal::lookup_tables<0>::lookup_attribute_data_2_pure[static_cast<unsigned char>(ch)];
    return 0;       // Should never be executed, to avoid warnings on Comeau
}

template<int Flags>
void xml_document::insert_coded_character(char*& text, unsigned long code)
{
    if (Flags & parse_no_utf8)
    {
        // Insert 8-bit ASCII character
        text[0] = static_cast<unsigned char>(code);
        text += 1;
    }
    else
    {
        // Insert UTF8 sequence
        if (code < 0x80)    // 1 byte sequence
        {
            text[0] = static_cast<unsigned char>(code);
            text += 1;
        }
        else if (code < 0x800)  // 2 byte sequence
        {
            text[1] = static_cast<unsigned char>((code | 0x80) & 0xBF); code >>= 6;
            text[0] = static_cast<unsigned char>(code | 0xC0);
            text += 2;
        }
        else if (code < 0x10000)    // 3 byte sequence
        {
            text[2] = static_cast<unsigned char>((code | 0x80) & 0xBF); code >>= 6;
            text[1] = static_cast<unsigned char>((code | 0x80) & 0xBF); code >>= 6;
            text[0] = static_cast<unsigned char>(code | 0xE0);
            text += 3;
        }
        else if (code < 0x110000)   // 4 byte sequence
        {
            text[3] = static_cast<unsigned char>((code | 0x80) & 0xBF); code >>= 6;
            text[2] = static_cast<unsigned char>((code | 0x80) & 0xBF); code >>= 6;
            text[1] = static_cast<unsigned char>((code | 0x80) & 0xBF); code >>= 6;
            text[0] = static_cast<unsigned char>(code | 0xF0);
            text += 4;
        }
        else    // Invalid, only codes up to 0x10FFFF are allowed in Unicode
        {
            throw parse_error("invalid numeric character entity", text);
        }
    }
}

template<class StopPred, int Flags>
void xml_document::skip(char*& text)
{
    char *tmp = text;
    while (StopPred::test(*tmp))
        ++tmp;
    text = tmp;
}

template<class StopPred, class StopPredPure, int Flags>
char* xml_document::skip_and_expand_character_refs(char*& text)
{
    // If entity translation, whitespace condense and whitespace trimming is disabled, use plain skip
    if (Flags & parse_no_entity_translation &&
        !(Flags & parse_normalize_whitespace) &&
        !(Flags & parse_trim_whitespace))
    {
        skip<StopPred, Flags>(text);
        return text;
    }

    // Use simple skip until first modification is detected
    skip<StopPredPure, Flags>(text);

    // Use translation skip
    char *src = text;
    char *dest = src;
    while (StopPred::test(*src))
    {
        // If entity translation is enabled
        if (!(Flags & parse_no_entity_translation))
        {
            // Test if replacement is needed
            if (src[0] == '&')
            {
                switch (src[1])
                {

                    // &amp; &apos;
                    case 'a':
                        if (src[2] == 'm' && src[3] == 'p' && src[4] == ';')
                        {
                            *dest = '&';
                            ++dest;
                            src += 5;
                            continue;
                        }
                        if (src[2] == 'p' && src[3] == 'o' && src[4] == 's' && src[5] == ';')
                        {
                            *dest = '\'';
                            ++dest;
                            src += 6;
                            continue;
                        }
                        break;

                        // &quot;
                    case 'q':
                        if (src[2] == 'u' && src[3] == 'o' && src[4] == 't' && src[5] == ';')
                        {
                            *dest = '"';
                            ++dest;
                            src += 6;
                            continue;
                        }
                        break;

                        // &gt;
                    case 'g':
                        if (src[2] == 't' && src[3] == ';')
                        {
                            *dest = '>';
                            ++dest;
                            src += 4;
                            continue;
                        }
                        break;

                        // &lt;
                    case 'l':
                        if (src[2] == 't' && src[3] == ';')
                        {
                            *dest = '<';
                            ++dest;
                            src += 4;
                            continue;
                        }
                        break;

                        // &#...; - assumes ASCII
                    case '#':
                        if (src[2] == 'x')
                        {
                            unsigned long code = 0;
                            src += 3;   // Skip &#x
                            while (1)
                            {
                                unsigned char digit = internal::lookup_tables<0>::lookup_digits[static_cast<unsigned char>(*src)];
                                if (digit == 0xFF)
                                    break;
                                code = code * 16 + digit;
                                ++src;
                            }
                            insert_coded_character<Flags>(dest, code);    // Put character in output
                        }
                        else
                        {
                            unsigned long code = 0;
                            src += 2;   // Skip &#
                            while (1)
                            {
                                unsigned char digit = internal::lookup_tables<0>::lookup_digits[static_cast<unsigned char>(*src)];
                                if (digit == 0xFF)
                                    break;
                                code = code * 10 + digit;
                                ++src;
                            }
                            insert_coded_character<Flags>(dest, code);    // Put character in output
                        }
                        if (*src == ';')
                            ++src;
                        else
                            throw parse_error("expected ;", src);
                        continue;

                        // Something else
                    default:
                        // Ignore, just copy '&' verbatim
                        break;

                }
            }
        }

        // If whitespace condensing is enabled
        if (Flags & parse_normalize_whitespace)
        {
            // Test if condensing is needed
            if (whitespace_pred::test(*src))
            {
                *dest = ' '; ++dest;    // Put single space in dest
                ++src;                      // Skip first whitespace char
                // Skip remaining whitespace chars
                while (whitespace_pred::test(*src))
                    ++src;
                continue;
            }
        }

        // No replacement, only copy character
        *dest++ = *src++;

    }

    // Return new end
    text = src;
    return dest;
}

template<int Flags>
void xml_document::parse_bom(char*& text)
{
    // UTF-8?
    if (static_cast<unsigned char>(text[0]) == 0xEF &&
        static_cast<unsigned char>(text[1]) == 0xBB &&
        static_cast<unsigned char>(text[2]) == 0xBF)
    {
        text += 3;      // Skup utf-8 bom
    }
}

template<int Flags>
xml_node* xml_document::parse_xml_declaration(char*& text)
{
    // If parsing of declaration is disabled
    if (!(Flags & parse_declaration_node))
    {
        // Skip until end of declaration
        while (text[0] != '?' || text[1] != '>')
        {
            if (!text[0])
                throw parse_error("unexpected end of data", text);
            ++text;
        }
        text += 2;    // Skip '?>'
        return nullptr;
    }

    // Create declaration
    xml_node *declaration = this->allocate_node(node_declaration);

    // Skip whitespace before attributes or ?>
    skip<whitespace_pred, Flags>(text);

    // Parse declaration attributes
    parse_node_attributes<Flags>(text, declaration);

    // Skip ?>
    if (text[0] != '?' || text[1] != '>')
        throw parse_error("expected ?>", text);
    text += 2;

    return declaration;
}

template<int Flags>
xml_node* xml_document::parse_comment(char*& text)
{
    // If parsing of comments is disabled
    if (!(Flags & parse_comment_nodes))
    {
        // Skip until end of comment
        while (text[0] != '-' || text[1] != '-' || text[2] != '>')
        {
            if (!text[0])
                throw parse_error("unexpected end of data", text);
            ++text;
        }
        text += 3;     // Skip '-->'
        return nullptr;      // Do not produce comment node
    }

    // Remember value start
    char *value = text;

    // Skip until end of comment
    while (text[0] != '-' || text[1] != '-' || text[2] != '>')
    {
        if (!text[0])
            throw parse_error("unexpected end of data", text);
        ++text;
    }

    // Create comment node
    xml_node *comment = this->allocate_node(node_comment);
    comment->value(value, text - value);

    // Place zero terminator after comment value
    if (!(Flags & parse_no_string_terminators))
        *text = '\0';

    text += 3;     // Skip '-->'
    return comment;
}

template<int Flags>
xml_node* xml_document::parse_doctype(char*& text)
{
    // Remember value start
    char *value = text;

    // Skip to >
    while (*text != '>')
    {
        // Determine character type
        switch (*text)
        {

            // If '[' encountered, scan for matching ending ']' using naive algorithm with depth
            // This works for all W3C test files except for 2 most wicked
            case '[':
            {
                ++text;     // Skip '['
                int depth = 1;
                while (depth > 0)
                {
                    switch (*text)
                    {
                        case '[': ++depth; break;
                        case ']': --depth; break;
                        case 0: throw parse_error("unexpected end of data", text);
                    }
                    ++text;
                }
                break;
            }

                // Error on end of text
            case '\0':
                throw parse_error("unexpected end of data", text);

                // Other character, skip it
            default:
                ++text;

        }
    }

    // If DOCTYPE nodes enabled
    if (Flags & parse_doctype_node)
    {
        // Create a new doctype node
        xml_node *doctype = this->allocate_node(node_doctype);
        doctype->value(value, text - value);

        // Place zero terminator after value
        if (!(Flags & parse_no_string_terminators))
            *text = '\0';

        text += 1;      // skip '>'
        return doctype;
    }
    else
    {
        text += 1;      // skip '>'
        return nullptr;
    }
}

template<int Flags>
xml_node* xml_document::parse_pi(char*& text)
{
    // If creation of PI nodes is enabled
    if (Flags & parse_pi_nodes)
    {
        // Create pi node
        xml_node *pi = this->allocate_node(node_pi);

        // Extract PI target name
        char *name = text;
        skip<node_name_pred, Flags>(text);
        if (text == name)
            throw parse_error("expected PI target", text);
        pi->name(name, text - name);

        // Skip whitespace between pi target and pi
        skip<whitespace_pred, Flags>(text);

        // Remember start of pi
        char *value = text;

        // Skip to '?>'
        while (text[0] != '?' || text[1] != '>')
        {
            if (*text == '\0')
                throw parse_error("unexpected end of data", text);
            ++text;
        }

        // Set pi value (verbatim, no entity expansion or whitespace normalization)
        pi->value(value, text - value);

        // Place zero terminator after name and value
        if (!(Flags & parse_no_string_terminators))
        {
            pi->name()[pi->name_size()] = '\0';
            pi->value()[pi->value_size()] = '\0';
        }

        text += 2;                          // Skip '?>'
        return pi;
    }
    else
    {
        // Skip to '?>'
        while (text[0] != '?' || text[1] != '>')
        {
            if (*text == '\0')
                throw parse_error("unexpected end of data", text);
            ++text;
        }
        text += 2;    // Skip '?>'
        return nullptr;
    }
}

template<int Flags>
char xml_document::parse_and_append_data(xml_node* node, char*& text, char* contents_start)
{
    // Backup to contents start if whitespace trimming is disabled
    if (!(Flags & parse_trim_whitespace))
        text = contents_start;

    // Skip until end of data
    char *value = text, *end;
    if (Flags & parse_normalize_whitespace)
        end = skip_and_expand_character_refs<text_pred, text_pure_with_ws_pred, Flags>(text);
    else
        end = skip_and_expand_character_refs<text_pred, text_pure_no_ws_pred, Flags>(text);

    // Trim trailing whitespace if flag is set; leading was already trimmed by whitespace skip after >
    if (Flags & parse_trim_whitespace)
    {
        if (Flags & parse_normalize_whitespace)
        {
            // Whitespace is already condensed to single space characters by skipping function, so just trim 1 char off the end
            if (*(end - 1) == ' ')
                --end;
        }
        else
        {
            // Backup until non-whitespace character is found
            while (whitespace_pred::test(*(end - 1)))
                --end;
        }
    }

    // If characters are still left between end and value (this test is only necessary if normalization is enabled)
    // Create new data node
    if (!(Flags & parse_no_data_nodes))
    {
        xml_node *data = this->allocate_node(node_data);
        data->value(value, end - value);
        node->append_node(data);
    }

    // Add data to parent node if no data exists yet
    if (!(Flags & parse_no_element_values))
        if (*node->value() == '\0')
            node->value(value, end - value);

    // Place zero terminator after value
    if (!(Flags & parse_no_string_terminators))
    {
        char ch = *text;
        *end = '\0';
        return ch;      // Return character that ends data; this is required because zero terminator overwritten it
    }

    // Return character that ends data
    return *text;
}

template<int Flags>
xml_node* xml_document::parse_cdata(char*& text)
{
    // If CDATA is disabled
    if (Flags & parse_no_data_nodes)
    {
        // Skip until end of cdata
        while (text[0] != ']' || text[1] != ']' || text[2] != '>')
        {
            if (!text[0])
                throw parse_error("unexpected end of data", text);
            ++text;
        }
        text += 3;      // Skip ]]>
        return nullptr;       // Do not produce CDATA node
    }

    // Skip until end of cdata
    char *value = text;
    while (text[0] != ']' || text[1] != ']' || text[2] != '>')
    {
        if (!text[0])
            throw parse_error("unexpected end of data", text);
        ++text;
    }

    // Create new cdata node
    xml_node *cdata = this->allocate_node(node_cdata);
    cdata->value(value, text - value);

    // Place zero terminator after value
    if (!(Flags & parse_no_string_terminators))
        *text = '\0';

    text += 3;      // Skip ]]>
    return cdata;
}

template<int Flags>
xml_node* xml_document::parse_element(char*& text)
{
    // Create element node
    xml_node *element = this->allocate_node(node_element);

    // Extract element name
    char *name = text;
    skip<node_name_pred, Flags>(text);
    if (text == name)
        throw parse_error("expected element name", text);
    element->name(name, text - name);

    // Skip whitespace between element name and attributes or >
    skip<whitespace_pred, Flags>(text);

    // Parse attributes, if any
    parse_node_attributes<Flags>(text, element);

    // Determine ending type
    if (*text == '>')
    {
        ++text;
        parse_node_contents<Flags>(text, element);
    }
    else if (*text == '/')
    {
        ++text;
        if (*text != '>')
            throw parse_error("expected >", text);
        ++text;
    }
    else
        throw parse_error("expected >", text);

    // Place zero terminator after name
    if (!(Flags & parse_no_string_terminators))
        element->name()[element->name_size()] = '\0';

    // Return parsed element
    return element;
}

template<int Flags>
xml_node* xml_document::parse_node(char*& text)
{
    // Parse proper node type
    switch (text[0])
    {

        // <...
        default:
            // Parse and append element node
            return parse_element<Flags>(text);

            // <?...
        case '?':
            ++text;     // Skip ?
            if ((text[0] == 'x' || text[0] == 'X') &&
                (text[1] == 'm' || text[1] == 'M') &&
                (text[2] == 'l' || text[2] == 'L') &&
                whitespace_pred::test(text[3]))
            {
                // '<?xml ' - xml declaration
                text += 4;      // Skip 'xml '
                return parse_xml_declaration<Flags>(text);
            }
            else
            {
                // Parse PI
                return parse_pi<Flags>(text);
            }

            // <!...
        case '!':

            // Parse proper subset of <! node
            switch (text[1])
            {

                // <!-
                case '-':
                    if (text[2] == '-')
                    {
                        // '<!--' - xml comment
                        text += 3;     // Skip '!--'
                        return parse_comment<Flags>(text);
                    }
                    break;

                    // <![
                case '[':
                    if (text[2] == 'C' && text[3] == 'D' && text[4] == 'A' &&
                        text[5] == 'T' && text[6] == 'A' && text[7] == '[')
                    {
                        // '<![CDATA[' - cdata
                        text += 8;     // Skip '![CDATA['
                        return parse_cdata<Flags>(text);
                    }
                    break;

                    // <!D
                case 'D':
                    if (text[2] == 'O' && text[3] == 'C' && text[4] == 'T' &&
                        text[5] == 'Y' && text[6] == 'P' && text[7] == 'E' &&
                        whitespace_pred::test(text[8]))
                    {
                        // '<!DOCTYPE ' - doctype
                        text += 9;      // skip '!DOCTYPE '
                        return parse_doctype<Flags>(text);
                    }

            }   // switch

            // Attempt to skip other, unrecognized node types starting with <!
            ++text;     // Skip !
            while (*text != '>')
            {
                if (*text == 0)
                    throw parse_error("unexpected end of data", text);
                ++text;
            }
            ++text;     // Skip '>'
            return nullptr;   // No node recognized

    }
}

template<int Flags>
void xml_document::parse_node_contents(char*& text, xml_node* node)
{
    // For all children and text
    while(true)
    {
        // Skip whitespace between > and node contents
        char *contents_start = text;      // Store start of node contents before whitespace is skipped
        skip<whitespace_pred, Flags>(text);
        char next_char = *text;

        // After data nodes, instead of continuing the loop, control jumps here.
        // This is because zero termination inside parse_and_append_data() function
        // would wreak havoc with the above code.
        // Also, skipping whitespace after data nodes is unnecessary.
        after_data_node:

        // Determine what comes next: node closing, child node, data node, or 0?
        switch (next_char)
        {

            // Node closing or child node
            case '<':
                if (text[1] == '/')
                {
                    // Node closing
                    text += 2;      // Skip '</'
                    if (Flags & parse_validate_closing_tags)
                    {
                        // Skip and validate closing tag name
                        char *closing_name = text;
                        skip<node_name_pred, Flags>(text);
                        if (!internal::compare(node->name(), node->name_size(), closing_name, text - closing_name, true))
                            throw parse_error("invalid closing tag name", text);
                    }
                    else
                    {
                        // No validation, just skip name
                        skip<node_name_pred, Flags>(text);
                    }
                    // Skip remaining whitespace after node name
                    skip<whitespace_pred, Flags>(text);
                    if (*text != '>')
                        throw parse_error("expected >", text);
                    ++text;     // Skip '>'
                    return;     // Node closed, finished parsing contents
                }
                else
                {
                    // Child node
                    ++text;     // Skip '<'
                    if (xml_node *child = parse_node<Flags>(text))
                        node->append_node(child);
                }
                break;

                // End of data - error
            case '\0':
                throw parse_error("unexpected end of data", text);

                // Data node
            default:
                next_char = parse_and_append_data<Flags>(node, text, contents_start);
                goto after_data_node;   // Bypass regular processing after data nodes

        }
    }
}

template<int Flags>
void xml_document::parse_node_attributes(char*& text, xml_node* node)
{
// For all attributes
    while (attribute_name_pred::test(*text))
    {
        // Extract attribute name
        char *name = text;
        ++text;     // Skip first character of attribute name
        skip<attribute_name_pred, Flags>(text);
        if (text == name)
            throw parse_error("expected attribute name", name);

        // Create new attribute
        xml_attribute *attribute = this->allocate_attribute();
        attribute->name(name, text - name);
        node->append_attribute(attribute);

        // Skip whitespace after attribute name
        skip<whitespace_pred, Flags>(text);

        // Skip =
        if (*text != '=')
            throw parse_error("expected =", text);
        ++text;

        // Add terminating zero after name
        if (!(Flags & parse_no_string_terminators))
            attribute->name()[attribute->name_size()] = 0;

        // Skip whitespace after =
        skip<whitespace_pred, Flags>(text);

        // Skip quote and remember if it was ' or "
        char quote = *text;
        if (quote != '\'' && quote != '"')
            throw parse_error("expected ' or \"", text);
        ++text;

        // Extract attribute value and expand char refs in it
        char *value = text, *end;
        const int AttFlags = Flags & ~parse_normalize_whitespace;   // No whitespace normalization in attributes
        if (quote == '\'')
            end = skip_and_expand_character_refs<attribute_value_pred<'\''>, attribute_value_pure_pred<'\''>, AttFlags>(text);
        else
            end = skip_and_expand_character_refs<attribute_value_pred<'"'>, attribute_value_pure_pred<'"'>, AttFlags>(text);

        // Set attribute value
        attribute->value(value, end - value);

        // Make sure that end quote is present
        if (*text != quote)
            throw parse_error("expected ' or \"", text);
        ++text;     // Skip quote

        // Add terminating zero after value
        if (!(Flags & parse_no_string_terminators))
            attribute->value()[attribute->value_size()] = 0;

        // Skip whitespace after attribute value
        skip<whitespace_pred, Flags>(text);
    }
}

}