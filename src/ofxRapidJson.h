#pragma once

#include <string>

#include "lib/rapidjson/document.h"
#include "lib/rapidjson/error/error.h"
#include "lib/rapidjson/error/en.h"
#include "lib/rapidjson/pointer.h"
#include "lib/rapidjson/istreamwrapper.h"
#include "lib/rapidjson/ostreamwrapper.h"
#include "lib/rapidjson/reader.h"
#include "lib/rapidjson/writer.h"
#include "lib/rapidjson/prettywriter.h"
#include "lib/rapidjson/stringbuffer.h"

#include "ofFileUtils.h"
#include "ofLog.h"

/*////////////////// ofxPrettyJsonWriter //////////////*/

class ofxPrettyJsonWriter {
public:
    ofxPrettyJsonWriter();
    ofxPrettyJsonWriter(const ofxPrettyJsonWriter&) = delete;
    ~ofxPrettyJsonWriter();
    ofxPrettyJsonWriter& operator=(const ofxPrettyJsonWriter&) = delete;

    template<typename T>
    ofxPrettyJsonWriter& operator<<(T&& val);

    void reset();
    bool saveToBuffer(char* data, size_t size);
    bool saveToBuffer(string& buffer);
    bool saveToBuffer(ofBuffer& buffer);
    bool saveToFile(const string& path);

    ofxPrettyJsonWriter& startObject();
    ofxPrettyJsonWriter& endObject();
    ofxPrettyJsonWriter& startArray();
    ofxPrettyJsonWriter& endArray();
    ofxPrettyJsonWriter& addKey(const string& s);
    ofxPrettyJsonWriter& addString(const string& s);
    template<typename T>
    ofxPrettyJsonWriter& addNumber(T n);
    ofxPrettyJsonWriter& addBool(bool b);
    ofxPrettyJsonWriter& addNull();
    template<typename T>
    ofxPrettyJsonWriter& addArray(const vector<T>& vec);
    template<typename T>
    ofxPrettyJsonWriter& addObject(const unordered_map<string, T>& map);
protected:
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer_;
    rapidjson::StringBuffer buffer_;
    void doAdd(uint32_t n);
    void doAdd(int32_t n);
    void doAdd(uint64_t n);
    void doAdd(int64_t n);
    void doAdd(float n);
    void doAdd(double n);
    void doAdd(const string& s);
    template<int N>
    void doAdd(const char(&s)[N]); // for string literals
    void doAdd(bool b);
    template<typename T>
    void doAdd(const vector<T>& vec);
    template<typename T>
    void doAdd(const unordered_map<string, T>& map);
};

class ofxJsonDocument;
class ofxJsonValueRef;
class ofxJsonArrayRef;
class ofxJsonObjectRef;
struct ofxJsonMemberRef;

enum ofxJsonValueType {
    OFX_JSON_BOOL,
    OFX_JSON_NUMBER,
    OFX_JSON_STRING,
    OFX_JSON_ARRAY,
    OFX_JSON_OBJECT,
    OFX_JSON_NULL
};

/*////////////////// ofxJsonIterator /////////////*/

/// wraps rapidjson GenericIterators.

template <typename IteratorType, typename ReferenceType, typename AllocatorType>
class ofxJsonIterator {
    friend class ofxJsonArrayRef;
    friend class ofxJsonObjectRef;
public:
    ofxJsonIterator(IteratorType ptr, AllocatorType& allocator)
        : ptr_(ptr), allocator_(&allocator) {}
    ofxJsonIterator(const ofxJsonIterator& mom)
        : ptr_(mom.ptr_), allocator_(mom.allocator_) {}
    ~ofxJsonIterator() {}

    ofxJsonIterator& operator=(const ofxJsonIterator& other){ptr_ = other.ptr_; allocator_ = other.allocator_; return *this;}

    ofxJsonIterator& operator++(){ ++ptr_; return *this; }
    ofxJsonIterator& operator--(){ --ptr_; return *this; }
    ofxJsonIterator  operator++(int){ ofxJsonIterator old(*this); ++ptr_; return old; }
    ofxJsonIterator  operator--(int){ ofxJsonIterator old(*this); --ptr_; return old; }

    ofxJsonIterator operator+(int n) const { return ofxJsonIterator(ptr_+n, *allocator_); }
    ofxJsonIterator operator-(int n) const { return ofxJsonIterator(ptr_-n, *allocator_); }

