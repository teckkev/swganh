// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include "anh/byte_buffer.h"
#include "base_swg_message.h"

namespace swganh {
namespace messages {

    struct NewTicketActivityMessage : public BaseSwgMessage
    {
    	uint16_t Opcount() const { return 2; }
    	uint32_t Opcode() const { return 0x274F4E78; }
    	
    	uint64_t ticket_id;
    	
    	void OnSerialize(anh::ByteBuffer& buffer) const
    	{
    		buffer.write(ticket_id);
    	}

    	void OnDeserialize(anh::ByteBuffer& buffer)
    	{
    		ticket_id = buffer.read<uint64_t>();
    	}
    };

}} // namespace swganh::messages
