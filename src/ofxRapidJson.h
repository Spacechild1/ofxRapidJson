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

// namespace ofxRapidJson {

class ofxJsonValueRef;
class ofxJsonIterator;

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
//    bool loadFromFile(ofFile& file);
    void loadFromBuffer(const string& buffer);
    void loadFromBuffer(const ofBuffer& buffer);

    /// save JSON data
    bool saveToFile(const string& path);
//    void saveToFile(ofFile& file);
    void saveToBuffer(string& buffer);
    void saveToBuffer(ofBuffer& buffer);

    /// does a key exist?
    /// return an ofxJsonIterator to the found Value
    /// or to document.end() if it doesn't exist.
    ///
    /// keys are rapidjson "Pointers", such as "/foo/bar".
    /// note that you always need a leading '/'!
    rapidjson::Value* find(const string& key);
    /// get value reference by key.
    /// if the key doesn't exist, create it.
//    ofxJsonValueRef get(const string& key);
    ofxJsonValueRef operator[](const string& key);

    /// get root value reference
    ofxJsonValueRef getRoot();

    /// return Iterator past end
//    ofxJsonIterator end();
    /// get the actual rapidjson::Document by reference
    /// to allow direct manipulation via the original rapidjson API
    rapidjson::Document& getDocument();
//    const rapidjson::Document& getDocument() const;
protected:
    rapidjson::Document document_;
};

enum ofxJsonValueType {
    OFX_JSON_BOOL,
    OFX_JSON_NUMBER,
    OFX_JSON_STRING,
    OFX_JSON_ARRAY,
    OFX_JSON_OBJECT,
    OFX_JSON_NULL
};

/// helper class which wraps a rapidjson::Value reference together with an allocater reference
/// around a new, simpler interface.
/// The allocator reference points to the allocator of the document the value belongs to.
/// This allows as to construct/change advanced value types such as strings, objects and arrays.
class ofxJsonValueRef {
public:
    /// constructors:
    ofxJsonValueRef(rapidjson::Value& ref, rapidjson::Document::AllocatorType& allocator);
    ofxJsonValueRef(const ofxJsonValueRef& mom);
//    ofxJsonValueRef(ofxJsonValueRef&& mom);
    ~ofxJsonValueRef();

    /// copy and move assignment
    ///
    /// since this is a reference wrapper, assignment doesn't change the reference itself
    /// (this would be possible anyway) but the value it points to!
    ofxJsonValueRef& operator=(const ofxJsonValueRef& other);
    ofxJsonValueRef& operator=(ofxJsonValueRef&& other);
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
    /// catches std::vector (makes an Array):
    ofxJsonValueRef& operator=(const vector<bool>& vec);
    ofxJsonValueRef& operator=(const vector<int>& vec);
    ofxJsonValueRef& operator=(const vector<float>& vec);
    ofxJsonValueRef& operator=(const vector<double>& vec);
    ofxJsonValueRef& operator=(const vector<string>& vec);
    /// set simply forwards to assignment operator
    template<typename T>
    ofxJsonValueRef& setValue(T&& value);

    /// get type info
    ofxJsonValueType getType() const;
    bool isBool() const;
    bool isNumber() const;
    bool isString() const;
    bool isArray() const;
    bool isObject() const;
    bool operator==(const ofxJsonValueRef& other);
    bool operator!=(const ofxJsonValueRef& other);
    /// for Arrays and Objects
    bool empty() const;
    size_t size() const;

//    ofxJsonIterator begin();
//    ofxJsonIterator end();


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
//    rapidjson::Value& getValue();
//    const rapidjson::Value& getValue() const;
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
//    operator rapidjson::Value&();
//    operator const rapidjson::Value&() const;

protected:
    template<typename T>
    void setVector(const vector<T>& vec);
    template<typename T>
    vector<T> getVector() const; // helper function

    rapidjson::Value& value_;
    rapidjson::Document::AllocatorType& allocator_;
};

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

    document_.Clear();

    rapidjson::IStreamWrapper isw(ifs);
    document_.ParseStream(isw);

    return true;
}

inline void ofxJsonDocument::loadFromBuffer(const string& buffer){
    document_.Clear();
    document_.Parse(buffer.data(), buffer.size());
}

