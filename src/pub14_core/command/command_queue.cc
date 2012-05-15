// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "command_queue.h"

#include "anh/logger.h"

#include "anh/service/service_manager.h"

#include "swganh/app/swganh_kernel.h"

#include "swganh/command/base_swg_command.h"
#include "swganh/command/command_interface.h"
#include "swganh/command/command_service_interface.h"

#include "swganh/object/creature/creature.h"
#include "swganh/object/tangible/tangible.h"
#include "swganh/object/object_controller.h"

using pub14_core::command::CommandQueue;
using pub14_core::command::CommandService;
using swganh::command::BaseSwgCommand;
using swganh::command::CommandCallback;
using swganh::command::CommandInterface;
using swganh::object::creature::Creature;
using swganh::object::tangible::Tangible;

CommandQueue::CommandQueue(
    swganh::app::SwganhKernel* kernel)
    : kernel_(kernel)
    , timer_(kernel->GetIoService())
    , active_(kernel->GetIoService())
    , processing_(false)
{
    command_service_ = kernel->GetServiceManager()->GetService<CommandService>("CommandService");
}

CommandQueue::~CommandQueue()
{}

void CommandQueue::EnqueueCommand(const std::shared_ptr<CommandInterface>& command)
{   
    auto swg_command = std::static_pointer_cast<BaseSwgCommand>(command);
 
    bool is_valid;
    uint32_t error;
    uint32_t action;

    std::tie(is_valid, error, action) = command_service_->ValidateForEnqueue(swg_command.get());

    if (is_valid)
    {
        if (swg_command->IsQueuedCommand())
        {
            queue_.push(swg_command);
            Notify();
        }
        else
        {
            ProcessCommand(swg_command);
        }
    }
    else
    {
        command_service_->SendCommandQueueRemove(swg_command->GetController(), swg_command->GetActionCounter(), swg_command->GetDefaultTime(), error, action);
    }
}

void CommandQueue::ProcessCommand(const std::shared_ptr<swganh::command::BaseSwgCommand>& command)
{
    try {
        bool is_valid;
        uint32_t error;
        uint32_t action;

        std::tie(is_valid, error, action) = command_service_->ValidateForProcessing(command.get());

        if (is_valid)
        {
            if (command->Validate())
            {
		        auto callback = command->Run();
                if (callback)
                {
                    HandleCallback(*callback);
                }
            }
            else
            {
                action = 1; // indicates a general error
            }
        }

        command_service_->SendCommandQueueRemove(command->GetController(), command->GetActionCounter(), command->GetDefaultTime(), error, action);
    } catch(const std::exception& e) {
        LOG(warning) << "Error Processing Command: " <<  command->GetCommandName() << "\n" << e.what();
    }
}

void CommandQueue::Notify()
{
    boost::unique_lock<boost::mutex> process_lg(process_mutex_);
    if (!processing_)
    {
        processing_ = true;
        process_lg.unlock();


        boost::unique_lock<boost::mutex> queue_lg(queue_mutex_);
        if (!queue_.empty())
        {
            std::shared_ptr<BaseSwgCommand> command = queue_.top();
            queue_.pop();
            queue_lg.unlock();
            
            timer_.expires_from_now(boost::posix_time::seconds(static_cast<uint64_t>(command->GetDefaultTime())));
            timer_.async_wait([this] (const boost::system::error_code& ec) 
            {
                {
                    boost::lock_guard<boost::mutex> lg(process_mutex_);
                    processing_ = false;
                }
            
                this->Notify();
            });

            
            ProcessCommand(command);
        }
        else
        {
            process_lg.lock();
            processing_ = false;
        }
    }		
}

void CommandQueue::HandleCallback(std::shared_ptr<CommandCallback> callback)
{    
    active_.AsyncDelayed(boost::posix_time::milliseconds(callback->GetDelayTimeInMs()),
        [this, callback] ()
    {
        auto new_callback = (*callback)();
        if (new_callback)
        {
            HandleCallback(*new_callback);
        }
    });
}