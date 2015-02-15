#pragma once

#include <stdint.h>
#include <vector>
#include <memory>
#include <list>

using namespace std;

/**
 * This channel buffer is designed for small message buffer.
 *
 * We assume that one message is less of the threshold most of time. So it's
 * rarely need to copy memory when receiving message.
 *
 * NOTE : channel buffer is not design for threading safe.
 */

namespace netio {

typedef vector<int8_t> VecData;
typedef shared_ptr<VecData> SpVecData;

/**
 * Implements base on shared_ptr reference of vector.
 * and now we use default allocator for vector buffer.
 *
 * We use smart pointer to manage memory, when VedData is not referenced, it will release automatically, 
 *
 * Write operator only append data to the tail left buffer, and only read from head of readable buffer, act like queue.
 * The capacity of the buffer can't be change because I think we don't need it.
 */
class VecBuffer {
 public:
  /**
   * for receiving buffer construct, we just create default size of buffer, because we don't know
   * how much size of the message.
   */
  explicit VecBuffer(size_t size) :
      _offset(0),
      _buffer(new VecData(size)),
      _len(0) {}

  /**
   * For sending buffer construct and avoid to alloc pack header each time, we can call this function to get buffer that
   * has prepend for pack header before sending data generation.
   */
  explicit VecBuffer(size_t size, size_t prepend) :
      _offset(prepend),
      _buffer(new VecData(size + prepend)),
      _len(0) {}

  /**
   * For sending buffer construct, we get continuous buffer from client clode most of time.
   */
  VecBuffer(const SpVecData& buffer, off_t offset, size_t len) :
      _offset(offset), _len(len), _buffer(buffer) {
    ASSERT((_offset + _len) <= _buffer->size());
  }

  size_t writtableSize() const {
    return _buffer->size() - (_offset + _len);
  }

  size_t readableSize() const {
    return _len;
  }

  int8_t* bufferPtr() {
    return &*_buffer->begin();
  }

  off_t getOffset() const {
    return _offset;
  }

  void resetOffset() {
    _offset = 0;
  }

  int8_t* writtablePtr() {
    int8_t* bufptr = &*_buffer->begin();
    return bufptr + _offset + _len;
  }

  const int8_t* readablePtr() const {
    int8_t* bufptr = &*_buffer->begin();
    return bufptr + _offset;
  }

  /**
   * mark append size readed.
   */
  void markRead(size_t size) {
    ASSERT(size <= readableSize());
    _offset += size;
  }

  /**
   * mark append size written.
   */
  void markWrite(size_t size) {
    ASSERT(size <= writtableSize());
    _len += size;
  }

  /**
   * append writtable size with size;
   */
  void enlarge(size_t size) {
    _buffer->resize(_buffer->size() + size);
  }
  
 private:
  SpVecData _buffer;
  off_t _offset;
  size_t _len;
};

typedef shared_ptr<VecBuffer> SpVecBuffer;

}

















