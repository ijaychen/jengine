#ifndef BASE_COMMAND_COMMANDBASE_H
#define BASE_COMMAND_COMMANDBASE_H

#include "../global.h"

namespace base
{
    namespace command
    {
        class CommandQueue;

        class Command
        {
        public:
            Command() : executed_(false), finish_(false) {}
            virtual ~Command() {}

            void Stop() {
                finish_ = true;
            }

        private:
            virtual bool OnSetup() {
                return true;
            }
            virtual void OnCleanup() {}

            void Execute() {
                executed_ = true;
                OnCommandExecute();
            }
            virtual void OnCommandExecute() {}

            bool executed_;
            bool finish_;

            friend class CommandQueue;
        };

        template<typename TARGET>
        class RunnerT;

        template<typename TARGET>
        class CommandT : public Command
        {
        public:
            CommandT()
                : target_(nullptr) {}
            virtual ~CommandT() {}

            TARGET* target() {
                return target_;
            }

        private:
            TARGET* target_;
            friend class RunnerT<TARGET>;
        };
    }
}

#endif // COMMANDBASE_H
