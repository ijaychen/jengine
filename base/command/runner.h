#ifndef BASE_COMMAND_RUNNER_H
#define BASE_COMMAND_RUNNER_H

#include "command.h"
#include <iostream>
namespace base
{
    namespace command
    {
        class Runner
        {
        public:
            Runner() ;
            virtual ~Runner() ;

            void PushCommand(Command* cmd) ;

        private:
            CommandQueue* queue_;
        };

        template<typename TARGET>
        class RunnerT : public Runner
        {
        public:
            RunnerT(TARGET& t) : target_(t) {}
            virtual ~RunnerT() {}

            void PushCommand(CommandT<TARGET>* cmd) {
                cmd->target_ = &target_;
		//std::cout << "&target = " << &target_ << std::endl;
                Runner::PushCommand(cmd);
            }

        private:
            TARGET& target_;
        };
    }
}

#endif // RUNNER_H
