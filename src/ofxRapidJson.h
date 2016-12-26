#pragma once

#include <string>

#include "lib/rapidjson/document.h"
#include "lib/rapidjson/pointer.h"
#include "lib/rapidjson/istreamwrapper.h"
#include "lib/rapidjson/ostreamwrapper.h"
#include "lib/rapidjson/reader.h"
#include "lib/rapidjson/writer.h"
#include "lib/rapidjson/prettywriter.h"
#include "lib/rapidjson/stringbuffer.h"

#include "ofFileUtils.h"

/*////////////////// ofxJsonWriter //////////////*/

class ofxJsonWriter {
public:
    ofxJsonWriter();
    ofxJsonWriter(const ofxJsonWriter&) = delete;
    ~ofxJsonWriter();
protected:

};


// namespace ofxRapidJson {

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
    void loadFromBuffer(const string& buffer);
    void loadFromBuffer(const ofBuffer& buffer);

    /// save JSON data
    bool saveToFile(const string& path);
    void saveToBuffer(string& buffer);
    void saveToBuffer(ofBuffer& buffer);

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

    /// return Iterator past end
    ofxJsonValueIterator end();

    /// get root value reference
    ofxJsonValueRef getRoot();

    /// get the actual rapidjson::Document by reference
    /// to allow direct manipulation via the original rapidjson API
    rapidjson::Document& getDocument();
//    const rapidjson::Document& getDocument() const;
protected:
    rapidjson::Document document_;
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




/// IMPLEMENTATION ///

/*///////////// ofxJsonDocument ////////////////////*/

/// constructors
inline ofxJsonDocument::ofxJsonDocument()
    : document_() {}

inline ofxJsonDocument::ofxJsonDocument(const ofxJsonDocument& mom)
    : document_() {
    document_.CopyFrom(mom.document_, document_.GetAllocator());
}

inline ofxJsonDocument::ofxJsonDocument(ofxJsonDocument&& mom)
    : document_(std::move(mom.document_)) {}

inline ofxJsonDocument::~ofxJsonDocument() {}

/// assignment
inline ofxJsonDocument& ofxJsonDocument::operator =(const ofxJsonDocument& mom){
    if (this != &mom){
        document_.CopyFrom(mom.document_, document_.GetAllocator());
    }
    return *this;
}

inline ofxJsonDocument& ofxJsonDocument::operator =(ofxJsonDocument&& mom){
    if (this != &mom){
        document_ = std::move(mom.document_);
    }

    return *this;
}

/// loading JSON data
inline bool ofxJsonDocument::loadFromFile(const string& path){
    ifstream ifs(path);

    if (!ifs.is_open()){
        return false;
    }

    rapidjson::IStreamWrapper isw(ifs);
    document_.ParseStream(isw);

    return true;
}

inline void ofxJsonDocument::loadFromBuffer(const string& buffer){
    document_.Parse(buffer.data(), buffer.size());
}

inline void ofxJsonDocument::loadFromBuffer(const ofBuffer& buffer){
    document_.Parse(buffer.getData(), buffer.size());
}

/// saving JSON data
inline bool ofxJsonDocument::saveToFile(const string& path){
    ofstream ofs(path);

    if (!ofs.is_open()){
        return false;
    }

    rapidjson::OStreamWrapper osw(ofs);
    rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);

    return (document_.Accept(writer));
}

inline void ofxJsonDocument::saveToBuffer(string& buffer){
    rapidjson::StringBuffer stringBuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(stringBuf);
    document_.Accept(writer);

    buffer.assign(stringBuf.GetString(), stringBuf.GetSize());
}

inline void ofxJsonDocument::saveToBuffer(ofBuffer& buffer){
    rapidjson::StringBuffer stringBuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(stringBuf);
    document_.Accept(writer);

    buffer.set(stringBuf.GetString(), stringBuf.GetSize());
}

/// find value by key
inline ofxJsonValueIterator ofxJsonDocument::find(const string& key){
    rapidjson::Pointer ptr(key.data(), key.size());
    return ofxJsonValueIterator(ptr.Get(document_), document_.GetAllocator());
}