    ofxJsonIterator& operator+=(int n) { ptr_+=n; return *this; }
    ofxJsonIterator& operator-=(int n) { ptr_-=n; return *this; }

    bool operator==(const ofxJsonIterator& that) const { return ptr_ == that.ptr_; }
    bool operator!=(const ofxJsonIterator& that) const { return ptr_ != that.ptr_; }
    bool operator<=(const ofxJsonIterator& that) const { return ptr_ <= that.ptr_; }
    bool operator>=(const ofxJsonIterator& that) const { return ptr_ >= that.ptr_; }
    bool operator< (const ofxJsonIterator& that) const { return ptr_ < that.ptr_; }
    bool operator> (const ofxJsonIterator& that) const { return ptr_ > that.ptr_; }

    ReferenceType operator*() const { return ReferenceType(*ptr_, *allocator_); }
    ReferenceType operator->() const { return ReferenceType(*ptr_, *allocator_); } // forwards to ReferenceType::operator->()
    ReferenceType operator[](size_t n) const { return ReferenceType(ptr_[n], *allocator_); }

    int operator-(const ofxJsonIterator& that) const { return ptr_-that.ptr_; }
private:
    IteratorType ptr_;
    AllocatorType* allocator_; // needs to be pointer to make assignment operator work correctly
};

using ofxJsonValueIterator = ofxJsonIterator<rapidjson::Value::ValueIterator, ofxJsonValueRef, rapidjson::Document::AllocatorType>;
using ofxJsonMemberIterator = ofxJsonIterator<rapidjson::Value::MemberIterator, ofxJsonMemberRef, rapidjson::Document::AllocatorType>;


/*///////////// ofxJsonDocument ////////////////*/

class ofxJsonDocument {
public:
    ofxJsonDocument();
    ofxJsonDocument(const ofxJsonDocument& mom);
    ofxJsonDocument(ofxJsonDocument&& mom);
    ~ofxJsonDocument();

    ofxJsonDocument& operator =(const ofxJsonDocument& mom);
    ofxJsonDocument& operator =(ofxJsonDocument&& mom);

    /// load JSON data
    bool loadFromFile(const string& path);
    bool loadFromBuffer(const char *data, size_t size);
    bool loadFromBuffer(const string& buffer);
    bool loadFromBuffer(const ofBuffer& buffer);

    /// save JSON data
    bool saveToFile(const string& path, bool pretty = true);
    bool saveToBuffer(char *data, size_t size, bool pretty = true);
    bool saveToBuffer(string& buffer, bool pretty = true);
    bool saveToBuffer(ofBuffer& buffer, bool pretty = true);

    /// does a key exist?
    /// return an ofxJsonValueIterator to the found Value
    /// or to document.end() if it doesn't exist.
    ///
    /// keys are rapidjson "Pointers", such as "/foo/bar".
    /// note that you always need a leading '/'!
    ofxJsonValueIterator find(const string& key);
    /// get value reference by key.
    /// if the key doesn't exist, create it.
    ofxJsonValueRef operator[](const string& key);

    /// return Iterator past 'end' of Document (used to check the result of ofxJsonDocument::find())
    ofxJsonValueIterator end();

    /// get root value reference
    ofxJsonValueRef getRoot();

    /// get the actual rapidjson::Document by reference
    /// to allow direct manipulation via the original rapidjson API
    rapidjson::Document& getDocument();
//    const rapidjson::Document& getDocument() const;
protected:
    rapidjson::Document document_;
    void printError(rapidjson::ParseErrorCode error, size_t offset);  
};

/*///////////// ofxJsonValueRef ////////////////////////////*/

/// helper class which wraps a rapidjson::Value reference together with an allocater reference
/// around a new, simpler interface.
/// The allocator reference points to the allocator of the document the value belongs to.
/// This allows as to construct/change advanced value types such as strings, objects and arrays.
class ofxJsonValueRef {
    friend class ofxJsonArrayRef;
    friend class ofxJsonObjectRef;
public:
    /// constructors:
    ofxJsonValueRef(rapidjson::Value& ref, rapidjson::Document::AllocatorType& allocator);
    ofxJsonValueRef(const ofxJsonValueRef& mom);
    ~ofxJsonValueRef();

