#include "BitmapTree.hpp"

BitmapLayer::BitmapLayer(size_t size) : _bits(nullptr), _lwords(0), _tailBits(0), _size(size) {
  if(size > 0) {
    /* allocate uint64_t array */
    _lwords = calculate_lwords(size);
    _bits = new uint64_t[_lwords];
    _tailBits = (size & 0x3F);

    /* set all bits initial */
    memset((void*)_bits, 0xFF,(_lwords << 3));

    /* fix last uint64_t */
    if(0 != _tailBits) {
      _bits[_lwords - 1] = ((_bits[_lwords - 1]) & (-1UL << ((sizeof(uint64_t) << 3) - _tailBits)));
    }

    //      printf("tail = %lu \n", _tailBits );
  }
}


BitmapLayer::~BitmapLayer() {
  delete[] _bits;
  _bits = nullptr;
  _lwords = 0;
}


void BitmapLayer::_print_dbg() const {
  for(uint i = 0; i < _lwords; i ++) {
    printf("%016lx ", _bits[i]);
  }
}


BitmapTree::BitmapTree(size_t size) : _mutex() {
  static_assert((8 == sizeof(unsigned long)), "long word is not 8 bytes");
  stack<BitmapLayer*> layers;
  size_t lsize = size;

  while (lsize > 1) {
    BitmapLayer* layer = new BitmapLayer(lsize);
    layers.push(layer);
    lsize = layer->_lwords;
  }

  _deep = layers.size();

  for(uint i = 0; i < ARRAY_SIZE(_layers); i++) {
    if(!layers.empty()) {
      _layers[i] = layers.top();
      layers.pop();
    } else {
      _layers[i] = nullptr;
    }
  }
}


BitmapTree::~BitmapTree() {
  _deep = 0;
  for(uint i = 0; i < ARRAY_SIZE(_layers); i++) {
    if(nullptr != _layers[i]) {
      delete _layers[i];
    } else {
      break;
    }
  }
}


off64_t BitmapTree::bitRequire() {
  lock_guard<mutex> lock(_mutex);
  off64_t bitsOffs[BMT_MAX_DEEP] = {0};
  off64_t lwOffs = 0;
  bool found = true;

  /* record bit offset */
  for(int i = 0; i < _deep; i ++) {
    bitsOffs[i] = _layers[i]->firstBitSet(lwOffs);
    lwOffs = bitsOffs[i];

    if(__builtin_expect((lwOffs < 0), 0)) {
      found = false;
      break;
    }
  }

  for(int i = 0; i < _deep; i++) {
    printf("bits offset = %ld \n", bitsOffs[i]);
  }

  /* if found, reset bit upward, calculate total offsets */
  if(found) {
    for(int i = (_deep -1); i >= 0; i--) {
      if(!_layers[i]->reset(bitsOffs[i])) {
        break;
      }
    }
    return lwOffs;
  }
  return -1;
}


