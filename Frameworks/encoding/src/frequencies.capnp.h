// Generated by Cap'n Proto compiler, DO NOT EDIT
// source: frequencies.capnp

#pragma once

#include <capnp/generated-header-support.h>
#include <kj/windows-sanity.h>

#ifndef CAPNP_VERSION
#error "CAPNP_VERSION is not defined, is capnp/generated-header-support.h missing?"
#elif CAPNP_VERSION != 1001000
#error "Version mismatch between generated code and library headers.  You must use the same version of the Cap'n Proto compiler and library."
#endif


CAPNP_BEGIN_HEADER

namespace capnp {
namespace schemas {

CAPNP_DECLARE_SCHEMA(ec0b183738c8b109);
CAPNP_DECLARE_SCHEMA(ddede62de353e1f5);
CAPNP_DECLARE_SCHEMA(cea921405c81ddee);
CAPNP_DECLARE_SCHEMA(f4318655ad6122d5);

}  // namespace schemas
}  // namespace capnp


struct Charset {
  Charset() = delete;

  class Reader;
  class Builder;
  class Pipeline;
  struct Pair;

  struct _capnpPrivate {
    CAPNP_DECLARE_STRUCT_HEADER(ec0b183738c8b109, 0, 3)
    #if !CAPNP_LITE
    static constexpr ::capnp::_::RawBrandedSchema const* brand() { return &schema->defaultBrand; }
    #endif  // !CAPNP_LITE
  };
};

struct Charset::Pair {
  Pair() = delete;

  class Reader;
  class Builder;
  class Pipeline;
  struct Type;

  struct _capnpPrivate {
    CAPNP_DECLARE_STRUCT_HEADER(ddede62de353e1f5, 2, 1)
    #if !CAPNP_LITE
    static constexpr ::capnp::_::RawBrandedSchema const* brand() { return &schema->defaultBrand; }
    #endif  // !CAPNP_LITE
  };
};

struct Charset::Pair::Type {
  Type() = delete;

  class Reader;
  class Builder;
  class Pipeline;
  enum Which: uint16_t {
    WORD,
    BYTE,
  };

  struct _capnpPrivate {
    CAPNP_DECLARE_STRUCT_HEADER(cea921405c81ddee, 2, 1)
    #if !CAPNP_LITE
    static constexpr ::capnp::_::RawBrandedSchema const* brand() { return &schema->defaultBrand; }
    #endif  // !CAPNP_LITE
  };
};

struct Frequencies {
  Frequencies() = delete;

  class Reader;
  class Builder;
  class Pipeline;

  struct _capnpPrivate {
    CAPNP_DECLARE_STRUCT_HEADER(f4318655ad6122d5, 1, 1)
    #if !CAPNP_LITE
    static constexpr ::capnp::_::RawBrandedSchema const* brand() { return &schema->defaultBrand; }
    #endif  // !CAPNP_LITE
  };
};

// =======================================================================================

class Charset::Reader {
public:
  typedef Charset Reads;

  Reader() = default;
  inline explicit Reader(::capnp::_::StructReader base): _reader(base) {}

  inline ::capnp::MessageSize totalSize() const {
    return _reader.totalSize().asPublic();
  }

#if !CAPNP_LITE
  inline ::kj::StringTree toString() const {
    return ::capnp::_::structString(_reader, *_capnpPrivate::brand());
  }
#endif  // !CAPNP_LITE

  inline bool hasCharset() const;
  inline  ::capnp::Text::Reader getCharset() const;

  inline bool hasWords() const;
  inline  ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>::Reader getWords() const;

  inline bool hasBytes() const;
  inline  ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>::Reader getBytes() const;

private:
  ::capnp::_::StructReader _reader;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::List;
  friend class ::capnp::MessageBuilder;
  friend class ::capnp::Orphanage;
};

class Charset::Builder {
public:
  typedef Charset Builds;

  Builder() = delete;  // Deleted to discourage incorrect usage.
                       // You can explicitly initialize to nullptr instead.
  inline Builder(decltype(nullptr)) {}
  inline explicit Builder(::capnp::_::StructBuilder base): _builder(base) {}
  inline operator Reader() const { return Reader(_builder.asReader()); }
  inline Reader asReader() const { return *this; }