inline void ofxJsonDocument::loadFromBuffer(const ofBuffer& buffer){
    document_.Clear();
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
inline rapidjson::Value* ofxJsonDocument::find(const string& key){
    return rapidjson::Pointer(key.c_str()).Get(document_);
}

/*
/// get value reference by key
inline ofxJsonValueRef ofxJsonDocument::get(const string& key){
    return operator[](key);
}
*/

inline ofxJsonValueRef ofxJsonDocument::operator[](const string& key){
    rapidjson::Pointer ptr(key.c_str());
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

/*
inline const rapidjson::Document& ofxJsonDocument::getDocument() const {
    return document_;
}
*/

/*///////////////////// ofxJsonValueRef /////////////////*/

/// constructors:
inline ofxJsonValueRef::ofxJsonValueRef(rapidjson::Value& ref, rapidjson::Document::AllocatorType& allocator)
    : value_(ref), allocator_(allocator) {}

inline ofxJsonValueRef::ofxJsonValueRef(const ofxJsonValueRef& mom)
    : value_(mom.value_), allocator_(mom.allocator_) {}
/*
inline ofxJsonValueRef::ofxJsonValueRef(ofxJsonValueRef&& mom)
    : value_(mom.value_), allocator_(mom.allocator_) {}
*/
inline ofxJsonValueRef::~ofxJsonValueRef() {}

/// copy/move assignment
inline ofxJsonValueRef& ofxJsonValueRef::operator=(const ofxJsonValueRef& other){
    if (this != &other){
        value_.CopyFrom(other.value_, allocator_); // copy value explicitly
    }
    return *this;
}

inline ofxJsonValueRef& ofxJsonValueRef::operator=(ofxJsonValueRef&& other){
    if (this != &other){
        value_.CopyFrom(other.value_, allocator_); // also copy! 'other' doesn't own data, it just holds a reference
    }
    return *this;
}

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

inline ofxJsonValueRef& ofxJsonValueRef::operator=(const vector<bool>& vec){
    setVector(vec); return *this;
}

inline ofxJsonValueRef& ofxJsonValueRef::operator=(const vector<int>& vec){
    setVector(vec); return *this;
}

inline ofxJsonValueRef& ofxJsonValueRef::operator=(const vector<float>& vec){
    setVector(vec); return *this;
}

inline ofxJsonValueRef& ofxJsonValueRef::operator=(const vector<double>& vec){
    setVector(vec); return *this;
}

inline ofxJsonValueRef& ofxJsonValueRef::operator=(const vector<string>& vec){
    value_.SetArray();
    value_.Clear();

    for (auto& s : vec){
        value_.PushBack(rapidjson::Value().SetString(s.data(), s.length(), allocator_).Move(), allocator_);
    }
    return *this;
}

template<typename T>
inline ofxJsonValueRef& ofxJsonValueRef::setValue(T&& value){
    return operator=(forward<T>(value));
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
    return (value_ == other.value_);
}

inline bool ofxJsonValueRef::operator!=(const ofxJsonValueRef& other){
    return !(operator==(other));
}

inline bool ofxJsonValueRef::empty() const{
    return !size();
}

inline size_t ofxJsonValueRef::size() const{
    if (value_.IsObject()){
        return value_.MemberCount();
    } else if (value_.IsArray()) {
        return value_.Size();
    } else {
        return 0;
    }
}


/// explicit getters
inline bool ofxJsonValueRef::getBool() const {
    if (value_.IsBool()){
        return value_.GetBool();
    } else {
        return static_cast<bool>(getInt()); // all other values are ints
    }
}
inline int ofxJsonValueRef::getInt() const {
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
inline float ofxJsonValueRef::getFloat() const {
    return static_cast<float>(getDouble());
}
inline double ofxJsonValueRef::getDouble() const {
    if (value_.IsDouble()){
        return value_.GetDouble();
    } else {
        return static_cast<double>(getInt()); // all other types are ints
    }
}
inline string ofxJsonValueRef::getString() const {
    if (value_.IsString()){
        return (string(value_.GetString()));
    } else {
        return string();
    }
}
inline vector<bool> ofxJsonValueRef::getBoolVector() const {
    return getVector<bool>();
}
inline vector<int> ofxJsonValueRef::getIntVector() const {
    return getVector<int>();
}
inline vector<float> ofxJsonValueRef::getFloatVector() const {
    return getVector<float>();
}
inline vector<double> ofxJsonValueRef::getDoubleVector() const {
    return getVector<double>();
}
inline vector<string> ofxJsonValueRef::getStringVector() const {
    return getVector<string>();
}
/*
inline rapidjson::Value& ofxJsonValueRef::getValue(){
    return value_;
}
inline const rapidjson::Value& ofxJsonValueRef::getValue() const{
    return value_;
}
*/
/// type casting (implicit getters)
inline ofxJsonValueRef::operator bool() const{
    return getBool();
}
inline ofxJsonValueRef::operator int() const{
    return getInt();
}
inline ofxJsonValueRef::operator float() const{
    return getFloat();
}
inline ofxJsonValueRef::operator double() const{
    return getDouble();
}
inline ofxJsonValueRef::operator string() const {
    return getString();
}
inline ofxJsonValueRef::operator vector<bool>() const {
    return getBoolVector();
}
inline ofxJsonValueRef::operator vector<int>() const {
    return getIntVector();
}
inline ofxJsonValueRef::operator vector<float>() const {
    return getFloatVector();
}
inline ofxJsonValueRef::operator vector<double>() const {
    return getDoubleVector();
}
inline ofxJsonValueRef::operator vector<string>() const {
    return getStringVector();
}
/*
inline ofxJsonValueRef::operator rapidjson::Value&(){
    return value_;
}
inline ofxJsonValueRef::operator const rapidjson::Value&() const{
    return value_;
}
*/

/// helper function
template<typename T>
inline void ofxJsonValueRef::setVector(const vector<T>& vec){
    value_.SetArray();
    value_.Clear();

    for (auto k : vec){
        value_.PushBack(rapidjson::Value(k), allocator_);
    }
}


template<typename T>
inline vector<T> ofxJsonValueRef::getVector() const{
    vector<T> vec{};

    if (value_.IsArray()){
        for (auto it = value_.Begin(), end = value_.End(); it != end; ++it){
            ofxJsonValueRef val(*it, allocator_); // not very efficient but convenient
            vec.push_back(static_cast<T>(val)); // since we can use ofxJasonValueRef's overloaded typecast operator
        }
    } else if (value_.IsObject()){
        for (auto it = value_.MemberBegin(), end = value_.MemberEnd(); it != end; ++it){
            ofxJsonValueRef val(it->value, allocator_); // MemberIterator has two fields: name + value
            vec.push_back(static_cast<T>(val));
        }
    } else if (value_.IsString() || value_.IsNumber()){
        vec.push_back(static_cast<T>(*this)); // single value
    }
    return vec;
}

// } // end of namespace