/// find value by key
inline ofxJsonValueIterator ofxJsonDocument::end(){
    return ofxJsonValueIterator(nullptr, document_.GetAllocator());
}

inline ofxJsonValueRef ofxJsonDocument::operator[](const string& key){
    rapidjson::Pointer ptr(key.data(), key.size());
    rapidjson::Value* value = ptr.Get(document_);
    if (value){
        return ofxJsonValueRef(*value, document_.GetAllocator());
    } else {
        return ofxJsonValueRef(ptr.Create(document_), document_.GetAllocator());
    }
}

/// get root value reference
inline ofxJsonValueRef ofxJsonDocument::getRoot(){
    return ofxJsonValueRef(document_, document_.GetAllocator());
}

/// get document
inline rapidjson::Document& ofxJsonDocument::getDocument(){
    return document_;
}


/*///////////////////// ofxJsonValueRef /////////////////*/

/// constructors:
inline ofxJsonValueRef::ofxJsonValueRef(rapidjson::Value& ref, rapidjson::Document::AllocatorType& allocator)
    : value_(ref), allocator_(allocator) {}

inline ofxJsonValueRef::ofxJsonValueRef(const ofxJsonValueRef& mom)
    : value_(mom.value_), allocator_(mom.allocator_) {}

inline ofxJsonValueRef::~ofxJsonValueRef() {}

/// copy assignment
inline ofxJsonValueRef& ofxJsonValueRef::operator=(const ofxJsonValueRef& other){
    if (this != &other){
        value_.CopyFrom(other.value_, allocator_); // copy value explicitly
    }
    return *this;
}

/// implicit setters
///
/// for plain types
template<typename T>
inline ofxJsonValueRef& ofxJsonValueRef::operator=(T value){
    value_ = value;
    return *this;
}
/// for string literal
template<int N>
inline ofxJsonValueRef& ofxJsonValueRef::operator=(const char(&s)[N]){
    value_.SetString(s, N-1); // we don't want to store the \0 character
    return *this;
}
/// for std::string
inline ofxJsonValueRef& ofxJsonValueRef::operator=(const string& s){
    value_.SetString(s.data(), s.length(), allocator_); // make copy via allocator!
    return *this;
}
/// for vectors (set to Array)
template<typename T>
inline ofxJsonValueRef& ofxJsonValueRef::operator=(const vector<T>& vec){
    value_.SetArray();

    for (auto& k : vec){
        value_.PushBack(rapidjson::Value(k), allocator_);
    }
    return *this;
}
/// for string vectors
inline ofxJsonValueRef& ofxJsonValueRef::operator=(const vector<string>& vec){
    value_.SetArray();

    for (auto& s : vec){
        value_.PushBack(rapidjson::Value(s.data(), s.length(), allocator_), allocator_);
    }
    return *this;
}
/// from Array reference
inline ofxJsonValueRef& ofxJsonValueRef::operator=(const ofxJsonArrayRef& array) {
    return operator=(array.valueRef_);
}

/// for maps (set to Object)
template<typename T>
inline ofxJsonValueRef& ofxJsonValueRef::operator=(const unordered_map<string, T>& map){
    value_.SetObject();

    for (auto& k : map){
        rapidjson::Value name(k.first.data(), k.first.size(), allocator_);
        rapidjson::Value value(k.second);
        value_.AddMember(name.Move(), value.Move(), allocator_);
    }
    return *this;
}
/// for string maps
inline ofxJsonValueRef& ofxJsonValueRef::operator=(const unordered_map<string, string>& map){
    value_.SetObject();

    for (auto& k : map){
        rapidjson::Value name(k.first.data(), k.first.size(), allocator_);
        rapidjson::Value value(k.second.data(), k.second.size(), allocator_);
        value_.AddMember(name.Move(), value.Move(), allocator_);
    }
    return *this;
}
/// from Object reference
inline ofxJsonValueRef& ofxJsonValueRef::operator=(const ofxJsonObjectRef& object) {
    return operator=(object.valueRef_);
}