  inline ::capnp::MessageSize totalSize() const { return asReader().totalSize(); }
#if !CAPNP_LITE
  inline ::kj::StringTree toString() const { return asReader().toString(); }
#endif  // !CAPNP_LITE

  inline bool hasCharset();
  inline  ::capnp::Text::Builder getCharset();
  inline void setCharset( ::capnp::Text::Reader value);
  inline  ::capnp::Text::Builder initCharset(unsigned int size);
  inline void adoptCharset(::capnp::Orphan< ::capnp::Text>&& value);
  inline ::capnp::Orphan< ::capnp::Text> disownCharset();

  inline bool hasWords();
  inline  ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>::Builder getWords();
  inline void setWords( ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>::Reader value);
  inline  ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>::Builder initWords(unsigned int size);
  inline void adoptWords(::capnp::Orphan< ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>>&& value);
  inline ::capnp::Orphan< ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>> disownWords();

  inline bool hasBytes();
  inline  ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>::Builder getBytes();
  inline void setBytes( ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>::Reader value);
  inline  ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>::Builder initBytes(unsigned int size);
  inline void adoptBytes(::capnp::Orphan< ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>>&& value);
  inline ::capnp::Orphan< ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>> disownBytes();

private:
  ::capnp::_::StructBuilder _builder;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  friend class ::capnp::Orphanage;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
};

#if !CAPNP_LITE
class Charset::Pipeline {
public:
  typedef Charset Pipelines;

  inline Pipeline(decltype(nullptr)): _typeless(nullptr) {}
  inline explicit Pipeline(::capnp::AnyPointer::Pipeline&& typeless)
      : _typeless(kj::mv(typeless)) {}

private:
  ::capnp::AnyPointer::Pipeline _typeless;
  friend class ::capnp::PipelineHook;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
};
#endif  // !CAPNP_LITE

class Charset::Pair::Reader {
public:
  typedef Pair Reads;

  Reader() = default;
  inline explicit Reader(::capnp::_::StructReader base): _reader(base) {}

  inline ::capnp::MessageSize totalSize() const {
    return _reader.totalSize().asPublic();
  }

#if !CAPNP_LITE
  inline ::kj::StringTree toString() const {
    return ::capnp::_::structString(_reader, *_capnpPrivate::brand());
  }
#endif  // !CAPNP_LITE

  inline typename Type::Reader getType() const;

  inline  ::uint64_t getCount() const;

private:
  ::capnp::_::StructReader _reader;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::List;
  friend class ::capnp::MessageBuilder;
  friend class ::capnp::Orphanage;
};

class Charset::Pair::Builder {
public:
  typedef Pair Builds;

  Builder() = delete;  // Deleted to discourage incorrect usage.
                       // You can explicitly initialize to nullptr instead.
  inline Builder(decltype(nullptr)) {}
  inline explicit Builder(::capnp::_::StructBuilder base): _builder(base) {}
  inline operator Reader() const { return Reader(_builder.asReader()); }
  inline Reader asReader() const { return *this; }

  inline ::capnp::MessageSize totalSize() const { return asReader().totalSize(); }
#if !CAPNP_LITE
  inline ::kj::StringTree toString() const { return asReader().toString(); }
#endif  // !CAPNP_LITE

  inline typename Type::Builder getType();
  inline typename Type::Builder initType();

  inline  ::uint64_t getCount();
  inline void setCount( ::uint64_t value);

private:
  ::capnp::_::StructBuilder _builder;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  friend class ::capnp::Orphanage;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
};

#if !CAPNP_LITE
class Charset::Pair::Pipeline {
public:
  typedef Pair Pipelines;

  inline Pipeline(decltype(nullptr)): _typeless(nullptr) {}
  inline explicit Pipeline(::capnp::AnyPointer::Pipeline&& typeless)
      : _typeless(kj::mv(typeless)) {}

  inline typename Type::Pipeline getType();
private:
  ::capnp::AnyPointer::Pipeline _typeless;
  friend class ::capnp::PipelineHook;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
};
#endif  // !CAPNP_LITE

class Charset::Pair::Type::Reader {
public:
  typedef Type Reads;

  Reader() = default;
  inline explicit Reader(::capnp::_::StructReader base): _reader(base) {}

