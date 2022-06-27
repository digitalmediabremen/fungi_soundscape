//
// Copyright (c) 2010 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier: MIT
//


#pragma once


#include <type_traits>
#include "ofx/IO/ByteBuffer.h"


namespace ofx {
namespace IO {


/// \brief A utility for reading variable types from a raw byte buffer.
class ByteBufferReader
{
public:
    /// \brief Create a ByteBufferReader from a byte buffer.
    /// \param buffer The ByteBuffer to read from.
    /// \param The byte offset.
    ByteBufferReader(const ByteBuffer& buffer, std::size_t offset = 0);

    /// \brief Read a value from the ByteBuffer.
    /// \tparam Type the type to read from the ByteBuffer.
    /// \param value A reference to the value to read.
    /// \returns std::size_t the number of bytes read.
    template <typename Type>
    std::size_t read(Type& value) const;

    /// \brief Read an array of values from the ByteBuffer.
    /// \tparam Type the type to read from the ByteBuffer.
    /// \param value A reference to the values to read.
    /// \param size The number of bytes to read into the array.
    /// \returns std::size_t the number of bytes read.
    template <typename Type>
    std::size_t read(Type* destination, std::size_t size) const;

    /// \brief Set the offset in the ByteBuffer.
    /// \param offset The byte offset.
    void setOffset(std::size_t offset);

    /// \brief Set the number of bytes to skip.
    /// \param offset The number of bytes to skip.
    void skip(std::size_t offset);

    /// \brief Get the current offset.
    /// \returns the current offset.
    std::size_t getOffset() const;

    /// \brief Get the size of the ByteBuffer.
    /// \returns the size of the ByteBuffer.
    std::size_t size() const;

    /// \brief Get the number of bytes remaining to read in the ByteBuffer.
    /// \returns the number of bytes remaining to read in the ByteBuffer.
    std::size_t remaining() const;

private:
    ByteBufferReader(const ByteBufferReader& that);
    ByteBufferReader& operator = (const ByteBufferReader& that);

    /// \brief An Internal function for reading bytes.
    /// \param destination The destination to be filled.
    /// \param size the number of bytes to read.
    /// \returns the number of bytes read.
    std::size_t _read(void* destination, std::size_t size) const;

    /// \brief A reference to the ByteBuffer being read.
    const ByteBuffer& _buffer;

    /// \brief The current offset.
    mutable std::size_t _offset;

};


template <typename Type>
std::size_t ByteBufferReader::read(Type& value) const
{
    static_assert(std::is_pod<Type>::value, "Type must be POD.");
    return _read(&value, sizeof(Type));
}


template <typename Type>
std::size_t ByteBufferReader::read(Type* destination, std::size_t size) const
{
    static_assert(std::is_pod<Type>::value, "Type must be POD.");
    return _read(destination, sizeof(Type) * size);
}


} } // namespace ofx::IO