/// explicit setters
template<typename T>
inline ofxJsonValueRef& ofxJsonValueRef::setValue(T&& value){
    return operator=(forward<T>(value));
}


inline ofxJsonValueRef& ofxJsonValueRef::setNull(){
    value_.SetNull();
    return *this;
}

inline ofxJsonArrayRef ofxJsonValueRef::setArray() {
    value_.SetArray();
    return ofxJsonArrayRef(*this);
}

inline ofxJsonArrayRef ofxJsonValueRef::setArray(const ofxJsonArrayRef& array) {
    return ofxJsonArrayRef(operator=(array));
}

template<typename T>
inline ofxJsonArrayRef ofxJsonValueRef::setArray(const vector<T>& vec){
    return ofxJsonArrayRef(operator=(vec)); // forward vector reference to assignment operator
}

inline ofxJsonObjectRef ofxJsonValueRef::setObject() {
    value_.SetObject();
    return ofxJsonObjectRef(*this);
}

inline ofxJsonObjectRef ofxJsonValueRef::setObject(const ofxJsonObjectRef& object) {
    return ofxJsonObjectRef(operator=(object));
}

template<typename T>
inline ofxJsonObjectRef ofxJsonValueRef::setObject(const unordered_map<string, T>& map){
    return ofxJsonObjectRef(operator=(map)); // forward vector reference to assignment operator
}

/// get type info
inline ofxJsonValueType ofxJsonValueRef::getType() const {
    if (value_.IsBool()){
        return OFX_JSON_BOOL;
    } else if (value_.IsNumber()){
        return OFX_JSON_NUMBER;
    } else if (value_.IsString()){
        return OFX_JSON_STRING;
    } else if (value_.IsArray()){
        return OFX_JSON_ARRAY;
    } else if (value_.IsObject()){
        return OFX_JSON_OBJECT;
    } else if (value_.IsNull()){
        return OFX_JSON_NULL;
    } else {
        cout << "Unknown JSON type!\n";
    }
}
inline bool ofxJsonValueRef::isBool() const{
    return value_.IsBool();
}
inline bool ofxJsonValueRef::isNumber() const{
    return value_.IsNumber();
}
inline bool ofxJsonValueRef::isString() const{
    return value_.IsString();
}
inline bool ofxJsonValueRef::isArray() const{
    return value_.IsArray();
}
inline bool ofxJsonValueRef::isObject() const{
    return value_.IsObject();
}

inline bool ofxJsonValueRef::operator==(const ofxJsonValueRef& other){
    return (&value_ == &other.value_); // compare addresses!
}

inline bool ofxJsonValueRef::operator!=(const ofxJsonValueRef& other){
    return !(operator==(other));
}


/// explicit getters
inline bool ofxJsonValueRef::getBool() const {
    return operator bool();
}
inline int ofxJsonValueRef::getInt() const {
    return operator int();
}
inline float ofxJsonValueRef::getFloat() const {
    return operator float();
}
inline double ofxJsonValueRef::getDouble() const {
    return operator double();
}
inline string ofxJsonValueRef::getString() const {
    return operator string();
}
inline vector<bool> ofxJsonValueRef::getBoolVector() const {
    return operator vector<bool>();
}
inline vector<int> ofxJsonValueRef::getIntVector() const {
    return operator vector<int>();
}
inline vector<float> ofxJsonValueRef::getFloatVector() const {
    return operator vector<float>();
}
inline vector<double> ofxJsonValueRef::getDoubleVector() const {
    return operator vector<double>();
}
inline vector<string> ofxJsonValueRef::getStringVector() const {
    return operator vector<string>();
}
inline unordered_map<string, bool> ofxJsonValueRef::getBoolMap() const {
    return operator unordered_map<string, bool>();
}
inline unordered_map<string, int> ofxJsonValueRef::getIntMap() const {
    return operator unordered_map<string, int>();
}
inline unordered_map<string, float> ofxJsonValueRef::getFloatMap() const {
    return operator unordered_map<string, float>();
}
inline unordered_map<string, double> ofxJsonValueRef::getDoubleMap() const {
    return operator unordered_map<string, double>();
}
inline unordered_map<string, string> ofxJsonValueRef::getStringMap() const {
    return operator unordered_map<string, string>();
}