  inline ::capnp::MessageSize totalSize() const {
    return _reader.totalSize().asPublic();
  }

#if !CAPNP_LITE
  inline ::kj::StringTree toString() const {
    return ::capnp::_::structString(_reader, *_capnpPrivate::brand());
  }
#endif  // !CAPNP_LITE

  inline Which which() const;
  inline bool isWord() const;
  inline bool hasWord() const;
  inline  ::capnp::Text::Reader getWord() const;

  inline bool isByte() const;
  inline  ::uint8_t getByte() const;

private:
  ::capnp::_::StructReader _reader;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::List;
  friend class ::capnp::MessageBuilder;
  friend class ::capnp::Orphanage;
};

class Charset::Pair::Type::Builder {
public:
  typedef Type Builds;

  Builder() = delete;  // Deleted to discourage incorrect usage.
                       // You can explicitly initialize to nullptr instead.
  inline Builder(decltype(nullptr)) {}
  inline explicit Builder(::capnp::_::StructBuilder base): _builder(base) {}
  inline operator Reader() const { return Reader(_builder.asReader()); }
  inline Reader asReader() const { return *this; }

  inline ::capnp::MessageSize totalSize() const { return asReader().totalSize(); }
#if !CAPNP_LITE
  inline ::kj::StringTree toString() const { return asReader().toString(); }
#endif  // !CAPNP_LITE

  inline Which which();
  inline bool isWord();
  inline bool hasWord();
  inline  ::capnp::Text::Builder getWord();
  inline void setWord( ::capnp::Text::Reader value);
  inline  ::capnp::Text::Builder initWord(unsigned int size);
  inline void adoptWord(::capnp::Orphan< ::capnp::Text>&& value);
  inline ::capnp::Orphan< ::capnp::Text> disownWord();

  inline bool isByte();
  inline  ::uint8_t getByte();
  inline void setByte( ::uint8_t value);

private:
  ::capnp::_::StructBuilder _builder;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  friend class ::capnp::Orphanage;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
};

#if !CAPNP_LITE
class Charset::Pair::Type::Pipeline {
public:
  typedef Type Pipelines;

  inline Pipeline(decltype(nullptr)): _typeless(nullptr) {}
  inline explicit Pipeline(::capnp::AnyPointer::Pipeline&& typeless)
      : _typeless(kj::mv(typeless)) {}

private:
  ::capnp::AnyPointer::Pipeline _typeless;
  friend class ::capnp::PipelineHook;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
};
#endif  // !CAPNP_LITE

class Frequencies::Reader {
public:
  typedef Frequencies Reads;

  Reader() = default;
  inline explicit Reader(::capnp::_::StructReader base): _reader(base) {}

  inline ::capnp::MessageSize totalSize() const {
    return _reader.totalSize().asPublic();
  }

#if !CAPNP_LITE
  inline ::kj::StringTree toString() const {
    return ::capnp::_::structString(_reader, *_capnpPrivate::brand());
  }
#endif  // !CAPNP_LITE

  inline  ::uint32_t getVersion() const;

  inline bool hasCharsets() const;
  inline  ::capnp::List< ::Charset,  ::capnp::Kind::STRUCT>::Reader getCharsets() const;

private:
  ::capnp::_::StructReader _reader;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::List;
  friend class ::capnp::MessageBuilder;
  friend class ::capnp::Orphanage;
};

class Frequencies::Builder {
public:
  typedef Frequencies Builds;

  Builder() = delete;  // Deleted to discourage incorrect usage.
                       // You can explicitly initialize to nullptr instead.
  inline Builder(decltype(nullptr)) {}
  inline explicit Builder(::capnp::_::StructBuilder base): _builder(base) {}
  inline operator Reader() const { return Reader(_builder.asReader()); }
  inline Reader asReader() const { return *this; }

  inline ::capnp::MessageSize totalSize() const { return asReader().totalSize(); }
#if !CAPNP_LITE
  inline ::kj::StringTree toString() const { return asReader().toString(); }
#endif  // !CAPNP_LITE

  inline  ::uint32_t getVersion();
  inline void setVersion( ::uint32_t value);

