#ifndef INTRUSIVE_LIST_H
#define INTRUSIVE_LIST_H

#include "../global.h"

#define INTRUSIVE_LIST(t) \
public:\
    base::utils::IntrusiveListMemberHook<t> list_member_hook_;\
    t* list_next() { return list_member_hook_.list_next_; } \
    bool list_linked() const { return list_member_hook_.list_ != nullptr; }
#include <iostream>
namespace base
{
    namespace utils
    {
        template<typename T>
        class IntrusiveList;
        
        template<typename T>
        struct IntrusiveListMemberHook
        {
            IntrusiveListMemberHook()
                : list_pre_(nullptr), list_next_(nullptr), list_(nullptr) {}
            T* list_pre_;
            T* list_next_;
            IntrusiveList<T>* list_;
        };

        template<typename T>
        class IntrusiveList
        {
        public:
            IntrusiveList()
                : head_(NULL), size_(0) {}

            bool empty() const {
                return size_ == 0;
            }

            size_t size() const {
                return size_;
            }

            T* front() {
                return head_;
            }

            bool contains(T* item) const {
	      /*std::cout << "this = " << this << std::endl;
	      std::cout << "item = " << item << std::endl;
	      std::cout << "&item->list_member_hook_ = " << &(item->list_member_hook_) << std::endl;
	      std::cout << "item->list_member_hook_.list_ = " << item->list_member_hook_.list_ << std::endl;*/
                return item->list_member_hook_.list_ == this;
            }
            
            template<typename T1>
            static T1* next(T1* item) {
                return item->list_member_hook_.list_next_;
            }

            T* erase(T* item) {
                T* next = nullptr;
                if (item == head_) {
                    if (head_->list_member_hook_.list_next_ != nullptr) {
                        head_->list_member_hook_.list_next_->list_member_hook_.list_pre_ = nullptr;
                    }
                    head_ = head_->list_member_hook_.list_next_;
                    next = head_;
                } else {
                    if (item->list_member_hook_.list_next_ != nullptr) {
                        item->list_member_hook_.list_next_->list_member_hook_.list_pre_ = item->list_member_hook_.list_pre_;
                    }
                    if (item->list_member_hook_.list_pre_ != nullptr) {
                        item->list_member_hook_.list_pre_->list_member_hook_.list_next_ = item->list_member_hook_.list_next_;
                    }
                    next = item->list_member_hook_.list_next_;
                }
                --size_;
                item->list_member_hook_.list_next_ = nullptr;
                item->list_member_hook_.list_pre_ = nullptr;
                item->list_member_hook_.list_ = nullptr;
                return next;
            }

            void push_front(T* item) {
                if (head_ == NULL) {
                    item->list_member_hook_.list_pre_ = nullptr;
                    item->list_member_hook_.list_next_ = nullptr;
                    head_ = item;
                } else {
                    item->list_member_hook_.list_next_ = head_;
                    item->list_member_hook_.list_pre_ = nullptr;
                    head_->list_member_hook_.list_pre_ = item;
                    head_ = item;
                }
                item->list_member_hook_.list_ = this;
                ++size_;
            }
            
            // TODO add push_back , use at action list

        private:
            T* head_;
            size_t size_;
        };
    }
}

#endif
