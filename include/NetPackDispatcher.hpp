#pragma once

/**
 * @file   NetPackDispatcher.hpp
 * @author liuzf <liuzf@liuzf-H61M-DS2>
 * @date   Sun Apr  5 01:37:30 2015
 * 
 * @brief  This file is use to dispatch message from vecbuffer directly use netpack for decode.
 * NPIMPL is abbreviate for NetPackImplement.
 * While a implementation must have type define MsgType to specify message type and MsgType must
 * have type define CmdType which dispatcher use to seperate message from each other.
 * For decode message, NPIMPL must have static function readMessage and peekPackLength.
 */

#include <memory>
#include <exception>

#include "Dispatcher.hpp"
#include "VecBuffer.hpp"

// FIXME : compile time check and good notice

using namespace std;

namespace netio {

template <typename NPIMPL, typename SrcType>
class NetPackDispatcher : public Dispatcher<typename NPIMPL::MsgType, SrcType> {
  typedef typename NPIMPL::MsgType MsgType;
  
  typedef shared_ptr<SrcType> SpSrcType;
  typedef shared_ptr<MsgType> SpMsgType;
 public:

  /** 
   * Decode message from vecbuffer and dispatch the message.
   * 
   * @param buffer : message parse from
   * @param source : buffer owner.
   */
  void dispatch(SpVecBuffer& buffer, SpSrcType& source) {
    while(true) {
      SpMsgType message  = NPIMPL::readMessage(buffer);

      if(nullptr != message) {
        this->dispatch(message->getCmd(), message, source);
      } else {
        // ensure buffer size to store following message.
        ssize_t expect = NPIMPL::peekPackLength(buffer);
        if(expect > 0) {
          buffer->ensure(expect);
        }

        // no more message to read, break the loop.
        break;
      }
    }
  }
};

}