  inline bool hasCharsets();
  inline  ::capnp::List< ::Charset,  ::capnp::Kind::STRUCT>::Builder getCharsets();
  inline void setCharsets( ::capnp::List< ::Charset,  ::capnp::Kind::STRUCT>::Reader value);
  inline  ::capnp::List< ::Charset,  ::capnp::Kind::STRUCT>::Builder initCharsets(unsigned int size);
  inline void adoptCharsets(::capnp::Orphan< ::capnp::List< ::Charset,  ::capnp::Kind::STRUCT>>&& value);
  inline ::capnp::Orphan< ::capnp::List< ::Charset,  ::capnp::Kind::STRUCT>> disownCharsets();

private:
  ::capnp::_::StructBuilder _builder;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  friend class ::capnp::Orphanage;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
};

#if !CAPNP_LITE
class Frequencies::Pipeline {
public:
  typedef Frequencies Pipelines;

  inline Pipeline(decltype(nullptr)): _typeless(nullptr) {}
  inline explicit Pipeline(::capnp::AnyPointer::Pipeline&& typeless)
      : _typeless(kj::mv(typeless)) {}

private:
  ::capnp::AnyPointer::Pipeline _typeless;
  friend class ::capnp::PipelineHook;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
};
#endif  // !CAPNP_LITE

// =======================================================================================

