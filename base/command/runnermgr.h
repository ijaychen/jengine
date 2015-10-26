#ifndef BASE_COMMAND_RUNNERMGR_H
#define BASE_COMMAND_RUNNERMGR_H

#include "command.h"
#include "../object.h"
#include "../utils/intrusive_list.h"
#include <queue>
#include <iostream>
namespace base
{
    namespace command
    {
        class CommandQueue : public base::Object
        {
            INTRUSIVE_LIST(CommandQueue)
        public:
            virtual ~CommandQueue() {
                while (!cmds_.empty()) {
                    delete cmds_.front();
                    cmds_.pop();
                }
            }

            bool empty() const {
                return cmds_.empty();
            }

            void Push(Command* cmd) {
                cmds_.push(cmd);
            }

            bool TryExecute() {
	      //std::cout << "bool TryExecute()" << std::endl;
                if (cmds_.empty()) {
                    return true;
                }
                //while (! cmds_.empty()) { //my test
                Command* top = cmds_.front();
		//std::cout << "top->executed_ = " << top->executed_ << std::endl;
                if (!top->executed_) {
                    if (top->OnSetup()) {
		      //std::cout << "top->OnSetup() = true" << std::endl;
                        top->Execute();
                    } else {
                        delete top;
                        top = nullptr;
                        cmds_.pop();
                    }
                } /*else { //<-orange test
		  delete top;
		  top = nullptr;
		  cmds_.pop();
		} //orange test->*/

                if (top && top->finish_) {
		  //std::cout << "top && top->finish_ = " << (top && top->finish_) << std::endl;
                    top->OnCleanup();
                    delete top;
                    cmds_.pop();
		  //std::cout << "cmds_.empty() = " << cmds_.empty() << std::endl;
                }
		//}
                return cmds_.empty();
            }

        private:
            std::queue<Command*> cmds_;
        };

        class RunnerMgr
        {
        public:
            static RunnerMgr& instance() {
                static RunnerMgr ins;
                return ins;
            }
            virtual ~RunnerMgr();

            bool empty() {
                return list_.empty();
            }


            // 注册执行队列
            void Register(CommandQueue* q) {
	      //std::cout << "list_ = " << &list_ << std::endl;
	      //std::cout << "queue_->Push(cmd):q = "<< q << std::endl;
                if (list_.contains(q)) {
		  //std::cout << "base::command::runnermgr.h:76 Register" << std::endl;
                    return;
                } else {
		  //std::cout << "base::command::runnermgr.h:79 Register" << std::endl;
                    list_.push_front(q);
                    q->Retain();
                }
            }
            // 更新，执行所有未执行的命令
            void Update(int64_t tick);

        private:
            RunnerMgr();
            base::utils::IntrusiveList<CommandQueue> list_;
        };
    }
}

#endif // RUNNERMGR_H