    /// copy and move assignment
    ///
    /// since this is a reference wrapper, assignment doesn't change the reference itself
    /// (this wouldn't be possible anyway) but the value it points to!
    ofxJsonValueRef& operator=(const ofxJsonValueRef& other);
    /// the assignment operator lets us assign values to the Value reference:
    ///
    /// document["/key"] = 4.7
    ///
    /// template for primitive types:
    template<typename T>
    ofxJsonValueRef& operator=(T value);
    /// catches string literals (without copying):
    template<int N>
    ofxJsonValueRef& operator=(const char(&s)[N]);
    /// catches std::string (makes a copy):
    ofxJsonValueRef& operator=(const string& s);

    /// assign from std::vector (create Array):
    template<typename T>
    ofxJsonValueRef& operator=(const vector<T>& vec);
    /// catches std::string vectors
    ofxJsonValueRef& operator=(const vector<string>& vec);
    /// assign from Array reference
    ofxJsonValueRef& operator=(const ofxJsonArrayRef& array);

    /// assign from std::unordered_map (create Object)
    template <typename T>
    ofxJsonValueRef& operator=(const unordered_map<string, T>& map);
    /// if value is string
    ofxJsonValueRef& operator=(const unordered_map<string, string>& map);
    /// assign from Object reference
    ofxJsonValueRef& operator=(const ofxJsonObjectRef& object);

    /// set Value (Number, String, Null)
    template<typename T>
    ofxJsonValueRef& setValue(T&& value);
    /// set to Null
    ofxJsonValueRef& setNull();
    /// set to empty Array
    ofxJsonArrayRef setArray();
    /// set from another Array
    ofxJsonArrayRef setArray(const ofxJsonArrayRef& array);
    /// set to Array from std::vector
    template<typename T>
    ofxJsonArrayRef setArray(const vector<T>& vec);
    /// set to empty Object
    ofxJsonObjectRef setObject();
    /// set from another Object
    ofxJsonObjectRef setObject(const ofxJsonObjectRef& object);
    /// set to Object from a std::unordered_map
    template <typename T>
    ofxJsonObjectRef setObject(const unordered_map<string, T>& map);

    /// get type info
    ofxJsonValueType getType() const;
    bool isBool() const;
    bool isNumber() const;
    bool isString() const;
    bool isArray() const;
    bool isObject() const;
    /// test if two ofxJsonValueRefs point to the same rapidjson::Value
    bool operator==(const ofxJsonValueRef& other);
    bool operator!=(const ofxJsonValueRef& other);

    /// explicit getters
    bool getBool() const;
    int getInt() const;
    float getFloat() const;
    double getDouble() const;
    string getString() const;
    vector<bool> getBoolVector() const;
    vector<int> getIntVector() const;
    vector<float> getFloatVector() const;
    vector<double> getDoubleVector() const;
    vector<string> getStringVector() const;
    unordered_map<string, bool> getBoolMap() const;
    unordered_map<string, int> getIntMap() const;
    unordered_map<string, float> getFloatMap() const;
    unordered_map<string, double> getDoubleMap() const;
    unordered_map<string, string> getStringMap() const;

    ofxJsonArrayRef getArray() const;
    ofxJsonObjectRef getObject() const;
    /// type casts (implicit getters)
    operator bool() const;
    operator int() const;
    operator float() const;
    operator double() const;
    operator string() const;
    operator vector<bool>() const;
    operator vector<int>() const;
    operator vector<float>() const;
    operator vector<double>() const;
    operator vector<string>() const;
    operator unordered_map<string, bool>() const;
    operator unordered_map<string, int>() const;
    operator unordered_map<string, float>() const;
    operator unordered_map<string, double>() const;
    operator unordered_map<string, string>() const;

    ofxJsonValueRef* operator->(); // needed for ofxJsonValueIterator::operator->()
protected:
    template<typename T>
    vector<T> getVector() const; // helper function
    template<typename T>
    unordered_map<string, T> getMap() const; // helper function
    rapidjson::Value& value_;
    rapidjson::Document::AllocatorType& allocator_;
};

/*///////////// ofxJsonArrayRef ////////////////////////////*/

class ofxJsonArrayRef {
    friend class ofxJsonValueRef;
public:
    ofxJsonArrayRef(const ofxJsonValueRef& value);
    ofxJsonArrayRef(const ofxJsonArrayRef& mom);
    ~ofxJsonArrayRef();

    ofxJsonArrayRef& operator=(const ofxJsonArrayRef& other);
    template<typename T>
    ofxJsonArrayRef& operator=(const vector<T>& vec);