inline bool Charset::Reader::hasCharset() const {
  return !_reader.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS).isNull();
}
inline bool Charset::Builder::hasCharset() {
  return !_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS).isNull();
}
inline  ::capnp::Text::Reader Charset::Reader::getCharset() const {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::get(_reader.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS));
}
inline  ::capnp::Text::Builder Charset::Builder::getCharset() {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::get(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS));
}
inline void Charset::Builder::setCharset( ::capnp::Text::Reader value) {
  ::capnp::_::PointerHelpers< ::capnp::Text>::set(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS), value);
}
inline  ::capnp::Text::Builder Charset::Builder::initCharset(unsigned int size) {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::init(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS), size);
}
inline void Charset::Builder::adoptCharset(
    ::capnp::Orphan< ::capnp::Text>&& value) {
  ::capnp::_::PointerHelpers< ::capnp::Text>::adopt(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::capnp::Text> Charset::Builder::disownCharset() {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::disown(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS));
}

inline bool Charset::Reader::hasWords() const {
  return !_reader.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS).isNull();
}
inline bool Charset::Builder::hasWords() {
  return !_builder.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS).isNull();
}
inline  ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>::Reader Charset::Reader::getWords() const {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>>::get(_reader.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS));
}
inline  ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>::Builder Charset::Builder::getWords() {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>>::get(_builder.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS));
}
inline void Charset::Builder::setWords( ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>::Reader value) {
  ::capnp::_::PointerHelpers< ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>>::set(_builder.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS), value);
}
inline  ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>::Builder Charset::Builder::initWords(unsigned int size) {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>>::init(_builder.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS), size);
}
inline void Charset::Builder::adoptWords(
    ::capnp::Orphan< ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>>&& value) {
  ::capnp::_::PointerHelpers< ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>>::adopt(_builder.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>> Charset::Builder::disownWords() {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>>::disown(_builder.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS));
}

inline bool Charset::Reader::hasBytes() const {
  return !_reader.getPointerField(
      ::capnp::bounded<2>() * ::capnp::POINTERS).isNull();
}
inline bool Charset::Builder::hasBytes() {
  return !_builder.getPointerField(
      ::capnp::bounded<2>() * ::capnp::POINTERS).isNull();
}
inline  ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>::Reader Charset::Reader::getBytes() const {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>>::get(_reader.getPointerField(
      ::capnp::bounded<2>() * ::capnp::POINTERS));
}
inline  ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>::Builder Charset::Builder::getBytes() {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>>::get(_builder.getPointerField(
      ::capnp::bounded<2>() * ::capnp::POINTERS));
}
inline void Charset::Builder::setBytes( ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>::Reader value) {
  ::capnp::_::PointerHelpers< ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>>::set(_builder.getPointerField(
      ::capnp::bounded<2>() * ::capnp::POINTERS), value);
}
inline  ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>::Builder Charset::Builder::initBytes(unsigned int size) {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>>::init(_builder.getPointerField(
      ::capnp::bounded<2>() * ::capnp::POINTERS), size);
}
inline void Charset::Builder::adoptBytes(
    ::capnp::Orphan< ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>>&& value) {
  ::capnp::_::PointerHelpers< ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>>::adopt(_builder.getPointerField(
      ::capnp::bounded<2>() * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>> Charset::Builder::disownBytes() {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::Charset::Pair,  ::capnp::Kind::STRUCT>>::disown(_builder.getPointerField(
      ::capnp::bounded<2>() * ::capnp::POINTERS));
}

inline typename Charset::Pair::Type::Reader Charset::Pair::Reader::getType() const {
  return typename Charset::Pair::Type::Reader(_reader);
}
inline typename Charset::Pair::Type::Builder Charset::Pair::Builder::getType() {
  return typename Charset::Pair::Type::Builder(_builder);
}
#if !CAPNP_LITE
inline typename Charset::Pair::Type::Pipeline Charset::Pair::Pipeline::getType() {
  return typename Charset::Pair::Type::Pipeline(_typeless.noop());
}
#endif  // !CAPNP_LITE
inline typename Charset::Pair::Type::Builder Charset::Pair::Builder::initType() {
  _builder.setDataField< ::uint16_t>(::capnp::bounded<0>() * ::capnp::ELEMENTS, 0);
  _builder.setDataField< ::uint8_t>(::capnp::bounded<2>() * ::capnp::ELEMENTS, 0);
  _builder.getPointerField(::capnp::bounded<0>() * ::capnp::POINTERS).clear();
  return typename Charset::Pair::Type::Builder(_builder);
}
inline  ::uint64_t Charset::Pair::Reader::getCount() const {
  return _reader.getDataField< ::uint64_t>(
      ::capnp::bounded<1>() * ::capnp::ELEMENTS);
}

inline  ::uint64_t Charset::Pair::Builder::getCount() {
  return _builder.getDataField< ::uint64_t>(
      ::capnp::bounded<1>() * ::capnp::ELEMENTS);
}
inline void Charset::Pair::Builder::setCount( ::uint64_t value) {
  _builder.setDataField< ::uint64_t>(
      ::capnp::bounded<1>() * ::capnp::ELEMENTS, value);
}

inline  ::Charset::Pair::Type::Which Charset::Pair::Type::Reader::which() const {
  return _reader.getDataField<Which>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS);
}
inline  ::Charset::Pair::Type::Which Charset::Pair::Type::Builder::which() {
  return _builder.getDataField<Which>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS);
}

inline bool Charset::Pair::Type::Reader::isWord() const {
  return which() == Charset::Pair::Type::WORD;
}
inline bool Charset::Pair::Type::Builder::isWord() {
  return which() == Charset::Pair::Type::WORD;
}
inline bool Charset::Pair::Type::Reader::hasWord() const {
  if (which() != Charset::Pair::Type::WORD) return false;
  return !_reader.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS).isNull();
}
inline bool Charset::Pair::Type::Builder::hasWord() {
  if (which() != Charset::Pair::Type::WORD) return false;
  return !_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS).isNull();
}
inline  ::capnp::Text::Reader Charset::Pair::Type::Reader::getWord() const {
  KJ_IREQUIRE((which() == Charset::Pair::Type::WORD),
              "Must check which() before get()ing a union member.");
  return ::capnp::_::PointerHelpers< ::capnp::Text>::get(_reader.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS));
}
inline  ::capnp::Text::Builder Charset::Pair::Type::Builder::getWord() {
  KJ_IREQUIRE((which() == Charset::Pair::Type::WORD),
              "Must check which() before get()ing a union member.");
  return ::capnp::_::PointerHelpers< ::capnp::Text>::get(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS));
}
inline void Charset::Pair::Type::Builder::setWord( ::capnp::Text::Reader value) {
  _builder.setDataField<Charset::Pair::Type::Which>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS, Charset::Pair::Type::WORD);
  ::capnp::_::PointerHelpers< ::capnp::Text>::set(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS), value);
}
inline  ::capnp::Text::Builder Charset::Pair::Type::Builder::initWord(unsigned int size) {
  _builder.setDataField<Charset::Pair::Type::Which>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS, Charset::Pair::Type::WORD);
  return ::capnp::_::PointerHelpers< ::capnp::Text>::init(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS), size);
}
inline void Charset::Pair::Type::Builder::adoptWord(
    ::capnp::Orphan< ::capnp::Text>&& value) {
  _builder.setDataField<Charset::Pair::Type::Which>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS, Charset::Pair::Type::WORD);
  ::capnp::_::PointerHelpers< ::capnp::Text>::adopt(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::capnp::Text> Charset::Pair::Type::Builder::disownWord() {
  KJ_IREQUIRE((which() == Charset::Pair::Type::WORD),
              "Must check which() before get()ing a union member.");
  return ::capnp::_::PointerHelpers< ::capnp::Text>::disown(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS));
}

