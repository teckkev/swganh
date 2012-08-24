// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#ifndef SWGANH_MESSAGES_GET_ATTRIBUTES_BATCH_H_
#define SWGANH_MESSAGES_GET_ATTRIBUTES_BATCH_H_

#include <cstdint>

#include "anh/byte_buffer.h"
#include "pub14_core/messages/obj_controller_message.h"

namespace swganh {
namespace messages {
namespace controllers {

    class GetAttributesBatchMessage : public ObjControllerMessage
    {    	
	public:
    	explicit GetAttributesBatchMessage(uint32_t controller_type = 0x0000000B)
            : ObjControllerMessage(controller_type, message_type())
        {}

        explicit GetAttributesBatchMessage(ObjControllerMessage controller_message)
            : ObjControllerMessage(std::move(controller_message))
        {
            OnControllerDeserialize(std::move(data));
        }

        static uint32_t message_type() { return 0x164550EF; }
                

        void OnControllerSerialize(anh::ByteBuffer& buffer) const
        {
            buffer.write<uint32_t>(0);            
        }

        void OnControllerDeserialize(anh::ByteBuffer buffer)
        {            
        }
    };

}}} // namespace swganh::messages::controllers

#endif // SWGANH_MESSAGES_GET_ATTRIBUTES_BATCH_H_