inline ofxJsonArrayRef ofxJsonValueRef::getArray() const {
    return ofxJsonArrayRef(*this);
}

inline ofxJsonObjectRef ofxJsonValueRef::getObject() const {
    return ofxJsonObjectRef(*this);
}

/// type casting (implicit getters)
inline ofxJsonValueRef::operator bool() const{
    if (value_.IsBool()){
        return value_.GetBool();
    } else {
        return static_cast<bool>(getInt()); // all other values are ints
    }
}
inline ofxJsonValueRef::operator int() const{
    if (value_.IsNumber()){
        if (value_.IsInt()){
            return value_.GetInt();
        } else if (value_.IsDouble()){
            return static_cast<int>(value_.GetDouble());
        } else if (value_.IsUint()){
            return static_cast<int>(value_.GetUint());
        } else if (value_.IsInt64()){
            return static_cast<int>(value_.GetInt64());
        } else if (value_.IsUint64()){
            return static_cast<int>(value_.GetUint64());
        } else {
            cout << "Error: value is a number but doesn't match a type!\n";
            return 0;
        }
    } else if (value_.IsBool()){
        return static_cast<int>(value_.GetBool());
    } else {
        return 0;
    }
}
inline ofxJsonValueRef::operator float() const{
    return static_cast<float>(operator double());
}
inline ofxJsonValueRef::operator double() const{
    if (value_.IsDouble()){
        return value_.GetDouble();
    } else {
        return static_cast<double>(getInt()); // all other types are ints
    }
}
inline ofxJsonValueRef::operator string() const {
    if (value_.IsString()){
        return (string(value_.GetString()));
    } else {
        return string();
    }
}
inline ofxJsonValueRef::operator vector<bool>() const {
    return getVector<bool>();
}
inline ofxJsonValueRef::operator vector<int>() const {
    return getVector<int>();
}
inline ofxJsonValueRef::operator vector<float>() const {
    return getVector<float>();
}
inline ofxJsonValueRef::operator vector<double>() const {
    return getVector<double>();
}
inline ofxJsonValueRef::operator vector<string>() const {
    return getVector<string>();
}

inline ofxJsonValueRef::operator unordered_map<string, bool>() const {
    return getMap<bool>();
}
inline ofxJsonValueRef::operator unordered_map<string, int>() const {
    return getMap<int>();
}
inline ofxJsonValueRef::operator unordered_map<string, float>() const {
    return getMap<float>();
}
inline ofxJsonValueRef::operator unordered_map<string, double>() const {
    return getMap<double>();
}
inline ofxJsonValueRef::operator unordered_map<string, string>() const {
    return getMap<string>();
}


// needed for ofxJsonValueIterator::operator->()
inline ofxJsonValueRef* ofxJsonValueRef::operator->() {
    return this;
}

// helper function
template<typename T>
vector<T> ofxJsonValueRef::getVector() const {
    if (value_.IsArray()){
        return getArray().getVector<T>();
    } else if (value_.IsNumber() || value_.IsString()) {
        vector<T> vec;
        vec.push_back(static_cast<T>(*this));
        return vec; // vector with single element
    } else {
        return vector<T>{}; // empty vector
    }
}

// helper function
template<typename T>
unordered_map<string, T> ofxJsonValueRef::getMap() const {
    if (value_.IsObject()){
        return getObject().getMap<T>();
    } else {
        return unordered_map<string, T>{}; // empty map
    }
}

/*///////////////////// ofxJsonArrayRef /////////////////////*/

inline ofxJsonArrayRef::ofxJsonArrayRef(const ofxJsonValueRef& value)
    : valueRef_(value) {}

inline ofxJsonArrayRef::ofxJsonArrayRef(const ofxJsonArrayRef& mom)
    : valueRef_(mom.valueRef_) {}

inline ofxJsonArrayRef::~ofxJsonArrayRef() {}