inline bool Charset::Pair::Type::Reader::isByte() const {
  return which() == Charset::Pair::Type::BYTE;
}
inline bool Charset::Pair::Type::Builder::isByte() {
  return which() == Charset::Pair::Type::BYTE;
}
inline  ::uint8_t Charset::Pair::Type::Reader::getByte() const {
  KJ_IREQUIRE((which() == Charset::Pair::Type::BYTE),
              "Must check which() before get()ing a union member.");
  return _reader.getDataField< ::uint8_t>(
      ::capnp::bounded<2>() * ::capnp::ELEMENTS);
}

inline  ::uint8_t Charset::Pair::Type::Builder::getByte() {
  KJ_IREQUIRE((which() == Charset::Pair::Type::BYTE),
              "Must check which() before get()ing a union member.");
  return _builder.getDataField< ::uint8_t>(
      ::capnp::bounded<2>() * ::capnp::ELEMENTS);
}
inline void Charset::Pair::Type::Builder::setByte( ::uint8_t value) {
  _builder.setDataField<Charset::Pair::Type::Which>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS, Charset::Pair::Type::BYTE);
  _builder.setDataField< ::uint8_t>(
      ::capnp::bounded<2>() * ::capnp::ELEMENTS, value);
}

inline  ::uint32_t Frequencies::Reader::getVersion() const {
  return _reader.getDataField< ::uint32_t>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS, 1u);
}

inline  ::uint32_t Frequencies::Builder::getVersion() {
  return _builder.getDataField< ::uint32_t>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS, 1u);
}
inline void Frequencies::Builder::setVersion( ::uint32_t value) {
  _builder.setDataField< ::uint32_t>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS, value, 1u);
}

inline bool Frequencies::Reader::hasCharsets() const {
  return !_reader.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS).isNull();
}
inline bool Frequencies::Builder::hasCharsets() {
  return !_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS).isNull();
}
inline  ::capnp::List< ::Charset,  ::capnp::Kind::STRUCT>::Reader Frequencies::Reader::getCharsets() const {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::Charset,  ::capnp::Kind::STRUCT>>::get(_reader.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS));
}
inline  ::capnp::List< ::Charset,  ::capnp::Kind::STRUCT>::Builder Frequencies::Builder::getCharsets() {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::Charset,  ::capnp::Kind::STRUCT>>::get(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS));
}
inline void Frequencies::Builder::setCharsets( ::capnp::List< ::Charset,  ::capnp::Kind::STRUCT>::Reader value) {
  ::capnp::_::PointerHelpers< ::capnp::List< ::Charset,  ::capnp::Kind::STRUCT>>::set(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS), value);
}
inline  ::capnp::List< ::Charset,  ::capnp::Kind::STRUCT>::Builder Frequencies::Builder::initCharsets(unsigned int size) {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::Charset,  ::capnp::Kind::STRUCT>>::init(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS), size);
}
inline void Frequencies::Builder::adoptCharsets(
    ::capnp::Orphan< ::capnp::List< ::Charset,  ::capnp::Kind::STRUCT>>&& value) {
  ::capnp::_::PointerHelpers< ::capnp::List< ::Charset,  ::capnp::Kind::STRUCT>>::adopt(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::capnp::List< ::Charset,  ::capnp::Kind::STRUCT>> Frequencies::Builder::disownCharsets() {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::Charset,  ::capnp::Kind::STRUCT>>::disown(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS));
}


CAPNP_END_HEADER

