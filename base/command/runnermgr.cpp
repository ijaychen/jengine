#include "runnermgr.h"

namespace base
{
    namespace command
    {
        RunnerMgr::RunnerMgr()
        {
        }

        RunnerMgr::~RunnerMgr()
        {
            CommandQueue* q = list_.front();
            while (q) {
                CommandQueue* tmp = q;
                q = list_.erase(q);
                tmp->Release();
            }
        }

        void RunnerMgr::Update(int64_t tick)
	{
            CommandQueue* q = list_.front();    
            while (q) {
	      //std::cout << "list_.front()" << q << std::endl;
                if (q->reference_count() == 2) {
		  //std::cout << "q->reference_count() == 2 is true" << std::endl;
                    if (q->TryExecute()) {
		      //std::cout << "q->TryExecute() = true" << std::endl;
                        // 执行完毕，从队列中移除
                        CommandQueue* tmp = q;
                        q = list_.erase(q);
                        tmp->Release();
                    } else {
                        q = q->list_next();
                    }
                } else {
                    CommandQueue* tmp = q;
                    q = list_.erase(q);
                    tmp->Release();
                }
            }
        }
    }
}