inline ofxJsonArrayRef& ofxJsonArrayRef::operator=(const ofxJsonArrayRef& other){
    valueRef_ = other.valueRef_; // simply copy assign one ofxJsonValueRef to another.
    return *this;
}

template<typename T>
inline ofxJsonArrayRef& ofxJsonArrayRef::operator=(const vector<T>& vec){
    // just forward to ofxJsonValueRef assignment operator which will handle the different types correctly
    valueRef_ = vec;
    return *this;
}

inline ofxJsonArrayRef& ofxJsonArrayRef::setArray(const ofxJsonArrayRef& other){
    return operator=(other);
}

template<typename T>
inline ofxJsonArrayRef& ofxJsonArrayRef::setArray(const vector<T>& vec){
    return operator=(vec);
}

inline ofxJsonValueRef ofxJsonArrayRef::operator[](size_t index) const {
    return ofxJsonValueRef(valueRef_.value_[index], valueRef_.allocator_);
}

inline size_t ofxJsonArrayRef::size() const {
    return valueRef_.value_.Size();
}

inline bool ofxJsonArrayRef::empty() const {
    return valueRef_.value_.Empty();
}

inline size_t ofxJsonArrayRef::capacity() const {
    return valueRef_.value_.Capacity();
}

inline void ofxJsonArrayRef::reserve(size_t n) {
    valueRef_.value_.Reserve(n, valueRef_.allocator_);
}

inline void ofxJsonArrayRef::resize(size_t n) {
    int diff = n - size();
    reserve(n);

    if (diff > 0){
         // if new size is larger than old size, append values (with default value Null)
        while (--diff){
            valueRef_.value_.PushBack(rapidjson::Value(), valueRef_.allocator_);
        }
    } else if (diff < 0){
        // if new size is smaller than old size, pop values.
        int k = diff * -1;
        while (--k){
            valueRef_.value_.PopBack();
        }
    }
}

inline void ofxJsonArrayRef::clear(){
    valueRef_.value_.Clear();
}


inline ofxJsonValueIterator ofxJsonArrayRef::begin() const {
    return ofxJsonValueIterator(valueRef_.value_.Begin(), valueRef_.allocator_);
}

inline ofxJsonValueIterator ofxJsonArrayRef::end() const {
    return ofxJsonValueIterator(valueRef_.value_.End(), valueRef_.allocator_);
}

inline ofxJsonValueRef ofxJsonArrayRef::front() const {
    return ofxJsonValueRef(valueRef_.value_[0], valueRef_.allocator_);
}

inline ofxJsonValueRef ofxJsonArrayRef::back() const {
    return ofxJsonValueRef(valueRef_.value_[size()-1], valueRef_.allocator_);
}

template<typename T>
inline void ofxJsonArrayRef::push_back(T&& value) {
    rapidjson::Value temp;
    ofxJsonValueRef dummy(temp, valueRef_.allocator_); // wrap into ofxJsonValueRef,
    dummy = forward<T>(value); // so we can utilize our typecast operator overloads
    valueRef_.value_.PushBack(temp.Move(), valueRef_.allocator_); // 'temp' is now assigned and we can push it
}

// specialization for void -> push back null value
inline void ofxJsonArrayRef::push_back() {
    valueRef_.value_.PushBack(rapidjson::Value(), valueRef_.allocator_);
}


inline void ofxJsonArrayRef::pop_back() {
    valueRef_.value_.PopBack();
}

inline ofxJsonValueIterator ofxJsonArrayRef::erase(const ofxJsonValueIterator& pos){
    return ofxJsonValueIterator(valueRef_.value_.Erase(pos.ptr_), valueRef_.allocator_);
}

inline ofxJsonValueIterator ofxJsonArrayRef::erase(const ofxJsonValueIterator& first, const ofxJsonValueIterator& last){
    return ofxJsonValueIterator(valueRef_.value_.Erase(first.ptr_, last.ptr_), valueRef_.allocator_);
}