    ofxJsonArrayRef& setArray(const ofxJsonArrayRef& other);
    template<typename T>
    ofxJsonArrayRef& setArray(const vector<T>& vec);

    ofxJsonValueRef operator[](size_t index) const;

    size_t size() const;
    bool empty() const;
    size_t capacity() const;
    void reserve(size_t n);
    void resize(size_t n);
    void clear();

    ofxJsonValueIterator begin() const;
    ofxJsonValueIterator end() const;
    ofxJsonValueRef front() const;
    ofxJsonValueRef back() const;

    template<typename T>
    void push_back(T&& value);
    void push_back();
    void pop_back();
    ofxJsonValueIterator erase(const ofxJsonValueIterator& pos);
    ofxJsonValueIterator erase(const ofxJsonValueIterator& first, const ofxJsonValueIterator& last);

    vector<bool> getBoolVector() const;
    vector<int> getIntVector() const;
    vector<float> getFloatVector() const;
    vector<double> getDoubleVector() const;
    vector<string> getStringVector() const;

    ofxJsonValueRef getValue() const;

    operator vector<bool>() const;
    operator vector<int>() const;
    operator vector<float>() const;
    operator vector<double>() const;
    operator vector<string>() const;
protected:
    template<typename T>
    vector<T> getVector() const; // helper function
    ofxJsonValueRef valueRef_;
};

/*///////////// ofxJsonObjectRef ////////////////////////////*/

class ofxJsonObjectRef {
    friend class ofxJsonValueRef;
public:
    ofxJsonObjectRef(const ofxJsonValueRef& value);
    ofxJsonObjectRef(const ofxJsonObjectRef& mom);
    ~ofxJsonObjectRef();

    ofxJsonObjectRef& operator=(const ofxJsonObjectRef& other);
    template<typename T>
    ofxJsonObjectRef& operator=(const unordered_map<string, T>& map);

    ofxJsonObjectRef& setObject(const ofxJsonObjectRef& other);
    template<typename T>
    ofxJsonObjectRef& setObject(const unordered_map<string, T>& map);

    /// return a reference to a member value.
    /// if the name doesn't exist yet, a new member (with value = Null) is inserted.
    ofxJsonValueRef operator[](const string& name) const;
    /// search for a member by name. return an iterator to the found member
    /// or to ofxJsonObjectRef::end() if it doesn't exist.
    ofxJsonMemberIterator find(const string& name) const;
    /// ask if a member exists
    int count(const string& name) const;

    size_t size() const;
    bool empty() const;
    void clear();

    ofxJsonMemberIterator begin() const;
    ofxJsonMemberIterator end() const;

    template<typename T>
    void insert(const string& name, T&& value);
    void insert(const string& name);
    ofxJsonMemberIterator erase(const string& name);
    ofxJsonMemberIterator erase(const ofxJsonMemberIterator& pos);
    ofxJsonMemberIterator erase(const ofxJsonMemberIterator& first, const ofxJsonMemberIterator& last);

    unordered_map<string, bool> getBoolMap() const;
    unordered_map<string, int> getIntMap() const;
    unordered_map<string, float> getFloatMap() const;
    unordered_map<string, double> getDoubleMap() const;
    unordered_map<string, string> getStringMap() const;

    ofxJsonValueRef getValue() const;

    operator unordered_map<string, bool>() const;
    operator unordered_map<string, int>() const;
    operator unordered_map<string, float>() const;
    operator unordered_map<string, double>() const;
    operator unordered_map<string, string>() const;
protected:
    template<typename T>
    unordered_map<string, T> getMap() const; // helper function
    ofxJsonValueRef valueRef_;
};

/*////////////////// ofxJsonMemberRef /////////////////*/

struct ofxJsonMemberRef {
    ofxJsonMemberRef(rapidjson::Value::Member& ref, rapidjson::Document::AllocatorType& allocator)
        : name(ref.name, allocator), value(ref.value, allocator) {}
    ofxJsonMemberRef(const ofxJsonMemberRef& mom)
        : name(mom.name), value(mom.value) {}
    ~ofxJsonMemberRef() {}

    ofxJsonMemberRef& operator=(const ofxJsonMemberRef& other){
        name = other.name; value = other.value; // copy name and value from other to this
    }

    ofxJsonMemberRef* operator->() {return this;} // needed by ofxJsonMemberIterator

    ofxJsonValueRef name;
    ofxJsonValueRef value;
};

/*//////////////// Implementation /////////////////////*/

#include "ofxRapidJsonImp.h"