inline vector<bool> ofxJsonArrayRef::getBoolVector() const {
    return operator vector<bool>();
}
inline vector<int> ofxJsonArrayRef::getIntVector() const {
    return operator vector<int>();
}
inline vector<float> ofxJsonArrayRef::getFloatVector() const {
    return operator vector<float>();
}
inline vector<double> ofxJsonArrayRef::getDoubleVector() const {
    return operator vector<double>();
}
inline vector<string> ofxJsonArrayRef::getStringVector() const {
    return operator vector<string>();
}

inline ofxJsonValueRef ofxJsonArrayRef::getValue() const {
    return valueRef_;
}

inline ofxJsonArrayRef::operator vector<bool>() const {
    return getVector<bool>();
}
inline ofxJsonArrayRef::operator vector<int>() const {
    return getVector<int>();
}
inline ofxJsonArrayRef::operator vector<float>() const {
    return getVector<float>();
}
inline ofxJsonArrayRef::operator vector<double>() const {
    return getVector<double>();
}
inline ofxJsonArrayRef::operator vector<string>() const {
    return getVector<string>();
}

/// helper function
template<typename T>
inline vector<T> ofxJsonArrayRef::getVector() const{
    vector<T> vec;
    for (auto it = valueRef_.value_.Begin(), end = valueRef_.value_.End(); it != end; ++it){
        ofxJsonValueRef val(*it, valueRef_.allocator_); // not very efficient but convenient
        vec.push_back(static_cast<T>(val)); // since we can utilize ofxJasonValueRef's overloaded typecast operator
    }
    return vec;
}


/*////////////////////// ofxJsonObjectRef /////////////////////*/

inline ofxJsonObjectRef::ofxJsonObjectRef(const ofxJsonValueRef& value)
    : valueRef_(value) {}

inline ofxJsonObjectRef::ofxJsonObjectRef(const ofxJsonObjectRef& mom)
    : valueRef_(mom.valueRef_) {}

inline ofxJsonObjectRef::~ofxJsonObjectRef() {}

inline ofxJsonObjectRef& ofxJsonObjectRef::operator=(const ofxJsonObjectRef& other){
    valueRef_ = other.valueRef_; // simply copy assign
}

template<typename T>
inline ofxJsonObjectRef& ofxJsonObjectRef::operator=(const unordered_map<string, T>& map){
    // just forward to ofxJsonValueRef assignment operator which will handle the different types correctly
    valueRef_ = map;
    return *this;
}

inline ofxJsonObjectRef& ofxJsonObjectRef::setObject(const ofxJsonObjectRef& other){
    return operator=(other);
}

template<typename T>
inline ofxJsonObjectRef& ofxJsonObjectRef::setObject(const unordered_map<string, T>& map){
    return operator=(map);
}
/// get reference to value or insert new member if the name doesn't exist yet.
inline ofxJsonValueRef ofxJsonObjectRef::operator[](const string& name) const {
    rapidjson::Value key(name.data(), name.size()); // treat it as a constant string
    auto it = valueRef_.value_.FindMember(key);
    if (it == valueRef_.value_.MemberEnd()){
        // doesn't exist -> insert name (with value = Null)
        valueRef_.value_.AddMember(rapidjson::Value(key, valueRef_.allocator_), rapidjson::Value(), valueRef_.allocator_); // copy the string!
        it = valueRef_.value_.FindMember(key); // find the new value. not sure it's necessarily the one before end()... so better check again.
    }
    return ofxJsonValueRef(it->value, valueRef_.allocator_);
}

inline ofxJsonMemberIterator ofxJsonObjectRef::find(const string& name) const {
    rapidjson::Value key(name.data(), name.size()); // treat it as a constant string
    return ofxJsonMemberIterator(valueRef_.value_.FindMember(key), valueRef_.allocator_);
}

inline int ofxJsonObjectRef::count(const string& name) const {
    rapidjson::Value key(name.data(), name.size());
    auto it = valueRef_.value_.FindMember(key);
    return (it != valueRef_.value_.MemberEnd());
}

inline size_t ofxJsonObjectRef::size() const {
    return valueRef_.value_.MemberCount();
}

inline bool ofxJsonObjectRef::empty() const {
    return valueRef_.value_.ObjectEmpty();
}

inline void ofxJsonObjectRef::clear() {
    return valueRef_.value_.RemoveAllMembers();
}

inline ofxJsonMemberIterator ofxJsonObjectRef::begin() const {
    return ofxJsonMemberIterator(valueRef_.value_.MemberBegin(), valueRef_.allocator_);
}
inline ofxJsonMemberIterator ofxJsonObjectRef::end() const {
    return ofxJsonMemberIterator(valueRef_.value_.MemberEnd(), valueRef_.allocator_);
}

template<typename T>
inline void ofxJsonObjectRef::insert(const string& name, T&& value){
    rapidjson::Value temp;
    ofxJsonValueRef dummy(temp, valueRef_.allocator_); // wrap into ofxJsonValueRef,
    dummy = forward<T>(value); // so we can utilize our typecast operator overloads
    // temp is now assigned, so we can finally add it together with the member name.
    valueRef_.value_.AddMember(rapidjson::Value(name.data(), name.size(), valueRef_.allocator_), temp.Move(), valueRef_.allocator_);
}

inline void ofxJsonObjectRef::insert(const string& name){
    valueRef_.value_.AddMember(rapidjson::Value(name.data(), name.size(), valueRef_.allocator_), rapidjson::Value(), valueRef_.allocator_);
}

inline ofxJsonMemberIterator ofxJsonObjectRef::erase(const string& name){
    auto it = valueRef_.value_.FindMember(rapidjson::Value(name.data(), name.size()));
    if (it != valueRef_.value_.MemberEnd()){
        return ofxJsonMemberIterator(valueRef_.value_.EraseMember(it), valueRef_.allocator_);
    } else {
        return ofxJsonMemberIterator(it, valueRef_.allocator_);
    }
}

inline ofxJsonMemberIterator ofxJsonObjectRef::erase(const ofxJsonMemberIterator& pos){
    return ofxJsonMemberIterator(valueRef_.value_.EraseMember(pos.ptr_), valueRef_.allocator_);
}

inline ofxJsonMemberIterator ofxJsonObjectRef::erase(const ofxJsonMemberIterator &first, const ofxJsonMemberIterator &last){
    return ofxJsonMemberIterator(valueRef_.value_.EraseMember(first.ptr_, last.ptr_), valueRef_.allocator_);
}


inline unordered_map<string, bool> ofxJsonObjectRef::getBoolMap() const {
    return operator unordered_map<string, bool>();
}
inline unordered_map<string, int> ofxJsonObjectRef::getIntMap() const {
    return operator unordered_map<string, int>();
}
inline unordered_map<string, float> ofxJsonObjectRef::getFloatMap() const {
    return operator unordered_map<string, float>();
}
inline unordered_map<string, double> ofxJsonObjectRef::getDoubleMap() const {
    return operator unordered_map<string, double>();
}
inline unordered_map<string, string> ofxJsonObjectRef::getStringMap() const {
    return operator unordered_map<string, string>();
}

inline ofxJsonValueRef ofxJsonObjectRef::getValue() const {
    return valueRef_;
}

inline ofxJsonObjectRef::operator unordered_map<string, bool>() const {
    return getMap<bool>();
}
inline ofxJsonObjectRef::operator unordered_map<string, int>() const {
    return getMap<int>();
}
inline ofxJsonObjectRef::operator unordered_map<string, float>() const {
    return getMap<float>();
}
inline ofxJsonObjectRef::operator unordered_map<string, double>() const {
    return getMap<double>();
}
inline ofxJsonObjectRef::operator unordered_map<string, string>() const {
    return getMap<string>();
}

/// helper function
template<typename T>
inline unordered_map<string, T> ofxJsonObjectRef::getMap() const{
    unordered_map<string, T> map;
    for (auto it = valueRef_.value_.MemberBegin(), end = valueRef_.value_.MemberEnd(); it != end; ++it){
        ofxJsonMemberRef member(*it, valueRef_.allocator_); // not very efficient but convenient
        map.emplace(static_cast<string>(member.name), static_cast<T>(member.value)); // since we can utilize ofxJasonValueRef's overloaded typecast operator
    }
    return map;
}

// } // end of namespace
